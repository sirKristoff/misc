/**
 ******************************************************************************
 * @file      Pid.c
 *
 * @brief     Implementation file for StopWatch
 ******************************************************************************
 */

 
 /*
 -------------------------
 Include files
 -------------------------
 */
#include <IPid.h>
#include "RoboticUtils.h"


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
 -------------------------
 Interface functions
 -------------------------
 */
void IPid_Default( tIPid_Vars* const me )
{
    me->dt = 50;
    me->integral = 0;
    me->imax = 100.0;
    me->kd = 0.1;
    me->ki = 0.1;
    me->kp = 0.5;
    me->omax = 1000.0;
    me->omin = -1000.0;
    me->pre = 0;
}


sint32 IPid_GetImax( const tIPid_Vars* const me )
{
    return DoubleToS32( me->imax * IPID_SCALE_K );
}


sint32 IPid_GetKd( const tIPid_Vars* const me )
{
    return DoubleToS32( me->kd * IPID_SCALE_K );
}


sint32 IPid_GetKi( const tIPid_Vars* const me )
{
    return DoubleToS32( me->ki * IPID_SCALE_K );
}


sint32 IPid_GetKp( const tIPid_Vars* const me )
{
    return DoubleToS32( me->kp * IPID_SCALE_K );
}


void IPid_Reset( tIPid_Vars* const me )
{
    me->integral = 0;
    me->pre = 0;
}


void IPid_SetImax( tIPid_Vars* const me, const sint32 imax )
{
    me->imax = (double)imax / IPID_SCALE_K;
}


void IPid_SetKd( tIPid_Vars* const me, const sint32 kd )
{
    me->kd = (double)kd / IPID_SCALE_K;
}


void IPid_SetKi( tIPid_Vars* const me, const sint32 ki )
{
    me->ki = (double)ki / IPID_SCALE_K;
}


void IPid_SetKp( tIPid_Vars* const me, const sint32 kp )
{
    me->kp = (double)kp / IPID_SCALE_K;
}


double IPid_Update( tIPid_Vars* const me ,double want, double is )
{
    double output = 0.0;

    // Calculate error
    double error = want - is;

    {   // Proportional term
        double Pout = me->kp * error;
        output += Pout;     // add P part
    }

    {   // Integral term
        if ( 0.0001 < me->ki )     // enabled
        {
            me->integral += error * me->dt;
            double Iout = me->ki * me->integral;
            if      ( Iout < -(me->imax) )  { Iout = -(me->imax); me->integral = -(me->imax)/me->ki; }  // avoid build up
            else if ( me->imax < Iout )  { Iout = me->imax; me->integral = me->imax/me->ki; }
            output += Iout;     // add I part
        }
    }

    {
        // Derivative term
        if ( 0.0001 < me->kd )     // enabled
        {
            double derivative = (error - me->pre) / me->dt;
            double Dout = me->kd * derivative;
            output += Dout;     // add D part
        }
    }

    // Restrict to max/min
    if      ( output < me->omin ) { output = me->omin; }
    else if ( me->omax < output ) { output = me->omax; }

    // Save error to previous error for D part
    me->pre = error;

    return output;
}


/*
------------------------------
   Private functions
------------------------------
*/
 
