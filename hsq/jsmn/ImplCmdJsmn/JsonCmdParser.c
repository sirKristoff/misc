/**
 ******************************************************************************
 * @file      JsonCmdParser.c
 *
 * @brief     Implementation of JSON Command Parser.
 ******************************************************************************
 */
/*
 ------------------------------------------------------------------------------
    Include files
 ------------------------------------------------------------------------------
 */
// Standard headers
#include <string.h> // memset, memcpy
#include <stdlib.h> // strtol
#include <stdio.h>  // sscanf
// The jsmn (jasmine) JSON parser
#include "jsmn.h"
// Other modules' headers
#include "RoboticTypes.h"
#include "IRoboticCfg.h"
#include "ILog.h"
#include "ISoftwareException.h"
// This module's headers
#include "IJsonCmdParser.h"    // public interface
#include "IJsonCmdParserCfg.h" // public configuration part
#include "JsonCmdParser.h"     // private interface
/*
 ------------------------------------------------------------------------------
    Definitions
 ------------------------------------------------------------------------------
 */
// The following parameters can be fine-tuned in the IJsonCmdParserCfg.h.
#ifndef JSONCMDPARSER_CFG_MAX_NUMBER_OF_HANDLES
#define JSONCMDPARSER_CFG_MAX_NUMBER_OF_HANDLES     ( 1 )
#endif
#ifndef JSONCMDPARSER_CFG_MAX_NUMBER_OF_JSMN_TOKENS
#define JSONCMDPARSER_CFG_MAX_NUMBER_OF_JSMN_TOKENS ( 256 )
#endif

#ifndef JSONCMDPARSER_CFG_MEMORY_PLACEMENT_FOR_VARIABLES
#define JSONCMDPARSER_CFG_MEMORY_PLACEMENT_FOR_VARIABLES
#endif

// The following defines are expected strings in the parsed JSON file.
// I.e. the format of the parsed JSON command file is partly defined by those
// strings.
#define JSONCMDPARSER_JSON_TOKEN_NODES          "nodes"
#define JSONCMDPARSER_JSON_TOKEN_COMMANDS       "commands"
#define JSONCMDPARSER_JSON_TOKEN_NAME           "name"
#define JSONCMDPARSER_JSON_TOKEN_TYPE           "type"
#define JSONCMDPARSER_JSON_TOKEN_MSG_TYPE       "msgType"
#define JSONCMDPARSER_JSON_TOKEN_SUB_CMD        "subCmd"
#define JSONCMDPARSER_JSON_TOKEN_PAYLOAD        "payload"
#define JSONCMDPARSER_JSON_TOKEN_EXPECTED_RSP   "expectedRsp"
#define JSONCMDPARSER_JSON_TOKEN_DEVICE_GROUP   "deviceGroup"
#define JSONCMDPARSER_JSON_TOKEN_DEVICE_TYPE    "deviceType"
#define JSONCMDPARSER_JSON_TOKEN_DEVICE_VARIANT "deviceVariant"
/*
 ------------------------------------------------------------------------------
    Type definitions
 ------------------------------------------------------------------------------
 */
/**
 * The handle is a data structure that contains "an active parser context".
 * This means that during parsing and subsequent working on the parsed result,
 * the handle is needed to store various data.
 */
typedef struct
{
    bool         inUse;                                                 /**< true in case this handle is "in-use", false if available                               */
    const char  *jsonStringBuffer;                                      /**< Pointer to the JSON file as a string buffer (provided by the user in the Parse() call) */
    jsmn_parser  parser;                                                /**< The jsmn parser                                                                        */
    int          tokensFound;                                           /**< Number of found tokens, when the jsmn parser parsed the JSON file                      */
    jsmntok_t    tokens[ JSONCMDPARSER_CFG_MAX_NUMBER_OF_JSMN_TOKENS ]; /**< The array containing the parsed JSON tokens                                            */
} tJsonCmdParser_Handle;

typedef struct
{
    bool                   isStarted;                                          /**< true in case the module has been started                              */
    tJsonCmdParser_Handle  handles[ JSONCMDPARSER_CFG_MAX_NUMBER_OF_HANDLES ]; /**< The array of possible handles, handed out by calls to Open() function */
} tJsonCmdParser_Vars;

/*
 ------------------------------------------------------------------------------
    Private data
 ------------------------------------------------------------------------------
 */
