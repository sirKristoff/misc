/**
 ******************************************************************************
 * @file      JsonCatalogueParser.c
 *
 * @brief     Implementation of JSON Catalogue Parser.
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
#include "IJsonCatalogueParser.h"    // public interface
#include "IJsonCatalogueParserCfg.h" // public configuration part
#include "JsonCatalogueParser.h"     // private interface
/*
 ------------------------------------------------------------------------------
    Definitions
 ------------------------------------------------------------------------------
 */
// The following parameters can be fine-tuned in the IJsonCatalogueParserCfg.h.
#ifndef JSONCATALOGUEPARSER_CFG_MAX_NUMBER_OF_HANDLES
#define JSONCATALOGUEPARSER_CFG_MAX_NUMBER_OF_HANDLES     ( 1 )
#endif
#ifndef JSONCATALOGUEPARSER_CFG_MAX_NUMBER_OF_JSMN_TOKENS
#define JSONCATALOGUEPARSER_CFG_MAX_NUMBER_OF_JSMN_TOKENS ( 256 )
#endif

#ifndef JSONCATALOGUEPARSER_CFG_MEMORY_PLACEMENT_FOR_VARIABLES
#define JSONCATALOGUEPARSER_CFG_MEMORY_PLACEMENT_FOR_VARIABLES
#endif

// The following defines are expected strings in the parsed JSON file.
// I.e. the format of the parsed JSON command file is partly defined by those
// strings.
#define JSONCATALOGUEPARSER_JSON_TOKEN_FILES         "files"
#define JSONCATALOGUEPARSER_JSON_TOKEN_NAME          "name"
#define JSONCATALOGUEPARSER_JSON_TOKEN_CHECKSUM      "checksum"
#define JSONCATALOGUEPARSER_JSON_TOKEN_HEIGHT        "height"
#define JSONCATALOGUEPARSER_JSON_TOKEN_WIDTH         "width"
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
    bool         inUse;                                                       /**< true in case this handle is "in-use", false if available                               */
    const char  *jsonStringBuffer;                                            /**< Pointer to the JSON file as a string buffer (provided by the user in the Parse() call) */
    jsmn_parser  parser;                                                      /**< The jsmn parser                                                                        */
    int          tokensFound;                                                 /**< Number of found tokens, when the jsmn parser parsed the JSON file                      */
    jsmntok_t    tokens[ JSONCATALOGUEPARSER_CFG_MAX_NUMBER_OF_JSMN_TOKENS ]; /**< The array containing the parsed JSON tokens                                            */
} tJsonCatalogueParser_Handle;

typedef struct
{
    bool                         isStarted;                                                /**< true in case the module has been started                              */
    tJsonCatalogueParser_Handle  handles[ JSONCATALOGUEPARSER_CFG_MAX_NUMBER_OF_HANDLES ]; /**< The array of possible handles, handed out by calls to Open() function */
} tJsonCatalogueParser_Vars;

/*
 ------------------------------------------------------------------------------
    Private data
 ------------------------------------------------------------------------------
 */
static bool isInitalized = false;                                           /**< true in case the module has been initialized                            */
static tJsonCatalogueParser_Vars jsonCatalogueParserVars JSONCATALOGUEPARSER_CFG_MEMORY_PLACEMENT_FOR_VARIABLES;    /**< The internal data structure with module's variables (RAM control block) */
/*
 ------------------------------------------------------------------------------
    Private function prototypes
 ------------------------------------------------------------------------------
 */
