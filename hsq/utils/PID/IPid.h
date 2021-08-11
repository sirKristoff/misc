/**
 ******************************************************************************
 * @file      	IPid.h
 *
 * @brief     	interface
 ******************************************************************************
 */
#ifndef IPID_H
#define IPID_H
 
/*
 ------------------------------------------------------------------------------
 Include files
 ------------------------------------------------------------------------------
 */
#include "RoboticTypes.h"


/*
-------------------------
   Defines
-------------------------
*/
#define IPID_SCALE_K    (1024.0)        /**< 12 bits       */


/*
 ------------------------------------------------------------------------------
 Type definitions
 ------------------------------------------------------------------------------
 */
typedef struct
{
    double dt;
    double integral;    // Integral part
    double imax;        // max integral part
    double kp;
    double kd;
    double ki;
    double omax;        // limit output
    double omin;
    double pre;         // last error for derived part
} tIPid_Vars;



/*
 ------------------------------------------------------------------------------
 Interface functions
 ------------------------------------------------------------------------------
 */
/**
 ******************************************************************************
 * @brief       Set default values.
 * @param[in]   me
 *              the pid object
 * @returns     -
 ******************************************************************************
 */
void IPid_Default( tIPid_Vars* const me );


/**
 ******************************************************************************
 * @brief       Get Imax value for the pid regulator.
 * @param[in]   me
 *              the pid object
 * @returns     imax value
 ******************************************************************************
 */
sint32 IPid_GetImax( const tIPid_Vars* const me );


/**
 ******************************************************************************
 * @brief       Get kd value for the pid regulator.
 * @param[in]   me
 *              the pid object
 * @returns     kd value
 ******************************************************************************
 */
sint32 IPid_GetKd( const tIPid_Vars* const me );


/**
 ******************************************************************************
 * @brief       Get ki value for the pid regulator.
 * @param[in]   me
 *              the pid object
 * @returns     ki value
 ******************************************************************************
 */
sint32 IPid_GetKi( const tIPid_Vars* const me );


/**
 ******************************************************************************
 * @brief       Get kp value for the pid regulator.
 * @param[in]   me
 *              the pid object
 * @returns     kp value
 ******************************************************************************
 */
sint32 IPid_GetKp( const tIPid_Vars* const me );


/**
 ******************************************************************************
 * @brief       Reset the pid state.
 * @param[in]   me
 *              the pid object
 * @returns     -
 ******************************************************************************
 */
void IPid_Reset( tIPid_Vars* const me );


/**
 ******************************************************************************
 * @brief       Set Imax value for the pid regulator.
 * @param[in]   me
 *              the pid object
 * @param[in]   imax
 *              imax value for the regulator
 * @returns     -
 ******************************************************************************
 */
void IPid_SetImax( tIPid_Vars* const me, const sint32 imax );


/**
 ******************************************************************************
 * @brief       Set kd value for the pid regulator.
 * @param[in]   me
 *              the pid object
 * @param[in]   kd
 *              kd value for the regulator
 * @returns     -
 ******************************************************************************
 */
void IPid_SetKd( tIPid_Vars* const me, const sint32 kd );


/**
 ******************************************************************************
 * @brief       Set ki value for the pid regulator.
 * @param[in]   me
 *              the pid object
 * @param[in]   ki
 *              ki value for the regulator
 * @returns     -
 ******************************************************************************
 */
void IPid_SetKi( tIPid_Vars* const me, const sint32 ki );


/**
 ******************************************************************************
 * @brief       Set kp value for the pid regulator.
 * @param[in]   me
 *              the pid object
 * @param[in]   kp
 *              kp value for the regulator
 * @returns     -
 ******************************************************************************
 */
void IPid_SetKp( tIPid_Vars* const me, const sint32 kp );


/**
 ******************************************************************************
 * @brief       Get K values for the .
 * @param[in]   me
 *              the pid object
 * @param[in]   want
 *              set point
 * @param[out]  is
 *              process value
 * @returns     output
 ******************************************************************************
 */
double IPid_Update( tIPid_Vars* const me, double want, double is );

 
#endif /* IPID_H */
