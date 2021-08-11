/**
 ******************************************************************************
 * @file      IChecksumCrc32.h
 *
 * @brief     Interface for CRC-32 checksum function.
 ******************************************************************************
 */
#ifndef ICHECKSUMCRC32_H
#define ICHECKSUMCRC32_H
/*
 ------------------------------------------------------------------------------
    Include files
 ------------------------------------------------------------------------------
 */
#include <stddef.h>         // size_t typedef
#include "RoboticTypes.h"   // std types
/*
 ------------------------------------------------------------------------------
    Interface functions
 ------------------------------------------------------------------------------
 */
/**
 ******************************************************************************
 * @brief   Calculate the 32 bit CRC-32 for a byte array using the following CRC
 *          settings:
 *          Name:                       "CRC-32"
 *          Width:                      32 bits
 *          Polynom:                    X^32 + X^26 + X^23 + X^22 + X^16 + X^12 + X^11 + X^10 + X^8 + X^7 + X^5 + X^4 + X^2 + 1
 *          Polynomial representation:  0x04C11DB7 (reversed 0xEDB88320)
 *          Initial value:              0
 *          NOTE! This CRC-32 is the one used in e.g. ZLIB and/or PNG.
 * @param   pBuffer
 *          Start pointer to data for CRC-32 calculation
 * @param   dataLength
 *          Number of bytes in the input buffer
 * @returns CRC-32 value
 ******************************************************************************
 */
uint32 IChecksum32_Crc32( const uint8* pBuffer, const size_t dataLength );
/**
 ******************************************************************************
 * @brief   Same function as IChecksum32_Crc32() but with the added possibility
 *          to provide the initial crc32 value. With this function it is
 *          possible to get support for a "stream-like" feeding of the CRC-32
 *          calculation.
 *          The provided crc32 variable shall be set to 0 at the 1st call, and
 *          set to the result from the previous call, during subsequent calls
 *          to this function.
 * @param   crc
 *          The initial value for this "run" of the CRC-32 calculation. Should
 *          be 0 (1st call) or the output result from a previous call of this
 *          function.
 * @param   pBuffer
 *          Start pointer to data for CRC-32 calculation
 * @param   dataLength
 *          Number of bytes in the input buffer
 * @returns CRC-32 value
 ******************************************************************************
 */
uint32 IChecksum32_Crc32Append( const uint32 crc, const uint8* pBuffer, const size_t dataLength );

#endif // ICHECKSUM32_H
