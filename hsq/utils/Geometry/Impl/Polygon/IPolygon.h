/**
 ******************************************************************************
 * @file      IPolygon.h
 *
 * @brief     IPolygon interface
 ******************************************************************************
 */

#ifndef IPOLYGON_H
#define IPOLYGON_H


/*
 ------------------------------------------------------------------------------
    Include files
 ------------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <stddef.h>  // size_t type

#include "ILine.h"
#include "IPath.h"
#include "IShape.h"
#include "RoboticTypes.h"


/*
 ------------------------------------------------------------------------------
    Defines
 ------------------------------------------------------------------------------
 */

#define IPOLYGON_MEMORY_ERROR (-1)


/*
 ------------------------------------------------------------------------------
    Type definitions
 ------------------------------------------------------------------------------
 */

typedef tIShape tIPolygon;

typedef struct
{
    uint16          index1;
    uint16          index2;
    tCoordinate2D   coordinate;

} tIPolygon_Intersection;

typedef enum tIPolygon_Orientation
{
    IPOLYGON_CLOCKWISE = 0,
    IPOLYGON_COUNTERCLOCKWISE,

} tIPolygon_OrientationEnum;

typedef uint8 tIPolygon_Orientation;


/*
 ------------------------------------------------------------------------------
    Interface functions
 ------------------------------------------------------------------------------
 */

/**
 ******************************************************************************
 * @brief   Check if a coordinate is interior to a polygon.
 *          Follows the standard convention that a point on a left or bottom
 *          edge is considered inside, and a point on a right or top edge is
 *          considered outside. This way, if two distinct polygons share a
 *          common boundary segment, then a point on that segment will be in
 *          one polygon or the other, but not both at the same time.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPoly
 *          pointer to the polygon
 * @param   pCoord
 *          pointer to the coordinate
 * @returns true if the coordinate is inside polygon, false otherwise
 ******************************************************************************
 */
bool IPolygon_IsWithin( const tIPolygon *pPoly, const tCoordinate2D *pCoord );

/**
 ******************************************************************************
 * @brief   Check if an intersection exists between two polygons.
 *          Runtime: O(n^2)
 *          NOTE: May be possible to find faster algorithms.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPoly1
 *          pointer to the first polygon
 * @param   pPoly2
 *          pointer to the second polygon
 * @returns true if intersection exists, false otherwise
 ******************************************************************************
 */
bool IPolygon_IsIntersecting( const tIPolygon *pPoly1, const tIPolygon *pPoly2 );

/**
 ******************************************************************************
 * @brief   Check if an intersection exists between a polygon and a line.
 *          Runtime: O(n^2)
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPoly
 *          pointer to the polygon
 * @param   pLine
 *          pointer to the line
 * @returns true if intersection exists, false otherwise
 ******************************************************************************
 */
bool IPolygon_IsLineIntersecting( const tIPolygon *pPoly, const tILine *pLine );

/**
 ******************************************************************************
 * @brief   Check if an intersection exists between a polygon and a path with width.
 *          The implementation is a pessimistic approach by transforming the line
 *          in different directions according to a compass.
 *          Runtime: O(n^2)
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPoly
 *          pointer to the polygon
 * @param   pPath
 *          pointer to the path
 * @returns true if intersection exists, false otherwise
 ******************************************************************************
 */
bool IPolygon_IsPathWidthIntersecting( const tIPolygon *pPoly, const tIPath *pPath );

/**
 ******************************************************************************
 * @brief   Calculate the area of a polygon.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPoly
 *          pointer to the polygon
 * @returns the area of the polygon
 ******************************************************************************
 */
sint64 IPolygon_Area( const tIPolygon *pPoly );

/**
 ******************************************************************************
 * @brief   Calculate the perimeter (circumference) of a polygon.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPoly
 *          pointer to the polygon
 * @returns the distance around the perimeter (circumference) of the polygon
 ******************************************************************************
 */
tDistance IPolygon_Perimeter( const tIPolygon *pPoly );

/**
 ******************************************************************************
 * @brief  "Slice" a line over a polygon.
 *          Calculates the intersections between the line and the polygon
 *          and uses them to split the line such that the resulting lines 
 *          are inside (or outside) the polygon.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPoly
 *          pointer to the polygon
 * @param   pLine
 *          pointer to the line
 * @param   inside
 *          whether the result should be inside or outside the given polygon
 * @param   pLinesOut
 *          pointer to an array for storing the resulting lines, *MUST* be
 *          initialized by user
 * @param   pLinesSizeOut
 *          pointer to an int for storing the number of resulting lines,
 *          (size of pLines) after slicing, *MUST* be initialized by user
 * @param   alloced
 *          memory allocated at pLines
 * @returns true if successful, false otherwise (including memory error)
 ******************************************************************************
 */
bool IPolygon_Slice( const tIPolygon *pPoly, 
                     const tILine *pLine, 
                     bool inside, 
                     tILine *pLinesOut, 
                     int *pLinesSizeOut, 
                     int alloced );

/**
 ******************************************************************************
 * @brief   Find intersection points of two polygons.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPoly1
 *          pointer to the first polygon
 * @param   pPoly2
 *          pointer to the second polygon
 * @param   pIntersectionsOut
 *          pointer to an array for storing the resulting intersection points, 
 *          *MUST* be initialized by user
 * @param   alloced
 *          memory allocated at pIntersectionsOut
 * @returns number of intersections
 ******************************************************************************
 */
int IPolygon_IntersectionPoints( const tIPolygon *pPoly1, 
                                 const tIPolygon *pPoly2,
                                 tIPolygon_Intersection *pIntersectionsOut, 
                                 int alloced );

