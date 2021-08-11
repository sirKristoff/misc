/**
 ******************************************************************************
 * @file      Coordinate2D.c
 *
 * @brief     Implementation of ICoordinate2D.h
 ******************************************************************************
 */

/*
-------------------------------------------------------------------------------
	Include files
-------------------------------------------------------------------------------
*/

#include <math.h>
#include <stddef.h>

#include "Coordinate2D.h"
#include "ICoordinate2D.h"
#include "ISoftwareException.h"
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
tAngle ICoordinate2D_Angle( const tCoordinate2D from, const tCoordinate2D to )
{
    if ( ICoordinate2D_IsEqual( from, to ) )
    {
        return 0;
    }

    tCoordinate2D relativeCoords = ICoordinate2D_Sub( to, from );

    double radians = atan2( relativeCoords.y, relativeCoords.x );
    return IAngle_RadToDeg( radians );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tDistance ICoordinate2D_Distance( const tCoordinate2D A, const tCoordinate2D B )
{
    const double sqDis = (double) ICoordinate2D_SqDistance(A, B);

    return DoubleToS32( sqrt(sqDis) );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void ICoordinate2D_SortByDistance( tCoordinate2D *pCoordArrBegin,
                                   tCoordinate2D *pCoordArrEnd,
                                   const tCoordinate2D coord )
{
    SOFTWARE_EXCEPTION_ASSERT( pCoordArrBegin != NULL );
    SOFTWARE_EXCEPTION_ASSERT( pCoordArrEnd   != NULL );

    const int n = pCoordArrEnd - pCoordArrBegin + 1;

    for ( int i = 0; i < n - 1; ++i )
    {
        for ( int j = 0; j < n - i - 1; ++j )
        {
            if ( ICoordinate2D_SqDistance(*(pCoordArrBegin + j), coord) >
                 ICoordinate2D_SqDistance(*(pCoordArrBegin + j + 1), coord) ) 
            {
                tCoordinate2D temp = *(pCoordArrBegin + j);
                *(pCoordArrBegin + j) = *(pCoordArrBegin + j + 1);
                *(pCoordArrBegin + j + 1) = temp;
            }
        }
    }
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool ICoordinate2D_IsEqual( const tCoordinate2D A, const tCoordinate2D B )
{
    return (A.x == B.x) && (A.y == B.y);
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool ICoordinate2D_IsNull( const tCoordinate2D coord )
{
    return ICoordinate2D_IsEqual(coord, nullCoord2D);
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
sint64 ICoordinate2D_SqDistance( const tCoordinate2D A, const tCoordinate2D B )
{
    return ( (sint64) A.x - B.x) * ( (sint64) A.x - B.x) +
           ( (sint64) A.y - B.y) * ( (sint64) A.y - B.y);
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D ICoordinate2D_Add( const tCoordinate2D A, const tCoordinate2D B )
{
    return (tCoordinate2D) { .x = A.x + B.x, .y = A.y + B.y };
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D ICoordinate2D_Sub( const tCoordinate2D A, const tCoordinate2D B )
{
    return (tCoordinate2D) { .x = A.x - B.x, .y = A.y - B.y };
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D ICoordinate2D_Rotate( const tCoordinate2D coord, const tAngle rotateAngle )
{
    const float angle = atan2f(S32ToFloat(coord.y), S32ToFloat(coord.x));
    const float newAngle = angle + IAngle_DegToRad( rotateAngle );
    const float distance = (float) ICoordinate2D_Distance(coord, zeroCoord2D);

    return (tCoordinate2D) { .x = FloatToS32(distance * cosf(newAngle)),
                             .y = FloatToS32(distance * sinf(newAngle)) };
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D ICoordinate2D_Transform( const tCoordinate2D coord,
                                       const tCoordinate2D translation,
                                       const tAngle rotateAngle )
{
    const tCoordinate2D C = ICoordinate2D_Rotate(coord, rotateAngle);
    return ICoordinate2D_Add(C, translation);
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D ICoordinate2D_InverseTransform( const tCoordinate2D coord,
                                              const tCoordinate2D translation,
                                              const tAngle rotateAngle )
{
    const tCoordinate2D C = ICoordinate2D_Sub(coord, translation);
    return ICoordinate2D_Rotate(C, -rotateAngle);
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D_Orientation ICoordinate2D_Orientation( const tCoordinate2D A,
                                                     const tCoordinate2D B,
                                                     const tCoordinate2D C )
{
    /*  Define the slopes
           s_1 := (B.y - A.y) / (B.x - A.x),
        and
           s_2 := (C.y - B.y) / (C.x - B.x).
        Then we have that
           s_1 > s_2
        is equivalent with ABC being clockwise.
        Avoiding division, the below is obtained. */
   const sint64 slopeDiff = ( (sint64) B.y - A.y) * ( (sint64) C.x - B.x) -
                            ( (sint64) C.y - B.y) * ( (sint64) B.x - A.x);

   if ( slopeDiff == 0 ) 
   {
       return ICOORDINATE_COLINEAR;
   }

   if ( slopeDiff > 0 ) 
   {
       return ICOORDINATE_CLOCKWISE;
   }

   return ICOORDINATE_COUNTERCLOCKWISE;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D ICoordinate2D_Offset( const tCoordinate2D coord,
                                    const tDistance dist,
                                    const tAngle angle )
{
    tCoordinate2D offsetVector = { .x = dist, .y = 0 };
    return ICoordinate2D_Transform( offsetVector, coord, angle );
}

/*
-------------------------------------------------------------------------------
    Implementation of private functions
-------------------------------------------------------------------------------
*/
