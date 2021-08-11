/**
 ******************************************************************************
 * @file      LedProxy.c
 *
 * @brief     Led proxy. Will implement the ILed interface.
 *            If ILed is to be used from a different node than from
 *            where it is implemented, use this proxy code.
 ******************************************************************************
 */

/*
 ------------------------------------------------------------------------------
    Include files
 ------------------------------------------------------------------------------
*/
#include <stdlib.h>

#include "LedProxyCfg.h"
#include "LedTif.h"
#include "LedProxy.h"

#include "CmdChain.h"
#include "IConnectionManager.h"
#include "ConnectionManager.h"
#include "ILog.h"
#include "LinkManagerTif.h"
#include "IRoboticsProtocol.h"
#include "RoboticsProtocol2.h"
#include "RoboticUtils.h"
#include "IScheduler.h"


/*
 ------------------------------------------------------------------------------
    Defines
 ------------------------------------------------------------------------------
 */
#define LEDPROXY_MAX_REGISTERED_CB     1u
#define LEDPROXY_CONNECTION_TIMEOUT    0       /**< (ms) Timeout for connection attempts. 0 means forever.         */
#define LEDPROXY_CMD_TIMEOUT           1000    /**< (ms) Timeout of how long we should wait for answer on command. */
#define LEDPROXY_LEDS_NUMBER           ARRSIZE( ledProxyCfg )
#define LEDPROXY_NULL_LED_INDEX        LEDPROXY_LEDS_NUMBER

/*
 ------------------------------------------------------------------------------
    Local types
 ------------------------------------------------------------------------------
 */
typedef enum
{
    LEDPROXY_STATE_INITIALIZED,        /**< Proxy has been initialized, but is not started                 */
    LEDPROXY_STATE_NOT_CONNECTED,      /**< Proxy is not connected to proxy server                         */
    LEDPROXY_STATE_LINK_SETUP,         /**< Connected to server, setting up link                           */
    LEDPROXY_STATE_PULL_FROM_SERVER,   /**< Pulls server attributes to local mirror                        */
    LEDPROXY_STATE_PUSH_TO_SERVER,     /**< Push local attributes to server                                */
    LEDPROXY_STATE_PROXY_READY,        /**< Proxy is ready to be used                                      */
    /************************************************************************************************************/
    LEDPROXY_STATE_COUNT,              /**< Not used, always the last entry in the enumeration             */
} tLedProxy_State;


typedef enum
{
    LEDPROXY_CMDCHAIN_LINKSETUP_START,
    LEDPROXY_CMDCHAIN_LINKSETUP_GET_STATIC_ID,
    LEDPROXY_CMDCHAIN_LINKSETUP_REGISTER_EVENTS,
    LEDPROXY_CMDCHAIN_LINKSETUP_END
} tLedProxy_CmdChainLinkSetup;

typedef enum
{
    LEDPROXY_CMDCHAIN_PULL_START,
    LEDPROXY_CMDCHAIN_PULL_MODE,
    LEDPROXY_CMDCHAIN_PULL_PERIOD,
    LEDPROXY_CMDCHAIN_PULL_END
} tLedProxy_CmdChainPull;

typedef enum
{
    LEDPROXY_CMDCHAIN_PUSH_START,
    LEDPROXY_CMDCHAIN_PUSH_MODE,
    LEDPROXY_CMDCHAIN_PUSH_PERIOD,
    LEDPROXY_CMDCHAIN_PUSH_END
} tLedProxy_CmdChainPush;


typedef struct
{
    tLedTif_Index       getReq;
    tILed_Mode          mode;
    tILed_MilliSeconds  period;
} tLedProxy_PullCache;

typedef struct
{
    tLedTif_Mode    setModeReq;
    tLedTif_Period  setPeriodReq;
} tLedProxy_PushCache;


typedef struct
{
    tLedProxy_State      state;
    tILinkManagerLinkId  linkId;
    tILinkManagerLinkId  staticNodeId;
    tLedProxy_PullCache  pullCache;
    tLedProxy_PushCache  pushCache;
    tCmdChain_Id         cmdChainLinkSetup;
    tCmdChain_Id         cmdChainPull;
    tCmdChain_Id         cmdChainPush;
} tLedProxy_LedVars;