static void ClearFile( tIJsonCatalogueParser_File *pFile );
static void ClearHandle( tJsonCatalogueParser_Handle *pHandle );
static bool IsValidHandle( const tIJsonCatalogueParser_Handle handle );
static bool IsValidJson( const tIJsonCatalogueParser_Handle handle );
static bool JsonTokenStringEquals( const tIJsonCatalogueParser_Handle handle, const size_t token, const char *s );
static bool GetFileTokenIndex( const tIJsonCatalogueParser_Handle handle, const size_t file, size_t *pFileIndex );
static bool GetFileInformationFromTokenIndex( const tIJsonCatalogueParser_Handle handle, const size_t fileIndex, tIJsonCatalogueParser_File *pFile );
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
void IJsonCatalogueParser_Init( void )
{
    if ( !isInitalized )
    {
        isInitalized = true;

        // Initialize private data.
        jsonCatalogueParserVars.isStarted = false;
        for ( size_t i = 0; i < JSONCATALOGUEPARSER_CFG_MAX_NUMBER_OF_HANDLES; ++i )
        {
            ClearHandle( &jsonCatalogueParserVars.handles[ i ] );
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
void IJsonCatalogueParser_Start( void )
{
    if ( !jsonCatalogueParserVars.isStarted )
    {
        jsonCatalogueParserVars.isStarted = true;

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
bool IJsonCatalogueParser_OpenHandle( tIJsonCatalogueParser_Handle *pHandle )
{
    // Module should be initialized and started.
    SOFTWARE_EXCEPTION_ASSERT( isInitalized && jsonCatalogueParserVars.isStarted );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pHandle );

    for ( size_t i = 0; i < JSONCATALOGUEPARSER_CFG_MAX_NUMBER_OF_HANDLES; ++i )
    {
        if ( !jsonCatalogueParserVars.handles[ i ].inUse )
        {
            jsonCatalogueParserVars.handles[ i ].inUse = true;
            *pHandle = i;

            ILOG( ILOG_LEVEL_DEBUG, "Open handle [%u]", "^%u", *pHandle );

            return true;
        }
    }
    ILOG( ILOG_LEVEL_WARNING, "No more handles available!", "^" );
    *pHandle = IJSONCATALOGUEPARSER_INVALID_HANDLE;
    return false;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IJsonCatalogueParser_CloseHandle( tIJsonCatalogueParser_Handle *pHandle )
{
    // Module should be initialized and started.
    SOFTWARE_EXCEPTION_ASSERT( isInitalized && jsonCatalogueParserVars.isStarted );

    if ( !IsValidHandle( *pHandle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid handle", "^" );
        return false;
    }
    // Make sure the handle is ready to be handed out again.
    ClearHandle( &jsonCatalogueParserVars.handles[ *pHandle ] );
    // The caller's handle shall be invalidated, just in case he tries to use
    // it again after this call.
    *pHandle = IJSONCATALOGUEPARSER_INVALID_HANDLE;
    return true;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IJsonCatalogueParser_Parse( const tIJsonCatalogueParser_Handle handle, const char *strJson )
{
    // Module should be initialized and started.
    SOFTWARE_EXCEPTION_ASSERT( isInitalized && jsonCatalogueParserVars.isStarted );
    SOFTWARE_EXCEPTION_ASSERT( NULL != strJson );

    if ( !IsValidHandle( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid handle", "^" );
        return false;
    }
    // Store a pointer to the JSON file string buffer.
    // The user is not allowed to remove this until close is called! This is
    // the contract!
    jsonCatalogueParserVars.handles[ handle ].jsonStringBuffer = strJson;

    // jsmn parser shall be initialized before using it to parse the file.
    jsmn_init( &jsonCatalogueParserVars.handles[ handle ].parser );

    // Use the jsmn parser to parse the JSON file.
    jsonCatalogueParserVars.handles[ handle ].tokensFound = jsmn_parse(
         &jsonCatalogueParserVars.handles[ handle ].parser                    // the jsmn parser to use
        ,jsonCatalogueParserVars.handles[ handle ].jsonStringBuffer           // the JSON file string to be parsed
        ,strlen( jsonCatalogueParserVars.handles[ handle ].jsonStringBuffer ) // the length of JSON file string to be parsed
        ,jsonCatalogueParserVars.handles[ handle ].tokens                     // (out) the array where the parsed tokens are stored
        ,JSONCATALOGUEPARSER_CFG_MAX_NUMBER_OF_JSMN_TOKENS                    // maximum number of tokens that we can store
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
bool IJsonCatalogueParser_GetNumberOfFiles( const tIJsonCatalogueParser_Handle handle, size_t *pFiles )
{
    // Module should be initialized and started.
    SOFTWARE_EXCEPTION_ASSERT( isInitalized && jsonCatalogueParserVars.isStarted );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pFiles );

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
    for ( size_t i = 1; i < jsonCatalogueParserVars.handles[ handle ].tokensFound; ++i )
    {
        if ( JsonTokenStringEquals( handle, i, JSONCATALOGUEPARSER_JSON_TOKEN_FILES ) )
        {
            if ( jsonCatalogueParserVars.handles[ handle ].tokens[ i + 1 ].type == JSMN_ARRAY )
            {
                // Simply return the number of entries in the "nodes" array.
                *pFiles = jsonCatalogueParserVars.handles[ handle ].tokens[ i + 1 ].size;
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
bool IJsonCatalogueParser_GetOneFile( const tIJsonCatalogueParser_Handle handle, const size_t file, tIJsonCatalogueParser_File *pFile )
{
    // Module should be initialized and started.
    SOFTWARE_EXCEPTION_ASSERT( isInitalized && jsonCatalogueParserVars.isStarted );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pFile );

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
    ClearFile( pFile );

    size_t fileIndex;

    // Determine the indices for this node, and its command.
    if ( !GetFileTokenIndex( handle, file, &fileIndex ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "command is not found", "^" );
        return false;
    }
    // Get the command information, specific for this command.
    if ( !GetFileInformationFromTokenIndex( handle, fileIndex, pFile ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "command information incorrect", "^" );
        return false;
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
static void ClearFile( tIJsonCatalogueParser_File *pFile )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pFile );

    memset( pFile->name, '\0', sizeof( pFile->name ) );

    pFile->height = 0;
    pFile->width  = 0;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static void ClearHandle( tJsonCatalogueParser_Handle *pHandle )
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
static bool IsValidHandle( const tIJsonCatalogueParser_Handle handle )
{
    return ( ( IJSONCATALOGUEPARSER_INVALID_HANDLE != handle ) && ( handle < JSONCATALOGUEPARSER_CFG_MAX_NUMBER_OF_HANDLES ) && ( jsonCatalogueParserVars.handles[ handle ].inUse ) );
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static bool IsValidJson( const tIJsonCatalogueParser_Handle handle )
{
    if ( !IsValidHandle( handle ) )
    {
        ILOG( ILOG_LEVEL_ERROR, "not a valid handle", "^" );
        return false;
    }
    if ( jsonCatalogueParserVars.handles[ handle ].tokensFound < 0 )
    {
        ILOG( ILOG_LEVEL_ERROR, "Not a valid JSON; tokens found [%i]", "^%i", jsonCatalogueParserVars.handles[ handle ].tokensFound );
        return false;
    }
    if ( jsonCatalogueParserVars.handles[ handle ].tokensFound < 1 )
    {
        ILOG( ILOG_LEVEL_ERROR, "Expected at least 1 token; tokens found [%i]", "^%i", jsonCatalogueParserVars.handles[ handle ].tokensFound );
        return false;
    }
    if ( JSONCATALOGUEPARSER_CFG_MAX_NUMBER_OF_JSMN_TOKENS < jsonCatalogueParserVars.handles[ handle ].tokensFound )
    {
        ILOG( ILOG_LEVEL_ERROR, "Tokens found [%i] seems to be larger than maximum capacity [%i]", "^%i^%i", jsonCatalogueParserVars.handles[ handle ].tokensFound, JSONCATALOGUEPARSER_CFG_MAX_NUMBER_OF_JSMN_TOKENS );
        return false;
    }
    if ( jsonCatalogueParserVars.handles[ handle ].tokens[ 0 ].type != JSMN_OBJECT )
    {
        ILOG( ILOG_LEVEL_ERROR, "Expected 1st token to be of Object type, but found [%u]", "^%u", jsonCatalogueParserVars.handles[ handle ].tokens[ 0 ].type );
        return false;
    }
    return true;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static bool JsonTokenStringEquals( const tIJsonCatalogueParser_Handle handle, const size_t token, const char *s )
{
    const char      *json = jsonCatalogueParserVars.handles[ handle ].jsonStringBuffer;
    const jsmntok_t *tok  = &jsonCatalogueParserVars.handles[ handle ].tokens[ token ];
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
static bool GetFileTokenIndex( const tIJsonCatalogueParser_Handle handle, const size_t file, size_t *pFileTokenIndex )
{
    // Module should be initialized and started.
    SOFTWARE_EXCEPTION_ASSERT( isInitalized && jsonCatalogueParserVars.isStarted );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pFileTokenIndex );

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
    size_t currentFile = 0;
    for ( size_t i = 1; i < jsonCatalogueParserVars.handles[ handle ].tokensFound; ++i )
    {
        // Look for the "files" keyword.
        if ( JsonTokenStringEquals( handle, i, JSONCATALOGUEPARSER_JSON_TOKEN_FILES ) )
        {
            // This should be followed by an array of files.
            if ( jsonCatalogueParserVars.handles[ handle ].tokens[ i + 1 ].type == JSMN_ARRAY )
            {
                size_t next_start = jsonCatalogueParserVars.handles[ handle ].tokens[ i + 1 ].start;

                for ( size_t j = ( i + 2 ); j < jsonCatalogueParserVars.handles[ handle ].tokensFound; ++j )
                {
                    if ( ( jsonCatalogueParserVars.handles[ handle ].tokens[ j ].type == JSMN_OBJECT ) && ( next_start < jsonCatalogueParserVars.handles[ handle ].tokens[ j ].start ) )
                    {
                        if ( file <= currentFile )
                        {
                            // We have found our node! Give back the token index.
                            *pFileTokenIndex = j;
                            return true;
                        }
                        ++currentFile;
                        next_start = jsonCatalogueParserVars.handles[ handle ].tokens[ j ].end;
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
static bool GetFileInformationFromTokenIndex( const tIJsonCatalogueParser_Handle handle, const size_t fileIndex, tIJsonCatalogueParser_File *pFile )
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

    bool foundName     = false;
    bool foundChecksum = false;
    bool foundHeight   = false;
    bool foundWidth    = false;

    for ( size_t i = fileIndex; i < jsonCatalogueParserVars.handles[ handle ].tokensFound; ++i )
    {
        // Make sure we don't iterate past the last entry in this node.
        // This is needed if we have optional elements in the node - but is also a sanity "abort".
        if ( jsonCatalogueParserVars.handles[ handle ].tokens[ fileIndex ].end < jsonCatalogueParserVars.handles[ handle ].tokens[ i ].start )
        {
            return ( foundName && foundChecksum && foundHeight && foundWidth );
        }
        // Abort as soon as all elements are found.
        if ( foundName && foundChecksum && foundHeight && foundWidth )
        {
            return true;
        }
        if ( JsonTokenStringEquals( handle, i, JSONCATALOGUEPARSER_JSON_TOKEN_NAME ) )
        {
            // Copy the name into the cmd struct's name buffer.
            // Make the buffer is a null-terminated string.
            memset( pFile->name, '\0', sizeof( pFile->name ) );
            // Determine the source string's length. Can't use strlen() due to
            // source string ending with quotation mark instead of null-
            // termination.
            size_t stringLength = ( jsonCatalogueParserVars.handles[ handle ].tokens[ i + 1 ].end - jsonCatalogueParserVars.handles[ handle ].tokens[ i + 1 ].start );
            // Too long source string will be truncated to fit within
            // destination buffer.
            if ( sizeof( pFile->name ) <= stringLength )
            {
                stringLength = ( sizeof( pFile->name ) - 1 );
            }
            memcpy( pFile->name, jsonCatalogueParserVars.handles[ handle ].jsonStringBuffer + jsonCatalogueParserVars.handles[ handle ].tokens[ i + 1 ].start, stringLength );
            foundName = true;
        }
        if ( JsonTokenStringEquals( handle, i, JSONCATALOGUEPARSER_JSON_TOKEN_CHECKSUM ) )
        {
            pFile->checksumSize = ( ( jsonCatalogueParserVars.handles[ handle ].tokens[ i + 1 ].end - jsonCatalogueParserVars.handles[ handle ].tokens[ i + 1 ].start ) / 2 );
            AsciiTextToByteArray( jsonCatalogueParserVars.handles[ handle ].jsonStringBuffer + jsonCatalogueParserVars.handles[ handle ].tokens[ i + 1 ].start, pFile->checksumSize, pFile->checksum );
            foundChecksum = true;
        }
        if ( JsonTokenStringEquals( handle, i, JSONCATALOGUEPARSER_JSON_TOKEN_HEIGHT ) )
        {
            char *pEnd;
            pFile->height = strtol( ( jsonCatalogueParserVars.handles[ handle ].jsonStringBuffer + jsonCatalogueParserVars.handles[ handle ].tokens[ i + 1 ].start ), &pEnd, 10 );
            foundHeight = true;
        }
        if ( JsonTokenStringEquals( handle, i, JSONCATALOGUEPARSER_JSON_TOKEN_WIDTH ) )
        {
            char *pEnd;
            pFile->width = strtol( ( jsonCatalogueParserVars.handles[ handle ].jsonStringBuffer + jsonCatalogueParserVars.handles[ handle ].tokens[ i + 1 ].start ), &pEnd, 10 );
            foundWidth = true;
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
