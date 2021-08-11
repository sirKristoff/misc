/**
 ******************************************************************************
 * @file      CmdChain.c
 *
 * @brief     Command chain base.
 ******************************************************************************
 */

/*
 ------------------------------------------------------------------------------
    Include files
 ------------------------------------------------------------------------------
*/

#include "IRoboticsProtocol.h"
#include "ILog.h"
#include "IScheduler.h"
#include "CmdChain.h"
#include "ICmdChainCfg.h"
#include "ILog.h"
#include "ISoftwareException.h"

/*
 ------------------------------------------------------------------------------
    Defines
 ------------------------------------------------------------------------------
 */

/*
 ------------------------------------------------------------------------------
    Local types
 ------------------------------------------------------------------------------
 */

typedef struct
{
    // Internal variables
    bool                                used;
    uint8                               currentIndex;
    uint64                              dirtyFlags;
    uint8                               lastTransactionId;
    bool                                lastCmdWasSent; // True if last command was sent and is expected (by TriggerChain())
                                                        // to be true until chain is called with CmdChain_Run() again.
    uint16                              lastCmdFamily;
    uint16                              lastCmdId;
    bool                                inFlightCmdSetDirty; // Somebody set the current "in-flight" command to dirty

    // User settings
    tILinkManagerLinkId                 linkId;
    uint8                               startIndex;
    uint8                               stopIndex;
    tIRoboticsProtocol_ResponseHandler  pResponseCb;
    tEventCallback                      pEventCb;
    tCmdChain_PrepareReqCallback        pPrepareRequest;
    uint32                              defaultTimeout;
} tCmdChain;

typedef struct
{
    uint8       transactionId;
    tCmdChain   chains[ CMDCHAINCFG_MAX_CHAINS ];
} tCmdChain_Vars;

/*
 ------------------------------------------------------------------------------
    Private data
 ------------------------------------------------------------------------------
 */

tCmdChain_Vars cmdChainVars;

/*
 ------------------------------------------------------------------------------
    Private function prototypes
 ------------------------------------------------------------------------------
 */


/**
 ******************************************************************************
 * @brief   Gets chain pointer from chain id.
 * @param   cmdChainId
 * @returns Chain pointer
 ******************************************************************************
 */
tCmdChain* GetChain( tCmdChain_Id cmdChainId );

/**
 ******************************************************************************
 * @brief   Gets chain id from chain pointer.
 * @param   pCmdChain
 * @returns Chain id
 ******************************************************************************
 */
tCmdChain_Id GetChainId( tCmdChain* pCmdChain );

/**
 ******************************************************************************
 * @brief   Sends next command in chain
 * @param   pCmdChain
 ******************************************************************************
 */
void SendNextCmd( tCmdChain* pCmdChain );

/**
 ******************************************************************************
 * @brief   Robotics protocol response handler
 ******************************************************************************
 */
void ResponseHandler( tIRoboticsProtocol_Response* pRsp );

/**
 ******************************************************************************
 * @brief   Triggers chain to keep on going. Used when a chain has paused
 *          because the link was busy.
 ******************************************************************************
 */
void TriggerChain( tCmdChain* pCmdChain );

/**
 ******************************************************************************
 * @brief   Triggers pending chains on a specific link, when that link is
 *          free to be used.
 ******************************************************************************
 */
void TriggerPendingChains( tILinkManagerLinkId linkId );

/**
 ******************************************************************************
 * @brief   Internal dirty flag handlers.
 ******************************************************************************
 */
bool SetDirty( tCmdChain* pCmdChain, uint8 chainIndex, bool dirty );
void SetAllDirty( tCmdChain* pCmdChain, bool dirty );
bool GetDirty( tCmdChain* pCmdChain, uint8 chainIndex );
bool GetAllCleared( tCmdChain* pCmdChain );
/**
 ******************************************************************************
 * @brief   Returns true in case the stop index is ok to use.
 ******************************************************************************
 */
static bool IsStopIndexOk( const uint8 stopIndex, const tCmdChain_Id cmdChainId );



