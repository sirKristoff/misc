/**
 ******************************************************************************
 * @file      CircularBuffer.h
 *
 * @brief     CircularBuffer module header file
 ******************************************************************************
 */
#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

/*
-------------------------------------------------------------------------------
	Include files
-------------------------------------------------------------------------------
*/
#include "RoboticTypes.h"

/*
-------------------------------------------------------------------------------
	Type definitions
-------------------------------------------------------------------------------
*/
struct tCircularBuffer_t {
    uint32 bufSize;		/**< attribute bufSize */
    /* Max nb. of bytes in buffer. One less than bufSize to be able to distinguish between empty and full buffer. */
    uint32 maxSize;		/**< attribute maxSize */
    uint8* pEnd;		/**< attribute pEnd */
    uint8* pHead;		/**< attribute pHead */
    uint8* pStart;		/**< attribute pStart */
    uint8* pTail;		/**< attribute pTail */
};

/*
-------------------------------------------------------------------------------
	Private data
-------------------------------------------------------------------------------
*/
typedef struct tCircularBuffer_t CircularBuffer; /**< CircularBuffer */

/*
-------------------------------------------------------------------------------
	Public function prototypes
-------------------------------------------------------------------------------
*/

/* Initialize with a buffer to use. */
void CircularBuffer_Init(CircularBuffer* const me, uint8* pBuf, uint32 size);

void CircularBuffer_Clear(CircularBuffer* const me);

/* Remove data from buffer. */
bool CircularBuffer_Erase(CircularBuffer* const me, uint32 size);

uint32 CircularBuffer_GetCount(CircularBuffer* const me);

/* Get data from buffer. */
uint8* CircularBuffer_Peek(CircularBuffer* const me, uint32* pSize);

/**
 ******************************************************************************
 * @brief   Peek with offset, similar to c++ std::deque:at
 *          Retrieves reference to element at an offset from the current Peek
 *          element. Does not modify the buffer, but returned reference can be
 *          used to directly read or write the buffer contents.
 *          Note: If buffer max size is not evenly dividable by the size of
 *                the elements put into it, user must handle the wrap around
 *                if me->pEnd - *ppData < size.
 * @param   me
 *              the circular buffer
 * @param   offset
 *              position in buffer relative Peek (zero gives same ref as Peek)
 * @param   size
 *              size of data intended to be read at offset
 * @param   ppData [out]
 *              will be set to point at requested element in buffer
 * @returns true if buffer contains at least offset+size bytes of data
 ******************************************************************************
 */
bool CircularBuffer_At( CircularBuffer *const me, uint32 offset, uint32 size, uint8** ppData );

/* Get data from buffer. */
bool CircularBuffer_Pop(CircularBuffer* const me, uint8* pData, uint32 size);

/* Copy data to buffer.  */
/* Returns nb. of bytes actually copied (can be less that size if buffer is full). */
uint32 CircularBuffer_Push(CircularBuffer* const me, const uint8* pData, uint32 size);

/* Check if data will fit. */
bool CircularBuffer_CheckPush(CircularBuffer* const me, uint32 size);

#endif /* CIRCULARBUFFER_H */
