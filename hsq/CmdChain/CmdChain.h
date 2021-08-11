/**
 ******************************************************************************
 * @file      CmdChain.h
 *
 * @brief     Command chain manager
 ******************************************************************************
 */

#ifndef CMDCHAIN_H
#define CMDCHAIN_H

/*
-------------------------------------------------------------------------------
   Include files
-------------------------------------------------------------------------------
*/
#include "IRoboticsProtocol.h"

/*
-------------------------------------------------------------------------------
    Type definitions
-------------------------------------------------------------------------------
*/

typedef uint32 tCmdChain_Id;

#define CMDCHAIN_INVALID_ID ( ( tCmdChain_Id ) MAX_uint32 )

typedef enum
{
    CMDCHAIN_EVENT_CHAIN_COMPLETED  = CmdChain_ID << 16,
    CMDCHAIN_EVENT_CHAIN_FAILED
} tCmdChain_Events;

/**
 ******************************************************************************
 * @brief   Callback for preparing a request
 * @param   cmdChainId
 *          Command chain id
 * @param   chainIndex
 *          Index to create request for
 * @param   pReq
 *          Request to fill in. Note that the payload used must be statically
 *          allocated, so that it is available when the request is actually
 *          sent.
 * @returns True if command shall be sent. False if command should be skipped.
 ******************************************************************************
 */
typedef bool ( *tCmdChain_PrepareReqCallback )( tCmdChain_Id cmdChainId, uint8 chainIndex, tIRoboticsProtocol_Request* pReq, uint32* pTimeout );



/*
-------------------------------------------------------------------------------
    Interface function prototypes
-------------------------------------------------------------------------------
*/

/**
 ******************************************************************************
 * @brief   Inits command chain module
 ******************************************************************************
 */
void CmdChain_Init( void );

/**
 ******************************************************************************
 * @brief   Starts command chain module
 ******************************************************************************
 */
void CmdChain_Start( void );

/**
 ******************************************************************************
 * @brief   Creates a command chain
 * @param   linkId
 *          Link to use for sending commands
 * @param   startIndex
 *          Starting index in command list
 * @param   stopIndex
 *          Stop index in command list
 * @param   pResponseCb
 *          Responses that are received are reported to this function.
 * @param   pEventCb
 *          Event callback where command chain events shall be reported.
 * @param   pPrepareRequestCb
 *          User function to prepare request to be sent, for a specific
 *          index in the command chain.
 * @param   defaultTimeout
 *          Default timeout for command responses. Can be specified in
 *          pPrepareRequestCb if needed for each command.
 ******************************************************************************
 */
tCmdChain_Id CmdChain_CreateChain( tILinkManagerLinkId                 linkId,
                                   uint8                               startIndex,
                                   uint8                               stopIndex,
                                   tIRoboticsProtocol_ResponseHandler  pResponseCb,
                                   tEventCallback                      pEventCb,
                                   tCmdChain_PrepareReqCallback        pPrepareRequestCb,
                                   uint32                              defaultTimeout );

/**
 ******************************************************************************
 * @brief   Re init chain with new values
 * @param   cmdChainId
 *          Command chain id
 * @param   linkId
 *          Link to use for sending commands
 * @param   startIndex
 *          Starting index in command list
 * @param   stopIndex
 *          Stop index in command list
 * @param   pResponseCb
 *          Responses that are received are reported to this function.
 * @param   pEventCb
 *          Event callback where command chain events shall be reported.
 * @param   pPrepareRequestCb
 *          User function to prepare request to be sent, for a specific
 *          index in the command chain.
 * @param   defaultTimeout
 *          Default timeout for command responses. Can be specified in
 *          pPrepareRequestCb if needed for each command.
 * @returns True if chain was reinit successfully, false otherwise.
 ******************************************************************************
 */
bool  CmdChain_ReInitChain(    tCmdChain_Id                        cmdChainId,
                               tILinkManagerLinkId                 linkId,
                               uint8                               startIndex,
                               uint8                               stopIndex,
                               tIRoboticsProtocol_ResponseHandler  pResponseCb,
                               tEventCallback                      pEventCb,
                               tCmdChain_PrepareReqCallback        pPrepareRequestCb,
                               uint32                              defaultTimeout );

/**
 ******************************************************************************
 * @brief   Runs a command chain
 * @param   cmdChainId
 *          Command chain id
 * @returns True if chain was started successfully, false otherwise.
 ******************************************************************************
 */
bool CmdChain_Run( tCmdChain_Id cmdChainId );

/**
 ******************************************************************************
 * @brief   Check if command chain is running
 * @param   cmdChainId
 *          Command chain id
 * @returns True if chain is running, false otherwise.
 ******************************************************************************
 */
bool CmdChain_IsRunning( tCmdChain_Id cmdChainId );

/**
 ******************************************************************************
 * @brief   Marks an index in the command chain as dirty. This means that this
 *          command will be sent the next time the chain is executed.
 * @param   cmdChainId
 *          Command chain id
 * @param   chainIndex
 *          Index to mark dirty
 * @returns True if dirty flag was set successfully, false otherwise.
 ******************************************************************************
 */
bool CmdChain_SetDirty( tCmdChain_Id cmdChainId, uint8 chainIndex );

/**
 ******************************************************************************
 * @brief   Marks entire command chain as dirty. This means that all
 *          commands will be sent the next time the chain is executed.
 * @param   cmdChainId
 *          Command chain id
 * @returns True if dirty flags where set successfully, false otherwise.
 ******************************************************************************
 */
bool CmdChain_SetAllDirty( tCmdChain_Id cmdChainId );

/**
 ******************************************************************************
 * @brief   Clears dirty flag on one index. This means that this command will
 *          not be sent the next time the chain is executed.
 * @param   cmdChainId
 *          Command chain id
 * @param   chainIndex
 *          Index to clear dirty flag on
 * @returns True if dirty flag was cleared successfully, false otherwise.
 ******************************************************************************
 */
bool CmdChain_ClearDirty( tCmdChain_Id cmdChainId, uint8 chainIndex );

/**
 ******************************************************************************
 * @brief   Clears all dirty flags in the command chain. This means that no
 *          commands will be sent the next time the chain is executed.
 * @param   cmdChainId
 *          Command chain id
 * @returns True if dirty flags where cleared successfully, false otherwise.
 ******************************************************************************
 */
bool CmdChain_ClearAllDirty( tCmdChain_Id cmdChainId );

/**
 ******************************************************************************
 * @brief   Get chainId for a response
 *          Note!! This function is only valid to be called in the context of
 *          robotics command response callback (pResponseCb passed to
 *          CmdChain_CreateChain()).
 * @param   pRsp
 *          Pointer to response
 * @returns Chain Id for the chain that generated the response
 ******************************************************************************
 */
tCmdChain_Id CmdChain_GetChainIdForResponse( tIRoboticsProtocol_Response* pRsp );


#endif /* CMDCHAIN_H */