/*
 ------------------------------------------------------------------------------
    Public functions
 ------------------------------------------------------------------------------
 */

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void CmdChain_Init( void )
{
    for ( uint32 i=0; i<CMDCHAINCFG_MAX_CHAINS; i++ )
    {
        tCmdChain* pCmdChain = GetChain( i );
        pCmdChain->used = false;
    }
    cmdChainVars.transactionId = 0;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void CmdChain_Start( void )
{
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCmdChain_Id CmdChain_CreateChain( tILinkManagerLinkId                 linkId,
                                   uint8                               startIndex,
                                   uint8                               stopIndex,
                                   tIRoboticsProtocol_ResponseHandler  pResponseCb,
                                   tEventCallback                      pEventCb,
                                   tCmdChain_PrepareReqCallback        pPrepareRequestCb,
                                   uint32                              defaultTimeout )
{
    tCmdChain* pCmdChain;

    // Find free slot
    for ( uint32 i=0; i<CMDCHAINCFG_MAX_CHAINS; i++ )
    {
        pCmdChain = GetChain( i );
        if ( !pCmdChain->used )
        {
            break;
        }
    }

    // Check if we found a free slot
    if ( pCmdChain->used )
    {
        SOFTWARE_EXCEPTION();
    }

    pCmdChain->used                = true;
    pCmdChain->linkId              = linkId;
    pCmdChain->currentIndex        = 0;
    pCmdChain->stopIndex           = stopIndex;
    pCmdChain->pResponseCb         = pResponseCb;
    pCmdChain->pEventCb            = pEventCb;
    pCmdChain->pPrepareRequest     = pPrepareRequestCb;
    pCmdChain->dirtyFlags          = 0;
    pCmdChain->lastTransactionId   = 0;
    pCmdChain->lastCmdFamily       = 0;
    pCmdChain->lastCmdId           = 0;
    pCmdChain->lastCmdWasSent      = false;
    pCmdChain->defaultTimeout      = defaultTimeout;
    pCmdChain->inFlightCmdSetDirty = false;

    tCmdChain_Id retval = GetChainId( pCmdChain );

	// Run this function just ot get a log message in case of error.
	// We can't currently do more than this (SoftwareException is not wanted).
    IsStopIndexOk( stopIndex, retval );

    return retval;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool  CmdChain_ReInitChain(    tCmdChain_Id                        cmdChainId,
                               tILinkManagerLinkId                 linkId,
                               uint8                               startIndex,
                               uint8                               stopIndex,
                               tIRoboticsProtocol_ResponseHandler  pResponseCb,
                               tEventCallback                      pEventCb,
                               tCmdChain_PrepareReqCallback        pPrepareRequestCb,
                               uint32                              defaultTimeout )
{
    tCmdChain* pCmdChain = GetChain( cmdChainId );

    if ( pCmdChain == NULL )
    {
        return false;
    }

    pCmdChain->used                = true;
    pCmdChain->linkId              = linkId;
    pCmdChain->currentIndex        = 0;
    pCmdChain->stopIndex           = stopIndex;
    pCmdChain->pResponseCb         = pResponseCb;
    pCmdChain->pEventCb            = pEventCb;
    pCmdChain->pPrepareRequest     = pPrepareRequestCb;
    pCmdChain->dirtyFlags          = 0;
    pCmdChain->lastTransactionId   = 0;
    pCmdChain->lastCmdFamily       = 0;
    pCmdChain->lastCmdId           = 0;
    pCmdChain->lastCmdWasSent      = false;
    pCmdChain->defaultTimeout      = defaultTimeout;
    pCmdChain->inFlightCmdSetDirty = false;

    return IsStopIndexOk( stopIndex, cmdChainId );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool CmdChain_Run( tCmdChain_Id cmdChainId )
{
    tCmdChain* pCmdChain = GetChain( cmdChainId );

    if ( pCmdChain == NULL )
    {
        return false;
    }

    // Check if command chain is already running, in that case we do nothing.
    if ( pCmdChain->currentIndex != 0 && pCmdChain->lastCmdWasSent)
    {
        return true;
    }

    pCmdChain->lastCmdWasSent = false;

    //LOG( ILOG_LEVEL_DEBUG, "Run chain %d", cmdChainId );
    SendNextCmd( pCmdChain );

    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool CmdChain_IsRunning( tCmdChain_Id cmdChainId )
{
    tCmdChain* pCmdChain = GetChain( cmdChainId );

    if ( pCmdChain == NULL )
    {
        return false;
    }

    return pCmdChain->currentIndex != 0;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool CmdChain_SetDirty( tCmdChain_Id cmdChainId, uint8 chainIndex )
{
    tCmdChain* pCmdChain = GetChain( cmdChainId );
    if ( pCmdChain == NULL )
    {
        return false;
    }

    //LOG( ILOG_LEVEL_DEBUG, "Set dirty on id %d on chain %d", chainIndex, cmdChainId );

    return SetDirty( pCmdChain, chainIndex, true );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool CmdChain_ClearDirty( tCmdChain_Id cmdChainId, uint8 chainIndex )
{
    tCmdChain* pCmdChain = GetChain( cmdChainId );
    if ( pCmdChain == NULL )
    {
        return false;
    }

    ILOG( ILOG_LEVEL_DEBUG, "Clear dirty on id %d on chain %d", "^%d^%d", chainIndex, cmdChainId );

    return SetDirty( pCmdChain, chainIndex, false );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool CmdChain_SetAllDirty( tCmdChain_Id cmdChainId )
{
    tCmdChain* pCmdChain = GetChain( cmdChainId );
    if ( pCmdChain == NULL )
    {
        return false;
    }

#if 0
    ILOG( ILOG_LEVEL_DEBUG, "Set all dirty on chain %d (link %u)", "^%d^%u", cmdChainId, pCmdChain->linkId);
#endif

    SetAllDirty( pCmdChain, true );
    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool CmdChain_ClearAllDirty( tCmdChain_Id cmdChainId )
{
    tCmdChain* pCmdChain = GetChain( cmdChainId );
    if ( pCmdChain == NULL )
    {
        return false;
    }

    ILOG( ILOG_LEVEL_DEBUG, "Clear all dirty on chain %d", "^%d", cmdChainId );

    SetAllDirty( pCmdChain, false );
    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCmdChain_Id CmdChain_GetChainIdForResponse( tIRoboticsProtocol_Response* pRsp )
{
    for ( tCmdChain_Id i=0; i<CMDCHAINCFG_MAX_CHAINS; i++ )
    {
        tCmdChain* pCmdChain = GetChain( i );
        if ( pCmdChain != NULL )
        {
            if ( pCmdChain->linkId == pRsp->linkId )
            {
                if ( pCmdChain->lastCmdWasSent ) //Verify this is an outstanding request or non active chain
                {
                    if ( pCmdChain->currentIndex != 0 ) //Verify this is an active chain
                    {
                        if ( pCmdChain->lastTransactionId == pRsp->transactionId )
                        {
                            return i;
                        }
                    }
                }
            }
        }
    }

    return CMDCHAIN_INVALID_ID;
}
/*
 ------------------------------------------------------------------------------
    Private functions
 ------------------------------------------------------------------------------
 */


/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCmdChain_Id GetChainId( tCmdChain* pCmdChain )
{
    for ( uint32 i=0; i<CMDCHAINCFG_MAX_CHAINS; i++ )
    {
        if ( GetChain( i ) == pCmdChain )
        {
            return i;
        }
    }

    // Return invalid id
    return ( tCmdChain_Id )-1;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCmdChain* GetChain( tCmdChain_Id cmdChainId )
{
    if ( cmdChainId >= CMDCHAINCFG_MAX_CHAINS )
    {
        return NULL;
    }

    return ( &cmdChainVars.chains[ cmdChainId ] );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void SendNextCmd( tCmdChain* pCmdChain )
{
    bool cmdSkipped = true;

    while ( cmdSkipped )
    {
        // Increase command index. Don't do this if last command was not sent.
        if ( pCmdChain->lastCmdWasSent )
        {
            pCmdChain->currentIndex++;
        }

        // Check if we are done
        if ( pCmdChain->currentIndex < pCmdChain->stopIndex )
        {
            // Check if we should send next command
            if ( GetDirty( pCmdChain, pCmdChain->currentIndex ))
            {
                tIRoboticsProtocol_Request req;
                uint32 timeout = pCmdChain->defaultTimeout;

                // Get request from user
                cmdSkipped = !pCmdChain->pPrepareRequest( GetChainId( pCmdChain ), pCmdChain->currentIndex, &req, &timeout );
                if ( !cmdSkipped )
                {
                    //LOG( ILOG_LEVEL_DEBUG, "Send command 0x%04x 0x%04x", req.commandFamily, req.commandId );

                    cmdChainVars.transactionId++;
                    req.transactionId = cmdChainVars.transactionId;

                    // Send command. Dirty flag is cleared when response is received.
                    pCmdChain->lastTransactionId = cmdChainVars.transactionId;
                    pCmdChain->lastCmdFamily     = req.commandFamily;
                    pCmdChain->lastCmdId         = req.commandId;
                    pCmdChain->lastCmdWasSent    = IRoboticsProtocol_SendRequest( pCmdChain->linkId, &req, ResponseHandler, timeout );
                }
                else
                {
                    // Command was skipped by user. Mark as not dirty anyway.
                    SetDirty( pCmdChain, pCmdChain->currentIndex, false );
                }
            }
            else
            {
                //Set lastCmdWasSent to make sure next iteration steps up currentIndex
                pCmdChain->lastCmdWasSent = true;
            }
        }
        else
        {
            // We reached the end. Check dirty flags.
            pCmdChain->currentIndex = 0;
            if ( GetAllCleared( pCmdChain ))
            {
                tEvent event;
                event.id = CMDCHAIN_EVENT_CHAIN_COMPLETED;
                event.data = GetChainId( pCmdChain );

                pCmdChain->pEventCb( event );

                // Trigger other command chains that may be pending
                TriggerPendingChains( pCmdChain->linkId );
            }
            else
            {
                // Dirty flags still remain. Start over.
                CmdChain_Run( GetChainId( pCmdChain ));
            }
            return;
        }
    }
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void ResponseHandler( tIRoboticsProtocol_Response* pRsp )
{
    if ( pRsp->result == IROBOTICSPROTOCOL_CMD_RESULT_TIMEOUT )
    {
        ILOG( ILOG_LEVEL_DEBUG, "Timeout for command 0x%04x 0x%04x", "^%04x^%04x", pRsp->commandFamily, pRsp->commandId );
    }
    else
    {
        //LOG( ILOG_LEVEL_DEBUG, "Response received 0x%04x 0x%04x", pRsp->commandFamily, pRsp->commandId );
    }

    // Find which command chain this response is related too
    for ( uint32 i=0; i<CMDCHAINCFG_MAX_CHAINS; i++ )
    {
        tCmdChain* pCmdChain = GetChain( i );
        if ( pCmdChain != NULL )
        {
            // Check if chain is active
            if ( pCmdChain->currentIndex != 0 )
            {
                // Check if link matches
                if ( pCmdChain->linkId == pRsp->linkId )
                {
                    // Check if last command was sent successfully
                    // Otherwise, the response cannot belong to this chain
                    if ( pCmdChain->lastCmdWasSent )
                    {
                        // Check if command matches
                        if ( pCmdChain->lastTransactionId == pRsp->transactionId )
                        {
                            if ( pRsp->result != IROBOTICSPROTOCOL_CMD_RESULT_TIMEOUT )
                            {
                                // There is a chance that someone has set the current step
                                // executing in the command chain to dirty whilst we've been
                                // waiting for the response of the step. In this case,
                                // we have a secondary flag ( inFlightCmdSetDirty ) to alert
                                // us to this situation, and we then do NOT clear the dirty
                                // flag (or the step will not be executed again).
                                //
                                // Typical situation this mitigates:
                                //    1. Step 1 is set dirty by external user.
                                //    2. CmdChain is started.
                                //    3. CmdChain sends command for step 1.
                                //    4. Step 1 is set dirty by external.
                                //    5. Response for step 1 is received, step 1 dirty flag cleared.
                                //    6. ==> second 'set dirty' from external is missed
                                //
                                if ( pCmdChain->inFlightCmdSetDirty == false )
                                {
                                    // Clear dirty flag
                                    SetDirty( pCmdChain, pCmdChain->currentIndex, false );
                                }
                                // Clear in-flight command dirty flag
                                pCmdChain->inFlightCmdSetDirty = false;

                                // Inform user
                                pCmdChain->pResponseCb( pRsp );

                                // Move on
                                SendNextCmd( pCmdChain );
                                return;
                            }
                            else
                            {
                                // Timeout!
                                // Inform user that the chain failed, and reset the chain
                                tEvent event;
                                event.id = CMDCHAIN_EVENT_CHAIN_FAILED;
                                event.data = GetChainId( pCmdChain );

                                pCmdChain->currentIndex = 0;
                                pCmdChain->pEventCb( event );

                                // Trigger other command chains that may be pending
                                TriggerPendingChains( pCmdChain->linkId );
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    ILOG( ILOG_LEVEL_WARNING, "Orphan response received! (Family:0x%04x, Id:0x%04x)", "^%04x^%04x", pRsp->commandFamily, pRsp->commandId );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void TriggerChain( tCmdChain* pCmdChain )
{
    // Check if chain is active
    if ( pCmdChain->currentIndex == 0 )
    {
        return;
    }

    // Check if last command was not sent
    if ( pCmdChain->lastCmdWasSent )
    {
        return;
    }

    SendNextCmd( pCmdChain );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void TriggerPendingChains( tILinkManagerLinkId linkId )
{
    // Find chains that use the same link
    for ( uint32 i=0; i<CMDCHAINCFG_MAX_CHAINS; i++ )
    {
        tCmdChain* pCmdChain = GetChain( i );
        if ( pCmdChain != NULL )
        {
            if ( pCmdChain->linkId == linkId )
            {
                TriggerChain( pCmdChain );
            }
        }
    }
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool SetDirty( tCmdChain* pCmdChain, uint8 chainIndex, bool dirty )
{
    uint8 internalIndex;

    if (( chainIndex <= pCmdChain->startIndex ) ||
        ( chainIndex >= pCmdChain->stopIndex ))
    {
        return false;
    }

    internalIndex = chainIndex - pCmdChain->startIndex;
    if ( dirty )
    {
        pCmdChain->dirtyFlags |= ( ( uint64 )1 << internalIndex );

        if ( pCmdChain->currentIndex != 0 && pCmdChain->currentIndex == chainIndex )
        {
            // Command chain is running and is currently at the step someone
            // wants to flag as dirty. Alert the command chain to not clear
            // the dirty flag upon response from the current step.
            pCmdChain->inFlightCmdSetDirty = true;
        }
    }
    else
    {
        pCmdChain->dirtyFlags &= ~( ( uint64 )1 << internalIndex );
    }

    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void SetAllDirty( tCmdChain* pCmdChain, bool dirty )
{
    for ( uint8 i=pCmdChain->startIndex+1; i<pCmdChain->stopIndex; i++ )
    {
        SetDirty( pCmdChain, i, dirty );
    }
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool GetDirty( tCmdChain* pCmdChain, uint8 chainIndex )
{
    uint8 internalIndex;

    if (( chainIndex <= pCmdChain->startIndex ) ||
        ( chainIndex >= pCmdChain->stopIndex ))
    {
        return false;
    }

    internalIndex = chainIndex - pCmdChain->startIndex;
    if ( pCmdChain->dirtyFlags & ( ( uint64 )1 << internalIndex ) )
    {
        return true;
    }

    return false;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool GetAllCleared( tCmdChain* pCmdChain )
{
    for ( uint8 i=pCmdChain->startIndex+1; i<pCmdChain->stopIndex; i++ )
    {
        if ( GetDirty( pCmdChain, i ))
        {
            return false;
        }
    }

    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static bool IsStopIndexOk( const uint8 stopIndex, const tCmdChain_Id cmdChainId )
{
    // Generate ERROR log in case the stopIndex is too large.
	// Dirty flag type is limiting the maximum number that can be used.
    if ( ( 8 * sizeof( uint64 ) ) <= stopIndex )
    {
        ILOG( ILOG_LEVEL_ERROR, "stopIndex (%u) too big in chain (%d) - commands will be skipped!", "^%u^%d", stopIndex, cmdChainId );
        return false;
    }
    return true;
}
