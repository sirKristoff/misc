/**
 ******************************************************************************
 * @file      Line.c
 *
 * @brief     Implementation of ILine.h
 ******************************************************************************
 */

/*
 ------------------------------------------------------------------------------
    Include files
 ------------------------------------------------------------------------------
 */

#include <math.h>

#include "ICoordinate2D.h"
#include "ILine.h"
#include "Line.h"
#include "ISoftwareException.h"
#include "BentleyOttmann.h"

#include "IAngle.h"

/*
 ------------------------------------------------------------------------------
    Local definitions
 ------------------------------------------------------------------------------
 */


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

/* */
static tILine NewLineFromPoint( const tILine* const line, const tDistance dist, const tCoordinate2D p );

/** 
 * Check if a colinear point pCoord to a line pLine is on the line, ie the
 * point is between the endpoints (including) defining the line.
 */
static bool IsColinearPointOnLine( const tILine* pLine, const tCoordinate2D* pCoord );


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
tDistance ILine_Length( const tILine *pLine )
{
    SOFTWARE_EXCEPTION_ASSERT( pLine != NULL );
    return ICoordinate2D_Distance(pLine->A, pLine->B);
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D ILine_Project( const tILine *pLine, const tCoordinate2D *pCoord )
{
    SOFTWARE_EXCEPTION_ASSERT( pLine  != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pCoord != NULL );

    if ( ICoordinate2D_IsEqual(pLine->A, pLine->B) )
    {
        return nullCoord2D;
    }

    const double ux = (double) pLine->B.x - pLine->A.x;
    const double uy = (double) pLine->B.y - pLine->A.y;
    const double vx = (double) pCoord->x - pLine->A.x;
    const double vy = (double) pCoord->y - pLine->A.y;

    const double dot = ux * vx + uy * vy;
    const double len = ux * ux + uy * uy;

    if ( dot < 0.0 || dot > len )
    {
        // the projection of pCoord does not fall between the endpoints of pLine
        return nullCoord2D;
    }

    if ( dot == 0.0 )
    {
        // the projection of pCoord falls right on the start point of the line
        return pLine->A;
    }
    else if ( dot == len )
    {
        // the projection of pCoord falls right on the end point of the line
        return pLine->B;
    }

    // the projection of pCoord does falls between the endpoints of pLine
    const double x = pLine->A.x + (dot * ux) / len;
    const double y = pLine->A.y + (dot * uy) / len;

    return (tCoordinate2D) { .x = DoubleToS32(x), .y = DoubleToS32(y) };
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D ILine_ClosestPoint( const tILine *pLine, const tCoordinate2D *pCoord )
{
    SOFTWARE_EXCEPTION_ASSERT( pLine  != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pCoord != NULL );
    
    if ( ICoordinate2D_IsEqual(pLine->A, pLine->B) )
    {
        return nullCoord2D;
    }

    const double ux = (double) pLine->B.x - pLine->A.x;
    const double uy = (double) pLine->B.y - pLine->A.y;
    const double vx = (double) pCoord->x - pLine->A.x;
    const double vy = (double) pCoord->y - pLine->A.y;

    const double dot = ux * vx + uy * vy;
    const double len = ux * ux + uy * uy;

    if ( dot <= 0.0 )
    {
        // the projection of pCoord falls before or right on the start of the line
        return pLine->A;
    }
    else if ( dot >= len )
    {
        // the projection of pCoord falls after or right on the end of the line
        return pLine->B;
    }

    // the projection falls between the endpoints of the line
    const double x = pLine->A.x + (dot * ux) / len;
    const double y = pLine->A.y + (dot * uy) / len;

    return (tCoordinate2D) { .x = DoubleToS32(x), .y = DoubleToS32(y) };
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D ILine_Intersection( const tILine *pLine1, const tILine *pLine2 )
{
    // https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection

    SOFTWARE_EXCEPTION_ASSERT( pLine1 != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pLine2 != NULL );

    const double x1 = (double) pLine1->A.x;
    const double y1 = (double) pLine1->A.y;
    const double x2 = (double) pLine1->B.x;
    const double y2 = (double) pLine1->B.y;
    const double x3 = (double) pLine2->A.x;
    const double y3 = (double) pLine2->A.y;
    const double x4 = (double) pLine2->B.x;
    const double y4 = (double) pLine2->B.y;

    const double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

    if ( denom == 0.0 ) // lines are parallel
    {
        return nullCoord2D;
    }

    const double t = ( (x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4) ) / denom;
    const double u = -( (x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3) ) / denom;

    tCoordinate2D I = nullCoord2D;

    if ( t >= 0.0 && t <= 1.0 && u >= 0.0 && u <= 1.0 )
    {
        // valid intersection
        I.x = DoubleToS32(x1 + t * (x2 - x1));
        I.y = DoubleToS32(y1 + t * (y2 - y1));
    }

    return I;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D ILine_IntersectionInfinite( const tILine *pLine1, const tILine *pLine2 )
{
    // https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection

    SOFTWARE_EXCEPTION_ASSERT( pLine1 != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pLine2 != NULL );

    const double x1 = (double) pLine1->A.x;
    const double y1 = (double) pLine1->A.y;
    const double x2 = (double) pLine1->B.x;
    const double y2 = (double) pLine1->B.y;
    const double x3 = (double) pLine2->A.x;
    const double y3 = (double) pLine2->A.y;
    const double x4 = (double) pLine2->B.x;
    const double y4 = (double) pLine2->B.y;

    const double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

    if ( denom == 0.0 ) // lines are parallel
    {
        return nullCoord2D;
    }

    const double crossLine1 = x1 * y2 - y1 * x2;
    const double crossLine2 = x3 * y4 - y3 * x4;

    const double ix = ( crossLine1 * (x3 - x4) - (x1 - x2) * crossLine2 ) / denom;
    const double iy = ( crossLine1 * (y3 - y4) - (y1 - y2) * crossLine2 ) / denom;

    return (tCoordinate2D) { .x = DoubleToS32(ix), .y = DoubleToS32(iy) };
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool ILine_IsIntersecting( const tILine *pLine1, const tILine *pLine2 )
{
    SOFTWARE_EXCEPTION_ASSERT( pLine1 != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pLine2 != NULL );

    if (    ICoordinate2D_IsEqual(pLine1->A, pLine2->A)
         && ICoordinate2D_IsEqual(pLine1->B, pLine2->B) )
    {
        // a line cannot intersect with a copy of itself
        return false;
    }

    tCoordinate2D_Orientation o1, o2, o3, o4;

    o1 = ICoordinate2D_Orientation(pLine1->A, pLine1->B, pLine2->A);
    o2 = ICoordinate2D_Orientation(pLine1->A, pLine1->B, pLine2->B);
    o3 = ICoordinate2D_Orientation(pLine2->A, pLine2->B, pLine1->A);
    o4 = ICoordinate2D_Orientation(pLine2->A, pLine2->B, pLine1->B);

    // general cases
    if ( o1 != o2 && o3 != o4 )
    {
        return true;
    }

    if (    o1 == ICOORDINATE_COLINEAR 
         && o2 == ICOORDINATE_COLINEAR
         && o3 == ICOORDINATE_COLINEAR
         && o4 == ICOORDINATE_COLINEAR )
    {
        // all points are colinear, no intersection even if they overlap
        // this is consistent with the ILine_Intersection function which
        // returns nullCoord2D in this case
        return false;
    }

    // special cases to handle colinearity, when a point of one line is colinear 
    // with the other line and the point falls on that line, in this case the 
    // lines intersect at that point, no need to handle the fourth point as 
    // that is covered in the if check for colinearity above 
    if ( o1 == ICOORDINATE_COLINEAR && IsColinearPointOnLine(pLine1, &pLine2->A) )
    {
        return true;
    }

    if ( o2 == ICOORDINATE_COLINEAR && IsColinearPointOnLine(pLine1, &pLine2->B) )
    {
        return true;
    }
    
    if ( o3 == ICOORDINATE_COLINEAR && IsColinearPointOnLine(pLine2, &pLine1->A) )
    {
        return true;
    }

    if ( o4 == ICOORDINATE_COLINEAR && IsColinearPointOnLine(pLine2, &pLine1->B) )
    {
        return true;
    }

    return false;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D ILine_Center( const tILine *pLine )
{
    SOFTWARE_EXCEPTION_ASSERT( pLine != NULL );

    /*
     * Hmm, this is an integer division, meaning that if we have 5 / 2, then
     * the result would be 2, whereas if we use doubles, we would have 2.5
     * which will then be rounded to 3. Nothing wrong with this calculation
     * since it is at least consistent, but the question is which is more
     * right? 2 or 3?
     */
    return (tCoordinate2D) { .x = (pLine->A.x + pLine->B.x) / 2,
                             .y = (pLine->A.y + pLine->B.y) / 2 };
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool ILine_Split( const tILine *pLine,
                  const tCoordinate2D *pCoord,
                  tILine *pLinesOut,
                  const int linesOutSize )
{
    SOFTWARE_EXCEPTION_ASSERT( pLine     != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pCoord    != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pLinesOut != NULL );

    if ( linesOutSize < 2 )
    {
        return false;
    }

    tCoordinate2D projection = ILine_Project(pLine, pCoord);

    if ( ICoordinate2D_IsNull(projection) )
    {
        return false;
    }

    if (    ICoordinate2D_IsEqual(projection, pLine->A) 
         || ICoordinate2D_IsEqual(projection, pLine->B) )
    {
        // does not make sense to split a line exactly at its endpoints
        // as one of the split lines will essentially be just a point
        return false;
    }

    pLinesOut[0] = (tILine) { .A = pLine->A, .B = projection };
    pLinesOut[1] = (tILine) { .A = projection, .B = pLine->B };

    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
sint64 ILine_SqDistance( const tILine *pLine, const tCoordinate2D *pCoord )
{
    SOFTWARE_EXCEPTION_ASSERT( pLine  != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pCoord != NULL );

    tCoordinate2D projection = ILine_Project(pLine, pCoord);

    if ( ICoordinate2D_IsNull(projection) )
    {
        return MIN( ICoordinate2D_SqDistance(*pCoord, pLine->A),
                    ICoordinate2D_SqDistance(*pCoord, pLine->B) );
    }

    return ICoordinate2D_SqDistance(projection, *pCoord);
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tDistance ILine_Distance( const tILine *pLine, const tCoordinate2D *pCoord )
{
    const double sqDis = (double) ILine_SqDistance(pLine, pCoord);

    return DoubleToS32( sqrt(sqDis) );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tAngle ILine_Angle( const tILine *pLine )
{
    SOFTWARE_EXCEPTION_ASSERT( pLine != NULL );

    return ICoordinate2D_Angle( pLine->A, pLine->B );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool ILine_SortByDistance( tILine *pLine, const tCoordinate2D *pCoord ) 
{
    SOFTWARE_EXCEPTION_ASSERT( pLine  != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pCoord != NULL );

    if ( ICoordinate2D_SqDistance(*pCoord, pLine->A) >
         ICoordinate2D_SqDistance(*pCoord, pLine->B) )
    {
        tCoordinate2D temp = pLine->A;
        pLine->A = pLine->B;
        pLine->B = temp;
    }

    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void ILine_Offset( tILine *pLine, const tDistance offsetDistance )
{
    SOFTWARE_EXCEPTION_ASSERT( pLine != NULL );

    const tAngle offsetAngle = IAngle_Normalize( ILine_Angle( pLine ) +
                                                 IANGLE_QUARTER_ROTATION );

    pLine->A = ICoordinate2D_Offset( pLine->A, offsetDistance, offsetAngle );
    pLine->B = ICoordinate2D_Offset( pLine->B, offsetDistance, offsetAngle );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D ILine_OffsetIntersection( tILine *pLineA,
                                        tILine *pLineB,
                                        const tDistance offsetDistance )
{
    /* Parallel offset line segment A */
    ILine_Offset( pLineA, offsetDistance );

    /* Parallel offset line segment B */
    ILine_Offset( pLineB, offsetDistance );

    /* Find intersection between offset segments */
    tCoordinate2D intersection = ILine_IntersectionInfinite( pLineA, pLineB );

    if ( ICoordinate2D_IsNull( intersection ) )
    {
        /* Check if line segments share any end point
         * (regarded as intersection if that's the case) */
        if ( ICoordinate2D_IsEqual( pLineA->A, pLineB->A ) ||
             ICoordinate2D_IsEqual( pLineA->A, pLineB->B ) )
        {
            intersection = pLineA->A;
        }
        else if ( ICoordinate2D_IsEqual( pLineA->B, pLineB->A ) ||
                  ICoordinate2D_IsEqual( pLineA->B, pLineB->B ) )
        {
            intersection = pLineA->B;
        }
    }

    return intersection;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool ILine_Intersections( const tILine* pLineSegments,
                          const uint16 linesSegmentsSize,
                          uint16 intersectionsSizeIn,
                          tCoordinate2D* pIntersectionsOut,
                          uint16* pIntersectionsSizeOut )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pLineSegments );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pIntersectionsOut );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pIntersectionsSizeOut );

    return BentleyOttmann_GetLineIntersections( pLineSegments,
                                                linesSegmentsSize,
                                                pIntersectionsOut,
                                                pIntersectionsSizeOut,
                                                intersectionsSizeIn );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool ILine_IsNull( const tILine* const pLine )
{
    if ( pLine == NULL )
    {
        return true;
    }

    return ICoordinate2D_IsEqual( pLine->A, nullLine.A ) &&
           ICoordinate2D_IsEqual( pLine->B, nullLine.B );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void ILine_Extend( tILine* const pLine, const tDistance distance )
{
    SOFTWARE_EXCEPTION_ASSERT( pLine != NULL );
    
    if ( distance == 0 )
    {
        return;
    }

    tILine lineToUse = *pLine;

    if ( distance < 0 )
    {
        lineToUse.A = pLine->B;
        lineToUse.B = pLine->A;
    }

    const tILine newLine = NewLineFromPoint( &lineToUse, abs( distance ), lineToUse.B );

    if ( ILine_IsNull( &newLine ) )
    {
        return;
    }

    if ( distance < 0 )
    {
        pLine->A = newLine.B;
    }
    else
    {
        pLine->B = newLine.B;
    }
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tILine ILine_FromPoint( const tILine* const pLine, const tDistance length, const tCoordinate2D p )
{
    SOFTWARE_EXCEPTION_ASSERT( pLine != NULL );

    if ( length <= 0 )
    {
        return nullLine;
    }

    return NewLineFromPoint( pLine, length, p );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool ILine_IsPointInFront( const tILine* const pLine, const tCoordinate2D p )
{
    SOFTWARE_EXCEPTION_ASSERT( pLine != NULL );

    /*
     * We have:
     *                       ^ B
     *                 p \   |
     *                    \  |
     *                     \ | 
     *                 .....\|.......
     *                        A
     * 
     *                 ->   ->
     *                 AB * PB > 0
     * 
     *                                      ->   ->
     * meaning that when the dot product of AB * PB > 0, then we say that
     * p is on the correct side of the dotted line at A.
     */
    tCoordinate2D u = { .x = pLine->B.x - pLine->A.x, .y = pLine->B.y - pLine->A.y };
    tCoordinate2D v = { .x = p.x - pLine->A.x, .y = p.y - pLine->A.y };

    const sint64 dot = ( u.x * v.x ) + ( u.y * v.y );

    return dot >= 0;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool ILine_IsPointOnLine( const tILine* const pLine, const tCoordinate2D p )
{
    SOFTWARE_EXCEPTION_ASSERT( pLine != NULL );
    
    tDistance dist = ILine_Distance( pLine, &p );

    /* Regard point as being on edge when abs(distance) == 1 to account for roundoff error */
    if ( dist == 0 || abs( dist ) == 1 )
    {
        return true;
    }

    return false;
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
tILine NewLineFromPoint( const tILine* const line, const tDistance dist, const tCoordinate2D p )
{
    SOFTWARE_EXCEPTION_ASSERT( line != NULL );

    const tCoordinate2D vec = { .x = line->B.x - line->A.x, .y = line->B.y - line->A.y };

    if ( ICoordinate2D_IsEqual( vec, zeroCoord2D ) )
    {
        return nullLine;
    }

    const double x = vec.x / sqrt( ((double)vec.x * vec.x) + ((double)vec.y * vec.y) );
    const double y = vec.y / sqrt( ((double)vec.x * vec.x) + ((double)vec.y * vec.y) );

    tILine newLine;
    newLine.A = p;
    newLine.B.x = p.x + DoubleToS32( x * dist );
    newLine.B.y = p.y + DoubleToS32( y * dist );

    return newLine;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static bool IsColinearPointOnLine( const tILine* pLine, const tCoordinate2D* pCoord )
{
    SOFTWARE_EXCEPTION_ASSERT( pLine  != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pCoord != NULL );

    if ( pLine->A.x != pLine->B.x )
    {
        if ( pLine->A.x <= pCoord->x && pCoord->x <= pLine->B.x )
        {
            return true;
        }

        if ( pLine->A.x >= pCoord->x && pCoord->x >= pLine->B.x )
        {
            return true;
        }
    }
    else
    {
        if ( pLine->A.y <= pCoord->y && pCoord->y <= pLine->B.y )
        {
            return true;
        }

        if ( pLine->A.y >= pCoord->y && pCoord->y >= pLine->B.y )
        {
            return true;
        }
    }
    
    return false;
}