typedef struct
{
    bool                 initialized;
    tEventCallback       pEventCb[ LEDPROXY_MAX_REGISTERED_CB ];
    tLedProxy_LedVars    ledVars[ LEDPROXY_LEDS_NUMBER ];
} tLedProxy_Vars;


/*
 ------------------------------------------------------------------------------
    Private data
 ------------------------------------------------------------------------------
 */

static tLedProxy_Vars proxyVars =
        {
          .initialized = false,
          .pEventCb = { NULL },
          .ledVars = { { .state = LEDPROXY_STATE_COUNT } },
        };

/*
 ------------------------------------------------------------------------------
    Private function prototypes
 ------------------------------------------------------------------------------
 */


 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // Command chain prepare request functions

static bool PrepareRequest_LinkSetup( tCmdChain_Id cmdChainId, uint8 chainIndex, tIRoboticsProtocol_Request* pReq, uint32* pTimeout );
static bool PrepareRequest_Pull     ( tCmdChain_Id cmdChainId, uint8 chainIndex, tIRoboticsProtocol_Request* pReq, uint32* pTimeout );
static bool PrepareRequest_Push     ( tCmdChain_Id cmdChainId, uint8 chainIndex, tIRoboticsProtocol_Request* pReq, uint32* pTimeout );

 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // Update functions

static void UpdateMode( const tILed_Index index, const tILed_Mode mode );
static void UpdatePeriod( const tILed_Index index, const tILed_MilliSeconds period );


 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // Sends event to all listeners

static void NotifyAll( tILed_Events event, tILed_Index index );


 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 //Changes internal state of proxy.

static void ChangeState( const tILedCfg_Index ledIdx, const tLedProxy_State newState );
static void EventHandler( tEvent event );


 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // Tif response handler.

static void RoboticsProtocolResponseCb( tIRoboticsProtocol_Response* pRsp );


 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // @brief       Tif event handler.

static void RoboticsProtocolEventCb( tIRoboticsProtocol_Event* pEvent );

 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // Helper functions

static tILedCfg_Index GetIndexFromLinkId( const tILinkManagerLinkId linkId );
static tILedCfg_Index GetIndexFromCmdChainId( const tCmdChain_Id cmdChainId );

