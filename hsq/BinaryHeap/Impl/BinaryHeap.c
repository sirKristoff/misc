/**
 ******************************************************************************
 * @file      BinaryHeap.c
 *
 * @brief     Implementation file for BinaryHeap
 ******************************************************************************
 */

/*
 ------------------------------------------------------------------------------
 Include files
 ------------------------------------------------------------------------------
 */

#include "BinaryHeap.h"
#include "IBinaryHeap.h"

#include "ISoftwareException.h"


/*
 ------------------------------------------------------------------------------
 Private function prototypes
 ------------------------------------------------------------------------------
 */

/* Index of left child. */
static inline uint16 Left( int idx );

/* Index of right child. */
static inline uint16 Right( int idx );

/* Index of parent. */
static inline uint16 Parent( int idx );

/* Return first position of element in the heap. */
static inline void * At( tIBinaryHeap *pHeap, uint16 idx );

/* Swap elements at indices i and j in the heap. */
static inline void Swap( tIBinaryHeap *pHeap, uint16 i, uint16 j );


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
void IBinaryHeap_Init( tIBinaryHeap *pHeap, 
                       void *pBuffer, 
                       uint16 capacity,
                       uint8 elementSize,
                       tIBinaryHeap_ComparisonFun compare )
{
    if ( pHeap == NULL || pBuffer == NULL || capacity == 0 || elementSize == 0 || compare == NULL )
    {
        SOFTWARE_EXCEPTION();
    }

    pHeap->size = 0;
    pHeap->capacity = capacity;
    pHeap->elementSize = elementSize;
    pHeap->data = pBuffer;
    pHeap->compare = compare;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IBinaryHeap_IsEmpty( tIBinaryHeap *pHeap )
{
    SOFTWARE_EXCEPTION_ASSERT( pHeap != NULL );

    return pHeap->size == 0;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void * IBinaryHeap_Top( tIBinaryHeap *pHeap )
{
    SOFTWARE_EXCEPTION_ASSERT( pHeap != NULL );

    return pHeap->data;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IBinaryHeap_Pop( tIBinaryHeap *pHeap )
{
    SOFTWARE_EXCEPTION_ASSERT( pHeap != NULL );

    memcpy( At(pHeap, 0), At(pHeap, pHeap->size - 1), pHeap->elementSize );
    --pHeap->size;

    uint16 idx = 0;
    bool isDone = false;

    while ( !isDone )
    {
        uint16 minidx = idx;

        if ( Left(idx) < pHeap->size &&
             pHeap->compare( At(pHeap, Left( idx )), At(pHeap, minidx )) )
        {
            minidx = Left(idx);
        }

        if ( Right(idx) < pHeap->size &&
                pHeap->compare( At(pHeap, Right(idx) ), At(pHeap, minidx) ) )
        {
            minidx = Right(idx);
        }

        if ( minidx != idx )
        {
            Swap(pHeap, minidx, idx);
            idx = minidx;
        }
        else
        {
            isDone = true;
        }
    }
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IBinaryHeap_Insert( tIBinaryHeap *pHeap, const void *pElem )
{
    SOFTWARE_EXCEPTION_ASSERT( pHeap != NULL && pHeap->data != NULL && pElem != NULL );

    if ( pHeap->size >= pHeap->capacity )
    {
        return false;
    }

    int idx = pHeap->size++;
    memcpy( At(pHeap, idx), pElem, pHeap->elementSize );

    while ( idx != 0 &&
            pHeap->compare( At(pHeap, idx), At(pHeap, Parent(idx)) ) )
    {
        Swap(pHeap, idx, Parent(idx));
        idx = Parent(idx);
    }

    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IBinaryHeap_Apply( tIBinaryHeap *pHeap, tIBinaryHeap_ApplyFun function )
{
    SOFTWARE_EXCEPTION_ASSERT( pHeap != NULL && pHeap->data != NULL && function != NULL );

    for ( int idx = 0; idx < pHeap->size; ++idx )
    {
        function( At(pHeap, idx) );
    }
}

/*
 ------------------------------------------------------------------------------
 Public functions
 ------------------------------------------------------------------------------
 */


/*
 ------------------------------------------------------------------------------
 Private functions
 ------------------------------------------------------------------------------
 */

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static inline uint16 Left( int idx )
{
    return ( 2 * idx ) + 1;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static inline uint16 Right( int idx )
{
    return ( 2 * idx ) + 2;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static inline uint16 Parent( int idx )
{
    return ( idx - 1 ) / 2;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static inline void * At( tIBinaryHeap *pHeap, uint16 idx )
{
    return (void *) ( (uint8 *) pHeap->data + idx * pHeap->elementSize );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static inline void Swap( tIBinaryHeap *pHeap, uint16 i, uint16 j )
{
    for ( int byte = 0; byte < pHeap->elementSize; ++byte )
    {
        uint8 temp = *( (uint8* ) At(pHeap, i) + byte );
        *( (uint8* ) At(pHeap, i) + byte ) = *( (uint8* ) At(pHeap, j) + byte );
        *( (uint8* ) At(pHeap, j) + byte ) = temp;
    }
}

