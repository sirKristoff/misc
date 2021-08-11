/**
 ******************************************************************************
 * @file        IPolar.h
 *
 * @brief       IPolar interface
 ******************************************************************************
 */

#ifndef IPOLAR_H
#define IPOLAR_H


/*
 ------------------------------------------------------------------------------
 Include files
 ------------------------------------------------------------------------------
 */

#include "FileId.h"
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

typedef struct 
{
    tDistance distance; /** distance */
    tAngle angle;       /** angle */

} tIPolar;


/*
 ------------------------------------------------------------------------------
 Interface functions
 ------------------------------------------------------------------------------
 */

/**
 ******************************************************************************
 * @brief   Convert a cartesian coordinate to polar with origo as reference.
 * @param   cartesian
 *          the coordinate to convert
 * @return  the coordinate converted as a distance and angle from (0,0)
 ******************************************************************************
 */
tIPolar IPolar_FromCoord( const tCoordinate2D cartesian );

/**
 ******************************************************************************
 * @brief   Convert a cartesian coordinate to polar with 2nd coordinate
 *          as reference point.
 * @param   a
 *          the coordinate to convert
 * @param   b
 *          the reference point
 * @return  the coordinate 'a' converted as a distance and angle from 'b'
 ******************************************************************************
 */
tIPolar IPolar_FromCoords( const tCoordinate2D a, const tCoordinate2D b );


#endif /* IPOLAR_H */
