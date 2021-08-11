/**
 ******************************************************************************
 * @file      ILine.h
 *
 * @brief     ILine interface
 ******************************************************************************
 */

#ifndef ILINEGEOMETRY_H
#define ILINEGEOMETRY_H


/*
 ------------------------------------------------------------------------------
 Include files
 ------------------------------------------------------------------------------
 */

#include "RoboticTypes.h"
#include "ICoordinate2D.h"


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

typedef struct
{
    tCoordinate2D A;
    tCoordinate2D B;
} tILine;

static const tILine nullLine = {ICOORDINATE2D_NULL_COORDINATE, ICOORDINATE2D_NULL_COORDINATE};


/*
 ------------------------------------------------------------------------------
 Interface functions
 ------------------------------------------------------------------------------
 */

/**
 ******************************************************************************
 * @brief   Calculate the length of a line.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pLine
 *          pointer to the line
 * @returns length of the line if it can be calculated
 ******************************************************************************
 */
tDistance ILine_Length( const tILine *pLine );

/**
 ******************************************************************************
 * @brief   Project a coordinate on a line.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pLine
 *          pointer to the line
 * @param   pCoord
 *          pointer to the coordinate to project
 * @returns the projected coordinate if successful, nullCoord2d otherwise
 ******************************************************************************
 */
tCoordinate2D ILine_Project( const tILine *pLine, const tCoordinate2D *pCoord );

/**
 ******************************************************************************
 * @brief   Find the point on a line that is closest to a reference coordinate.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pLine
 *          pointer to the line
 * @param   pCoord
 *          pointer to the reference coordinate
 * @returns the closest point to the ref coord if found, nullCoord2D otherwise
 ******************************************************************************
 */
tCoordinate2D ILine_ClosestPoint( const tILine *pLine, const tCoordinate2D *pCoord );


/**
 ******************************************************************************
 * @brief   Calculate the intersection between two finite lines defined by 
 *          their coordinates.
 *          NOTE: endpoints of the line are regarded as valid intersections, ie
 *                consecutive lines A->B and B->C will intersect in B
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pLine1
 *          pointer to the first line
 * @param   pLine2
 *          pointer to the second line
 * @returns the intersection point if it exists, nullCoord2D otherwise
 ******************************************************************************
 */
tCoordinate2D ILine_Intersection( const tILine *pLine1, const tILine *pLine2 );

/**
 ******************************************************************************
 * @brief   Calculate the intersection between two infinite lines defined by 
 *          the coordinates the lines pass through.
 *          NOTE: endpoints of the line are regarded as valid intersections, ie
 *                consecutive lines A->B and B->C will intersect in B
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pLine1
 *          pointer to the first line
 * @param   pLine2
 *          pointer to the second line
 * @returns the intersection point if it exists, nullCoord2D otherwise
 ******************************************************************************
 */
tCoordinate2D ILine_IntersectionInfinite( const tILine *pLine1, const tILine *pLine2 );

/**
 ******************************************************************************
 * @brief   Check whether two finite lines defined by their coordinates
 *          intersect or not.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pLine1
 *          pointer to the first line
 * @param   pLine2
 *          pointer to the second line
 * @returns true if the lines intersect, false otherwise
 ******************************************************************************
 */
bool ILine_IsIntersecting( const tILine *pLine1, const tILine *pLine2 );

/**
 ******************************************************************************
 * @brief   Calculate the center of a line.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pLine
 *          pointer to the line
 * @returns the center of the line if it can be calculated, nullCoord2D otherwise
 ******************************************************************************
 */
tCoordinate2D ILine_Center( const tILine *pLine );

/**
 ******************************************************************************
 * @brief   Split a line around the projection of a coordinate.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pLine
 *          pointer to the line
 * @param   pCoord
 *          pointer to the coordinate
 * @param   pLinesOut
 *          pointer to an array of lines
 * @param   linesOutSize
 *          size of pLinesOut
 * @returns true if successful, false otherwise
 ******************************************************************************
 */
bool ILine_Split( const tILine *pLine, 
                  const tCoordinate2D *pCoord, 
                  tILine *pLinesOut, 
                  const int linesOutSize );

/**
 ******************************************************************************
 * @brief   Calculate the square distance between a line and a coordinate. If 
 *          a projection of the coordinate on the line exists, calculate the 
 *          distance between the projection and the coordinate, else calculate
 *          the distance between the projection and the nearest endpoint to the
 *          line.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pLine
 *          pointer to the line
 * @param   pCoord
 *          pointer to the coordinate
 * @returns the square distance if it can be calculated, MAX_sint64 otherwise
 ******************************************************************************
 */
sint64 ILine_SqDistance( const tILine *pLine, const tCoordinate2D *pCoord );

/**
 ******************************************************************************
 * @brief   Calculate the distance between a line and a coordinate.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pLine
 *          pointer to the line
 * @param   pCoord
 *          pointer to the coordinate
 * @returns the distance if it can be calculated, -1 otherwise
 ******************************************************************************
 */
