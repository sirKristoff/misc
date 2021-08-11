/**
 ******************************************************************************
 * @file        IPath.h
 *
 * @brief       IPath interface
 ******************************************************************************
 */

#ifndef IPATH_H
#define IPATH_H


/*
 ------------------------------------------------------------------------------
 Include files
 ------------------------------------------------------------------------------
 */

#include "ILine.h"
#include "IShape.h"
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

typedef tIShape tIPath;


/*
 ------------------------------------------------------------------------------
 Interface functions
 ------------------------------------------------------------------------------
 */

/**
 ******************************************************************************
 * @brief   Calculate the total length of a path.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPath
 *          pointer to the path
 * @returns the total length of the path
 ******************************************************************************
 */
tDistance IPath_Length( const tIPath *pPath );

/**
 ******************************************************************************
 * @brief   Find the closest point on a path relative to a reference coordinate.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPath
 *          pointer to the path
 * @param   pCoord
 *          pointer to the reference coordinate
 * @param   pLineOut
 *          ointer to the line in the path where closest point was found
 * @returns the closest point on the path if found, nullCoord2D otherwise
 ******************************************************************************
 */
tCoordinate2D IPath_ClosestPoint( const tIPath *pPath, 
                                  const tCoordinate2D *pCoord, 
                                  tILine *pLineOut );

/**
 ******************************************************************************
 * @brief   Check if a coordinate is on a path.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPath
 *          pointer to the path
 * @param   pCoord
 *          pointer to the coordinate to check
 * @returns true if the coordinate is on the path, false otherwise
 ******************************************************************************
 */
bool IPath_OnPath( const tIPath *pPath, const tCoordinate2D *pCoord );

/**
 ******************************************************************************
 * @brief   Return the path segment from IShape_At index to index + 1
 *          Similar to IPolygon_Edge but for tIPath
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPath
 *          pointer to the path
 * @param   index
 *          index of segment, from 0 to size -2
 * @returns The indexed line segment, or nullLine if index out of bounds
 ******************************************************************************
 */
tILine IPath_Segment( const tIPath *pPath, uint16 index );

/**
 ******************************************************************************
 * @brief   Checks if a coordinate is within the width of a path.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPath
 *          pointer to the path
 * @param   pCoord
 *          pointer to the coordinate to check
 * @returns true if the coordinate is within the path, false otherwise
 ******************************************************************************
 */
bool IPath_WithinPathWidth( const tIPath *pPath, const tCoordinate2D *pCoord );

/**
 ******************************************************************************
 * @brief   Gets the distance to the closest path boarder from given coordinates.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pPath
 *          pointer to the path
 * @param   pCoord
 *          pointer to the coordinate to check
 * @returns tDistance, positive distance if within path width, else negative
 ******************************************************************************
 */
tDistance IPath_DistanceToClosestPathBorder( const tIPath *pPath, const tCoordinate2D *pCoord );

/**
 ******************************************************************************
 * @brief   Gets the closest border point relative to given coordinates. The
 *          borders of the path are first offsetted parallel to the segments
 *          defining the path with distance half of the path width. The closest
 *          border point is then the projection of the given coordinate on the
 *          offsetted borders.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 *          NOTE: works only for non self-intersecting borders, ie the offsetted
 *                path curve is not self-intersecting
 * @param   pPath
 *          pointer to the path
 * @param   pCoord
 *          pointer to the coordinate to check
 * @returns closest border point if it exists, otherwise nullCoord2D
 ******************************************************************************
 */
tCoordinate2D IPath_ClosestBorderPoint( const tIPath *pPath, const tCoordinate2D *pCoord );

/**
 ******************************************************************************
 * @brief   Gets the closest border interesection with a reference line relative
 *          to given coordinates. The borders of the path are first offsetted 
 *          parallel to the segments defining the path with distance half of the 
 *          path width.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 *          NOTE: works only for non self-intersecting borders, ie the offsetted
 *                path curve is not self-intersecting
 * @param   pPath
 *          pointer to the path
 * @param   pLine
 *          pointer to the reference line
 * @param   pCoord
 *          pointer to the reference coordinates
 * @returns closest border intersection if it exists, otherwise nullCoord2D
 ******************************************************************************
 */
tCoordinate2D IPath_ClosestBorderIntersection( const tIPath *pPath, const tILine *pLine, const tCoordinate2D *pCoord );

#endif /* IPATH_H */
