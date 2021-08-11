/**
 ******************************************************************************
 * @file      Shape.c
 *
 * @brief     A base container class for series of coordinates
 ******************************************************************************
 */
/*
 ------------------------------------------------------------------------------
 Include files
 ------------------------------------------------------------------------------
 */

#include "ICoordinate2D.h"
#include "IShape.h"
#include "ISoftwareException.h"
#include "Shape.h"


/*
 ------------------------------------------------------------------------------
 Defines
 ------------------------------------------------------------------------------
 */

/*
 ---------------------------------
 Local definitions
 ---------------------------------
 */

/*
 -------------------------------------------------------------------------------
 Private function prototypes
 -------------------------------------------------------------------------------
 */

/*
 -------------------------------------------------------------------------------
 Implementation of interface functions
 -------------------------------------------------------------------------------
 */
/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IShape_Init( tIShape * const pShape, const size_t capacity, void * const pMem )
{
    SOFTWARE_EXCEPTION_ASSERT( pShape != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pMem   != NULL );

    pShape->__private_alloced__  = capacity / sizeof(tCoordinate2D);
    pShape->__private_size__     = 0;
    pShape->__private_vertices__ = pMem;
    pShape->__private_width__    = 0;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IShape_DeInit( tIShape* const pShape )
{
    if ( pShape != NULL )
    {
        pShape->__private_alloced__  = 0;
        pShape->__private_size__     = 0;
        pShape->__private_vertices__ = NULL;
        pShape->__private_width__    = 0;
    }
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IShape_Copy( const tIShape* const pSrc, tIShape* const pDst )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pSrc ) );
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pDst ) );

    bool ret;

    if ( pDst->__private_alloced__ < pSrc->__private_size__ )
    {
        // Not enough memory allocated in destination to make the copy
        ret = false;
    }
    else // size and pointers are ok
    {
        pDst->__private_size__ = pSrc->__private_size__;
        memcpy(
                pDst->__private_vertices__,
                pSrc->__private_vertices__,
                pSrc->__private_size__ * sizeof( tCoordinate2D )
        );
        pDst->__private_width__ = pSrc->__private_width__;
        ret = true;
    }

    return ret;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D IShape_At( const tIShape* const pShape, const size_t n )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pShape ) );

    if ( n >= pShape->__private_size__ )
    {
        return nullCoord2D;
    }
    return pShape->__private_vertices__[ n ];
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D IShape_Front( const tIShape * const pShape )
{
    return IShape_At( pShape, 0 );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D IShape_Back( const tIShape * const pShape )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pShape ) );

    if ( pShape->__private_size__ == 0 )
    {
        return nullCoord2D;
    }
    return pShape->__private_vertices__[ pShape->__private_size__ - 1 ];
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IShape_Clear( tIShape* const pShape )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pShape ) );

    pShape->__private_size__  = 0;
    pShape->__private_width__ = 0;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IShape_IsEmpty( const tIShape* const pShape )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pShape ) );
    return ( 0 == pShape->__private_size__ ) ;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IShape_IsNull( const tIShape * const pShape )
{
    return ( pShape == NULL ) || ( pShape->__private_vertices__ == NULL );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D* IShape_GetElements( const tIShape* const pShape )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pShape ) );
    return pShape->__private_vertices__;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IShape_PushBack( tIShape* const pShape, const tCoordinate2D * const pCoordinate )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pShape ) );
    SOFTWARE_EXCEPTION_ASSERT( pCoordinate != NULL );

    bool retval;

    if ( pShape->__private_size__ >= pShape->__private_alloced__ )
    {
        retval = false;
    }
    else
    {
        pShape->__private_vertices__[ pShape->__private_size__++ ] = *pCoordinate;
        retval = true;
    }

    return retval;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IShape_PopBack( tIShape *pShape )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pShape ) );

    if ( pShape->__private_size__ > 0 )
    {
        pShape->__private_size__--;
    }
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IShape_Replace( const tIShape* const pShape, const uint16 index, const tCoordinate2D* const pCoord )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pShape ) );
    SOFTWARE_EXCEPTION_ASSERT( pCoord != NULL );

    if ( index >= pShape->__private_size__ )
    {
        return false;
    }
    pShape->__private_vertices__[index] = *pCoord;
    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IShape_ShrinkToFit( tIShape* const pShape )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pShape ) );
    pShape->__private_alloced__ = pShape->__private_size__;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
uint16 IShape_Size( const tIShape* const pShape )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pShape ) );
    return pShape->__private_size__;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
uint16 IShape_MaxSize( const tIShape* const pShape )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pShape ) );
    return pShape->__private_alloced__;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IShape_Extend( tIShape * const pShapeDst, const tIShape * const pShapeSrc )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pShapeDst ) );
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pShapeSrc ) );

    if ( IShape_MaxSize(pShapeDst) < IShape_Size(pShapeSrc) + IShape_Size(pShapeDst) )
    {
        return false;
    }

    int dstSize = IShape_Size(pShapeDst);

    for ( int i = 0; i < IShape_Size(pShapeSrc); ++i )
    {
        tCoordinate2D c = IShape_At(pShapeSrc, i);

        if ( !IShape_PushBack(pShapeDst, &c) )
        {
            pShapeDst->__private_size__ = dstSize;
            return false;
        }
    }

    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tDistance IShape_GetLineWidth( const tIShape* const pShape )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pShape ) );
    return pShape->__private_width__;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IShape_SetLineWidth( tIShape* const pShape, const tDistance lineWidth )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pShape ) );
    pShape->__private_width__ = lineWidth;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IShape_BoundingBox( const tIShape * const pShape,
                         tCoordinate2D *pMinMin,
                         tCoordinate2D *pMaxMax )
{
    SOFTWARE_EXCEPTION_ASSERT( pMinMin != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pMaxMax != NULL );

    const uint32 size = IShape_Size(pShape);
    if ( size < 1 )
    {
        return false;
    }

    // Iterate through vertices and construct the minimal covering rectangle
    // as high/low water-marks.
    // Start with 1st coordinate base.
    *pMinMin = IShape_At(pShape, 0);
    *pMaxMax = IShape_At(pShape, 0);

    for ( size_t i = 1; i < size; ++i )
    {
        const tCoordinate2D currPoint = IShape_At(pShape, i);

        // X-axis; left -> right
        // A new leftmost coordinate?
        if ( currPoint.x < pMinMin->x )
        {
            pMinMin->x = currPoint.x;
        }

        // A new rightmost coordinate?
        if ( pMaxMax->x < currPoint.x )
        {
            pMaxMax->x = currPoint.x;
        }

        // Y-axis; bottom -> top
        // A new bottom coordinate?
        if ( currPoint.y < pMinMin->y )
        {
            pMinMin->y = currPoint.y;
        }

        // A new top coordinate?
        if ( pMaxMax->y < currPoint.y )
        {
            pMaxMax->y = currPoint.y;
        }
    }

    return true;
}
