/**
 ******************************************************************************
 * @file      ZipCompressionZLib.c
 *
 * @brief     ZipCompression module implementation for ZLib
 ******************************************************************************
 */
/*
 ------------------------------------------------------------------------------
 Include files
 ------------------------------------------------------------------------------
 */
#include "ZipCompressionZLib.h"
#include "IZipCompressionCfg.h"

#include "ILog.h"
#include "ISoftwareException.h"

#include "zlib.h"

#include <stdlib.h>

/*
 ------------------------------------------------------------------------------
 Local defines
 ------------------------------------------------------------------------------
 */
#define ZIPCOMPRESSION_GZIP_ENCODING 16 /* windowBit value for GZIP mode */
#define ZIPCOMPRESSION_WINDOW_BITS ( 15 | ZIPCOMPRESSION_GZIP_ENCODING ) /* window bit to use in deflateInit2() and inflateInit2() */

#define ZIPCOMPRESSION_INVALID_HANDLE 0xffffffff
/*
-------------------------------------------------------------------------------
    Local types
-------------------------------------------------------------------------------
*/
typedef struct
{
    tIZipCompression_StreamHandle handle; /* object handle */
    z_stream    zlibStream;     /* object used by zlib */
} tZipCompression_Stream;

typedef struct
{
    tZipCompression_Stream stream[ IZIPCOMPRESSIONCFG_MAX_OPEN_STREAMS ];
} tZipCompression_Vars;

/*
 ------------------------------------------------------------------------------
 Private data
 ------------------------------------------------------------------------------
 */
static tZipCompression_Vars zipCompressionVars;


/*
 ------------------------------------------------------------------------------
 Private function prototypes
 ------------------------------------------------------------------------------
 */
static voidpf ZipMalloc(voidpf opaque, uInt items, uInt size);

static void  ZipFree( voidpf opaque, voidpf address );
/**
 ******************************************************************************
 * @brief   Allocate a stream resource
 * @returns pointer to stream object, NULL if failed
 ******************************************************************************
*/
static tZipCompression_Stream* AllocateStream();
/**
 ******************************************************************************
 * @brief   Get stream resource
 * @param   handle
 *          zip stream handle
 * @returns pointer to file object, NULL if failed
 ******************************************************************************
*/
static tZipCompression_Stream* GetStream( tIZipCompression_StreamHandle handle );

