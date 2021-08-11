/**
 ******************************************************************************
 * @file        Polar.c
 * 
 * @brief       Implementation of IPolar.h
 ******************************************************************************
 */

/*
 -------------------------
 Include files
 -------------------------
 */

#include <math.h>

#include "ICoordinate2D.h"
#include "IPolar.h"
#include "Polar.h"
#include "IAngle.h"

/*
-------------------------
   Defines
-------------------------
*/


/*
 -------------------------
 Type definitions
 -------------------------
 */


/*
 ---------------------------------
    Private data
 ---------------------------------
 */


/*
---------------------------------
   Private function prototypes
---------------------------------
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
tIPolar IPolar_FromCoord( const tCoordinate2D cartesian )
{
    tIPolar polar;

    polar.distance = ICoordinate2D_Distance( zeroCoord2D, cartesian );
    polar.angle    = ICoordinate2D_Angle(    zeroCoord2D, cartesian );

    return polar;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tIPolar IPolar_FromCoords( const tCoordinate2D a, const tCoordinate2D b )
{
    const tCoordinate2D c = ICoordinate2D_Sub(a, b);
    return IPolar_FromCoord(c);
}


/*
-------------------------------------------------------------------------------
    Implementation of private functions
-------------------------------------------------------------------------------
*/
