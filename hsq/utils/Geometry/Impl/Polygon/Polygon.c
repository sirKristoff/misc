/**
 ******************************************************************************
 * @file      Polygon.c
 *
 * @brief     Implementation of IPolygon.h
 ******************************************************************************
 */

/*
 -------------------------
 Include files
 -------------------------
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "ICoordinate2D.h"
#include "ILine.h"
#include "IPolygon.h"
#include "Polygon.h"
#include "ISoftwareException.h"
#include "IAngle.h"
#include "BentleyOttmann.h"
#include "IRoboticCfg.h"


/*
-------------------------
   Defines
-------------------------
*/

#define POLYGON_ARR_SIZE ( 100 )
#define OFFSET_LINES_MAX_SIZE ( 2500 )
#define OFFSET_INTERSECTION_MAX_SIZE ( 100 )
#define OFFSET_EXTENSION ( 100 )  /* mm */
#define OFFSET_START_POINT_EDGE_LEN ( 500 )  /* mm */

/*
 -------------------------
 Type definitions
 -------------------------
 */

typedef enum
{
    POLYGON_SELECTION_X = 0,
    POLYGON_SELECTION_Y
} tPolygon_Selection;

typedef enum
{
    POLYGON_TRANSFORM_DECREASE = 0,
    POLYGON_TRANSFORM_INCREASE
} tPolygon_Transform_Direction;

typedef enum
{
    POLYGON_TRANSFORM_A = 0,
    POLYGON_TRANSFORM_B
} tPolygon_Transform_AB;

typedef enum
{
    POLYGON_TRANSFORM_X = 0,
    POLYGON_TRANSFORM_Y
} tPolygon_Transform_XY;

typedef struct
{
    int refEdgeIdx;
    int crossingEdgeIdx;
    sint64 dist;
    tCoordinate2D inter;
} tPolygon_OffsetIntersection;

typedef struct
{
    int size;
    tPolygon_OffsetIntersection inters[ OFFSET_LINES_MAX_SIZE ];
} tPolygon_OffsetIntersections;

typedef struct
{
    bool processed;
    tILine line;
} tPolygon_OffsetLine;

typedef struct
{
    int size;
    tPolygon_OffsetLine lines[ OFFSET_LINES_MAX_SIZE ];
} tPolygon_OffsetLines;

/*
 ---------------------------------
    Private data
 ---------------------------------
 */

static tPolygon_OffsetIntersections mOffsetIntersections STATIC_EXT_VAR;
static tPolygon_OffsetLines mOffsetLines STATIC_EXT_VAR;

/*
-------------------------------------------------------------------------------
    Private function prototypes
-------------------------------------------------------------------------------
*/

/*
 ******************************************************************************
 * Calculate the centroid (geometric center) of a polygon.
 * 
 * The centroid is the arithmetic mean position of all points in the figure.
 *
 * See https://en.wikipedia.org/wiki/Centroid
 ******************************************************************************
 */
static tCoordinate2D CalculateCentroid( const tIPolygon *pPoly );

/*
 ******************************************************************************
 * Calculate the winding number for a point and a polygon.
 *
 * Imagine a ray starting at the point and going infinitely in any direction.
 * The winding number is the number of edges of the polygon  that cross this
 * ray in a 'left' direction minus the number of edges that cross it in a
 * 'right' direction.
 *
 * If the point is outside of the polygon this sum will always be zero.
 *
 * For simple non-intersecting polygons the value will be +-1 if the point
 * is inside the polygon. Values larger than +-1 indicate that the point is
 * in a self-intersecting part of the polygon.
 *
 * See https://en.wikipedia.org/wiki/Winding_number
 * See http://geomalgorithms.com/a03-_inclusion.html
 ******************************************************************************
 */
static sint16 WindingNumber( const tIPolygon *pPoly, const tCoordinate2D *pPoint );

/*
 ******************************************************************************
 * Calculate the area of a polygon.
 ******************************************************************************
 */
static sint64 SignedArea( const tIPolygon *pPoly );

/*
 ******************************************************************************
 * Find the intersections of a line with a polygon.
 ******************************************************************************
 */
static int LineIntersections( const tIPolygon *pPoly, 
                              const tILine *pLine,
                              tCoordinate2D *pIntersectionsOut, 
                              int alloced );

/*
 ******************************************************************************
 * Find point of intersection between two lines.
 * Returns nullCoord2D if no intersection point exists.
 * Note that this is very similar to ILine_Intersection,
 * but intersection check includes both end points of a line,
 * and l1 can be of infinite length
 ******************************************************************************
 */
static tCoordinate2D GetLineIntersection( const tILine l1, const tILine l2);

/*
 ******************************************************************************
 * Get coordinate with minimum X/Y value from array of coordinates
 ******************************************************************************
 */
static tCoordinate2D getMinCoordinate( tPolygon_Selection selection,
                                       tCoordinate2D *coordinates,
                                       int coordinatesSize,
                                       int *index );

/*
 ******************************************************************************
 * Check if a point is located on a polygon edge (line segment)
 ******************************************************************************
 */
static bool IsPointOnEdge( tCoordinate2D point, tILine edge );

/*
 ******************************************************************************
 * Check if a path edge is intersecting
 ******************************************************************************
 */
static bool IsPathEdgeIntersecting( const tIPolygon *pPoly, const tILine *pPathEdge );

/*
 ******************************************************************************
 * Transform path edge
 ******************************************************************************
 */
static void TransformPathEdge( tPolygon_Transform_Direction direction,
                               tPolygon_Transform_AB ab,
                               tPolygon_Transform_XY xy,
                               tILine *pPathEdge,
                               const tDistance width );

/* 
 * Find the intersection point for two offsetted edges. If the intersection is
 * further away than the allowed offset, then make a line between the two edges
 * so that the line runs at the offset distance. In case the intersection is 
 * closer than the offset distance, only the line A point is valid and B point
 * is null coordinate. Otherwise, if the intersection is further away than the
 * offset, both points of the line area valid.
 */
static tILine OffsetIntersectionForExpandingEdges( const tILine* const inEdge,
                                                   const tILine* const outEdge,
                                                   const tDistance offset );

