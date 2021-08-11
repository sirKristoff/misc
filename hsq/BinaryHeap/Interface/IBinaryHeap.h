/**
 ******************************************************************************
 * @file      IBinaryHeap.h
 *
 * @brief     BinaryHeap interface
 ******************************************************************************
 */

#ifndef IBINARYHEAP_H
#define IBINARYHEAP_H

/*
 ------------------------------------------------------------------------------
    Include files
 ------------------------------------------------------------------------------
 */

#include "RoboticTypes.h"


/*
 ------------------------------------------------------------------------------
    Defines
 ------------------------------------------------------------------------------
 */


/*
 ------------------------------------------------------------------------------
    Type definitions
 ------------------------------------------------------------------------------
 */

// comparison function for two elements in the heap
// for a min heap (aka min priority queue), return true if a < b
// for a max heap (aka max priority queue), return true if a > b
typedef bool (*tIBinaryHeap_ComparisonFun)( const void *a, const void *b );

// function to apply to each element in the queue
typedef void (*tIBinaryHeap_ApplyFun)( const void *elem );

typedef struct 
{
    uint16 size;
    uint16 capacity;
    uint8 elementSize;
    void *data;
    tIBinaryHeap_ComparisonFun compare;

} tIBinaryHeap;


/*
 ------------------------------------------------------------------------------
    Interface functions
 ------------------------------------------------------------------------------
 */

/**
 ******************************************************************************
 * @brief   Initialize the module.
 *          NOTE: SOFTWARE EXCEPTION if input params are not valid
 * @param   pHeap
 *          pointer to heap struct
 * @param   pBuffer
 *          user allocated buffer for heap data
 * @param   capacity
 *          max number of elements to store in the heap
 * @param   elementSize
 *          size of element to store in the heap
 * @param   compare
 *          pointer to function for comparing two elements in the heap
 ******************************************************************************
 */
void IBinaryHeap_Init( tIBinaryHeap *pHeap, 
                       void *pBuffer, 
                       uint16 capacity,
                       uint8 elementSize, 
                       tIBinaryHeap_ComparisonFun compare );

/**
 ******************************************************************************
 * @brief   Check if heap is empty.
 *          NOTE: SOFTWARE EXCEPTION if pHeap == NULL
 * @param   pHeap
 *          pointer to heap struct
 * @returns true if empty, false otherwise
 ******************************************************************************
 */
bool IBinaryHeap_IsEmpty( tIBinaryHeap *pHeap );

/**
 ******************************************************************************
 * @brief   Get a pointer to the top element in the heap.
 *          NOTE: SOFTWARE EXCEPTION if pHeap == NULL
 * @param   pHeap
 *          pointer to heap struct
 * @returns pointer to the top element in the heap
 ******************************************************************************
 */
void * IBinaryHeap_Top( tIBinaryHeap *pHeap );

/**
 ******************************************************************************
 * @brief   Remove the top element in the heap.
 *          NOTE: SOFTWARE EXCEPTION if pHeap == NULL
 * @param   pHeap
 *          pointer to heap struct
 ******************************************************************************
 */
void IBinaryHeap_Pop( tIBinaryHeap *pHeap );

/**
 ******************************************************************************
 * @brief   Insert an element in the heap.
 *          NOTE: SOFTWARE EXCEPTION if input params == NULL
 * @param   pHeap
 *          pointer to heap struct
 * @param   pElem
 *          pointer to element to insert
 * @returns true if successful, false otherwise
 ******************************************************************************
 */
bool IBinaryHeap_Insert( tIBinaryHeap *pHeap, const void *pElem );

/**
 ******************************************************************************
 * @brief   Apply a function to each element in the heap. A typical case is to
 *          print out an element.
 *          NOTE: SOFTWARE EXCEPTION if input params == NULL
 * @param   pHeap
 *          pointer to heap struct
 * @param   function
 *          pointer to function to apply
 ******************************************************************************
 */
void IBinaryHeap_Apply( tIBinaryHeap *pHeap, tIBinaryHeap_ApplyFun function );


#endif /* IBINARYHEAP_H */
