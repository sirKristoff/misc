/**
 ******************************************************************************
 * @file      CircularBuffer.c
 *
 * @brief     CircularBuffer module implementation
 ******************************************************************************
 */
/*
-------------------------------------------------------------------------------
	Include files
-------------------------------------------------------------------------------
*/
#include "CircularBuffer.h"
#include "IOs.h"

#include <string.h>

/*
-------------------------------------------------------------------------------
	Defines
-------------------------------------------------------------------------------
*/
#ifndef MIN
#define MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
#endif

/*
-------------------------------------------------------------------------------
	Private function prototypes
-------------------------------------------------------------------------------
*/
/* Returning pointer after stepping forward in circular buffer */
static uint8* StepPointer(CircularBuffer* const me, uint8* pStartPointer, uint32 steps);

/*
-------------------------------------------------------------------------------
	Implementation of interface functions
-------------------------------------------------------------------------------
*/
void CircularBuffer_Init(CircularBuffer* const me, uint8* pBuf, uint32 size) {
    me->bufSize = 0;
    me->maxSize = 0;
    {
        /*#[ operation Init(uint8*,uint32) */
        /* initialize pointers */ 
        me->pStart = pBuf;
        me->pEnd = pBuf + size;
        me->pHead = pBuf;
        me->pTail = pBuf;
        me->bufSize = size;
        me->maxSize = size - 1;
    
        /*#]*/
    }
}

void CircularBuffer_Clear(CircularBuffer* const me) {
    IOs_EnterCritical(); /* begin critical section */
    
    /* reset head and tail pointers */ 
    me->pHead = me->pStart;
    me->pTail = me->pStart;
    
    
    IOs_ExitCritical(); /* end critical section */
}

bool CircularBuffer_Erase(CircularBuffer* const me, uint32 size) {
    uint32 cnt = 0;
    cnt = CircularBuffer_GetCount( me );
    
    if ( size > cnt )
    {
        return false;
    }
    
    IOs_EnterCritical(); /* begin critical section */
    
    /* step tail */
    me->pTail = StepPointer( me, me->pTail, size );
    
    IOs_ExitCritical();  /* end critical section */
    
    return true;
}

uint32 CircularBuffer_GetCount(CircularBuffer* const me) {
    sint32 diff = 0;
    IOs_EnterCritical(); /* begin critical section */
    
    diff = (sint32) ( me->pHead - me->pTail );
    
    IOs_ExitCritical(); /* end critical section */
    
    if ( diff < 0 )
    {
        return (uint32) (diff + me->bufSize);
    }
    else
    {
        return (uint32) diff;
    }
}

uint8* CircularBuffer_Peek(CircularBuffer* const me, uint32* pSize) {
    uint8* pHead = NULL;
    IOs_EnterCritical();  /* begin critical section */
    
    pHead = me->pHead;
    
    if ( pHead < me->pTail )
    {
        /* peek until wrap-around */
        *pSize = me->pEnd - me->pTail;
    }
    else
    {
        *pSize = pHead - me->pTail;
    }
    
    IOs_ExitCritical(); /* end critical section */
    
    return me->pTail;
}

bool CircularBuffer_At( CircularBuffer *const me, uint32 offset, uint32 size, uint8** ppData )
{
    uint8 *pTailOffset = NULL;
    uint32 cnt;

    cnt = CircularBuffer_GetCount( me );

    if ( ( size + offset ) > cnt )
    {
        return false;
    }

    IOs_EnterCritical(); /* begin critical section */

    pTailOffset = me->pTail + offset;
    if ( pTailOffset >= me->pEnd )
    {
        /* wrap-around pointer */
        pTailOffset -= me->bufSize;
    }

    IOs_ExitCritical(); /* end critical section */

    *ppData = pTailOffset;
    return true;
}

bool CircularBuffer_Pop(CircularBuffer* const me, uint8* pData, uint32 size) {
    uint32 copyCntUntilWrapAround = 0;
    uint32 cnt = 0;
    cnt = CircularBuffer_GetCount( me );
    
    if ( size > cnt )
    {
        return false;
    }
    
    IOs_EnterCritical();  /* begin critical section */
    
    /* copy until wrap-around */
    copyCntUntilWrapAround = MIN( (uint32) (me->pEnd - me->pTail), size );
    
    memcpy( pData, me->pTail, copyCntUntilWrapAround );
    
    /* copy remaining "wrap-around data" */
    if ( copyCntUntilWrapAround < size )
    {
        memcpy( pData + copyCntUntilWrapAround, me->pStart, 
            size - copyCntUntilWrapAround);
    }
    
    /* step tail */
    me->pTail = StepPointer( me, me->pTail, size );
    
    IOs_ExitCritical();   /* end critical section */
    return true;
}

uint32 CircularBuffer_Push(CircularBuffer* const me, const uint8* pData, uint32 size) {
    uint32 copyCntUntilWrapAround = 0;
    uint32 cnt = 0;
    cnt = CircularBuffer_GetCount( me );
    
    if ( size > (me->maxSize - cnt) )
    {
        /* data does not fit */
        return 0;
    }
    
    IOs_EnterCritical(); /* begin critical section */
    
    /* copy until wrap-around */
    copyCntUntilWrapAround = MIN( (uint32) (me->pEnd - me->pHead), size );
    
    memcpy( me->pHead, pData, copyCntUntilWrapAround );
    
    /* copy remaining "wrap-around data" */
    if ( copyCntUntilWrapAround < size )
    {
        memcpy( me->pStart, pData + copyCntUntilWrapAround, 
            size - copyCntUntilWrapAround);
    }
    
    /* step head */
    me->pHead = StepPointer( me, me->pHead, size );
    
    IOs_ExitCritical(); /* end critical section */
    /* return nb of bytes copied */
    return size;
}

bool CircularBuffer_CheckPush(CircularBuffer* const me, uint32 size)
{
    uint32 cnt = 0;
    cnt = CircularBuffer_GetCount( me );
    
    if ( size > (me->maxSize - cnt) )
    {
        /* data does not fit */
        return false;
    }
    return true;
}

static uint8* StepPointer(CircularBuffer* const me, uint8* pStartPointer, uint32 steps) {
    /* Check pointer */
    if( pStartPointer >= me->pEnd || pStartPointer < me->pStart )
    {
        return NULL;
    }   
    
    if ( (pStartPointer + steps) < me->pEnd )
    {
        return pStartPointer + steps;
    }                                
    else
    {
        return me->pStart + ( pStartPointer + steps - me->pEnd );
    }                           
}