/* Compare function for qsort for offset polygon. */
static int QSortCompareForOffsetPolygon( const void* a, const void* b );


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
bool IPolygon_IsWithin( const tIPolygon *pPoly, const tCoordinate2D *pCoord )
{
    SOFTWARE_EXCEPTION_ASSERT( pCoord != NULL );

    int windingNumber = 0;

    if ( !IShape_IsEmpty(pPoly) )
    {
        windingNumber = WindingNumber(pPoly, pCoord);
    }

    return (windingNumber != 0); // Outside if zero
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IPolygon_IsIntersecting( const tIPolygon *pPoly1, const tIPolygon *pPoly2 )
{
    if ( IShape_Size(pPoly1) < 3 || IShape_Size(pPoly2) < 3 )
    {
        return false;
    }

    if ( pPoly1 == pPoly2 )
    {
        // a polygon cannot intersect with a copy of itself
        return false;
    }

    for ( int i = 0; i < IShape_Size(pPoly1); ++i )
    {
        const tILine p1_edge = IPolygon_Edge(pPoly1, i);

        for ( int j = 0; j < IShape_Size(pPoly2); ++j )
        {
            const tILine p2_edge = IPolygon_Edge(pPoly2, j);

            if ( ILine_IsIntersecting(&p1_edge, &p2_edge) )
            {
                return true;
            }
        }
    }

    return false;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IPolygon_IsLineIntersecting( const tIPolygon *pPoly, const tILine *pLine )
{
    if ( IShape_Size(pPoly) < 3 )
    {
        return false;
    }

    for ( int i = 0; i < IShape_Size(pPoly); ++i )
    {
        const tILine polygon_edge = IPolygon_Edge(pPoly, i);

        if ( ILine_IsIntersecting(&polygon_edge, pLine) )
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
        N
       )|(
     )  |  (
 W-)----O----(-E
     )  |  (
       )|(
        S
Function below transforms the line in different directions with half line width
as in a compass.
*/
bool IPolygon_IsPathWidthIntersecting( const tIPolygon *pPoly, const tIPath *pPath )
{
    if ( IShape_Size( pPoly ) < 3 || IShape_Size( pPath ) < 2 )
    {
        return false;
    }

    const tDistance halfLineWidth = ( ( pPath->__private_width__ + 1 ) / 2 );

    // Mathematical line
    {
        tILine path_edge    = { .A = IShape_Front( pPath ) };   // Front of path

        for ( size_t path_index = 1; path_index < IShape_Size( pPath ); ++path_index )
        {
            path_edge.B = IShape_At( pPath, path_index );

            if ( IsPathEdgeIntersecting( pPoly, &path_edge ) )
            {
                return true;
            }
            path_edge.A = path_edge.B;
        }
    }

    // Transform line NorthWest with halfLineWidth
    {
        tILine path_edge    = { .A = IShape_Front( pPath ) };   // Front of path
        TransformPathEdge( POLYGON_TRANSFORM_DECREASE, POLYGON_TRANSFORM_A, POLYGON_TRANSFORM_Y,
                           &path_edge, halfLineWidth ); // a north
        TransformPathEdge( POLYGON_TRANSFORM_DECREASE, POLYGON_TRANSFORM_A, POLYGON_TRANSFORM_X,
                           &path_edge, halfLineWidth ); // a west

        for ( size_t path_index = 1; path_index < IShape_Size( pPath ); ++path_index )
        {
            path_edge.B = IShape_At( pPath, path_index );
            TransformPathEdge( POLYGON_TRANSFORM_DECREASE, POLYGON_TRANSFORM_B, POLYGON_TRANSFORM_Y,
                               &path_edge, halfLineWidth ); // b north
            TransformPathEdge( POLYGON_TRANSFORM_DECREASE, POLYGON_TRANSFORM_B, POLYGON_TRANSFORM_X,
                               &path_edge, halfLineWidth ); // b west

            if ( IsPathEdgeIntersecting( pPoly, &path_edge ) )
            {
                return true;
            }
            path_edge.A = path_edge.B;
        }
    }

    // Transform line SouthEast with halfLineWidth
    {
        tILine path_edge    = { .A = IShape_Front( pPath ) };   // Front of path
        TransformPathEdge( POLYGON_TRANSFORM_INCREASE, POLYGON_TRANSFORM_A, POLYGON_TRANSFORM_Y,
                           &path_edge, halfLineWidth ); // a south
        TransformPathEdge( POLYGON_TRANSFORM_INCREASE, POLYGON_TRANSFORM_A, POLYGON_TRANSFORM_X,
                           &path_edge, halfLineWidth ); // a east

        for ( size_t path_index = 1; path_index < IShape_Size( pPath ); ++path_index )
        {
            path_edge.B = IShape_At( pPath, path_index );
            TransformPathEdge( POLYGON_TRANSFORM_INCREASE, POLYGON_TRANSFORM_B, POLYGON_TRANSFORM_Y,
                               &path_edge, halfLineWidth ); // b south
            TransformPathEdge( POLYGON_TRANSFORM_INCREASE, POLYGON_TRANSFORM_B, POLYGON_TRANSFORM_X,
                               &path_edge, halfLineWidth ); // b east

            if ( IsPathEdgeIntersecting( pPoly, &path_edge ) )
            {
                return true;
            }
            path_edge.A = path_edge.B;
        }
    }

    // Transform line NorthEast with halfLineWidth
    {
        tILine path_edge    = { .A = IShape_Front( pPath ) };   // Front of path
        TransformPathEdge( POLYGON_TRANSFORM_DECREASE, POLYGON_TRANSFORM_A, POLYGON_TRANSFORM_Y,
                           &path_edge, halfLineWidth ); // a north
        TransformPathEdge( POLYGON_TRANSFORM_INCREASE, POLYGON_TRANSFORM_A, POLYGON_TRANSFORM_X,
                           &path_edge, halfLineWidth ); // a east

        for ( size_t path_index = 1; path_index < IShape_Size( pPath ); ++path_index )
        {
            path_edge.B = IShape_At( pPath, path_index );
            TransformPathEdge( POLYGON_TRANSFORM_DECREASE, POLYGON_TRANSFORM_B, POLYGON_TRANSFORM_Y,
                               &path_edge, halfLineWidth ); // b north
            TransformPathEdge( POLYGON_TRANSFORM_INCREASE, POLYGON_TRANSFORM_B, POLYGON_TRANSFORM_X,
                               &path_edge, halfLineWidth ); // b east

            if ( IsPathEdgeIntersecting( pPoly, &path_edge ) )
            {
                return true;
            }
            path_edge.A = path_edge.B;
        }
    }

    // Transform line SouthWest with halfLineWidth
    {
        tILine path_edge    = { .A = IShape_Front( pPath ) };   // Front of path
        TransformPathEdge( POLYGON_TRANSFORM_INCREASE, POLYGON_TRANSFORM_A, POLYGON_TRANSFORM_Y,
                                           &path_edge, halfLineWidth ); // a south
        TransformPathEdge( POLYGON_TRANSFORM_DECREASE, POLYGON_TRANSFORM_A, POLYGON_TRANSFORM_X,
                                                   &path_edge, halfLineWidth ); // a west

        for ( size_t path_index = 1; path_index < IShape_Size( pPath ); ++path_index )
        {
            path_edge.B = IShape_At( pPath, path_index );
            TransformPathEdge( POLYGON_TRANSFORM_INCREASE, POLYGON_TRANSFORM_B, POLYGON_TRANSFORM_Y,
                               &path_edge, halfLineWidth ); // b south
            TransformPathEdge( POLYGON_TRANSFORM_DECREASE, POLYGON_TRANSFORM_B, POLYGON_TRANSFORM_X,
                               &path_edge, halfLineWidth ); // b west

            if ( IsPathEdgeIntersecting( pPoly, &path_edge ) )
            {
                return true;
            }
            path_edge.A = path_edge.B;
        }
    }

    return false;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
sint64 IPolygon_Area( const tIPolygon *pPoly )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pPoly ) );

    sint64 area = SignedArea(pPoly);
    return area < 0 ? -area : area;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tDistance IPolygon_Perimeter( const tIPolygon *pPoly )
{
    tDistance circumference = 0;

    if ( IShape_IsEmpty(pPoly) || IShape_Size(pPoly) < 3 )
    {
        return circumference;
    }

    for ( int i = 0; i < IShape_Size(pPoly); ++i ) 
    {
        const tILine edge = IPolygon_Edge(pPoly, i);
        circumference += ILine_Length(&edge);
    }

    return circumference;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IPolygon_Slice( const tIPolygon *pPoly, 
                     const tILine *pLine, 
                     bool inside, 
                     tILine *pLinesOut, 
                     int *pLinesSizeOut, 
                     int alloced )
{
    SOFTWARE_EXCEPTION_ASSERT( pLine         != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pLinesOut     != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pLinesSizeOut != NULL );

    tCoordinate2D coords[POLYGON_ARR_SIZE];
    tCoordinate2D transformedCoords[POLYGON_ARR_SIZE];

    if ( IShape_Size(pPoly) < 3 || alloced == 0 )
    {
        return false;
    }

    int noIntersections;

    if ( inside )
    {
        noIntersections = LineIntersections( pPoly, pLine, &coords[0], POLYGON_ARR_SIZE - 2);
        if ( noIntersections == IPOLYGON_MEMORY_ERROR )
        {
            return false;
        }
    }
    else
    {
        noIntersections = LineIntersections( pPoly, pLine, &coords[1], POLYGON_ARR_SIZE - 2);
        if ( noIntersections == IPOLYGON_MEMORY_ERROR )
        {
            return false;
        }
        noIntersections += 2;
        coords[ 0 ] = pLine->A;
        coords[ noIntersections - 1 ] = pLine->B;
    }

    tAngle lineHeading = ILine_Angle( pLine );
    int minIndex;

    for ( int i = 0; i < noIntersections; i++ )
    {
        transformedCoords[ i ] = ICoordinate2D_InverseTransform( coords[ i ],
                                                                 zeroCoord2D,
                                                                 lineHeading );
    }

    (void) getMinCoordinate( POLYGON_SELECTION_X,
                             transformedCoords,
                             noIntersections,
                             &minIndex );

    ICoordinate2D_SortByDistance( &coords[ 0 ],
                                  &coords[ noIntersections - 1 ],
                                  coords[ minIndex ] );

    int idx = 0;

    for ( int i = 0; i + 1 < noIntersections; ++i )
    {
        tILine newLine = {.A = coords[i], .B = coords[i + 1] };

        if ( ICoordinate2D_IsEqual(newLine.A, newLine.B) )
        {
            continue;
        }

        tCoordinate2D lineCenter = ILine_Center(&newLine);

        bool isOnEdge = false;

        for ( int j = 0; j < IShape_Size( pPoly ); ++j )
        {
            const tILine edge = IPolygon_Edge( pPoly, j );

            if ( IsPointOnEdge( lineCenter, edge ) )
            {
                isOnEdge = true;
                break;
            }
        }

        bool isInside = isOnEdge || IPolygon_IsWithin(pPoly, &lineCenter);

        if ( isInside == inside )
        {
            if ( idx == alloced ) 
            {
                return false;
            }

            pLinesOut[idx++] = newLine;
        }
    }

    *pLinesSizeOut = idx;
    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
int IPolygon_IntersectionPoints( const tIPolygon *pPoly1, 
                                 const tIPolygon *pPoly2,
                                 tIPolygon_Intersection *pIntersectionsOut, 
                                 int alloced )
{
    SOFTWARE_EXCEPTION_ASSERT( pIntersectionsOut != NULL );

    if ( IShape_Size(pPoly1) < 3 || IShape_Size(pPoly2) < 3 || alloced == 0 )
    {
        return 0;
    }

    if ( pPoly1 == pPoly2 )
    {
        // a polygon cannot intersect with a copy of itself
        return 0;
    }

    int cnt = 0;

    for ( int i = 0; i < IShape_Size(pPoly1); ++i )
    {
        const tILine edge1 = IPolygon_Edge(pPoly1, i);

        for ( int j = 0; j < IShape_Size(pPoly2); ++j )
        {
            const tILine edge2 = IPolygon_Edge(pPoly2, j);
            const tCoordinate2D IP = ILine_Intersection(&edge1, &edge2);

            if ( ICoordinate2D_IsNull(IP) )
            {
                continue;
            }

            bool insert = true;
            for ( int k = 0; k < cnt; ++k )
            {
                if ( ICoordinate2D_IsEqual(IP, pIntersectionsOut[k].coordinate) )
                {
                    insert = false;
                    break;
                }
            }

            if ( insert )
            {
                if ( cnt < alloced )
                {
                    pIntersectionsOut[cnt].coordinate = IP;
                    pIntersectionsOut[cnt].index1 = i;
                    pIntersectionsOut[cnt].index2 = j;
                    ++cnt;
                }
                else
                {
                    return IPOLYGON_MEMORY_ERROR;
                }
            }
        }
    } 

    return cnt;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IPolygon_Scale( const tIPolygon *pPoly, 
                     const float scaleFactor, 
                     tIPolygon *pScaledPolyOut )
{
    SOFTWARE_EXCEPTION_ASSERT( !IShape_IsNull( pScaledPolyOut ) );

    if (    IShape_Size(pPoly) < 3 
         || IShape_MaxSize(pScaledPolyOut) < IShape_Size(pPoly) )
    {
        return false;
    }

    tCoordinate2D c = CalculateCentroid(pPoly);
    IShape_Clear(pScaledPolyOut);

    for ( int i = 0; i < IShape_Size(pPoly); ++i )
    {
        tCoordinate2D scaledPoint;
        scaledPoint.x = FloatToS32( (scaleFactor * (IShape_At(pPoly, i).x - c.x)) + c.x );
        scaledPoint.y = FloatToS32( (scaleFactor * (IShape_At(pPoly, i).y - c.y)) + c.y );

        if ( !IShape_PushBack( pScaledPolyOut, &scaledPoint ) )
        {
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
tDistance IPolygon_Distance( const tIPolygon *pPoly, const tCoordinate2D *pCoord )
{
    SOFTWARE_EXCEPTION_ASSERT( pCoord != NULL );

    if ( IShape_Size(pPoly) < 3 )
    {
        return -1;
    }

    double minSqDist = MAX_sint64;

    for ( int i = 0; i < IShape_Size(pPoly); ++i ) 
    {
        const tILine edge = IPolygon_Edge(pPoly, i);
        minSqDist = MIN(minSqDist, (double) ILine_SqDistance(&edge, pCoord));
    }

    return DoubleToS32( sqrt(minSqDist) );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D IPolygon_ClosestPoint( const tIPolygon *pPoly, const tCoordinate2D *pCoord )
{
    SOFTWARE_EXCEPTION_ASSERT( pCoord != NULL );

    tCoordinate2D closest = nullCoord2D;

    if ( IShape_Size(pPoly) < 3 )
    {
        return closest;
    }

    sint64 dist = MAX_sint64;

    for ( int i = 0; i < IShape_Size(pPoly); ++i ) 
    {
        const tILine edge = IPolygon_Edge(pPoly, i);
        const tCoordinate2D edgeCoord = ILine_ClosestPoint(&edge, pCoord);
        const sint64 edgeDist = ICoordinate2D_SqDistance(*pCoord, edgeCoord);

        if ( edgeDist < dist ) 
        {
            dist = edgeDist;
            closest = edgeCoord;
        }
    }

    return closest;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IPolygon_Orientation( const tIPolygon *pPoly, tIPolygon_Orientation *pOrientOut )
{
    // https://en.wikipedia.org/wiki/Curve_orientation
    
    SOFTWARE_EXCEPTION_ASSERT( pOrientOut != NULL );

    if ( IShape_Size(pPoly) < 3 )
    {
        return false;
    }

    const uint16 polySize = IShape_Size(pPoly);

    /* 
     * Find the lowest, rightmost vertex of the polygon. It is
     * guaranteed to be convex, therefore we can just do a simple
     * orientation check on the vertices around it instead of 
     * calculating the area of the whole polygon.
     */ 
    uint16 idx = 0;
    tCoordinate2D min = IShape_At(pPoly, 0);

    for ( uint16 i = 1; i < polySize; ++i )
    {
        const tCoordinate2D cur = IShape_At(pPoly, i);
        
        if ( cur.y < min.y || (cur.y == min.y && cur.x > min.x ) )
        {
            idx = i;
            min = cur;
        }
    }

    const uint16 idxPrev = idx == 0 ? polySize - 1 : idx - 1;
    const uint16 idxNext = idx == polySize - 1 ? 0 : idx + 1;

    const tCoordinate2D prev = IShape_At(pPoly, idxPrev);
    const tCoordinate2D next = IShape_At(pPoly, idxNext);
    tCoordinate2D_Orientation orient = ICoordinate2D_Orientation(prev, min, next);
    
    if ( orient == ICOORDINATE_COUNTERCLOCKWISE )
    {
        *pOrientOut = IPOLYGON_COUNTERCLOCKWISE; 
    }
    else if ( orient == ICOORDINATE_CLOCKWISE )
    {
        *pOrientOut = IPOLYGON_CLOCKWISE;
    }
    else
    {
        // cannot determine polygon orientation, points are colinear
        return false;
    }
    
    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IPolygon_Orient( tIPolygon *pPoly, const tIPolygon_Orientation orient )
{
    if ( IShape_Size(pPoly) < 3 )
    {
        return false;
    }

    tIPolygon_Orientation currOrient;
    if ( IPolygon_Orientation( pPoly, &currOrient ) )
    {
        if ( currOrient == orient )
        {
            return true;
        }
    }

    const int size = IShape_Size( pPoly );
    for ( int i = 0; i < size / 2; ++i )
    {
        const int j = ( size - 1 ) - i;
        const tCoordinate2D oldI = IShape_At( pPoly, i );
        const tCoordinate2D oldJ = IShape_At( pPoly, j );

        IShape_Replace( pPoly, i, &oldJ );
        IShape_Replace( pPoly, j, &oldI );
    }

    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tILine IPolygon_Edge( const tIPolygon* pPoly, const uint16 idx )
{
    tILine       edge     = nullLine;
    const uint16 polySize = IShape_Size( pPoly );

    if ( polySize < 3 || idx >= polySize )
    {
        return edge;
    }

    edge.A = IShape_At( pPoly, idx );
    edge.B = IShape_At( pPoly, idx + 1 == polySize ? 0 : idx + 1 );

    return edge;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D IPolygon_OffsetVertex( const tIPolygon* pPolygon,
                                     const uint16 idx,
                                     const tDistance offsetDistance )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pPolygon );

    tILine currentEdge = { .A = nullCoord2D, .B = nullCoord2D };
    tILine prevEdge    = { .A = nullCoord2D, .B = nullCoord2D };

    const size_t polygonSize = IShape_Size( pPolygon );

    if ( idx >= polygonSize )
    {
        /* Vertex index not valid */
        return nullCoord2D;
    }

    currentEdge.A = IShape_At( pPolygon, idx );
    currentEdge.B = ( idx == polygonSize - 1 ) ?
                    IShape_At( pPolygon, 0 ) : IShape_At( pPolygon, idx + 1 );

    prevEdge.A = ( idx == 0 ) ? IShape_Back( pPolygon ) :
                 IShape_At( pPolygon, idx - 1 );
    prevEdge.B = currentEdge.A;

    return ILine_OffsetIntersection( &currentEdge, &prevEdge,  offsetDistance  );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IPolygon_SelfIntersections( const tIPolygon* pPolygon,
                                 uint16 intersectionsSizeIn,
                                 tCoordinate2D* pIntersectionsOut,
                                 uint16* pIntersectionsSizeOut )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pPolygon );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pIntersectionsOut );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pIntersectionsSizeOut );

    return BentleyOttmann_GetPolygonIntersections( pPolygon,
                                                   pIntersectionsOut,
                                                   pIntersectionsSizeOut,
                                                   intersectionsSizeIn );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IPolygon_IsConvex( const tIPolygon* pPoly )
{
    if ( IShape_Size(pPoly) < 3 )
    {
        return false;
    }

    tCoordinate2D prev = IShape_At(pPoly, IShape_Size(pPoly) - 2);
    tCoordinate2D curr = IShape_Back(pPoly);
    tCoordinate2D next = IShape_Front(pPoly);

    // checking against next, initial check, next is first point in the polygon
    const tCoordinate2D_Orientation sample = ICoordinate2D_Orientation(prev, curr, next);

    for ( int i = 0; i < IShape_Size(pPoly) - 1; ++i )
    {
        if ( i == 0 )
        {
            prev = IShape_Back(pPoly);
            curr = IShape_At(pPoly, i);
            next = IShape_At(pPoly, i + 1);
        }
        else
        {
            prev = IShape_At(pPoly, i - 1);
            curr = IShape_At(pPoly, i);
            next = IShape_At(pPoly, i + 1);    
        }
        
        tCoordinate2D_Orientation orien = ICoordinate2D_Orientation(prev, curr, next);

        if ( orien != sample )
        {
            // not convex, a convex polygon has all its orientations equal
            return false;
        }
    }

    // polygon is convex
    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IPolygon_Offset( tIPolygon* pPoly, const int offset, tIPolygon* pOffsetPolyOut )
{
    IShape_Clear( pOffsetPolyOut );  // asserts pOffsetPolyOut != NULL

    const uint16 polySize = IShape_Size( pPoly );

    if ( polySize < 3 )
    {
        return false;
    }

    /* Offset is 0, copy over contents from origin to out polygon. */
    if ( offset == 0 )
    {
        if ( !IShape_Copy( pPoly, pOffsetPolyOut ) )
        {
            return false;
        }

        return true;
    }

    tIPolygon_Orientation polyOrient;
    if ( !IPolygon_Orientation( pPoly, &polyOrient ) )
    {
        return false;
    }

    if ( polyOrient != IPOLYGON_CLOCKWISE )
    {
        if ( !IPolygon_Orient( pPoly, IPOLYGON_CLOCKWISE ) )
        {
            return false;
        }
    }

    /*
     * Figure out where to start, prefer expanding vertices.
     */
    int startInOrigin = 0;
    for ( int i = 0; i < polySize; ++i )
    {
        int prevIndex = ( i == 0 ) ? ( polySize - 1 ) : ( i - 1 );
        int nextIndex = ( i == polySize - 1 ) ? ( 0 ) : ( i + 1 );

        tILine idxEdge = IPolygon_Edge( pPoly, i );
        tILine prevEdge = IPolygon_Edge( pPoly, prevIndex );
        tILine nextEdge = IPolygon_Edge( pPoly, nextIndex );

        // edges orientation to one another, ie. are we making left or right turns 
        const tCoordinate2D_Orientation orientAtIdx = ICoordinate2D_Orientation( prevEdge.A, prevEdge.B, idxEdge.B );
        const tCoordinate2D_Orientation orientAtNext = ICoordinate2D_Orientation( idxEdge.A, idxEdge.B, nextEdge.B );

        if ( ( offset > 0 && orientAtIdx == ICOORDINATE_CLOCKWISE ) 
             || ( offset < 0 && orientAtIdx == ICOORDINATE_COUNTERCLOCKWISE ) )
        {
            startInOrigin = i;

            if ( orientAtNext == orientAtIdx 
                 && ILine_Length( &idxEdge ) > OFFSET_START_POINT_EDGE_LEN
                 && ILine_Length( &prevEdge ) > OFFSET_START_POINT_EDGE_LEN )
            {
                break;
            }
        }
    }

    mOffsetLines.size = 0;
    for ( int i = 0; i < OFFSET_LINES_MAX_SIZE; ++i )
    {
        mOffsetLines.lines[ i ].processed = false;
    }

    /*
     * Offset all the lines of the original polygon and insert new lines in case
     * we have sharp corners.
     */
    int stopInOrigin = startInOrigin;
    do 
    {
        int prevIndex = ( startInOrigin == 0 ) ? ( polySize - 1 ) : ( startInOrigin - 1 );
        int nextIndex = ( startInOrigin == polySize - 1 ) ? ( 0 ) : ( startInOrigin + 1 );

        tILine idxEdge = IPolygon_Edge( pPoly, startInOrigin );
        tILine prevEdge = IPolygon_Edge( pPoly, prevIndex );
        tILine nextEdge = IPolygon_Edge( pPoly, nextIndex );

        // edges orientation to one another, ie. are we making left or right turns 
        const tCoordinate2D_Orientation orientAtIdx = ICoordinate2D_Orientation( prevEdge.A, prevEdge.B, idxEdge.B );
        const tCoordinate2D_Orientation orientAtNext = ICoordinate2D_Orientation( idxEdge.A, idxEdge.B, nextEdge.B );

        ILine_Offset( &idxEdge, offset );
        ILine_Offset( &prevEdge, offset );
        ILine_Offset( &nextEdge, offset );

        tILine tempEdge;

        if ( orientAtIdx == ICOORDINATE_COLINEAR )
        {
            // discard colinear points
            startInOrigin = ( startInOrigin + 1 ) % polySize;
            continue;
        }

        if ( ( offset > 0 && orientAtIdx == ICOORDINATE_COUNTERCLOCKWISE ) 
             || ( offset < 0 && orientAtIdx == ICOORDINATE_CLOCKWISE ) )
        {
            /* the edges are shrinking */
            
            tILine extension;
            if ( orientAtIdx != orientAtNext )
            {
                tempEdge = OffsetIntersectionForExpandingEdges( &idxEdge, &nextEdge, offset );
                idxEdge.B = tempEdge.A;
            }
            else
            {
                extension = ILine_FromPoint( &idxEdge, OFFSET_EXTENSION, idxEdge.B );
                idxEdge.B = extension.B;
            }

            if ( mOffsetLines.size + 1 >= OFFSET_LINES_MAX_SIZE )
            {
                (void) IPolygon_Orient( pPoly, polyOrient );
                return false;
            }

            tILine idxEdgeRev = { .A = idxEdge.B, .B = idxEdge.A };
            extension = ILine_FromPoint( &idxEdgeRev, OFFSET_EXTENSION, idxEdgeRev.B );
            idxEdge.A = extension.B;

            mOffsetLines.lines[ mOffsetLines.size++ ].line = idxEdge;
        }
        else
        {
            /* the edges are expanding */

            tempEdge = OffsetIntersectionForExpandingEdges( &prevEdge, &idxEdge, offset );

            if ( !ICoordinate2D_IsNull( tempEdge.B ) )
            {
                tILine firstEdge = { .A = tempEdge.A, .B = tempEdge.B };
                tILine extension = ILine_FromPoint( &idxEdge, OFFSET_EXTENSION, idxEdge.B );
                tILine secondEdge = { .A = tempEdge.B, .B = extension.B };

                if ( mOffsetLines.size + 1 >= OFFSET_LINES_MAX_SIZE )
                {
                    (void) IPolygon_Orient( pPoly, polyOrient );
                    return false;
                }

                mOffsetLines.lines[ mOffsetLines.size++ ].line = firstEdge;

                if ( orientAtIdx == orientAtNext )
                {
                    tempEdge = OffsetIntersectionForExpandingEdges( &idxEdge, &nextEdge, offset );
                    secondEdge.B = tempEdge.A;
                }

                if ( mOffsetLines.size + 1 >= OFFSET_LINES_MAX_SIZE )
                {
                    (void) IPolygon_Orient( pPoly, polyOrient );
                    return false;
                }

                mOffsetLines.lines[ mOffsetLines.size++ ].line = secondEdge;
            }
            else
            {
                tILine firstEdge = { .A = tempEdge.A, .B = idxEdge.B };

                if ( orientAtIdx == orientAtNext )
                {
                    tempEdge = OffsetIntersectionForExpandingEdges( &idxEdge, &nextEdge, offset );
                    firstEdge.B = tempEdge.A;
                }
                else
                {
                    tILine extension = ILine_FromPoint( &firstEdge, OFFSET_EXTENSION, firstEdge.B );
                    firstEdge.B = extension.B;
                }

                if ( mOffsetLines.size + 1 >= OFFSET_LINES_MAX_SIZE )
                {
                    (void) IPolygon_Orient( pPoly, polyOrient );
                    return false;
                }

                mOffsetLines.lines[ mOffsetLines.size++ ].line = firstEdge;
            }
        }

        if ( startInOrigin == polySize - 1 )
        {
            startInOrigin = 0;
        }
        else
        {
            ++startInOrigin;
        }

    } while ( startInOrigin != stopInOrigin );

    (void) IPolygon_Orient( pPoly, polyOrient );

    /* 
     * Do the offsetting of the polygon. Go through the offsetted lines and 
     * check for intersections. All the intersections with the current edge we
     * are checking are taken into account. Of them the one closest to the 
     * start point of the edge is taken as valid.
     */
    for ( int i = 0; i < mOffsetLines.size; ++i )
    {
        tPolygon_OffsetLine* currLine = &mOffsetLines.lines[ i ];
        
        if ( currLine->processed )
        {
            continue;
        }

        int jStart = ( i == mOffsetLines.size - 1 ) ? 0 : i + 1;
        int jStop = ( i == 0 ) ? mOffsetLines.size - 1 : i - 1;

        mOffsetIntersections.size = 0;

        bool firstIter = true;
        while ( jStart != jStop )
        {
            tPolygon_OffsetLine* lineToCheck = &mOffsetLines.lines[ jStart ];

            if ( !lineToCheck->processed || firstIter )
            {
                tCoordinate2D inter = ILine_Intersection( &currLine->line, &lineToCheck->line );

                if ( !ICoordinate2D_IsNull( inter ) )
                {
                    mOffsetIntersections.inters[ mOffsetIntersections.size ].inter = inter;
                    mOffsetIntersections.inters[ mOffsetIntersections.size ].refEdgeIdx = i;
                    mOffsetIntersections.inters[ mOffsetIntersections.size ].crossingEdgeIdx = jStart;

                    tDistance dist = ICoordinate2D_SqDistance( currLine->line.A, inter );
                    if ( firstIter )
                    {
                        mOffsetIntersections.inters[ mOffsetIntersections.size ].dist = dist + 10;
                    }
                    else if ( ICoordinate2D_IsEqual( inter, lineToCheck->line.B ) )
                    {
                        mOffsetIntersections.inters[ mOffsetIntersections.size ].dist = dist - 10;
                    }
                    else
                    {
                        mOffsetIntersections.inters[ mOffsetIntersections.size ].dist = dist;
                    }

                    ++mOffsetIntersections.size;
                }

                firstIter = false;
            }

            if ( jStart == mOffsetLines.size - 1 )
            {
                jStart = 0;
            }
            else
            {
                ++jStart;
            }
        }
        
        if ( mOffsetIntersections.size > 0 )
        {
            qsort( mOffsetIntersections.inters, mOffsetIntersections.size, sizeof(tPolygon_OffsetIntersection), QSortCompareForOffsetPolygon );

            tCoordinate2D pointToAdd = nullCoord2D;
            tCoordinate2D extraPoint = nullCoord2D;

            for ( int j = 0; j < mOffsetIntersections.size; ++j )
            {
                tPolygon_OffsetIntersection* inter = &mOffsetIntersections.inters[ j ];
                tPolygon_OffsetLine* refLine = &mOffsetLines.lines[ inter->refEdgeIdx ];
                int nextIdxAfterRefIdx = ( inter->refEdgeIdx == mOffsetLines.size - 1 ) ? 0 : inter->refEdgeIdx + 1;

                if ( ICoordinate2D_IsEqual( inter->inter, refLine->line.B ) && inter->crossingEdgeIdx == nextIdxAfterRefIdx )
                {
                    if ( ICoordinate2D_IsNull( pointToAdd ) )
                    {
                        pointToAdd = inter->inter;
                    }
                    else
                    {
                        extraPoint = inter->inter;
                    }

                    refLine->processed = true;
                    break;
                }

                /*
                * Figure out how the edges intersect at this intersection and
                * which edges we need to mark as processed. 
                */
                tPolygon_OffsetLine* crossingLine = &mOffsetLines.lines[ inter->crossingEdgeIdx ];
                tCoordinate2D_Orientation orientRefCrossing = ICoordinate2D_Orientation( refLine->line.A, 
                                                                                         refLine->line.B, 
                                                                                         crossingLine->line.B );
                tCoordinate2D_Orientation orientCrossingRef = ICoordinate2D_Orientation( crossingLine->line.A, 
                                                                                         crossingLine->line.B, 
                                                                                         refLine->line.B );

                if ( orientRefCrossing == ICOORDINATE_COLINEAR && orientCrossingRef == ICOORDINATE_COLINEAR )
                {
                    tILine extendedRefLine = ILine_FromPoint( &refLine->line, 1000, refLine->line.B );
                    extendedRefLine.A = refLine->line.A;

                    tILine extendedCrossingLine = ILine_FromPoint( &crossingLine->line, 1000, crossingLine->line.B );
                    extendedCrossingLine.A = crossingLine->line.A;

                    orientRefCrossing = ICoordinate2D_Orientation( extendedRefLine.A, extendedRefLine.B, extendedCrossingLine.B );
                    orientCrossingRef = ICoordinate2D_Orientation( extendedCrossingLine.A, extendedCrossingLine.B, extendedRefLine.B );
                }

                if ( ( offset > 0 && orientRefCrossing == ICOORDINATE_COUNTERCLOCKWISE )
                     || ( offset < 0 && orientRefCrossing == ICOORDINATE_CLOCKWISE ) )
                {
                    if ( ICoordinate2D_IsNull( pointToAdd ) )
                    {
                        pointToAdd = inter->inter;
                    }
                    else
                    {
                        extraPoint = inter->inter;
                    }

                    int start = ( inter->refEdgeIdx == mOffsetLines.size - 1 ) ? 0 : inter->refEdgeIdx + 1;
                    int stop = inter->crossingEdgeIdx;

                    while ( start != stop )
                    {
                        mOffsetLines.lines[ start ].processed = true;
                        start = ( start == mOffsetLines.size - 1 ) ? 0 : start + 1;
                    }

                    refLine->processed = true;

                    break;
                }
                else  if ( ( offset > 0 && orientCrossingRef == ICOORDINATE_COUNTERCLOCKWISE )
                           || ( offset < 0 && orientCrossingRef == ICOORDINATE_CLOCKWISE ) )
                {
                    
                    if ( !crossingLine->processed )
                    {
                        pointToAdd = inter->inter;

                        int start = ( inter->crossingEdgeIdx == mOffsetLines.size - 1 ) ? 0 : inter->crossingEdgeIdx + 1;
                        int stop = inter->refEdgeIdx;

                        while ( start != stop )
                        {
                            mOffsetLines.lines[ start ].processed = true;
                            start = ( start == mOffsetLines.size - 1 ) ? 0 : start + 1;
                        }

                        refLine->processed = true;
                        crossingLine->processed = true;
                    }
                }
            }

            if ( !ICoordinate2D_IsNull( pointToAdd ) )
            {
                if ( !IShape_PushBack( pOffsetPolyOut, &pointToAdd ) )
                {
                    return false;
                }
            }

            if ( !ICoordinate2D_IsNull( extraPoint ) )
            {
                if ( !IShape_PushBack( pOffsetPolyOut, &extraPoint ) )
                {
                    return false;
                }
            }
        }
    }

    /*
     * Clean up colinear points if any.
     */
    mOffsetLines.size = 0;
    for ( int i = 0; i < IShape_Size( pOffsetPolyOut ); ++i )
    {
        int prevIdx = ( i == 0 ) ? IShape_Size( pOffsetPolyOut ) - 1 : i - 1;
        int nextIdx = ( i == IShape_Size( pOffsetPolyOut ) - 1 ) ? 0 : i + 1;

        tCoordinate2D curr = IShape_At( pOffsetPolyOut, i );
        tCoordinate2D prev = IShape_At( pOffsetPolyOut, prevIdx );
        tCoordinate2D next = IShape_At( pOffsetPolyOut, nextIdx );

        tCoordinate2D_Orientation orient = ICoordinate2D_Orientation( prev, curr, next );

        if ( orient != ICOORDINATE_COLINEAR )
        {
            mOffsetLines.lines[ mOffsetLines.size++ ].line.A = curr;
        }
    }

    if ( ICoordinate2D_IsEqual( mOffsetLines.lines[ 0 ].line.A, mOffsetLines.lines[ mOffsetLines.size - 1 ].line.A ) )
    {
        --mOffsetLines.size;
    }

    IShape_Clear( pOffsetPolyOut );

    for ( int i = 0; i < mOffsetLines.size; ++i )
    {
        if ( !IShape_PushBack( pOffsetPolyOut, &mOffsetLines.lines[ i ].line.A ) )
        {
            return false;
        }
    }

    /* 
     * Basic sanity check for the offsetted polygon.
     */
    tIPolygon_Orientation offsetPolyOrient;
    if ( !IPolygon_Orientation( pOffsetPolyOut, &offsetPolyOrient) )
    {
        // we should be able to determine the orientation
        return false;
    }

    if ( offsetPolyOrient != IPOLYGON_CLOCKWISE )
    {
        // and it should still be CW
        return false;
    }

    return true;
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
static tCoordinate2D CalculateCentroid( const tIPolygon *pPoly )
{
    double x = 0, y = 0, areaSum = 0;
    const uint16 size = IShape_Size(pPoly);

    for ( int i = 1; i < size - 1; ++i )
    {
        tCoordinate2D P0 = IShape_Front(pPoly); // ToDo: Only works on convex shapes
        tCoordinate2D P1 = IShape_At(pPoly, i);
        tCoordinate2D P2 = IShape_At(pPoly, i + 1);

        double cX = P0.x + P1.x + P2.x;
        double cY = P0.y + P1.y + P2.y;

        double area = (P1.x - P0.x) * (P2.y - P0.y) - (P2.x -P0.x) * (P1.y - P0.y);

        x += area * cX;
        y += area * cY;
        areaSum += area;
    }

    x /= 3 * areaSum;
    y /= 3 * areaSum;

    return (tCoordinate2D) { .x = DoubleToS32(x), .y = DoubleToS32(y) };
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static sint16 WindingNumber( const tIPolygon *pPoly, const tCoordinate2D *pPoint )
{
    sint16 windingNumber = 0;

    // Use a ray starting at pPoly and extending in positive x direction
    for (int i = 0; i < IShape_Size(pPoly); i++)
    {
        const tILine edge = IPolygon_Edge(pPoly, i);

        if ( edge.A.y <= pPoint->y )
        {
            if (edge.B.y > pPoint->y )
            {
                if ( ICoordinate2D_Orientation( edge.A, edge.B, *pPoint ) == ICOORDINATE_COUNTERCLOCKWISE )
                {
                    // Found an edge crossing left
                    ++windingNumber;
                }
            }
        }
        else
        {
            if ( edge.B.y <= pPoint->y)
            {
                if ( ICoordinate2D_Orientation( edge.A, edge.B, *pPoint ) == ICOORDINATE_CLOCKWISE )
                {
                    // Found an edge crossing right
                    --windingNumber;
                }
            }
        }
    }

    return windingNumber;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static sint64 SignedArea( const tIPolygon *pPoly )
{
    if ( IShape_Size(pPoly) < 3 )
    {
        return 0;
    }

    sint64 area = IShape_Back(pPoly).x * IShape_Front(pPoly).y -
                  IShape_Back(pPoly).y * IShape_Front(pPoly).x;

    for ( int i = 0; i + 1 < IShape_Size(pPoly); ++i )
    {
        area += IShape_At(pPoly, i).x * IShape_At(pPoly, i + 1).y -
                IShape_At(pPoly, i).y * IShape_At(pPoly, i + 1).x;
    }

    /* Negative if clockwise */
    return area/2;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static int LineIntersections( const tIPolygon *pPoly, 
                              const tILine *pLine,
                              tCoordinate2D *pIntersectionsOut, 
                              int alloced )
{
    int cnt = 0;

    for ( int i = 0; i < IShape_Size(pPoly); ++i ) 
    {
        const tILine edge = IPolygon_Edge(pPoly, i);
        tCoordinate2D intersect = GetLineIntersection( *pLine, edge );

        if ( ICoordinate2D_IsNull(intersect) ) 
        {
            continue;
        }

        if ( cnt == alloced ) 
        {
            return IPOLYGON_MEMORY_ERROR;
        }

        pIntersectionsOut[cnt++]= intersect;
    }

    return cnt;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static tCoordinate2D GetLineIntersection( const tILine l1, const tILine l2 )
{
    /* Quotient represents relative distance on edge where intersection occurs (if any) */
    double quotient = 0.0;

    const double denominator = (double)( l2.A.x - l2.B.x ) * (double)( l1.A.y - l1.B.y ) -
                               (double)( l2.A.y - l2.B.y ) * (double)( l1.A.x - l1.B.x );
    const double numerator = (double)( l2.A.x - l1.A.x ) * (double)( l1.A.y - l1.B.y ) -
                             (double)( l2.A.y - l1.A.y ) * (double)( l1.A.x - l1.B.x );

    if ( numerator != 0.0 )
    {
        /* Check for division by zero */
        if ( denominator == 0.0 )
        {
            return nullCoord2D;
        }

        quotient = numerator / denominator;

        /* Quotient not in [0, 1] => intersection not on line */
        /* Consider intersection on edge if quotient = 0 and on next edge if quotient = 1
         * to avoid duplicates when iterating over a polygon's edges */
        if ( quotient < 0.0 || quotient >= 1.0 )
        {
            return nullCoord2D;
        }
    }

    tCoordinate2D intersectionPoint;

    intersectionPoint.x = l2.A.x + DoubleToS32( quotient * (double)( l2.B.x - l2.A.x ) );
    intersectionPoint.y = l2.A.y + DoubleToS32( quotient * (double)( l2.B.y - l2.A.y ) );

    return intersectionPoint;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static tCoordinate2D getMinCoordinate( tPolygon_Selection selection,
                                       tCoordinate2D *coordinates,
                                       int coordinatesSize,
                                       int *index )
{
    tCoordinate2D currentMin = coordinates[0];
    *index = 0;

    for ( int i = 1; i < coordinatesSize; i++ )
    {
        if ( selection == POLYGON_SELECTION_X )
        {
            if ( coordinates[i].x < currentMin.x )
            {
                currentMin = coordinates[i];
                *index = i;
            }
        }
        else if ( selection == POLYGON_SELECTION_Y )
        {
            if ( coordinates[i].y < currentMin.y )
            {
                currentMin = coordinates[i];
                *index = i;
            }
        }
    }

    return currentMin;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static bool IsPointOnEdge( tCoordinate2D point, tILine edge )
{
    tDistance distanceFromEdge = ILine_Distance( &edge, &point );

    /* Regard point as being on edge when abs(distance) == 1 to account for roundoff error */
    if ( distanceFromEdge == 0 || abs( distanceFromEdge ) == 1 )
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
static bool IsPathEdgeIntersecting( const tIPolygon *pPoly, const tILine *pPathEdge )
{
    for ( size_t polygon_index = 0; polygon_index < IShape_Size( pPoly ); ++polygon_index )
    {
        const tILine polygon_edge = IPolygon_Edge(pPoly, polygon_index);

        if ( ILine_IsIntersecting( &polygon_edge, pPathEdge ) )
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
static void TransformPathEdge( tPolygon_Transform_Direction direction,
                               tPolygon_Transform_AB ab,
                               tPolygon_Transform_XY xy,
                               tILine *pPathEdge,
                               const tDistance width )
{
    if ( POLYGON_TRANSFORM_INCREASE == direction )
    {
        if ( POLYGON_TRANSFORM_A == ab )
        {
            if ( POLYGON_TRANSFORM_X == xy )
            {
                if ( ( MAX_sint32 - width ) < pPathEdge->A.x )
                {
                    pPathEdge->A.x = MAX_sint32;
                }
                else
                {
                    pPathEdge->A.x += width;
                }
            }
            else
            {
                if ( ( MAX_sint32 - width ) < pPathEdge->A.y )
                {
                    pPathEdge->A.y = MAX_sint32;
                }
                else
                {
                    pPathEdge->A.y += width;
                }
            }
        }
        else
        {
            if ( POLYGON_TRANSFORM_X == xy )
            {
                if ( ( MAX_sint32 - width ) < pPathEdge->B.x )
                {
                    pPathEdge->B.x = MAX_sint32;
                }
                else
                {
                    pPathEdge->B.x += width;
                }
            }
            else
            {
                if ( ( MAX_sint32 - width ) < pPathEdge->B.y )
                {
                    pPathEdge->B.y = MAX_sint32;
                }
                else
                {
                    pPathEdge->B.y += width;
                }
            }
        }
    }
    else
    {
        if ( POLYGON_TRANSFORM_A == ab )
        {
            if ( POLYGON_TRANSFORM_X == xy )
            {
                if ( pPathEdge->A.x < ( MIN_sint32 + width ) )
                {
                    pPathEdge->A.x = MIN_sint32;
                }
                else
                {
                    pPathEdge->A.x -= width;
                }
            }
            else
            {
                if ( pPathEdge->A.y < ( MIN_sint32 + width ) )
                {
                    pPathEdge->A.y = MIN_sint32;
                }
                else
                {
                    pPathEdge->A.y -= width;
                }
            }
        }
        else
        {
            if ( POLYGON_TRANSFORM_X == xy )
            {
                if ( pPathEdge->B.x < ( MIN_sint32 + width ) )
                {
                    pPathEdge->B.x = MIN_sint32;
                }
                else
                {
                    pPathEdge->B.x -= width;
                }
            }
            else
            {
                if ( pPathEdge->B.y < ( MIN_sint32 + width ) )
                {
                    pPathEdge->B.y = MIN_sint32;
                }
                else
                {
                    pPathEdge->B.y -= width;
                }
            }
        }
    }
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static tILine OffsetIntersectionForExpandingEdges( const tILine* const inEdge,
                                                   const tILine* const outEdge,
                                                   const tDistance offset )
{
    SOFTWARE_EXCEPTION_ASSERT( inEdge != NULL );
    SOFTWARE_EXCEPTION_ASSERT( outEdge != NULL );

    /*
     * inEdge and outEdge are consecutive with inEdge going to the point and
     * outEdge going from the point. Example:
     *  
     *                outEdge
     *                 |
     *                 v
     *         ^ -------------- >
     *         |
     *         |
     *         | <-- inEdge 
     *         |
     *         |
     * 
     * NOTE: the edges are NOT offsetted here, the user should offset the edges
     *       before calling this function
     */
    const tAngle inEdgeAngle = ILine_Angle( inEdge );
    const tAngle outEdgeAngle = ILine_Angle( outEdge );
    const tAngle norm = IAngle_Normalize( outEdgeAngle - inEdgeAngle );

    const int extendDist = abs( DoubleToS32( (double) offset * tan( IAngle_DegToRad( norm ) / 4.0 ) ) );

    tILine outEdgeRev = { .A = outEdge->B, .B = outEdge->A };
    tILine extendedInEdge;
    tILine extendedOutEdge;

    if ( extendDist == 0 )
    {
        extendedInEdge = *inEdge;
        extendedOutEdge = outEdgeRev;
    }
    else
    {
        extendedInEdge = ILine_FromPoint( inEdge, extendDist, inEdge->B );
        extendedInEdge.A = inEdge->A;
        extendedOutEdge = ILine_FromPoint( &outEdgeRev, extendDist, outEdgeRev.B );
        extendedOutEdge.A = outEdgeRev.A;
    }

    const tCoordinate2D inter = ILine_IntersectionInfinite( &extendedInEdge, &extendedOutEdge );

    tILine offsetInter;
    if ( !ILine_IsPointOnLine( &extendedInEdge, inter ) && !ILine_IsPointOnLine( &extendedOutEdge, inter ) )
    {
        offsetInter.A = extendedInEdge.B;
        offsetInter.B = extendedOutEdge.B;
    }
    else
    {
        offsetInter.A = inter;
        offsetInter.B = nullCoord2D;
    }

    return offsetInter;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static int QSortCompareForOffsetPolygon( const void* a, const void* b )
{
    tPolygon_OffsetIntersection* pA = (tPolygon_OffsetIntersection*) a;
    tPolygon_OffsetIntersection* pB = (tPolygon_OffsetIntersection*) b;

    if ( pA->dist < pB->dist ) { return -1; }
    else if ( pA->dist > pB->dist ) { return 1; }
    else { return 0; }
}