/*
 ------------------------------------------------------------------------------
    Interface functions
 ------------------------------------------------------------------------------
 */

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void ILed_Init( void )
{
    // Init only once
    if ( !proxyVars.initialized )
    {
        proxyVars.initialized = true;

        // Init pull cache values and state
        for ( tILedCfg_Index ledIdx = (tILedCfg_Index)0; ledIdx < LEDPROXY_LEDS_NUMBER; ++ledIdx )
        {
            proxyVars.ledVars[ledIdx].state = LEDPROXY_STATE_INITIALIZED;

            proxyVars.ledVars[ledIdx].linkId = 0;
            proxyVars.ledVars[ledIdx].staticNodeId = 0;

            proxyVars.ledVars[ledIdx].pullCache.getReq.index = (tILedTif_Index)ledProxyCfg[ledIdx].remoteIndex;
            proxyVars.ledVars[ledIdx].pullCache.mode = ILED_MODE_OFF;
            proxyVars.ledVars[ledIdx].pullCache.period = 0;

            proxyVars.ledVars[ledIdx].pushCache.setModeReq.index = (tILedTif_Index)ledProxyCfg[ledIdx].remoteIndex;
            proxyVars.ledVars[ledIdx].pushCache.setPeriodReq.index = (tILedTif_Index)ledProxyCfg[ledIdx].remoteIndex;
        }

        // Clear callback list
        for ( unsigned i = 0; i < LEDPROXY_MAX_REGISTERED_CB; ++i )
        {
            proxyVars.pEventCb[ i ] = NULL;
        }

        // Initialize modules used by us.
        CmdChain_Init();
        ILinkManager_Init();
        IRoboticsProtocol_Init();
        RoboticsProtocol2_Init();
        IConnectionManager_Init();
        ILog_Init();
    }
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void ILed_Start( void )
{
    // Start only once.
    if ( LEDPROXY_STATE_INITIALIZED != proxyVars.ledVars[0].state )
        return;

    // Start modules used by us.
    ILinkManager_Start();
    IRoboticsProtocol_Start();
    RoboticsProtocol2_Start();
    IConnectionManager_Start();
    ILog_Start();

    IRoboticsProtocol_RegisterEventHandler(
            &RoboticsProtocolEventCb, LED_TIF_FAMILY, ROBOTICS_PROTOCOL_FILTER_ALL );

    // Setup connection manager to connect to remote server
    for ( tILedCfg_Index ledIdx = (tILedCfg_Index)0; ledIdx < LEDPROXY_LEDS_NUMBER; ++ledIdx )
        if ( !IConnectionManager_ConnectToNode(
                ledProxyCfg[ledIdx].remoteNodeType,
                ledProxyCfg[ledIdx].remoteNodeName,
                &EventHandler,
                LEDPROXY_CONNECTION_TIMEOUT,
                &proxyVars.ledVars[ledIdx].linkId ) )
        {
            ILOG( ILOG_LEVEL_ERROR, "Error calling IConnectionManager_ConnectToNode", "^" );
        }



    for ( tILedCfg_Index ledIdx = (tILedCfg_Index)0; ledIdx < LEDPROXY_LEDS_NUMBER; ++ledIdx )
    {
        // Create command chains
        proxyVars.ledVars[ ledIdx ].cmdChainLinkSetup =
                CmdChain_CreateChain(
                        proxyVars.ledVars[ledIdx].linkId,
                        LEDPROXY_CMDCHAIN_LINKSETUP_START,
                        LEDPROXY_CMDCHAIN_LINKSETUP_END,
                        &RoboticsProtocolResponseCb,
                        &EventHandler,
                        &PrepareRequest_LinkSetup,
                        LEDPROXY_CMD_TIMEOUT );
        proxyVars.ledVars[ ledIdx ].cmdChainPull =
                CmdChain_CreateChain(
                        proxyVars.ledVars[ledIdx].linkId,
                        LEDPROXY_CMDCHAIN_PULL_START,
                        LEDPROXY_CMDCHAIN_PULL_END,
                        &RoboticsProtocolResponseCb,
                        &EventHandler,
                        &PrepareRequest_Pull,
                        LEDPROXY_CMD_TIMEOUT );
        proxyVars.ledVars[ ledIdx ].cmdChainPush =
                CmdChain_CreateChain(
                        proxyVars.ledVars[ledIdx].linkId,
                        LEDPROXY_CMDCHAIN_PUSH_START,
                        LEDPROXY_CMDCHAIN_PUSH_END,
                        &RoboticsProtocolResponseCb,
                        &EventHandler,
                        &PrepareRequest_Push,
                        LEDPROXY_CMD_TIMEOUT );

        // Change state. This will start connection attempts to proxy server.
        ChangeState( ledIdx, LEDPROXY_STATE_NOT_CONNECTED );
    }
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void ILed_SetMode( tILed_Index ledIdx, tILed_Mode mode )
{
    // Update cache
    proxyVars.ledVars[ledIdx].pushCache.setModeReq.mode = mode;

    //LOG( ILOG_LEVEL_DEBUG, "%s: LedIndex(%d), mode(%d)", __func__, ledIdx, mode ); // Log removed to clear output file

    // Trigger push chain
    CmdChain_SetDirty( proxyVars.ledVars[ledIdx].cmdChainPush, LEDPROXY_CMDCHAIN_PUSH_MODE );
    CmdChain_Run( proxyVars.ledVars[ledIdx].cmdChainPush );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tILed_Mode ILed_GetMode( tILed_Index ledIdx )
{
    tILed_Mode mode = proxyVars.ledVars[ledIdx].pullCache.mode;
    if ( LEDPROXY_STATE_PROXY_READY == proxyVars.ledVars[ ledIdx ].state )
    {
        ILOG( ILOG_LEVEL_DEBUG, "%s: LedIndex(%d), mode(%d)", "^%s^%d^%d", __func__, ledIdx, mode );
    }
    else
    {
        ILOG( ILOG_LEVEL_WARNING, "%s: Proxy not ready: ledIdx(%d), state(%d)", "^%s^%d^%d",
             __func__, ledIdx, proxyVars.ledVars[ ledIdx ].state );
    }
    return mode;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void ILed_SetMSPerPeriod( tILed_Index ledIdx, tILed_MilliSeconds period )
{
    // Update cache
    proxyVars.ledVars[ledIdx].pushCache.setPeriodReq.period = period;

    ILOG( ILOG_LEVEL_DEBUG, "%s: LedIndex(%d), period(%d)", "^%s^%d^%d", __func__, ledIdx, period );

    // Trigger push chain
    CmdChain_SetDirty( proxyVars.ledVars[ledIdx].cmdChainPush, LEDPROXY_CMDCHAIN_PUSH_PERIOD );
    CmdChain_Run( proxyVars.ledVars[ledIdx].cmdChainPush );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tILed_MilliSeconds ILed_GetMSPerPeriod( tILed_Index ledIdx )
{
    tILed_MilliSeconds period = proxyVars.ledVars[ledIdx].pullCache.period;
    if ( LEDPROXY_STATE_PROXY_READY == proxyVars.ledVars[ ledIdx ].state )
    {
        ILOG( ILOG_LEVEL_DEBUG, "%s: LedIndex(%d), period(%d)", "^%s^%d^%d", __func__, ledIdx, period );
    }
    else
    {
        ILOG( ILOG_LEVEL_WARNING, "%s: Proxy not ready: ledIdx(%d), state(%d)", "^%s^%d^%d",
             __func__, ledIdx, proxyVars.ledVars[ ledIdx ].state );
    }
    return period;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool ILed_RegisterEventCb( tEventCallback pEventCb )
{
    if ( NULL == pEventCb )
    {
        return true;
    }

    // Find free slot
    for ( unsigned i = 0; i < LEDPROXY_MAX_REGISTERED_CB; ++i )
    {
        if ( proxyVars.pEventCb[ i ] == NULL ||
             proxyVars.pEventCb[ i ] == pEventCb   )
        {
            proxyVars.pEventCb[ i ] = pEventCb;
            return true;
        }
    }

    ILOG( ILOG_LEVEL_ERROR, "Callback list full!!!", "^" );
    return false;
}



/*
-------------------------------------------------------------------------------
    Implementation of private functions
-------------------------------------------------------------------------------
*/

/*
 ******************************************************************************
 * Notify all subscribers about change for concrete LED
 * (To subscribe, IMowerApp_RegisterEventCb function had to be called).
 ******************************************************************************
 */
static void NotifyAll( tILed_Events event, tILed_Index index )
{
    tEvent sendEvent;

    sendEvent.id = (uint32) event;
    sendEvent.data = (uint32) index;

    for ( unsigned i = 0; i < LEDPROXY_MAX_REGISTERED_CB; ++i )
    {
        if ( NULL != proxyVars.pEventCb[ i ] )
        {
            IScheduler_PushEvent( sendEvent, proxyVars.pEventCb[ i ] );
        }
    }
}


/*
 ******************************************************************************
 * Changes internal state of proxy.
 * Does exit actions on leave from old state and
 * entry actions on entering state.
 ******************************************************************************
 */
static void ChangeState( const tILedCfg_Index ledIdx, const tLedProxy_State newState )
{
    if( LEDPROXY_LEDS_NUMBER <= ledIdx )
    {
        ILOG( ILOG_LEVEL_WARNING, "%s: Unsupported LED: LedIndex(%d)", "^%s^%d", __func__, ledIdx );
        return;
    }

    // Exit actions
    switch ( proxyVars.ledVars[ledIdx].state )
    {
        case LEDPROXY_STATE_PROXY_READY:
        {
            if ( LEDPROXY_STATE_PROXY_READY != newState )
            {
                // Signal not ready on leaving PROXY_READY state
                NotifyAll( ILED_EVENT_NOT_READY, ledIdx );
            }
            break;
        }
    }

    proxyVars.ledVars[ledIdx].state = newState;

    switch ( newState )
    {
        case LEDPROXY_STATE_LINK_SETUP:
        {
            // Run all link setup commands
            CmdChain_SetAllDirty( proxyVars.ledVars[ledIdx].cmdChainLinkSetup );
            CmdChain_Run( proxyVars.ledVars[ledIdx].cmdChainLinkSetup );
            break;
        }
        case LEDPROXY_STATE_PULL_FROM_SERVER:
        {
            // Run all pull commands
            CmdChain_SetAllDirty( proxyVars.ledVars[ ledIdx ].cmdChainPull );
            CmdChain_Run( proxyVars.ledVars[ ledIdx ].cmdChainPull );
            break;
        }
        case LEDPROXY_STATE_PUSH_TO_SERVER:
        {
            // Run push commands that may have been marked dirty before
            // connection occured.
            CmdChain_Run( proxyVars.ledVars[ ledIdx ].cmdChainPush );
            break;
        }
        case LEDPROXY_STATE_PROXY_READY:
        {
            // Signal ready to application
            NotifyAll( ILED_EVENT_READY, ledIdx );
            break;
        }
    }
}

/*
 ******************************************************************************
 * Handle incoming RoboticProtocol event by changing state of proper led.
 ******************************************************************************
 */
static void EventHandler( tEvent event )
{
    tILedCfg_Index ledIdx = (tILedCfg_Index)LEDPROXY_LEDS_NUMBER;

    switch ( event.id )
    {
        case ICONNECTIONMANAGER_EVENT_CONNECTED_TO_NODE:
        {
            ledIdx = GetIndexFromLinkId( (tILinkManagerLinkId)event.data );
            ChangeState( ledIdx, LEDPROXY_STATE_LINK_SETUP );
            break;
        }

        case ICONNECTIONMANAGER_EVENT_DISCONNECTED:
        {
            ledIdx = GetIndexFromLinkId( (tILinkManagerLinkId)event.data );
            ChangeState( ledIdx, LEDPROXY_STATE_NOT_CONNECTED );
            break;
        }


        case CMDCHAIN_EVENT_CHAIN_COMPLETED:
        {
            ledIdx = GetIndexFromCmdChainId( (tCmdChain_Id)event.data );
            break;
        }

        case CMDCHAIN_EVENT_CHAIN_FAILED:
        {
            ledIdx = GetIndexFromCmdChainId( (tCmdChain_Id)event.data );

            if( proxyVars.ledVars[ledIdx].cmdChainLinkSetup == ( tCmdChain_Id )event.data )
            {
                CmdChain_SetAllDirty( proxyVars.ledVars[ledIdx].cmdChainLinkSetup );
                CmdChain_Run( proxyVars.ledVars[ledIdx].cmdChainLinkSetup );
            }
            else if( proxyVars.ledVars[ledIdx].cmdChainPush == ( tCmdChain_Id )event.data )
            {
                CmdChain_Run( proxyVars.ledVars[ledIdx].cmdChainPush );
            }
            else if( proxyVars.ledVars[ledIdx].cmdChainPull == ( tCmdChain_Id )event.data )
            {
                CmdChain_SetAllDirty( proxyVars.ledVars[ledIdx].cmdChainPull );
                CmdChain_Run( proxyVars.ledVars[ledIdx].cmdChainPull );
            }

            break;
        }
    }


    if ( ledIdx == LEDPROXY_LEDS_NUMBER )
    {
        ILOG( ILOG_LEVEL_WARNING, "%s: Unsupported event: id(%u) data(%u)", "^%s^%u^%u",
             __func__, event.id, event.data );
        return;
    }


    switch ( proxyVars.ledVars[ ledIdx ].state )
    {
        case LEDPROXY_STATE_INITIALIZED:

            break;

        case LEDPROXY_STATE_NOT_CONNECTED:

            break;

        case LEDPROXY_STATE_LINK_SETUP:
        {
            switch ( event.id )
            {
                case CMDCHAIN_EVENT_CHAIN_COMPLETED:
                {
                    ChangeState( ledIdx, LEDPROXY_STATE_PULL_FROM_SERVER );
                    break;
                }
                case CMDCHAIN_EVENT_CHAIN_FAILED:
                {
                    ChangeState( ledIdx, LEDPROXY_STATE_LINK_SETUP );
                    break;
                }
            }
            break;
        }

        case LEDPROXY_STATE_PULL_FROM_SERVER:
        {
            switch ( event.id )
            {
                case CMDCHAIN_EVENT_CHAIN_COMPLETED:
                {
                    ChangeState( ledIdx, LEDPROXY_STATE_PUSH_TO_SERVER );
                    break;
                }
                case CMDCHAIN_EVENT_CHAIN_FAILED:
                {
                    ChangeState( ledIdx, LEDPROXY_STATE_PULL_FROM_SERVER );
                    break;
                }
            }
            break;
        }

        case LEDPROXY_STATE_PUSH_TO_SERVER:
        {
            switch ( event.id )
            {
                case CMDCHAIN_EVENT_CHAIN_COMPLETED:
                {
                    ChangeState( ledIdx, LEDPROXY_STATE_PROXY_READY );
                    break;
                }
                case CMDCHAIN_EVENT_CHAIN_FAILED:
                {
                    ChangeState( ledIdx, LEDPROXY_STATE_PUSH_TO_SERVER );
                    break;
                }
            }
            break;
        }

        case LEDPROXY_STATE_PROXY_READY:

            break;
    }
}

/*
 ******************************************************************************
 * Prepare request for link setup (Subscription for events on remote node).
 ******************************************************************************
 */
static bool PrepareRequest_LinkSetup(
        tCmdChain_Id                cmdChainId,
        uint8                       chainIndex,
        tIRoboticsProtocol_Request* pReq,
        uint32*                     pTimeout )
{
    *pTimeout = LEDPROXY_CMD_TIMEOUT;   /* Default timeout for all commands in chain */

    if ( LEDPROXY_NULL_LED_INDEX == GetIndexFromCmdChainId( cmdChainId ) )
        /* Returning false means skip,
         * which means that the command chain will continue to the next command */
        return false;


    switch ( chainIndex )
    {
        case LEDPROXY_CMDCHAIN_LINKSETUP_GET_STATIC_ID:
        {
            // Get static ID of destination node
            IRoboticsProtocol_RequestInit(
                    pReq, LINKMANAGER_TIF_FAMILY, LINKMANAGER_TIF_COMMAND_GET_NODE_ID );
            break;
        }
        case LEDPROXY_CMDCHAIN_LINKSETUP_REGISTER_EVENTS:
        {
            // Send subscribe all events.
            IRoboticsProtocol_RequestInit(
                    pReq, LED_TIF_FAMILY, LED_TIF_COMMAND_SUBSCRIBE_ALL_EVENTS );
            break;
        }

        default:

            return false;
    }

    return true;
}

/*
 ******************************************************************************
 * Prepare RoboticProtocol request for pull command chain
 ******************************************************************************
 */
static bool PrepareRequest_Pull(
        tCmdChain_Id                cmdChainId,
        uint8                       chainIndex,
        tIRoboticsProtocol_Request* pReq,
        uint32*                     pTimeout )
{
    *pTimeout = LEDPROXY_CMD_TIMEOUT;  /* Default timeout for all commands in chain */

    const tILedCfg_Index ledIdx = GetIndexFromCmdChainId( cmdChainId );
    if ( LEDPROXY_NULL_LED_INDEX == ledIdx )
        /* Returning false means skip,
         * which means that the command chain will continue to the next command */
        return false;


    switch ( chainIndex )
    {
        case LEDPROXY_CMDCHAIN_PULL_MODE:
        {
            // Prepare GetMode request.
            IRoboticsProtocol_RequestInit( pReq, LED_TIF_FAMILY, LED_TIF_COMMAND_GET_MODE );
            IRoboticsProtocol_RequestSetParameters(
                    pReq, sizeof( tLedTif_Index ),
                    (uint8*)&proxyVars.ledVars[ledIdx].pullCache.getReq );
            break;
        }

        case LEDPROXY_CMDCHAIN_PULL_PERIOD:
        {
            // Prepare GetMSPerPeriod request
            IRoboticsProtocol_RequestInit( pReq, LED_TIF_FAMILY, LED_TIF_COMMAND_GET_MS_PER_PERIOD );
            IRoboticsProtocol_RequestSetParameters(
                    pReq, sizeof( tLedTif_Index ),
                    (uint8*)&proxyVars.ledVars[ledIdx].pullCache.getReq );
            break;
        }

        default:

            return false;
    }

    return true;
}

/*
 ******************************************************************************
 * Prepare RoboticProtocol request for push command chain
 ******************************************************************************
 */
static bool PrepareRequest_Push(
        tCmdChain_Id                cmdChainId,
        uint8                       chainIndex,
        tIRoboticsProtocol_Request* pReq,
        uint32*                     pTimeout )
{
    *pTimeout = LEDPROXY_CMD_TIMEOUT;  /* Default timeout for all commands in chain */

    const tILedCfg_Index ledIdx = GetIndexFromCmdChainId( cmdChainId );
    if ( LEDPROXY_NULL_LED_INDEX == ledIdx )
        /* Returning false means skip,
         * which means that the command chain will continue to the next command */
        return false;


    switch ( chainIndex )
    {
        case LEDPROXY_CMDCHAIN_PUSH_MODE:
        {
            // Prepare GetMode request.
            IRoboticsProtocol_RequestInit( pReq, LED_TIF_FAMILY, LED_TIF_COMMAND_SET_MODE );
            IRoboticsProtocol_RequestSetParameters(
                    pReq, sizeof( tLedTif_Mode ),
                    (uint8*)&proxyVars.ledVars[ledIdx].pushCache.setModeReq );
            break;
        }

        case LEDPROXY_CMDCHAIN_PUSH_PERIOD:
        {
            // Prepare GetMSPerPeriod request
            IRoboticsProtocol_RequestInit( pReq, LED_TIF_FAMILY, LED_TIF_COMMAND_SET_MS_PER_PERIOD );
            IRoboticsProtocol_RequestSetParameters(
                    pReq, sizeof( tLedTif_Period ),
                    (uint8*)&proxyVars.ledVars[ledIdx].pushCache.setPeriodReq );
            break;
        }

        default:

            return false;
    }

    return true;
}


/*
 ******************************************************************************
 * Handle RoboticProtocol response from remote node (Update module attributes)
 ******************************************************************************
 */
static void RoboticsProtocolResponseCb( tIRoboticsProtocol_Response* pRsp )
{
    if ( NULL == pRsp )
    {
        ILOG( ILOG_LEVEL_WARNING, "%s: Response(NULL)", "^%s", __func__ );
        return;
    }

    if ( IROBOTICSPROTOCOL_CMD_RESULT_OK != pRsp->result )
    {
        ILOG( ILOG_LEVEL_WARNING, "%s: result(%u), linkId(%u), commandFamily(%u), commandId(%u)", "^%s^%u^%u^%u^%u",
             __func__, pRsp->result, pRsp->linkId, pRsp->commandFamily, pRsp->commandId );
    }

    switch ( pRsp->commandFamily )
    {
        case LINKMANAGER_TIF_FAMILY:
        {
            switch ( pRsp->commandId )
            {
                case LINKMANAGER_TIF_COMMAND_GET_NODE_ID:
                {
                    if ( pRsp->pDataBuffer != NULL )
                    {
                        const tILedCfg_Index ledIdx =
                                GetIndexFromLinkId( (tILinkManagerLinkId ) pRsp->linkId );

                        if ( LEDPROXY_NULL_LED_INDEX == ledIdx )
                        {
                            LOG( ILOG_LEVEL_WARNING,
                                 "%s: Unknown LinkId: linkId(%u), commandFamily(%u), commandId(%u)",
                                 __func__, pRsp->linkId, pRsp->commandFamily, pRsp->commandId );
                            return;
                        }
                        proxyVars.ledVars[ ledIdx ].staticNodeId =
                                ( (tLinkManagerTif_GetNodeIdRsp* ) ( pRsp->pDataBuffer ) )->nodeId;
                    }
                    break;
                }
            }
            break;
        }

        case LED_TIF_FAMILY:
        {
            switch ( pRsp->commandId )
            {
                case LED_TIF_COMMAND_GET_MODE:
                {
                    UpdateMode(
                            ((tLedTif_Mode*)( pRsp->pDataBuffer))->index,
                            ((tLedTif_Mode*)( pRsp->pDataBuffer))->mode );
                    break;
                }

                case LED_TIF_COMMAND_GET_MS_PER_PERIOD:
                {
                    UpdatePeriod(
                            ((tLedTif_Period*)( pRsp->pDataBuffer))->index,
                            ((tLedTif_Period*)( pRsp->pDataBuffer))->period );
                    break;
                }
            }
        }
    }
}

/*
 ******************************************************************************
 * Handle RoboticProtocol events sent from TIF on remote node
 ******************************************************************************
 */
static void RoboticsProtocolEventCb( tIRoboticsProtocol_Event* pEvent )
{
    if ( NULL == pEvent )
        return;

    if ( LED_TIF_FAMILY != pEvent->eventFamily )
        return;


    uint8 staticSenderId = 0;
    if ( ILinkManager_IsLinkBroadcast( pEvent->linkId ) )
    {
        uint16 family;
        uint8 subchannel;
        if ( !ILinkManager_DecodeBroadcastLinkId( pEvent->linkId, &staticSenderId, &family, &subchannel ) )
        {
            return;
        }
    }

    const tILedCfg_Index ledIdx = GetIndexFromLinkId( (tILinkManagerLinkId) pEvent->linkId );
    if ( LEDPROXY_NULL_LED_INDEX == ledIdx                                   ||
         staticSenderId          != proxyVars.ledVars[ ledIdx ].staticNodeId ||
         pEvent->linkId          != proxyVars.ledVars[ ledIdx ].linkId )
    {
        return;
    }


    switch ( pEvent->eventId )
    {
        case LED_TIF_EVENT_MODE_UPDATED:
        {
            UpdateMode(
                    ((tLedTif_Mode*)(pEvent->pDataBuffer))->index,
                    ((tLedTif_Mode*)(pEvent->pDataBuffer))->mode );
            break;
        }

        case LED_TIF_EVENT_PERIOD_UPDATED:
        {
            UpdatePeriod(
                    ((tLedTif_Period*)(pEvent->pDataBuffer))->index,
                    ((tLedTif_Period*)(pEvent->pDataBuffer))->period );
            break;
        }
        default:
            break;
    }
}


/*
 ******************************************************************************
 *
 ******************************************************************************
 */
static void UpdateMode( const tILed_Index ledIdx, const tILed_Mode mode )
{
    if( LEDPROXY_LEDS_NUMBER <= ledIdx )
    {
        ILOG( ILOG_LEVEL_WARNING, "%s: Unsupported LED: LedIndex(%d)", "^%s^%d", __func__, ledIdx );
        return;
    }

    if( mode == proxyVars.ledVars[ ledIdx ].pullCache.mode )
        return; /* No change, ignore */

    proxyVars.ledVars[ ledIdx ].pullCache.mode = mode;

    ILOG( ILOG_LEVEL_DEBUG, "%s: LedIndex(%d), mode(%d)", "^%s^%d^%d", __func__, ledIdx, mode );
    // Notify all about period update in 'ix' LED
    NotifyAll( ILED_EVENT_MODE_UPDATED, ledIdx );
}

/*
 ******************************************************************************
 *
 ******************************************************************************
 */
static void UpdatePeriod( const tILed_Index ledIdx, const tILed_MilliSeconds period )
{
    if( LEDPROXY_LEDS_NUMBER <= ledIdx )
    {
        ILOG( ILOG_LEVEL_WARNING, "%s: Unsupported LED: LedIndex(%d)", "^%s^%d", __func__, ledIdx );
        return;
    }

    if( period == proxyVars.ledVars[ ledIdx ].pullCache.period )
        return; /* No change, ignore */

    proxyVars.ledVars[ ledIdx ].pullCache.period = period;

    ILOG( ILOG_LEVEL_DEBUG, "%s: LedIndex(%d), period(%d)", "^%s^%d^%d", __func__, ledIdx, period );
    // Notify all about period update in 'ix' LED
    NotifyAll( ILED_EVENT_PERIOD_UPDATED, ledIdx );
}


/*
 ******************************************************************************
 * Return local led index associated with link ID.
 * LEDPROXY_NULL_LED_INDEX is returned if no index was found.
 ******************************************************************************
 */
static tILedCfg_Index GetIndexFromLinkId( const tILinkManagerLinkId linkId )
{
    tILedCfg_Index ledIdx;
    for ( ledIdx = (tILedCfg_Index)0; ledIdx < LEDPROXY_LEDS_NUMBER; ++ledIdx )
        if ( linkId == proxyVars.ledVars[ ledIdx ].linkId )
            return ledIdx;
    return (tILedCfg_Index)LEDPROXY_NULL_LED_INDEX;
}

/*
 ******************************************************************************
 * Return local led index associated with command chain ID.
 * LEDPROXY_NULL_LED_INDEX is returned if no index was found.
 ******************************************************************************
 */
static tILedCfg_Index GetIndexFromCmdChainId( const tCmdChain_Id cmdChainId )
{
    tILedCfg_Index ledIdx;
    for ( ledIdx = (tILedCfg_Index)0; ledIdx < LEDPROXY_LEDS_NUMBER; ++ledIdx )
    {
        if ( cmdChainId == proxyVars.ledVars[ ledIdx ].cmdChainLinkSetup  ||
             cmdChainId == proxyVars.ledVars[ ledIdx ].cmdChainPull  ||
             cmdChainId == proxyVars.ledVars[ ledIdx ].cmdChainPush )
        {
            return ledIdx;
        }
    }
    return (tILedCfg_Index)LEDPROXY_NULL_LED_INDEX;
}
