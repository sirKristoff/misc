/**
 ******************************************************************************
 * @file      Checksum.h
 *
 * @brief     Interface for Checksum.
 ******************************************************************************
 */
#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <stddef.h>
#include <RoboticTypes.h>


/**
 ******************************************************************************
 * @brief   Calculate 8 bit CRC for a byte array using the following
 *          CRC settings:
 *          Name    : "CRC-8"
 *          Width   : 8
 *          Polynom : X^8 + X^5 + X^4 + 1
 *          Init    : 0
 *
 *          Re-entrant: Yes
 *
 * @param   pBuffer
 *          Start pointer to data for CRC calculation
 * @param   dataLength
 *          Number of bytes in buffer
 * @returns CRC value
 ******************************************************************************
 */
uint8 CalcCRC8(const uint8* pBuffer, uint16 dataLength);

/**
 ******************************************************************************
 * @brief   Same functionality as the CalcCRC8, but with this function it is
 *          possible to get support for a "stream-like" feeding of the CRC
 *          calculation. The provided crc variable shall be set to 0 at the 1st
 *          call, but shall be set to the result from the previous call, during
 *          following calls to this function.
 ******************************************************************************
 */
uint8 CalcCRC8Append( uint8 crc, const uint8* pBuffer, size_t dataLength );

/**
 ******************************************************************************
 * @brief   Calculate 8 bit CRC for one byte at the time using the following
 *          CRC settings:
 *          Name    : "CRC-8"
 *          Width   : 8
 *          Polynom : X^8 + X^5 + X^4 + 1
 *          Init    : 0
 *
 *          Re-entrant: Yes
 *
 * @param   seed
 *          seed usually last checksum that you want to be added with byte value
 * @param   byte
 *          Byte of data for CRC calculation
 * @returns CRC value
 ******************************************************************************
 */
uint8 CalcCRC8Byte( uint8 seed, const uint8 byte );


#endif /* CHECKSUM_H */
