/**
 ******************************************************************************
 * @file      IAngle.h
 *
 * @brief     IAngle interface
 ******************************************************************************
 */

#ifndef IANGLE_H
#define IANGLE_H

/*
-------------------------------------------------------------------------------
    Include files
-------------------------------------------------------------------------------
*/

#include "RoboticTypes.h"

/*
-------------------------------------------------------------------------------
    Type definitions
-------------------------------------------------------------------------------
*/

#define IANGLE_QUARTER_ROTATION ( 900 )    /* Given in deci degrees */
#define IANGLE_HALF_ROTATION    ( 1800 )   /* Given in deci degrees */
#define IANGLE_FULL_ROTATION    ( 3600 )   /* Given in deci degrees */

/*
 ------------------------------------------------------------------------------
    Interface functions
 ------------------------------------------------------------------------------
 */

/**
 ******************************************************************************
 * @brief   Normalizes an angle to the range (-1800,1800]
 * @param   angle
 *          unnormalized angle
 * @returns normalized angle
 ******************************************************************************
 */
tAngle IAngle_Normalize( tAngle angle );

/**
 ******************************************************************************
 * @brief   Normalizes an angle to the range (-PI,PI]
 * @param   rad
 *          unnormalized angle in radians
 * @returns normalized angle in radians
 ******************************************************************************
 */
double IAngle_NormalizeRad( double rad );

/**
 ******************************************************************************
 * @brief   Convert from radians to deci-degrees
 * @param   rad
 *          angle value in radians
 * @returns angle value in deci-degrees
 ******************************************************************************
 */
tAngle IAngle_RadToDeg( double rad );

/**
 ******************************************************************************
 * @brief   Convert from deci-degrees to radians
 * @param   angle
 *          angle value in deci-degrees
 * @returns angle value in radians
 ******************************************************************************
 */
double IAngle_DegToRad( tAngle angle );


#endif  // IANGLE_H
