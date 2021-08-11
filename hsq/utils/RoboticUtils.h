
/**
 ******************************************************************************
 * @file      RoboticUtils.h
 *
 * @brief     Interface for Robotic Utils.
 ******************************************************************************
 */
#ifndef ROBOTICCORE_GENERIC_DEFINE_ROBOTICUTILS_H_
#define ROBOTICCORE_GENERIC_DEFINE_ROBOTICUTILS_H_

#include <RoboticTypes.h>

#ifndef MIN
  #define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
  #define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef CONFINE
    #define CONFINE(min, val, max) (((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)))
#endif

#ifndef ARRSIZE
    #define ARRSIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef FATAL_ERROR
#define FATAL_ERROR { while ( true ) {} }
#endif

#if defined(WIN32) || defined(__linux__)
#define STATIC_ASSERT(condition, msg) _Static_assert(condition, msg)
#else
#define STATIC_ASSERT(condition, msg)  static_assert(condition, msg)
#endif

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif 

#ifndef HIBYTE
#define HIBYTE(x) (((x)>>8)&0xff)
#endif

#ifndef LOBYTE
#define LOBYTE(x) ((x)&0xff)
#endif

#ifndef WIN32

#ifndef MAKEWORD
#define MAKEWORD(h, l) ((LOBYTE(h)<<8) | LOBYTE(l))
#endif

#ifndef HIWORD
#define HIWORD(x) (((x)>>16)&0xffff)
#endif

#ifndef LOWORD
#define LOWORD(x) ((x)&0xffff)
#endif

#ifndef MAKELONG
#define MAKELONG(h, l) (LOWORD(h)<<16 | LOWORD(l))
#endif

#endif /* Win32 */

#ifndef __HKV_QUEUE__
#define __HKV_QUEUE__
/*****************************************************************************/
/* QUEUE_INIT
 * type - type of items in queue (type must be able to copy itself)
 * invalid - when emtpy q this item is returned
 * size - size of queue
 * name - of the queue
 *****************************************************************************/

/* Setup as global or local queue */
#define QUEUE_INIT(type, invalid, size, name) \
struct { \
    int r,w; \
    type inv; \
    type q[size + 1]; \
} name = { 0, 0, invalid }

/* when definining in header and c file use these inits */
/* Usage QUEUE_INIT_H(type, name, invalid) */
#define QUEUE_INIT_H(type, size, name) \
 struct { \
    int r,w; \
    type inv; \
    type q[size + 1]; \
} name
/* Usage name = QUEUE_INIT_C(size, invalid) */
#define QUEUE_INIT_C(name, invalid) \
    { name.r = name.w = 0; name.inv = invalid; }

/* Init from Rhapsody */
/* Create attribute myHistory */
/* Attribute type C Declaration: QUEUE_INIT_H(int, 10, QUEUE) */
/* Initial value: "" */
/* In init operation:  QUEUE_INIT_C(me->myHistory, -1); */


/* Helpers */
/* Helpers */
#define QUEUE_IX(name, expr) ((expr) % ARRSIZE(name.q))
#define QUEUE_isFULL(name) (QUEUE_IX(name, name.r) == QUEUE_IX(name, name.w + 1))
#define QUEUE_isEMPTY(name) (QUEUE_IX(name, name.r) == QUEUE_IX(name, name.w))

/* Empty the queue */
#define QUEUE_CLEAR(name) (name.r = name.w = 0)

/* Queue with wrap around */
#define QUEUE_ENQ(name, d) { QUEUE_isFULL(name) ? name.r++ : name.r; name.q[QUEUE_IX(name, name.w++)] = d; }
#define QUEUE_DEQ(name) (QUEUE_isEMPTY(name) ? name.inv : name.q[QUEUE_IX(name, name.r++)])

/* Stack usage */
#define QUEUE_PUSH(name, d) (QUEUE_isFULL(name) ? name.inv : name.q[QUEUE_IX(name, name.w++)] = d)
#define QUEUE_POP(name) (QUEUE_isEMPTY(name) ? name.inv : name.q[QUEUE_IX(name, --name.w)])

/* Iterator starting at read position */
#define QUEUE_ITER(name, ix) (QUEUE_IX(name, name.r + ix) == QUEUE_IX(name, name.w) ? name.inv : name.q[QUEUE_IX(name, name.r + ix)])

/* Typical usage:
 * QUEUE_INIT(uint16, 0xffff, 10, name);
 * uint16 d;
 * QUEUE_ENQ(name, 1);
 * QUEUE_ENQ(name, 2);
 * QUEUE_ENQ(name, 3);
 * for (int i = 0; (d = QUEUE_ITER(name, i)) != 0xffff; i++) {}
 **/
#endif /* __HKV_QUEUE__ */

#ifndef FloatingAverageFilter
/*
 ******************************************************************************
 * Macro
 * By successive calls this filter will generate a floating average value.
 * The average value will be changed proportional to the difference between
 * the new value and average value by using the 'divider' parameter.
 * An approximation of the influence from the 'divider' is that:
 *  - after 'divider' number of calls the value is on about 65 %
 *  - after 'divider*2' number of calls the value is on about 85 %
 *  - after 'divider*3' number of calls the value is on about 95 %
 *
 * Note! This macro requires the first parameter 'averageValue' to be
 * a signed variable.
 ******************************************************************************
 */