static bool isInitalized = false;             /**< true in case the module has been initialized                            */
static tJsonCmdParser_Vars jsonCmdParserVars JSONCMDPARSER_CFG_MEMORY_PLACEMENT_FOR_VARIABLES; /**< The internal data structure with module's variables (RAM control block) */
/*
 ------------------------------------------------------------------------------
    Private function prototypes
 ------------------------------------------------------------------------------
 */
static void ClearCommand( tIJsonCmdParser_Command *pCommand );
static void ClearHandle( tJsonCmdParser_Handle *pHandle );
static bool IsValidHandle( const tIJsonCmdParser_Handle handle );
static bool IsValidJson( const tIJsonCmdParser_Handle handle );
static bool JsonTokenStringEquals( const tIJsonCmdParser_Handle handle, const size_t token, const char *s );
static bool GetNodeTokenIndex( const tIJsonCmdParser_Handle handle, const size_t node, size_t *pNodeTokenIndex );
static bool GetCommandTokenIndex( const tIJsonCmdParser_Handle handle, const size_t node, const size_t command, size_t *pNodeIndex, size_t *pCommandIndex );
static bool GetNodeInformationFromTokenIndex( const tIJsonCmdParser_Handle handle, const size_t nodeIndex, tIJsonCmdParser_Command *pCommand );
static bool GetCommandInformationFromTokenIndex( const tIJsonCmdParser_Handle handle, const size_t commandIndex, tIJsonCmdParser_Command *pCommand );
static void AsciiTextToByteArray( const char *str, const size_t len, uint8 dest[] );
/*
 ------------------------------------------------------------------------------
    Interface functions implementation
 ------------------------------------------------------------------------------
 */
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IJsonCmdParser_Init( void )
{
    if ( !isInitalized )
    {
        isInitalized = true;

        // Initialize private data.
        memset(&jsonCmdParserVars, 0, sizeof(tJsonCmdParser_Vars));
        jsonCmdParserVars.isStarted = false;
        for ( size_t i = 0; i < JSONCMDPARSER_CFG_MAX_NUMBER_OF_HANDLES; ++i )
        {
            ClearHandle( &jsonCmdParserVars.handles[ i ] );
        }

        // Initialize modules used by us.
        ILog_Init();
        ISoftwareException_Init();
    }
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IJsonCmdParser_Start( void )
{
    if ( !jsonCmdParserVars.isStarted )
    {
        jsonCmdParserVars.isStarted = true;

        // Start modules used by us.
        ILog_Start();
        ISoftwareException_Start();
    }
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IJsonCmdParser_OpenHandle( tIJsonCmdParser_Handle *pHandle )
{
    // Module should be initialized and started.
    SOFTWARE_EXCEPTION_ASSERT( isInitalized && jsonCmdParserVars.isStarted );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pHandle );

    for ( size_t i = 0; i < JSONCMDPARSER_CFG_MAX_NUMBER_OF_HANDLES; ++i )
    {
        if ( !jsonCmdParserVars.handles[ i ].inUse )
        {
            jsonCmdParserVars.handles[ i ].inUse = true;
            *pHandle = i;

            ILOG( ILOG_LEVEL_DEBUG, "Open handle [%u]", "^%u", *pHandle );

            return true;
        }
    }
    ILOG( ILOG_LEVEL_WARNING, "No more handles available!", "^" );
    *pHandle = IJSONCMDPARSER_INVALID_HANDLE;
    return false;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IJsonCmdParser_CloseHandle( tIJsonCmdParser_Handle *pHandle )
{
    // Module should be initialized and started.
    SOFTWARE_EXCEPTION_ASSERT( isInitalized && jsonCmdParserVars.isStarted );

    if ( !IsValidHandle( *pHandle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid handle", "^" );
        return false;
    }
    ILOG( ILOG_LEVEL_DEBUG, "Close handle [%u]", "^%u", *pHandle );

    // Make sure the handle is ready to be handed out again.
    ClearHandle( &jsonCmdParserVars.handles[ *pHandle ] );
    // The caller's handle shall be invalidated, just in case he tries to use
    // it again after this call.
    *pHandle = IJSONCMDPARSER_INVALID_HANDLE;
    return true;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IJsonCmdParser_Parse( const tIJsonCmdParser_Handle handle, const char *strJson )
{
    // Module should be initialized and started.
    SOFTWARE_EXCEPTION_ASSERT( isInitalized && jsonCmdParserVars.isStarted );
    SOFTWARE_EXCEPTION_ASSERT( NULL != strJson );

    if ( !IsValidHandle( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid handle", "^" );
        return false;
    }
    // Store a pointer to the JSON file string buffer.
    // The user is not allowed to remove this until close is called! This is
    // the contract!
    jsonCmdParserVars.handles[ handle ].jsonStringBuffer = strJson;

    // jsmn parser shall be initialized before using it to parse the file.
    jsmn_init( &jsonCmdParserVars.handles[ handle ].parser );

    // Use the jsmn parser to parse the JSON file.
    jsonCmdParserVars.handles[ handle ].tokensFound = jsmn_parse(
         &jsonCmdParserVars.handles[ handle ].parser                    // the jsmn parser to use
        ,jsonCmdParserVars.handles[ handle ].jsonStringBuffer           // the JSON file string to be parsed
        ,strlen( jsonCmdParserVars.handles[ handle ].jsonStringBuffer ) // the length of JSON file string to be parsed
        ,jsonCmdParserVars.handles[ handle ].tokens                     // (out) the array where the parsed tokens are stored
        ,JSONCMDPARSER_CFG_MAX_NUMBER_OF_JSMN_TOKENS                    // maximum number of tokens that we can store
    );
    // Make a check of the parsed result.
    if ( !IsValidJson( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid JSON", "^" );
        return false;
    }
    return true;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IJsonCmdParser_GetNumberOfNodes( const tIJsonCmdParser_Handle handle, size_t *pNodes )
{
    // Module should be initialized and started.
    SOFTWARE_EXCEPTION_ASSERT( isInitalized && jsonCmdParserVars.isStarted );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pNodes );

    if ( !IsValidHandle( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid handle", "^" );
        return false;
    }
    if ( !IsValidJson( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid JSON", "^" );
        return false;
    }
    // Find the "nodes" entry in the JSON file.
    // It shall be followed by an array, which has the "number of nodes" as size.
    for ( size_t i = 1; i < jsonCmdParserVars.handles[ handle ].tokensFound; ++i )
    {
        if ( JsonTokenStringEquals( handle, i, JSONCMDPARSER_JSON_TOKEN_NODES ) )
        {
            if ( jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].type == JSMN_ARRAY )
            {
                // Simply return the number of entries in the "nodes" array.
                *pNodes = jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].size;
                return true;
            }
        }
    }
    return false;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IJsonCmdParser_GetNumberOfCommands( const tIJsonCmdParser_Handle handle, const size_t node, size_t *pCommands )
{
    // Module should be initialized and started.
    SOFTWARE_EXCEPTION_ASSERT( isInitalized && jsonCmdParserVars.isStarted );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pCommands );

    if ( !IsValidHandle( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid handle", "^" );
        return false;
    }
    if ( !IsValidJson( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid JSON", "^" );
        return false;
    }
    // 1st find the node's token index, this is needed to find the correct
    // commands list.
    size_t nodeTokenIndex;
    if ( !GetNodeTokenIndex( handle, node, &nodeTokenIndex ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "node is not found", "^" );
        return false;
    }
    // Given the node's token index, we shall now be able to find its
    // "commands" list.
    for ( size_t i = nodeTokenIndex; i < jsonCmdParserVars.handles[ handle ].tokensFound; ++i )
    {
        if ( JsonTokenStringEquals( handle, i, JSONCMDPARSER_JSON_TOKEN_COMMANDS ) )
        {
            if ( jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].type == JSMN_ARRAY )
            {
                // Simply return the number of entries in the "commands" array.
                *pCommands = jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].size;
                return true;
            }
            else
            {
                /* The commands string was not followed by an array, hence there is zero commands */
                *pCommands = 0;
                return true;
            }
        }
    }
    return false;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IJsonCmdParser_GetOneCommand( const tIJsonCmdParser_Handle handle, const size_t node, const size_t command, tIJsonCmdParser_Command *pCommand )
{
    // Module should be initialized and started.
    SOFTWARE_EXCEPTION_ASSERT( isInitalized && jsonCmdParserVars.isStarted );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pCommand );

    if ( !IsValidHandle( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid handle", "^" );
        return false;
    }
    if ( !IsValidJson( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid JSON", "^" );
        return false;
    }
    // Clear the provided command, in case there is garbage in it from before.
    ClearCommand( pCommand );

    size_t nodeIndex;
    size_t commandIndex;

    // Determine the indices for this node, and its command.
    if ( !GetCommandTokenIndex( handle, node, command, &nodeIndex, &commandIndex ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "command is not found", "^" );
        return false;
    }
    // Get the node information 1st, common for all commands belonging to this node.
    if ( !GetNodeInformationFromTokenIndex( handle, nodeIndex, pCommand ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "node information incorrect", "^" );
        return false;
    }
    // Get the command information, specific for this command.
    if ( !GetCommandInformationFromTokenIndex( handle, commandIndex, pCommand ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "command information incorrect", "^" );
        return false;
    }
    return true;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IJsonCmdParser_GetProdType( const tIJsonCmdParser_Handle handle, tProductType *pProdType )
{
    // Module should be initialized and started.
    SOFTWARE_EXCEPTION_ASSERT( isInitalized && jsonCmdParserVars.isStarted );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pProdType );

    if ( !IsValidHandle( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid handle", "^" );
        return false;
    }
    if ( !IsValidJson( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid JSON", "^" );
        return false;
    }

    // reset answers
    pProdType->group = IJSONCMDPARSER_PROD_TYPE_UNKOWN;
    pProdType->type = IJSONCMDPARSER_PROD_TYPE_UNKOWN;
    pProdType->variant = IJSONCMDPARSER_PROD_TYPE_UNKOWN;

    // Find the "device type/group/Variant" entry in the JSON file.

    for ( size_t i = 1; i < jsonCmdParserVars.handles[ handle ].tokensFound; ++i )
    {
        if ( JsonTokenStringEquals( handle, i, JSONCMDPARSER_JSON_TOKEN_DEVICE_GROUP ) )
        {
            char *pEnd;
            pProdType->group = (uint8 ) strtol( ( jsonCmdParserVars.handles[ handle ].jsonStringBuffer + jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start ), &pEnd, 10 );

            //if no characters were converted these pointers are equal
            if ( pEnd == jsonCmdParserVars.handles[ handle ].jsonStringBuffer + jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start )
            {
                ILOG( ILOG_LEVEL_ERROR, "failed to parse type value", "^" );
                pProdType->group = IJSONCMDPARSER_PROD_TYPE_UNKOWN;
            }
        }
        else if ( JsonTokenStringEquals( handle, i, JSONCMDPARSER_JSON_TOKEN_DEVICE_TYPE ) )
        {
            char *pEnd;
            pProdType->type = (uint8 ) strtol( ( jsonCmdParserVars.handles[ handle ].jsonStringBuffer + jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start ), &pEnd, 10 );

            //if no characters were converted these pointers are equal
            if ( pEnd == jsonCmdParserVars.handles[ handle ].jsonStringBuffer + jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start )
            {
                ILOG( ILOG_LEVEL_ERROR, "failed to parse type value", "^" );
                pProdType->type = IJSONCMDPARSER_PROD_TYPE_UNKOWN;
            }
        }
        else if ( JsonTokenStringEquals( handle, i, JSONCMDPARSER_JSON_TOKEN_DEVICE_VARIANT ) )
        {
            char *pEnd;
            pProdType->variant = (uint8 ) strtol( ( jsonCmdParserVars.handles[ handle ].jsonStringBuffer + jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start ), &pEnd, 10 );

            //if no characters were converted these pointers are equal
            if ( pEnd == jsonCmdParserVars.handles[ handle ].jsonStringBuffer + jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start )
            {
                ILOG( ILOG_LEVEL_ERROR, "failed to parse type value", "^" );
                pProdType->variant = IJSONCMDPARSER_PROD_TYPE_UNKOWN;
            }
        }
    }
    return true;
}
/*
 ------------------------------------------------------------------------------
    Private functions implementation
 ------------------------------------------------------------------------------
 */
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static void ClearCommand( tIJsonCmdParser_Command *pCommand )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pCommand );

    memset( pCommand->nodeName,    '\0', sizeof( pCommand->nodeName    ) );
    memset( pCommand->payload,     '\0', sizeof( pCommand->payload     ) );
    memset( pCommand->expectedRsp, '\0', sizeof( pCommand->expectedRsp ) );

    pCommand->nodeType        = 0;
    pCommand->msgType         = 0;
    pCommand->subCmd          = 0;
    pCommand->payloadSize     = 0;
    pCommand->expectedRspSize = 0;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static void ClearHandle( tJsonCmdParser_Handle *pHandle )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pHandle );

    memset( pHandle->tokens, '\0', sizeof( pHandle->tokens ) );

    pHandle->inUse            = false;
    pHandle->tokensFound      = 0;
    pHandle->jsonStringBuffer = NULL;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static bool IsValidHandle( const tIJsonCmdParser_Handle handle )
{
    return ( ( IJSONCMDPARSER_INVALID_HANDLE != handle ) && ( handle < JSONCMDPARSER_CFG_MAX_NUMBER_OF_HANDLES ) && ( jsonCmdParserVars.handles[ handle ].inUse ) );
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static bool IsValidJson( const tIJsonCmdParser_Handle handle )
{
    if ( !IsValidHandle( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid handle", "^" );
        return false;
    }
    if ( jsonCmdParserVars.handles[ handle ].tokensFound < 0 )
    {
        ILOG( ILOG_LEVEL_ERROR, "Not a valid JSON; tokens found [%i]", "^%i", jsonCmdParserVars.handles[ handle ].tokensFound );
        return false;
    }
    if ( jsonCmdParserVars.handles[ handle ].tokensFound < 1 )
    {
        ILOG( ILOG_LEVEL_ERROR, "Expected at least 1 token; tokens found [%i]", "^%i", jsonCmdParserVars.handles[ handle ].tokensFound );
        return false;
    }
    if ( JSONCMDPARSER_CFG_MAX_NUMBER_OF_JSMN_TOKENS < jsonCmdParserVars.handles[ handle ].tokensFound )
    {
        ILOG( ILOG_LEVEL_ERROR, "Tokens found [%i] seems to be larger than maximum capacity [%i]", "^%i^%i", jsonCmdParserVars.handles[ handle ].tokensFound, JSONCMDPARSER_CFG_MAX_NUMBER_OF_JSMN_TOKENS );
        return false;
    }
    if ( jsonCmdParserVars.handles[ handle ].tokens[ 0 ].type != JSMN_OBJECT )
    {
        ILOG( ILOG_LEVEL_ERROR, "Expected 1st token to be of Object type, but found [%u]", "^%u", jsonCmdParserVars.handles[ handle ].tokens[ 0 ].type );
        return false;
    }
    return true;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static bool JsonTokenStringEquals( const tIJsonCmdParser_Handle handle, const size_t token, const char *s )
{
    const char      *json = jsonCmdParserVars.handles[ handle ].jsonStringBuffer;
    const jsmntok_t *tok  = &jsonCmdParserVars.handles[ handle ].tokens[ token ];
    if ( ( tok->type == JSMN_STRING ) && ( (int) strlen(s) == tok->end - tok->start ) && ( strncmp( json + tok->start, s, tok->end - tok->start ) == 0 ) )
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
static bool GetNodeTokenIndex( const tIJsonCmdParser_Handle handle, const size_t node, size_t *pNodeTokenIndex )
{
    // Module should be initialized and started.
    SOFTWARE_EXCEPTION_ASSERT( isInitalized && jsonCmdParserVars.isStarted );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pNodeTokenIndex );

    if ( !IsValidHandle( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid handle", "^" );
        return false;
    }
    if ( !IsValidJson( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid JSON", "^" );
        return false;
    }
    // Loop through the entire JSON, searching for "node" until the caller's
    // provided node index is found. There we will have our desired token
    // index.
    size_t currentNode = 0;
    for ( size_t i = 1; i < jsonCmdParserVars.handles[ handle ].tokensFound; ++i )
    {
        // Look for the "nodes" keyword.
        if ( JsonTokenStringEquals( handle, i, JSONCMDPARSER_JSON_TOKEN_NODES ) )
        {
            // This should be followed by an array of nodes.
            if ( jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].type == JSMN_ARRAY )
            {
                size_t next_start = jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start;

                for ( size_t j = ( i + 2 ); j < jsonCmdParserVars.handles[ handle ].tokensFound; ++j )
                {
                    if ( ( jsonCmdParserVars.handles[ handle ].tokens[ j ].type == JSMN_OBJECT ) && ( next_start < jsonCmdParserVars.handles[ handle ].tokens[ j ].start ) )
                    {
                        if ( node <= currentNode )
                        {
                            // We have found our node! Give back the token index.
                            *pNodeTokenIndex = j;
                            return true;
                        }
                        ++currentNode;
                        next_start = jsonCmdParserVars.handles[ handle ].tokens[ j ].end;
                    }
                }
            }
        }
    }
    return false;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static bool GetCommandTokenIndex( const tIJsonCmdParser_Handle handle, const size_t node, const size_t command, size_t *pNodeIndex, size_t *pCommandIndex )
{
    // Module should be initialized and started.
    SOFTWARE_EXCEPTION_ASSERT( isInitalized && jsonCmdParserVars.isStarted );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pNodeIndex );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pCommandIndex );

    if ( !IsValidHandle( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid handle", "^" );
        return false;
    }
    if ( !IsValidJson( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid JSON", "^" );
        return false;
    }
    // We can start to find the correct node index, before trying to find its
    // commands.
    if ( !GetNodeTokenIndex( handle, node, pNodeIndex ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "node is not found", "^" );
        return false;
    }
    // Loop through the JSON (from the appropriate node's token index),
    // searching for "command" until the caller's provided command index is
    // found. There we will have our desired token index.
    size_t currentCommand = 0;
    for ( size_t i = *pNodeIndex; i < jsonCmdParserVars.handles[ handle ].tokensFound; ++i )
    {
        // Look for the "commands" keyword.
        if ( JsonTokenStringEquals( handle, i, JSONCMDPARSER_JSON_TOKEN_COMMANDS ) )
        {
            // This should be followed by an array of commands.
            if ( jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].type == JSMN_ARRAY )
            {
                size_t next_start = jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start;

                for ( size_t j = ( i + 2 ); j < jsonCmdParserVars.handles[ handle ].tokensFound; ++j )
                {
                    if ( ( jsonCmdParserVars.handles[ handle ].tokens[ j ].type == JSMN_OBJECT ) && ( next_start < jsonCmdParserVars.handles[ handle ].tokens[ j ].start ) )
                    {
                        if ( command <= currentCommand )
                        {
                            // We have found our command! Give back the token index.
                            *pCommandIndex = j;
                            return true;
                        }
                        ++currentCommand;
                        next_start = jsonCmdParserVars.handles[ handle ].tokens[ j ].end;
                    }
                }
            }
            else
            {
                // If there is array following the commands, there is nothing to return.
                return false;
            }
        }
    }
    return false;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static bool GetNodeInformationFromTokenIndex( const tIJsonCmdParser_Handle handle, const size_t nodeIndex, tIJsonCmdParser_Command *pCommand )
{
    if ( !IsValidHandle( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid handle", "^" );
        return false;
    }
    if ( !IsValidJson( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid JSON", "^" );
        return false;
    }

    bool foundName = false;
    bool foundType = false;

    for ( size_t i = nodeIndex; i < jsonCmdParserVars.handles[ handle ].tokensFound; ++i )
    {
        // Make sure we don't iterate past the last entry in this node.
        // This is needed if we have optional elements in the node - but is also a sanity "abort".
        if ( jsonCmdParserVars.handles[ handle ].tokens[ nodeIndex ].end < jsonCmdParserVars.handles[ handle ].tokens[ i ].start )
        {
            return ( foundName && foundType );
        }
        // Abort as soon as all elements are found.
        if ( foundName && foundType )
        {
            return true;
        }
        if ( JsonTokenStringEquals( handle, i, JSONCMDPARSER_JSON_TOKEN_NAME ) )
        {
            // Copy the name into the cmd struct's name buffer.
            // Make the buffer is a null-terminated string.
            memset( pCommand->nodeName, '\0', sizeof( pCommand->nodeName ) );
            // Determine the source string's length. Can't use strlen() due to
            // source string ending with quotation mark instead of null-
            // termination.
            size_t stringLength = ( jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].end - jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start );
            // Too long source string will be truncated to fit within
            // destination buffer.
            if ( sizeof( pCommand->nodeName ) <= stringLength )
            {
                stringLength = ( sizeof( pCommand->nodeName ) - 1 );
            }
            memcpy( pCommand->nodeName, jsonCmdParserVars.handles[ handle ].jsonStringBuffer + jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start, stringLength );
            foundName = true;
        }
        if ( JsonTokenStringEquals( handle, i, JSONCMDPARSER_JSON_TOKEN_TYPE ) )
        {
            char *pEnd;
            pCommand->nodeType = strtol( ( jsonCmdParserVars.handles[ handle ].jsonStringBuffer + jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start ), &pEnd, 10 );
            foundType = true;
        }
    }
    // All elements searched for are mandatory.
    return false;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static bool GetCommandInformationFromTokenIndex( const tIJsonCmdParser_Handle handle, const size_t commandIndex, tIJsonCmdParser_Command *pCommand )
{
    if ( !IsValidHandle( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid handle", "^" );
        return false;
    }
    if ( !IsValidJson( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid JSON", "^" );
        return false;
    }

    bool foundMsgType     = false;
    bool foundSubCmd      = false;
    bool foundPayload     = false;
    bool foundExpectedRsp = false;

    for ( size_t i = commandIndex; i < jsonCmdParserVars.handles[ handle ].tokensFound; ++i )
    {
        // Make sure we don't iterate past the last entry in this command.
        // This is needed as we have optional elements in the command - but is also a sanity "abort".
        if ( jsonCmdParserVars.handles[ handle ].tokens[ commandIndex ].end < jsonCmdParserVars.handles[ handle ].tokens[ i ].start )
        {
            return ( foundMsgType && foundSubCmd && foundPayload );
        }
        // Abort as soon as all elements are found.
        if ( foundMsgType && foundSubCmd && foundPayload && foundExpectedRsp )
        {
            return true;
        }
        if ( JsonTokenStringEquals( handle, i, JSONCMDPARSER_JSON_TOKEN_MSG_TYPE ) )
        {
            char *pEnd;
            pCommand->msgType = strtol( ( jsonCmdParserVars.handles[ handle ].jsonStringBuffer + jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start ), &pEnd, 10 );
            foundMsgType = true;
        }
        if ( JsonTokenStringEquals( handle, i, JSONCMDPARSER_JSON_TOKEN_SUB_CMD ) )
        {
            char *pEnd;
            pCommand->subCmd = strtol( ( jsonCmdParserVars.handles[ handle ].jsonStringBuffer + jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start ), &pEnd, 10 );
            foundSubCmd = true;
        }
        // We can't really say if the byte array is valid, as we shall handle a dynamic size here.
        // The payload and expected rsp is required to be a string with a even number of characters (2 per byte, coded as HEX values).
        // E.g. "00" or "DEADBEEF" are ok, where "0" or "100" are invalid, even if we could assume there is a missing 0, but the file shall never be incorrect!
        if ( JsonTokenStringEquals( handle, i, JSONCMDPARSER_JSON_TOKEN_PAYLOAD ) )
        {
            const size_t characters = ( jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].end - jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start );
            if ( characters % 2 )
            {
                ILOG( ILOG_LEVEL_ERROR, "byte array size incorrect", "^" );
            }
            else
            {
                pCommand->payloadSize = ( characters / 2 );
                AsciiTextToByteArray( jsonCmdParserVars.handles[ handle ].jsonStringBuffer + jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start, pCommand->payloadSize, pCommand->payload );
                foundPayload = true;
            }
        }
        if ( JsonTokenStringEquals( handle, i, JSONCMDPARSER_JSON_TOKEN_EXPECTED_RSP ) )
        {
            const size_t characters = ( jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].end - jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start );
            if ( characters % 2 )
            {
                ILOG( ILOG_LEVEL_ERROR, "byte array size incorrect", "^" );
            }
            else
            {
                pCommand->expectedRspSize = ( characters / 2 );
                AsciiTextToByteArray( jsonCmdParserVars.handles[ handle ].jsonStringBuffer + jsonCmdParserVars.handles[ handle ].tokens[ i + 1 ].start, pCommand->expectedRspSize, pCommand->expectedRsp );
                foundExpectedRsp = true;
            }
        }
    }
    // Not all elements searched for are mandatory (expectedRsp is optional).
    if ( foundMsgType && foundSubCmd && foundPayload )
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
static void AsciiTextToByteArray( const char *str, const size_t len, uint8 dest[] )
{
    // Extremely simple conversion of ASCII text into a byte array.
    // sscanf may not cover all potential errors, but the JSON file should be
    // automatically generated and thus the risk of it being badly format is
    // very low (this will probably come back and haunt me).
    const char   *pBuff = str;
    unsigned int  tmp;  // not initialized
    for ( size_t i = 0; i < len; ++i )
    {
        sscanf( pBuff, "%2x", &tmp );
        dest[ i ] = ( uint8 )tmp;
        pBuff += 2;
    }
}