tDistance ILine_Distance( const tILine *pLine, const tCoordinate2D *pCoord );

/**
 ******************************************************************************
 * @brief   Calculate the angle of a line relative to the X -axis.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pLine
 *          pointer to the line
 * @returns angle in the range (-1800, 1800] if it can be calculated, 0 otherwise
 ******************************************************************************
 */
tAngle ILine_Angle( const tILine *pLine );

/**
 ******************************************************************************
 *  @brief  Orient a line such that A is closest to a reference coordinate.
 *          NOTE: modifies the line in-place
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 *  @param  pLine
 *          pointer to the line
 *  @param  pCoord
 *          pointer to the reference coordinate
 * @returns true if successful, false otherwise
 ******************************************************************************
 */
bool ILine_SortByDistance( tILine *pLine, const tCoordinate2D *pCoord );

/**
 ******************************************************************************
 * @brief  Parallel offset a line with a given distance.
 *         Positive offset distance results in a parallel offset to the 'left'
 *         (counter-clockwise) relative line heading.
 *         Negative offset distance results in a parallel offset to the 'right'
 *         (clockwise) relative line heading.
 *         NOTE: modifies the line in-place
 *         NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param  pLine
 *         pointer to line to offset
 * @param  offsetDistance
 *         offset distance to use
 ******************************************************************************
 */
void ILine_Offset( tILine *pLine, const tDistance offsetDistance );

/**
 ******************************************************************************
 * @brief   Get intersection (if existing) between two lines that has been
 *          parallel offset with a given distance.
 *          Positive offset distance results in a parallel offset to the 'left'
 *          (counter-clockwise) relative line heading for both lines.
 *          Negative offset distance results in a parallel offset to the 'right'
 *          (clockwise) relative line heading for both lines.
 *          NOTE: modifies the lines in-place
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pLineA
 *          pointer to line
 * @param   pLineB
 *          pointer to line
 * @param   offsetDistance
 *          offset distance to use
 * @returns Intersection point between lines if successful, otherwise nullCoord2D
 ******************************************************************************
 */
tCoordinate2D ILine_OffsetIntersection( tILine *pLineA,
                                        tILine *pLineB,
                                        const tDistance offsetDistance );

/**
 ******************************************************************************
 * @brief   Get all intersections among a set of lines.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pLineSegments
 *          Pointer to set of line segments.
 * @param   linesSegmentsSize
 *          Number of line segments in pLineSegments.
 * @param   intersectionsSizeIn
 *          Total size allocated for pIntersectionsOut.
 * @param   pIntersectionsOut
 *          Pointer to found intersection points.
 * @param   pIntersectionsSizeOut
 *          Pointer to number of intersection points found.
 * @returns True if successful, otherwise false.
 ******************************************************************************
 */
bool ILine_Intersections( const tILine* pLineSegments,
                          const uint16 linesSegmentsSize,
                          uint16 intersectionsSizeIn,
                          tCoordinate2D* pIntersectionsOut,
                          uint16* pIntersectionsSizeOut );

/**
 ******************************************************************************
 * @brief   Check if a line is equal to nullLine.
 * @param   pLine
 *          pointer to line
 * @returns true if equal to nullLine, false otherwise
 ******************************************************************************
 */
bool ILine_IsNull( const tILine* const pLine );

/**
 ******************************************************************************
 * @brief   Extend a line in the general direction of the line. If the distance
 *          is positive, the line is extended beyond its B point. If the distance
 *          is negative, then the A point of the line is extended backwards. If
 *          the distance is 0, the line is not modified.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 *          NOTE: the line is modified in-place
 * @param   pLine
 *          pointer to the line to extend
 * @param   distance
 *          the distance to extend
 ******************************************************************************
 */
void ILine_Extend( tILine* const pLine, const tDistance distance );

/**
 ******************************************************************************
 * @brief   Make a new line with a given start point and length. The new line
 *          has the same direction as the given reference line.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pLine
 *          pointer to the reference line
 * @param   length
 *          the length for the new line
 * @param   p
 *          the start point for the new line
 * @returns nullLine if length <= 0 or if reference line is a point
 ******************************************************************************
 */
tILine ILine_FromPoint( const tILine* const pLine, const tDistance length, const tCoordinate2D p );

/**
 ******************************************************************************
 * @brief   Check if a point lies in front of a line in the direction of the 
 *          line. In front is counted from the starting point of the line.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pLine
 *          pointer to the line
 * @param   p
 *          the point to check
 * @returns true if point is in front, false otherwise
 ******************************************************************************
 */
bool ILine_IsPointInFront( const tILine* const pLine, const tCoordinate2D p );

/**
 ******************************************************************************
 * @brief   Check if a point lies on a line.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pLine
 *          pointer to the line
 * @param   p
 *          the point to check
 * @returns true if the point lies on the line, false otherwise
 ******************************************************************************
 */
bool ILine_IsPointOnLine( const tILine* const pLine, const tCoordinate2D p );

#endif  // ILINEGEOMETRY_H
