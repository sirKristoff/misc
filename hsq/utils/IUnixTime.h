/**
 ******************************************************************************
 * @file      IUnixTime.h
 *
 * @brief     Interface for UnixConverter
 ******************************************************************************
 */
#ifndef IUNIXTIME_H
#define IUNIXTIME_H

#include "RoboticTypes.h"

/**
 ******************************************************************************
 * Date in regular binary format (not BCD)
 ******************************************************************************
 */
typedef struct {
    /** Name:   Year,
     *  Range:  1970-2099,
     *  Desc:   The year (1970-2099) */
    uint16 year;
    /** Name:   Month,
     *  Range:  1-12,
     *  Desc:   The month (1-12) */
    uint8 month;
    /** Name:   Weekday,
     *  Range:  1-7,
     *  Desc:   The day of the week. 1=Mon ... 7=Sun */
    uint8 weekday;
    /** Name:   Date,
     *  Range:  1-31,
     *  Desc:   Day of the month (1-31) */
    uint8 date;
    /** Name:   Hour,
     *  Range:  0-23,
     *  Desc:   The hour (0-23) */
    uint8 hour;
    /** Name:   Minute,
     *  Range:  0-59,
     *  Desc:   The minute (0-59) */
    uint8 minute;
    /** Name:   Second,
     *  Range:  0-59,
     *  Desc:   The second (0-59) */
    uint8 second;
} tCalendar;

/**
 ******************************************************************************
 * @brief   Helper to convert from calendar time structure to Unix time
 * @param   pCalendar
 *          Calendar time to convert
 * @returns time as number of seconds from Unix Epoch
 ******************************************************************************
 */
uint32 IUnixTime_CalendarTime2UnixTime(const tCalendar* pCalendar);

/**
******************************************************************************
* @brief   Helper to convert from Unix time to calendar time structure
* @param   unixTime
*          time as number of seconds from Unix Epoch
* @param   pCalendar
*          Calendar time to convert to [out]
******************************************************************************
*/
void IUnixTime_UnixTime2CalendarTime(const uint32 unixTime, tCalendar* pCalendar);

#endif /* IUNIXTIME_H */
