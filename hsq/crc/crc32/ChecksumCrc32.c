/**
 ******************************************************************************
 * @file      ChecksumCrc32.c
 *
 * @brief     Implementation of CRC32 checksum using zlib crc32 function.
 ******************************************************************************
 */
#include "zlib.h"
#include "IChecksumCrc32.h"
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
uint32 IChecksum32_Crc32( const uint8* pBuffer, const size_t dataLength )
{
    // Simply call the zlib crc32 function.
    uint32 crc = 0;
    crc = crc32( crc, pBuffer, dataLength );
    return crc;
}
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
uint32 IChecksum32_Crc32Append( const uint32 crc, const uint8* pBuffer, const size_t dataLength )
{
    // Simply call the zlib crc32 function.
    return crc32( crc, pBuffer, dataLength );
}