#define FloatingAverageFilter(averageValue, newValue, divider)  \
/* NOTE: This macro is intended to be used with a SIGNED variable as 'averageValue' */\
    if( newValue > averageValue )\
    {   /* Average value too low. Inc with a proportional value */\
        averageValue += ((abs(newValue - averageValue) / divider) + 1);\
    }\
    else if ( newValue < averageValue )\
    {   /* Average value too high. Dec with a proportional value */\
        averageValue -= ((abs(newValue - averageValue) / divider) + 1);\
    }
#endif /* FloatingAverageFilter */    

#ifndef FloatingAverageFilterUnsigned    
/*
 ******************************************************************************
 * Macro
 * By successive calls this filter will generate a floating average value.
 * The average value will be changed proportional to the difference between
 * the new value and average value by using the 'divider' parameter.
 * An approximation of the influence from the 'divider' is that:
 *  - after 'divider' number of calls the value is on about 65 %
 *  - after 'divider*2' number of calls the value is on about 85 %
 *  - after 'divider*3' number of calls the value is on about 95 %
 *
 * This macro should be used when the first parameter 'averageValue' is
 * an unsigned variable.
 ******************************************************************************
 */
#define FloatingAverageFilterUnsigned(averageValue, newValue, divider)  \
/* NOTE: This macro is intended to be used with a UNSIGNED variable as 'averageValue' */\
    if( newValue > averageValue )\
    {   /* Average value too low. Inc with a proportional value */\
        averageValue += ((abs(newValue - averageValue) / divider) + 1);\
    }\
    else if (((abs(newValue - averageValue) / divider) + 1) > averageValue)\
    {   /* Average value too high. Dec with a proportional value */\
        averageValue = 0;\
    }\
    else\
    {   /* Average value too high. Dec with a proportional value */\
        averageValue -= ((abs(newValue - averageValue) / divider) + 1);\
    }
#endif /* FloatingAverageFilterUnsigned */    


/**
 ******************************************************************************
 * @brief   diffDown32 difference between decrementing values and handles wrap.
 * @param   start
 * @param   stop
 * @param   end
 *          reload value of counter
 * @returns difference
 ******************************************************************************
 */
static inline uint32 diffDown32( const uint32 start, const uint32 stop, const uint32 end )     { return stop<=start ? start-stop : (end-stop)+start;    } // count down

/**
 ******************************************************************************
 * @brief   diff32 difference between incrementing values and handles wrap.
 * @param   start
 * @param   stop
 * @param   end
 *          reload value of counter
 * @returns difference
 ******************************************************************************
 */
static inline uint32 diff32( const uint32 start, const uint32 stop, const uint32 end )         { return start<=stop ? stop-start : (end-start) + stop;  } // count up


/**
 ******************************************************************************
 * @brief       Mean filter.
 *              Similar to the floating average, but
 *              - not abs dependent, and
 *              - diff = 0 does not change mean value.
 * @param[in]   m
 *              mean value
 * @param[in]   diff
 *              difference between the values
 * @param[in]   den
 *              denominator, filter length
 * @returns     new mean value
 ******************************************************************************
 */
static inline sint32 meanFilter(const sint32 m, const sint32 diff, const sint32 den)        { return 0 == diff ? m : ( diff < 0 ? m + diff/den - 1 : m + diff/den + 1); }
static inline sint16 meanFilterS16(const sint16 m, const sint16 diff, const sint16 den)     { return 0 == diff ? m : ( diff < 0 ? m + diff/den - 1 : m + diff/den + 1); }


/**
 ******************************************************************************
 * @brief   divides a signed value with rounding
 ******************************************************************************
 */
#define DIV_S(n,d) ((((n) < 0) != ((d) < 0)) ? (((n) - (d)/2)/(d)) : (((n) + (d)/2)/(d)))
static inline sint32 DivS( const sint32 n, const sint32 d ) { return DIV_S(n,d); }


/**
 ******************************************************************************
 * @brief   divides a unsigned value with rounding
 ******************************************************************************
 */
#define DIV_U(n,d) ((((n) + (d)/2)/(d)))
static inline uint32 DivU( const uint32 n, const uint32 d ) { return DIV_U(n,d); }


/**
 ******************************************************************************
 * @brief   Converts a float or double to a signed 32 bits value 
 ******************************************************************************
 */
#define DOUBLE_TO_S32(f) ((sint32)(f >= 0.0 ? (f + 0.5) : (f - 0.5)))
static inline sint32 DoubleToS32( const double value ) { return DOUBLE_TO_S32(value); }
static inline sint32 FloatToS32( const float value ) { return DOUBLE_TO_S32(value); }


/**
 ******************************************************************************
 * @brief   Converts a signed value to a double
 ******************************************************************************
 */
#define S32_TO_DOUBLE(i) ((double) (i) )
#define S32_TO_FLOAT(i) ((float) (i) )
static inline double S32ToDouble( const sint32 value ) { return (double)value; }
static inline float S32ToFloat( const sint32 value ) { return (float)value; }

/**
 ******************************************************************************
 * @brief  Get sign of (x). Returns 1 for positive integers and -1 for negative integers
 ******************************************************************************
 */
#define SIGN(x) (int)( ((x) >= 0) - ((x) < 0) )

#endif /* ROBOTICCORE_GENERIC_DEFINE_ROBOTICUTILS_H_ */
