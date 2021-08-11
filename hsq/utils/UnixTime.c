/**
 ******************************************************************************
 * @file      UnixTime.c
 *
 * @brief     Implementation of UnixTime.
 ******************************************************************************
 */
#include "UnixTime.h"


/**
 ******************************************************************************
 * @brief   Helper to convert from calendar time structure to Unix time
 * @param   pCalendar Calendar time to convert
 * @returns Unix time as number of seconds from Unix Epoch
 ******************************************************************************
 */
uint32 IUnixTime_CalendarTime2UnixTime(const tCalendar* pCalendar)
{
    uint32  unixTime;
    sint16  month;    /* Month can temporary be negative */
    uint16  year;

    /* Convert calendar time to seconds since 00:00 1 Jan 1970 */
    year = pCalendar->year;
    /* Check if year within valid interval */
    if ((year < 1970) || (year > 2106))
    {   /* Error, invalid year */
        return 0;
    }
    month = (sint16)pCalendar->month - 2;
    if (month <= 0)
    {   /* Month is Jan or Feb, i.e. we has not reach the leap day this year*/
        month += 12;
        year--;
    }
    unixTime = year / 4;             /* Calculate max number of leap days   */
    unixTime -= year / 100;           /* Each 100:th year is not a leap year*/
    unixTime += year / 400;           /* Each 400:th year is a leap year    */
    unixTime += 367 * month / 12;     /* Inc. with number of days this year */
    unixTime += pCalendar->date;      /* Inc. with days in this month       */
    unixTime += (uint32)year * 365;  /* None leap days before this year    */
    unixTime -= 719499;               /* Number of days from year 0 to 1970 */
                                      /*unixTime -= 693932;*/           /* Number of days from year 0 to 1900 */
    unixTime = unixTime * 24 + pCalendar->hour;       /* Convert to hours   */
    unixTime = unixTime * 60 + pCalendar->minute;     /* Convert to minutes */
    unixTime = unixTime * 60 + pCalendar->second;     /* Convert to seconds */

    return unixTime;
}

void IUnixTime_UnixTime2CalendarTime(const uint32 unixTime, tCalendar* pCalendar)
{
#define BASE_YEAR                  1970
#define FIRST_WEEKDAY_IN_BASE_YEAR 3         /* 0=Mon ... 6=Sun     */
#define SECONDS_IN_NORMAL_YEAR     31536000  /* 365 * 24 * 60 * 60  */
#define SECONDS_IN_LEAP_YEAR       31622400  /* 366 * 24 * 60 * 60  */
#define SECONDS_IN_DAY             86400     /* 24 * 60 * 60 */
#define SECONDS_IN_4_YEARS         ((3*SECONDS_IN_NORMAL_YEAR)+SECONDS_IN_LEAP_YEAR)

    /* This macro checks if year is a normal year (0) or a leap year (1) */
#define IS_LEAP_YEAR(year)  ( ((year % 4 == 0) && \
                                   (year % 100 != 0  || year % 400 == 0))? 1 : 0 )

    /* Last day in month for normal year and leap year */
    static const uint32 LAST_DAY_IN_MONTH[2][13] =
    { /*  -  Jan Feb Mar Apr  May  Jun  Jul  Aug  Sep  Oct  Nov  Dec */
        { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
        { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
    };


    uint32   remainingSeconds; /* Remaining number of s to calculate with */
    uint32   temp;
    uint32   secondsInYear;

    remainingSeconds = unixTime;

    /*--- Calculate weekday. 1=Mon ... 7=Sun ---*/
    temp = ((remainingSeconds / SECONDS_IN_DAY) % 7 + FIRST_WEEKDAY_IN_BASE_YEAR) % 7 + 1;
    pCalendar->weekday = (uint8)temp;

    /*--- Calculate year (1970-2099) ---*/
    pCalendar->year = BASE_YEAR;
    /* Calculate number of years modulo 4 */
    temp = remainingSeconds / SECONDS_IN_4_YEARS; /* Years modulo 4 */
    pCalendar->year += ((uint16)temp * 4);
    remainingSeconds -= (temp * SECONDS_IN_4_YEARS);
    /* Calculate the last 0-3 years */
    secondsInYear = SECONDS_IN_NORMAL_YEAR; /* Seconds in BASE_YEAR */

    while (remainingSeconds >= secondsInYear)
    {
        remainingSeconds -= secondsInYear;
        pCalendar->year++;
        if (!IS_LEAP_YEAR(pCalendar->year))
        {
            secondsInYear = SECONDS_IN_NORMAL_YEAR;
        }
        else
        {
            secondsInYear = SECONDS_IN_LEAP_YEAR;
        }
    }

    /*--- Calculate month (1-12) ---*/
    temp = remainingSeconds / SECONDS_IN_DAY;/* Calculate day number (0-365)*/
    temp++;                                  /* Day in range 1-366          */
    pCalendar->month = (uint8)(temp / 32) + 1; /* Step to come near the month */
    while (temp > LAST_DAY_IN_MONTH[IS_LEAP_YEAR(pCalendar->year)][pCalendar->month])
    {
        pCalendar->month++;                  /* Month in range 1-12 */
    }
    remainingSeconds -= (LAST_DAY_IN_MONTH[IS_LEAP_YEAR(pCalendar->year)][pCalendar->month - 1] * SECONDS_IN_DAY);

    /*--- Calculate date (1-31) ---*/
    pCalendar->date = (uint8)(remainingSeconds / SECONDS_IN_DAY);
    remainingSeconds -= (uint32)(pCalendar->date * SECONDS_IN_DAY);
    pCalendar->date++;                      /* Date in range 1-31 */

                                            /*--- Calculate hour (0-23) ---*/
    pCalendar->hour = (uint8)(remainingSeconds / (3600));
    remainingSeconds -= (uint32)pCalendar->hour * 3600;

    /*--- Calculate minute (0-59) and second (0-59) ---*/
    pCalendar->minute = (uint8)(remainingSeconds / 60);
    pCalendar->second = (uint8)(remainingSeconds % 60);
}
