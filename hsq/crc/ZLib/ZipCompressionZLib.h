/**
 ******************************************************************************
 * @file      IZipCompression.h
 *
 * @brief     Zip Compression interface
 ******************************************************************************
 */
#ifndef IZIPCOMPRESSION_H
#define IZIPCOMPRESSION_H
/*
 ------------------------------------------------------------------------------
    Include files
 ------------------------------------------------------------------------------
 */
#include "RoboticTypes.h"
/*
-------------------------------------------------------------------------------
   Type definitions
-------------------------------------------------------------------------------
*/
#define IZIPCOMPRESSION_FUNCTION_POINTERS   { IZipCompression_Init, IZipCompression_Start, IZipCompression_CompressGZip, }

typedef uint32 tIZipCompression_StreamHandle; /* handle to a gzip stream */
/*
 ------------------------------------------------------------------------------
    Interface functions
 ------------------------------------------------------------------------------
 */
/**
 ******************************************************************************
 * @brief   Initialize module
 * @param   -
 * @returns -
 ******************************************************************************
 */
void IZipCompression_Init();
/**
 ******************************************************************************
 * @brief   Start module
 * @param   -
 * @returns -
 ******************************************************************************
 */
void IZipCompression_Start();

/**
 ******************************************************************************
 * @brief       Compress a buffer
 *              Will be compressed in the gzip format
 * @param[in]   pInData  - pointer to data to be compressed
 * @param[in]   inSize   - size of data to be compressed
 * @param[in]   pOutData -  pointer to buffer that compressed data
 *                          will be placed
 * @param[in]   outSize  - size of out buffer
 * @param[out]  pOutResSize  - result size of compressed data
 * @returns     True if successful.
 ******************************************************************************
 */
bool IZipCompression_CompressGZip( uint8*  pInData, uint32 inSize, uint8*  pOutData, uint32 outSize, uint32* pOutResSize );

/**
 ******************************************************************************
 * @brief       Start decompress of a stream
 *              Support data in gzip format
 * @param[out]  pHandle - pointer to variable that receive stream object handle
 * @returns     true if successful.
 ******************************************************************************
 */
bool IZipCompression_DecompressGZipStreamStart( tIZipCompression_StreamHandle* pHandle );

/**
 ******************************************************************************
 * @brief       Decompress of a stream
 *              Support data in gzip format
 * @param[in]   handle - stream object handle
 * @param[in]   pInData - in buffer filled with compressed data
 * @param[in]   inSize - size of data in in-buffer
 * @param[out]  pInConsumedCnt - pointer to variable that will have number
 *                               of bytes that was consumed
 * @param[in]   pOutData - out buffer to fill with decompressed data
 * @param[in]   outSize - size of out buffer
 * @param[out]  pOutProducedCnt - pointer to variable that will have number
 *                                of bytes added to out-buffer after the call
 * @returns     true if successful.
 ******************************************************************************
 */
bool IZipCompression_DecompressGZipStream( tIZipCompression_StreamHandle handle,
                                           uint8* pInData, uint32 inSize, uint32* pInConsumedCnt,
                                           uint8* pOutData, uint32 outSize, uint32* pOutProducedCnt );

/**
 ******************************************************************************
 * @brief       End decompress of a stream
 * @param[in]   handle - stream object handle
 * @returns     true if successful.
 ******************************************************************************
 */
bool IZipCompression_DecompressGZipStreamEnd( tIZipCompression_StreamHandle handle );


#endif /* IZIPCOMPRESSION_H */
