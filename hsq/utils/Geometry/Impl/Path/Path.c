/**
 ******************************************************************************
 * @file        Path.c
 *
 * @brief       Implementation of IPath.h
 ******************************************************************************
 */

/*
 ------------------------------------------------------------------------------
    Include files
 ------------------------------------------------------------------------------
 */

#include <stdlib.h>

#include "ICoordinate2D.h"
#include "ILine.h"
#include "IPath.h"
#include "Path.h"
#include "ISoftwareException.h"
#include "IRoboticCfg.h"


/*
 ------------------------------------------------------------------------------
    Local definitions
 ------------------------------------------------------------------------------
 */

#define PATH_BORDERS_MAX (100)


 /*
 ------------------------------------------------------------------------------
    Local types
 ------------------------------------------------------------------------------
 */


/*
 ------------------------------------------------------------------------------
    Private data
 ------------------------------------------------------------------------------
 */


/*
-------------------------------------------------------------------------------
    Private function prototypes
-------------------------------------------------------------------------------
*/

static int CreatePathBorders( const tIPath* path, 
                              const tDistance offset, 
                              tILine* bordersOut, 
                              const int bordersSize );

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
tDistance IPath_Length( const tIPath *pPath )
{
    tDistance totalLength = 0;

    if ( IShape_IsEmpty(pPath) )
    {
        return 0;
    }

    for ( int i = 0; i < IShape_Size(pPath) - 1; ++i )
    {
        totalLength += ICoordinate2D_Distance( IShape_At(pPath, i), 
                                               IShape_At(pPath, i + 1) );
    }

    return totalLength;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D IPath_ClosestPoint( const tIPath *pPath, 
                                  const tCoordinate2D *pCoord, 
                                  tILine *pLineOut )
{
    SOFTWARE_EXCEPTION_ASSERT( pCoord   != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pLineOut != NULL );

    tCoordinate2D closest = nullCoord2D;
    sint64        minSqDist  = MAX_sint64;

    if ( IShape_IsEmpty(pPath) )
    {
        return nullCoord2D;
    }

    for ( uint16 i = 0; i < IShape_Size(pPath) - 1; ++i )
    {
        tILine pathSegment = IPath_Segment( pPath, i );

        tCoordinate2D closestOnSegment = ILine_ClosestPoint(&pathSegment, pCoord);
        sint64        sqDistSegment    = ICoordinate2D_SqDistance( closestOnSegment, *pCoord );

        if ( sqDistSegment < minSqDist )
        {
            minSqDist = sqDistSegment;
            closest   = closestOnSegment;
            *pLineOut = pathSegment;
        }
    }

    return closest;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IPath_OnPath( const tIPath *pPath, const tCoordinate2D *pCoord )
{
    SOFTWARE_EXCEPTION_ASSERT( pCoord != NULL );

    if ( IShape_IsEmpty(pPath) )
    {
        return false;
    }

    if ( IShape_Size(pPath) == 1 &&
         ICoordinate2D_IsEqual(IShape_Front(pPath), *pCoord) )
    {
        return true;
    }

    for ( int i = 0; i < IShape_Size(pPath) - 1; ++i )
    {
        tCoordinate2D pathPointA = IShape_At(pPath, i);
        tCoordinate2D pathPointB = IShape_At(pPath, i + 1);

        sint16 crossProduct = ( pCoord->y - pathPointA.y ) * ( pathPointB.x - pathPointA.x ) -
                              ( pCoord->x - pathPointA.x ) * ( pathPointB.y - pathPointA.y );

        // Check if path point pair and point are collinear
        if ( abs(crossProduct) != 0 )
        {
            // Not collinear, hence not on current path segment, continue to next
            continue;
        }

        sint16 dotProduct = ( pCoord->x - pathPointA.x ) * ( pathPointB.x - pathPointA.x ) +
                            ( pCoord->y - pathPointA.y ) * ( pathPointB.y - pathPointA.y );

        // Check if point is between path point pair
        if ( dotProduct < 0 || dotProduct > ICoordinate2D_SqDistance( pathPointB, pathPointA ) )
        {
            // Not between current path point pair, continue to next
            continue;
        }

        return true;
    }

    return false;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tILine IPath_Segment( const tIPath *pPath, uint16 index )
{
    if ( index + 1 >= IShape_Size( pPath ) )
    {
        return nullLine;
    }

    tILine segment;

    segment.A = IShape_At( pPath, index );
    segment.B = IShape_At( pPath, index + 1 );

    return segment;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IPath_WithinPathWidth( const tIPath *pPath, const tCoordinate2D *pCoord )
{
    SOFTWARE_EXCEPTION_ASSERT( pCoord != NULL );

    if ( IShape_IsEmpty( pPath ) )
    {
        return false;
    }

    sint64 sqWidthFromCenter = ( pPath->__private_width__ / 2 ) * ( pPath->__private_width__ / 2 );

    for ( int i = 0; i < IShape_Size( pPath ) - 1; ++i )
    {
        tILine pathSegment = IPath_Segment( pPath, i );
        if ( ILine_SqDistance( &pathSegment, pCoord ) <= sqWidthFromCenter )
        {
            return true;
        }
    }
    return false;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tDistance IPath_DistanceToClosestPathBorder( const tIPath *pPath, const tCoordinate2D *pCoord )
{
    SOFTWARE_EXCEPTION_ASSERT( pCoord != NULL );

    tILine        closestSegment;
    tCoordinate2D closestPoint = IPath_ClosestPoint( pPath, pCoord, &closestSegment );

    if ( ICoordinate2D_IsNull( closestPoint ) )
    {
        return -1;
    }

    tDistance minDistance = ICoordinate2D_Distance( closestPoint, *pCoord );

    /* Return distance to boundary */
    return ((pPath->__private_width__ / 2) - abs(minDistance));
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D IPath_ClosestBorderPoint( const tIPath *pPath, const tCoordinate2D *pCoord )
{
    SOFTWARE_EXCEPTION_ASSERT( pPath != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pCoord != NULL );

    static tILine borders[ PATH_BORDERS_MAX ] STATIC_EXT_VAR;
    tCoordinate2D closestPoint = nullCoord2D;

    if ( IShape_Size( pPath ) < 2 )
    {
        return closestPoint;
    }

    if ( 2 * ( IShape_Size( pPath ) - 1 ) > PATH_BORDERS_MAX )
    {
        // not enough memory allocated for all borders, path is too long
        return closestPoint;
    }

    const tDistance offset = pPath->__private_width__ / 2;
    int bordersCount = CreatePathBorders( pPath, offset, borders, PATH_BORDERS_MAX );

    if ( bordersCount == 0 )
    {
        return closestPoint;
    }

    for ( int i = 0; i < bordersCount; ++i )
    {
        const tILine *border = &borders[ i ];
        const tCoordinate2D closestOnBorder = ILine_ClosestPoint( border, pCoord );
        const sint64 d1 = ICoordinate2D_SqDistance( closestOnBorder, *pCoord );
        const sint64 d2 = ICoordinate2D_SqDistance( closestPoint, *pCoord );

        if ( d1 < d2 )
        {
            closestPoint = closestOnBorder;
        }
    }

    return closestPoint;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D IPath_ClosestBorderIntersection( const tIPath *pPath, const tILine *pLine, const tCoordinate2D *pCoord )
{
    SOFTWARE_EXCEPTION_ASSERT( pPath != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pLine != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pCoord != NULL );

    static tILine borders[ PATH_BORDERS_MAX ] STATIC_EXT_VAR;
    tCoordinate2D closestInter = nullCoord2D;

    if ( ICoordinate2D_IsNull( *pCoord ) )
    {
        return closestInter;
    }

    if ( IShape_Size( pPath ) < 2 )
    {
        return closestInter;
    }

    if ( 2 * ( IShape_Size( pPath ) - 1 ) > PATH_BORDERS_MAX )
    {
        // not enough memory allocated for all borders, path is too long
        return closestInter;
    }

    const tDistance offset = pPath->__private_width__ / 2;
    int bordersCount = CreatePathBorders( pPath, offset, borders, PATH_BORDERS_MAX );

    if ( bordersCount == 0 )
    {
        return closestInter;
    }

    sint64 dist = MAX_sint64;
    for ( int i = 0; i < bordersCount; ++i )
    {
        const tILine *border = &borders[ i ];
        const tCoordinate2D inter = ILine_Intersection( border, pLine );

        if ( !ICoordinate2D_IsNull( inter ) )
        {
            const sint64 currDist = ICoordinate2D_SqDistance( inter, *pCoord );

            if ( currDist < dist )
            {
                closestInter = inter;
                dist = currDist;
            }
        }
    }

    return closestInter;
}


/*
-------------------------------------------------------------------------------
    Implementation of private functions
-------------------------------------------------------------------------------
*/

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static int CreatePathBorders( const tIPath* path, 
                              const tDistance offset, 
                              tILine* bordersOut, 
                              const int bordersSize )
{
    SOFTWARE_EXCEPTION_ASSERT( path != NULL );
    SOFTWARE_EXCEPTION_ASSERT( bordersOut != NULL );

    if ( bordersSize < 2 * ( IShape_Size( path ) - 1 ) )
    {
        return 0;
    }

    int bordersAdded = 0;
    tCoordinate2D prevInter = nullCoord2D;

    // left side of the path, positive offset
    for ( int i = 0; i <= IShape_Size( path ) - 2; ++i )
    {
        tILine borderToAdd;

        if ( i == IShape_Size( path ) - 2 )
        {
            // last border in the path
            borderToAdd.A = IShape_At( path, i );
            borderToAdd.B = IShape_Back( path );
            ILine_Offset( &borderToAdd, offset );

            if ( !ICoordinate2D_IsNull( prevInter ) )
            {
                borderToAdd.A = prevInter;
            }

            bordersOut[ bordersAdded++ ] = borderToAdd;
        }
        else
        {
            borderToAdd       = IPath_Segment( path, i );
            tILine nextBorder = IPath_Segment( path, i + 1 );

            ILine_Offset( &borderToAdd, offset );
            ILine_Offset( &nextBorder, offset );

            const tCoordinate2D inter = ILine_IntersectionInfinite( &borderToAdd, &nextBorder );

            if ( !ICoordinate2D_IsNull( inter ) )
            {
                borderToAdd.B = inter;

                if ( !ICoordinate2D_IsNull( prevInter ) )
                {
                    borderToAdd.A = prevInter;
                }

                prevInter = inter;
            }

            bordersOut[ bordersAdded++ ] = borderToAdd;
        }
    }

    // reset 
    prevInter = nullCoord2D;

    // right side of the path, negative offset
    for ( int i = 0; i <= IShape_Size( path ) - 2; ++i )
    {
        tILine borderToAdd;

        if ( i == IShape_Size( path ) - 2 )
        {
            // last border in the path
            borderToAdd.A = IShape_At( path, i );
            borderToAdd.B = IShape_Back( path );
            ILine_Offset( &borderToAdd, -offset );

            if ( !ICoordinate2D_IsNull( prevInter ) )
            {
                borderToAdd.A = prevInter;
            }

            bordersOut[ bordersAdded++ ] = borderToAdd;
        }
        else
        {
            borderToAdd       = IPath_Segment( path, i );
            tILine nextBorder = IPath_Segment( path, i + 1 );

            ILine_Offset( &borderToAdd, -offset );
            ILine_Offset( &nextBorder, -offset );

            const tCoordinate2D inter = ILine_IntersectionInfinite( &borderToAdd, &nextBorder );

            if ( !ICoordinate2D_IsNull( inter ) )
            {
                borderToAdd.B = inter;

                if ( !ICoordinate2D_IsNull( prevInter ) )
                {
                    borderToAdd.A = prevInter;
                }
                
                prevInter = inter;
            }

            bordersOut[ bordersAdded++ ] = borderToAdd;
        }
    }

    // sanity check for added borders
    if ( bordersAdded != 2 * ( IShape_Size( path ) - 1 ) )
    {
        return 0;
    }

    return bordersAdded;
}