/**
 ******************************************************************************
 * @brief   Scale a polygon up or down by a scale factor. The method uses 
 *          homothetic transformation for scaling the polygon.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPoly
 *          pointer to the polygon to scale
 * @param   scaleFactor
 *          scale factor > 1 -> scale up
 *          1 > scale factor > 0 -> scale down
 *          scale factor == 1 -> no scale
 * @param   pScaledPolyOut
 *          pointer to a struct to store the scaled polygon, *MUST* be 
 *          initialized by user
 * @returns true if successful, false otherwise
 ******************************************************************************
 */
bool IPolygon_Scale( const tIPolygon *pPoly, 
                     const float scaleFactor, 
                     tIPolygon *pScaledPolyOut );

/**
 ******************************************************************************
 * @brief   Calculate the distance from a coordinate to the closest
 *          coordinate on a polygon.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPoly
 *          pointer to the polygon 
 * @param   pCoord
 *          pointer to the coordinate
 * @returns the distance from pCoord to the closest coordinate on the polygon,
 *          -1 otherwise
 ******************************************************************************
 */
tDistance IPolygon_Distance( const tIPolygon *pPoly, const tCoordinate2D *pCoord );

/**
 ******************************************************************************
 * @brief   Find the point on a polygon closest to a coordinate.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPoly
 *          pointer to the polygon
 * @param   pCoord
 *          pointer to the coordinate
 * @returns the point on the polygon closest to pCoord if found, 
 *          nullCoord2D otherwise
 ******************************************************************************
 */
tCoordinate2D IPolygon_ClosestPoint( const tIPolygon *pPoly, const tCoordinate2D *pCoord );

/**
 ******************************************************************************
 * @brief   Check the orientation of a polygon.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPoly
 *          pointer to the polygon
 * @param   pOrientOut
 *          pointer to struct for storing the polygon orientation, *MUST* be
 *          initialized by user
 * @return  orientation of the polygon, i.e. clockwise or counterclockwise
 ******************************************************************************
 */
bool IPolygon_Orientation( const tIPolygon *pPoly, tIPolygon_Orientation *pOrientOut );

/**
 ******************************************************************************
 * @brief   Orient a polygon to specified orientation. 
 *          NOTE: The function changes the polygon in-place.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPoly
 *          the polygon
 * @param   orient
 *          the desired orientation for the polygon
 * @returns true if successful, false otherwise
 ******************************************************************************
 */
bool IPolygon_Orient( tIPolygon *pPoly, const tIPolygon_Orientation orient );

/**
 ******************************************************************************
 * @brief   Get the edge in the polygon from idx to idx + 1. Handles wrap
 *          around the last element.
 * @param   pPoly
 *          the polygon
 * @param   idx
 *          start index for the edge in the polygon
 * @returns the edge in the polygon from idx -> idx + 1 if successful, a line 
 *          with nullCoord2D as its ends otherwise
 ******************************************************************************
 */
tILine IPolygon_Edge( const tIPolygon* pPoly, const uint16 idx );

/**
 ******************************************************************************
 * @brief   Offset a polygon's vertex with a given distance.
 *          If polygon is oriented clockwise, offset vertex will point outwards for a positive distance.
 *          If polygon is oriented counterclockwise, offset vertex will point inwards for a positive distance.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPolygon
 *          The polygon
 * @param   idx
 *          Index of vertex to offset
 * @param   offsetDistance
 *          Wanted offset distance
 * @returns Offset vertex if successful (valid vertex index), otherwise nullCoord2D
 ******************************************************************************
 */
tCoordinate2D IPolygon_OffsetVertex( const tIPolygon* pPolygon,
                                     const uint16 idx,
                                     const tDistance offsetDistance );

/**
 ******************************************************************************
 * @brief   Get a polygon's self intersections (if non-simple).
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized.
 * @param   pPolygon
 *          Pointer to polygon.
 * @param   intersectionsSizeIn
 *          Total size allocated for pIntersectionsOut.
 * @param   pIntersectionsOut
 *          Pointer to found intersection points.
 * @param   pIntersectionsSizeOut
 *          Pointer to number of intersection points found.
 * @returns True if successful, otherwise false.
 ******************************************************************************
 */
bool IPolygon_SelfIntersections( const tIPolygon* pPolygon,
                                 uint16 intersectionsSizeIn,
                                 tCoordinate2D* pIntersectionsOut,
                                 uint16* pIntersectionsSizeOut );

/**
 ******************************************************************************
 * @brief   Check if a polygon is convex.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPoly
 *          The polygon to check
 * @returns True if polygon is convex, otherwise false
 ******************************************************************************
 */
bool IPolygon_IsConvex( const tIPolygon* pPoly );

/**
 ******************************************************************************
 * @brief   Offset a polygon. Negative offset distance shrinks the polygon (offset
 *          inwards) and positive offset distance expands it (offset outwards). 
 *          If offset is 0 then the offset polygon is a copy of the origin. 
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPoly
 *          Pointer to the polygon to offset
 * @param   offset
 *          The offset distance 
 * @param   pOffsetPolyOut
 *          Pointer to a struct to store the offset polygon, *MUST* be 
 *          initialized by user with sufficient space (2 * size of origin will
 *          cover all cases)
 * @returns true if successful, false otherwise
 ******************************************************************************
 */
bool IPolygon_Offset( tIPolygon* pPoly, const int offset, tIPolygon* pOffsetPolyOut );

#endif  // IPOLYGON_H
