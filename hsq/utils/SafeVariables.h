/**
 ******************************************************************************
 * @file      SafeVariables.h
 *
 * @brief     Interface for Safe Variables.
 *			  The inverted value is stored with the real value and a 
 *            check is performed to validate the data consistency.
 ******************************************************************************
 */
#ifndef SafeVariables_H
#define SafeVariables_H


/*
 ------------------------------------------------------------------------------
 Include files
 ------------------------------------------------------------------------------
 */
#include <RoboticTypes.h>

/*
 ------------------------------------------------------------------------------
 Type definitions
 ------------------------------------------------------------------------------
 */
typedef struct SafeBool_t SafeBool;
struct SafeBool_t 
{
    union 
	{
        uint16 atom;
        struct {
			uint8 attribute;
			uint8 attributeInverted;
        };
    };
};

typedef struct SafeSint8_t SafeSint8;
struct SafeSint8_t 
{
    union
    {
        uint16 atom;
        struct
        {
			sint8 attribute;
			sint8 attributeInverted;
        };
    };
};

typedef struct SafeSint16_t SafeSint16;
struct SafeSint16_t 
{
    union
    {
        uint32 atom;
        struct
        {
			sint16 attribute;
			sint16 attributeInverted;
        };
    };
};

typedef struct SafeSint32_t SafeSint32;
struct SafeSint32_t 
{
    sint32 attribute;
    sint32 attributeInverted;
};


typedef struct SafeSint64_t SafeSint64;
struct SafeSint64_t 
{
    sint64 attribute;
    sint64 attributeInverted;
};

typedef struct SafeUint8_t SafeUint8;
struct SafeUint8_t {
    union
    {
        uint16 atom;
        struct
        {
			uint8 attribute;
			uint8 attributeInverted;
        };
    };
};

typedef struct SafeUint16_t SafeUint16;
struct SafeUint16_t 
{
    union
    {
        uint32 atom;
        struct
        {
			uint16 attribute;
			uint16 attributeInverted;
        };
    };
};

typedef struct SafeUint32_t SafeUint32;
struct SafeUint32_t 
{
    uint32 attribute;
    uint32 attributeInverted;
};

typedef struct SafeUint64_t SafeUint64;
struct SafeUint64_t 
{
    uint64 attribute;
    uint64 attributeInverted;
};

typedef struct SafeVoidPtr_t SafeVoidPtr;
struct SafeVoidPtr_t 
{
    void* attribute;
    void* attributeInverted;
};


/*
 ------------------------------------------------------------------------------
 Public functions
 ------------------------------------------------------------------------------
 */
bool SafeBool_Get( SafeBool* const me );
sint8 SafeSint8_Get( SafeSint8* const me );
sint16 SafeSint16_Get( SafeSint16* const me );
sint32 SafeSint32_Get( SafeSint32* const me );
sint64 SafeSint64_Get( SafeSint64* const me );
uint8  SafeUint8_Get( SafeUint8* const me );
uint16 SafeUint16_Get( SafeUint16* const me );
uint32 SafeUint32_Get( SafeUint32* const me );
uint64 SafeUint64_Get( SafeUint64* const me );
void* SafeVoidPtr_Get( SafeVoidPtr* const me );

void SafeBool_Set(SafeBool* me, bool argument);
void SafeSint8_Set(SafeSint8* me, sint8 argument);
void SafeSint16_Set(SafeSint16* me, sint16 argument);
void SafeSint32_Set( SafeSint32* me, sint32 argument );
void SafeSint64_Set( SafeSint64* me, sint64 argument );
void SafeUint8_Set( SafeUint8* me, uint8 argument );
void SafeUint16_Set( SafeUint16* me, uint16 argument );
void SafeUint32_Set( SafeUint32* me, uint32 argument );
void SafeUint64_Set( SafeUint64* me, uint64 argument );
void SafeVoidPtr_Set( SafeVoidPtr* me, void* argument);


#endif	/* SafeVariables_H */
