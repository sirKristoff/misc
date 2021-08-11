/**
 ******************************************************************************
 * @file      ICoordinate2D.h
 *
 * @brief     ICoordinate2D interface
 ******************************************************************************
 */

#ifndef ICOORDINATE2D_H
#define ICOORDINATE2D_H


/*
-------------------------------------------------------------------------------
	Include files
-------------------------------------------------------------------------------
*/

#include "RoboticTypes.h"


/*
-------------------------------------------------------------------------------
    Defines
-------------------------------------------------------------------------------
*/
#define ICOORDINATE2D_NULL_COORDINATE { MIN_sint32/10, MIN_sint32/10 }


/*
-------------------------------------------------------------------------------
	Type definitions
-------------------------------------------------------------------------------
*/

typedef enum tCoordinate2D_Orientation
{
    ICOORDINATE_COLINEAR = 0,
    ICOORDINATE_CLOCKWISE = 1,
    ICOORDINATE_COUNTERCLOCKWISE = 2,

} tCoordinate2D_OrientationEnum;

typedef uint8 tCoordinate2D_Orientation;

/* Using min or max for sint32 may cause overflow, even in sint64 */
static const tCoordinate2D nullCoord2D = ICOORDINATE2D_NULL_COORDINATE;
static const tCoordinate2D zeroCoord2D = {.x = 0, .y = 0};


/*
 ------------------------------------------------------------------------------
    Interface functions
 ------------------------------------------------------------------------------
 */

/**
 ******************************************************************************
 * @brief   Compute the angle from one coordinates to another
 * @param   from
 * @param   to
 * @returns angle in the range (-1800, 1800] if it can be calculated, 0 otherwise
 ******************************************************************************
 */
tAngle ICoordinate2D_Angle( const tCoordinate2D from, const tCoordinate2D to );

/**
 ******************************************************************************
 * @brief   Compute the distance between two coordinates.
 * @param   A
 *          the first coordinate
 * @param   B
 *          the second coordinate
 * @returns Distance
 ******************************************************************************
 */
tDistance ICoordinate2D_Distance( const tCoordinate2D A, const tCoordinate2D B );

/**
 ******************************************************************************
 * @brief   Sort an array of coordinates by distance to a reference coordinate.
 *          NOTE: Uses bubble-sort algorithm for sorting the array
 *          NOTE: Changes the array in-place
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pCoordArrBegin
 *          pointer to start of coordinate array
 * @param   pCoordArrEnd
 *          pointer to end of coordinate array
 * @param   coord
 *          the reference coordinate
 * @returns -
 ******************************************************************************
 */
void ICoordinate2D_SortByDistance( tCoordinate2D *pCoordArrBegin,
                                   tCoordinate2D *pCoordArrEnd,
                                   const tCoordinate2D coord );

/**
 ******************************************************************************
 * @brief   Check if two coordinates are equal.
 * @param   A
 *          the first coordinate
 * @param   B
 *          the second coordinate
 * @returns true if equal, false otherwise
 ******************************************************************************
 */
bool ICoordinate2D_IsEqual( const tCoordinate2D A, const tCoordinate2D B );

/**
 ******************************************************************************
 * @brief   Check if a coordinate is equal to nullCoord2D.
 * @param   coord
 *          the coordinate to check
 * @returns true if equal to nullCoord2D, false otherwise
 ******************************************************************************
 */
bool ICoordinate2D_IsNull( const tCoordinate2D coord );

/**
 ******************************************************************************
 * @brief   Compute the square distance between two coordinates.
 * @param   A
 *          the first coordinate
 * @param   B
 *          the second coordinate
 * @returns the square distance between the two coordinates
 ******************************************************************************
 */
sint64 ICoordinate2D_SqDistance( const tCoordinate2D A, const tCoordinate2D B );

/**
 ******************************************************************************
 * @brief   Add two coordinates.
 * @param   A
 *          the first coordinate
 * @param   B
 *          the second coordinate
 * @returns a coordinate comprising of the addition of the two coordinates
 ******************************************************************************
 */
tCoordinate2D ICoordinate2D_Add( const tCoordinate2D A, const tCoordinate2D B );

/**
 ******************************************************************************
 * @brief   Subtract two coordinates, A - B.
 * @param   A
 *          the first coordinate
 * @param   B
 *          the second coordinate
 * @returns a coordinate comprising of the substraction of the two coordinates
 ******************************************************************************
 */
tCoordinate2D ICoordinate2D_Sub( const tCoordinate2D A, const tCoordinate2D B );

/**
 ******************************************************************************
 * @brief   Rotate a coordinate around the origin.
 * @param   coord
 *          the coordinate to rotate
 * @param   rotateAngle
 *          the angle to rotate
 * @returns a coordinate comprising of the rotation
 ******************************************************************************
 */
tCoordinate2D ICoordinate2D_Rotate( const tCoordinate2D coord, const tAngle rotateAngle );

/**
 ******************************************************************************
 * @brief   Transform a coordinate by first rotating it and then adding it
 *          with a translation coordinate.
 * @param   coord
 *          the coordinate to transform
 * @param   translation
 *          the translation coordinate
 * @param   rotateAngle
 *          the angle to rotate
 * @returns a coordinate comprising of the result of the transformation
 ******************************************************************************
 */
tCoordinate2D ICoordinate2D_Transform( const tCoordinate2D coord,
                                       const tCoordinate2D translation,
                                       const tAngle rotateAngle );

/**
 ******************************************************************************
 * @brief   Inverse transform a coordinate by first substracting it with a 
 *          translation coordinate and then rotating it.
 * @param   coord
 *          the coordinate to transform
 * @param   translation
 *          the translation coordinate
 * @param   rotateAngle
 *          the angle to rotate
 * @returns a coordinate comprising of the result of the inverse transformation
 ******************************************************************************
 */
tCoordinate2D ICoordinate2D_InverseTransform( const tCoordinate2D coord,
                                              const tCoordinate2D translation,
                                              const tAngle rotateAngle );


/**
 ******************************************************************************
 * @brief   Determine the orientation of a coordinate relative to a line. The
 *          line is defined by coordinates A --> B and the coordinate to check
 *          is C.
 * @param   A
 *          the first coordinate of the line 
 * @param   B
 *          the second coordinate of the line
 * @param   C
 *          the coordinate for which to check the orientation
 * @returns the orientation of the coordinate relative to the line
 ******************************************************************************
 */
tCoordinate2D_Orientation ICoordinate2D_Orientation( const tCoordinate2D A,
                                                     const tCoordinate2D B,
                                                     const tCoordinate2D C );

/**
 ******************************************************************************
 * @brief   Offsets a coordinate a given distance in a given angle
 *          This can be viewed as a simplified version of Transform
 * @param   coord
 *          The starting position
 * @param   dist
 *          The straight line distance to offset the coordinate with
 * @param   angle
 *          The angle in which the offset shall be applied
 * @returns A new coordinate "dist" away from "coord" in "angle" direction
 ******************************************************************************
 */
tCoordinate2D ICoordinate2D_Offset( const tCoordinate2D coord,
                                    const tDistance dist,
                                    const tAngle angle );

#endif  // ICOORDINATE2D_H