/*
 ------------------------------------------------------------------------------
 Implementation of interface functions
 ------------------------------------------------------------------------------
 */
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IZipCompression_Init( void )
{
    for ( uint32 i = 0; i < IZIPCOMPRESSIONCFG_MAX_OPEN_STREAMS; ++i )
    {
        zipCompressionVars.stream[i].handle = ZIPCOMPRESSION_INVALID_HANDLE;
    }

    ISoftwareException_Init();
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IZipCompression_Start( void )
{
    static bool started = FALSE;
    if (!started)
    {
        started = TRUE;
    }

    ISoftwareException_Start();
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IZipCompression_CompressGZip( uint8*  pInData, uint32 inSize, uint8*  pOutData, uint32 outSize, uint32* pOutResSize )
{
    tZipCompression_Stream* pStream = AllocateStream();
    if ( NULL == pStream )
    {
        return false;
    }

    // init zlib struct
    pStream->zlibStream.zalloc = ZipMalloc;
    pStream->zlibStream.zfree  = ZipFree;

    pStream->zlibStream.opaque = 0;  // not used at the moment

    // compressed data will be compressed as a .gzip file.
    if ( Z_OK != deflateInit2 ( &pStream->zlibStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                             ZIPCOMPRESSION_WINDOW_BITS, 8,
                             Z_DEFAULT_STRATEGY) )
    {
        pStream->handle = ZIPCOMPRESSION_INVALID_HANDLE; /* free stream object */
        return FALSE;
    }

    pStream->zlibStream.avail_in = inSize;
    pStream->zlibStream.next_in  = pInData;

    pStream->zlibStream.avail_out = outSize;
    pStream->zlibStream.next_out = pOutData;
    pStream->zlibStream.total_out = 0;
    if ( outSize < deflateBound( &pStream->zlibStream, inSize ) )
    {
        // not enough out buffer.
        deflateEnd( &pStream->zlibStream );
        pStream->handle = ZIPCOMPRESSION_INVALID_HANDLE;
        return FALSE;
    }


    int zlibRes = deflate( &pStream->zlibStream, Z_FINISH );

    if( Z_STREAM_END != zlibRes )
    {
        deflateEnd( &pStream->zlibStream );
        pStream->handle = ZIPCOMPRESSION_INVALID_HANDLE;
        return FALSE;
    }

    *pOutResSize = pStream->zlibStream.total_out;
    deflateEnd( &pStream->zlibStream );
    pStream->handle = ZIPCOMPRESSION_INVALID_HANDLE; /* free stream object */
    return TRUE;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IZipCompression_DecompressGZipStreamStart( tIZipCompression_StreamHandle* pHandle )
{
    /* set up zlib stream object */
    tZipCompression_Stream* pStream = AllocateStream();
    if ( NULL == pStream )
    {
        return false;
    }
    pStream->zlibStream.zalloc = ZipMalloc;
    pStream->zlibStream.zfree  = ZipFree;
    pStream->zlibStream.opaque = 0;  /* not used at the moment */
    pStream->zlibStream.avail_in = 0;
    pStream->zlibStream.next_in = NULL;
    pStream->zlibStream.avail_out = 0;
    pStream->zlibStream.next_out = NULL;

    bool result = ( inflateInit2( &(pStream->zlibStream), ZIPCOMPRESSION_WINDOW_BITS ) == Z_OK );
    if ( !result )
    {
        pStream->handle = ZIPCOMPRESSION_INVALID_HANDLE; /* free */
    }
    SOFTWARE_EXCEPTION_ASSERT( NULL != pHandle );
    (*pHandle) = pStream->handle;

    return result;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IZipCompression_DecompressGZipStream( tIZipCompression_StreamHandle handle,
                                           uint8* pInData, uint32 inSize, uint32* pInConsumedCnt,
                                           uint8* pOutData, uint32 outSize, uint32* pOutProducedCnt )
{
    SOFTWARE_EXCEPTION_ASSERT( pInConsumedCnt != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pOutProducedCnt != NULL );

    tZipCompression_Stream* pStream = GetStream( handle );
    if ( NULL == pStream )
    {
        return false;
    }

    pStream->zlibStream.avail_in = (uInt) inSize;
    pStream->zlibStream.next_in = (z_const Bytef*) pInData;

    pStream->zlibStream.avail_out = (uInt) outSize;
    pStream->zlibStream.next_out = (Bytef*) pOutData;

    int zipResult = inflate( &(pStream->zlibStream), Z_NO_FLUSH );
    bool result = ( zipResult >= 0 ) && ( zipResult != Z_NEED_DICT ); /* negative error codes are real errors and no support for dictionary */

    /* set out parameters */
    if ( result )
    {
        *pInConsumedCnt = inSize - ( uint32 ) pStream->zlibStream.avail_in;
        *pOutProducedCnt = outSize - ( uint32 ) pStream->zlibStream.avail_out;
    }

    return result;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IZipCompression_DecompressGZipStreamEnd( tIZipCompression_StreamHandle handle )
{
    tZipCompression_Stream* pStream = GetStream( handle );
    if ( NULL == pStream )
    {
        return false;
    }

    bool result = false;
    if ( inflateEnd( &(pStream->zlibStream) ) == Z_OK )
    {
        result = true;
    }
    pStream->handle = ZIPCOMPRESSION_INVALID_HANDLE;

    return result;
}

/*
 ------------------------------------------------------------------------------
         Private functions
 ------------------------------------------------------------------------------
 */

static voidpf ZipMalloc(voidpf opaque, uInt items, uInt size)
{
    return (voidpf) malloc( items*size );
}

static void  ZipFree( voidpf opaque, voidpf address )
{
    free( address );
}

static tZipCompression_Stream* AllocateStream()
{
    /* find first available file object */
    for ( uint32 i = 0; i < IZIPCOMPRESSIONCFG_MAX_OPEN_STREAMS; ++i )
    {
        if ( ZIPCOMPRESSION_INVALID_HANDLE == zipCompressionVars.stream[i].handle )
        {
            /* available set handle to index to indicate that it is in use */
            zipCompressionVars.stream[i].handle = i;
            return &(zipCompressionVars.stream[i]);
        }
    }

    return NULL;
}

static tZipCompression_Stream* GetStream( tIZipCompression_StreamHandle handle )
{
    /* handle is index in list of availble objects */
    if ( handle < IZIPCOMPRESSIONCFG_MAX_OPEN_STREAMS )
    {
        SOFTWARE_EXCEPTION_ASSERT( zipCompressionVars.stream[handle].handle == handle ); /* sanity check */
        return &(zipCompressionVars.stream[handle]);
    }

    return NULL;
}
