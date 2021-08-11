/**
 ******************************************************************************
 * @file      SafeVariables.c
 *
 * @brief     Implementation of Safe Variables.
 ******************************************************************************
 */
/*
 -------------------------
 Include files
 -------------------------
 */
#include "SafeVariables.h"
#include "IOs.h"
#include "ISoftwareException.h"


/*
-------------------------
   Defines
-------------------------
*/
#define SAFE_TRUE  ((uint8)(0x55))
#define SAFE_FALSE ((uint8)(~SAFE_TRUE))


/*
-------------------------
   Private prototypes
-------------------------
*/
static void wdReset( void );


/*
 -------------------------
 Public functions
 -------------------------
 */
bool SafeBool_Get( SafeBool* const me )
{
    const SafeBool copy = { .atom = me->atom };
    
    if ( (copy.attribute^copy.attributeInverted) == (SAFE_TRUE^SAFE_FALSE) )
    {     
        if ( SAFE_TRUE  == copy.attribute )
        {
    	    return true;  
        }
        
        if ( SAFE_FALSE == copy.attribute )
        {
    	    return false;  
        }
    }
    
    wdReset();
    return 0xFF;
}


sint8 SafeSint8_Get( SafeSint8* const me )
{
    const SafeSint8 copy = {.atom = me->atom};

    if( ( (copy.attribute^copy.attributeInverted) & 0xFF) == 0xFF )
    {
    	return copy.attribute;
    }
    
    wdReset();
    return 0xFF;
}


sint16 SafeSint16_Get( SafeSint16* const me )
{
    const SafeSint16 copy = {.atom = me->atom};

    if ( ( (copy.attribute^copy.attributeInverted) & 0xFFFF ) == 0xFFFF )
    {
    	return copy.attribute;
    }
    
    wdReset();
    return 0xFFFF;
}


sint32 SafeSint32_Get( SafeSint32* const me )
{
    sint32 value;
    sint32 valueInv;

    IOs_EnterCritical();

    value = me->attribute;
    valueInv = me->attributeInverted;
    
    IOs_ExitCritical();
    
    if ( (value^valueInv) == 0xFFFFFFFF )
    {
    	return value;  
    }
    
    wdReset();
    return 0xFFFFFFFF;
}



sint64 SafeSint64_Get( SafeSint64* const me )
{
    sint64 value;
    sint64 valueInv;

    IOs_EnterCritical();
    
    value = me->attribute;
    valueInv = me->attributeInverted;
    
    IOs_ExitCritical();
    
    if ( (value^valueInv) == 0xFFFFFFFFFFFFFFFF )
    {
    	return value;  
    }
    
    wdReset();
    return 0xFFFFFFFFFFFFFFFF;
}


uint8 SafeUint8_Get( SafeUint8* const me )
{
    const SafeUint8 copy = {.atom = me->atom};
    
    if ( ( (copy.attribute^copy.attributeInverted) & 0xFF ) == 0xFF )
    {
    	return copy.attribute;
    }
    
    wdReset();
    return 0xFF;
}


uint16 SafeUint16_Get( SafeUint16* const me )
{
    const SafeUint16 copy = {.atom = me->atom};

    if ( ( (copy.attribute^copy.attributeInverted) & 0xFFFF ) == 0xFFFF )
    {
    	return copy.attribute;
    }
    
    wdReset();
    return 0xFFFF;
}

uint32 SafeUint32_Get( SafeUint32* const me )
{
    uint32 value;
    uint32 valueInv;

    IOs_EnterCritical();
    
    value = me->attribute;
    valueInv = me->attributeInverted;
    
    IOs_ExitCritical();
    
    if ( (value^valueInv) == 0xFFFFFFFF )
    {
    	return value;  
    }
    
    wdReset();
    return 0xFFFFFFFF;
}


uint64 SafeUint64_Get( SafeUint64* const me )
{
    uint64 value;
    uint64 valueInv;

    IOs_EnterCritical();
    
    value = me->attribute;
    valueInv = me->attributeInverted;
    
    IOs_ExitCritical();
    
    if ( (value^valueInv) == 0xFFFFFFFFFFFFFFFF )
    {
    	return value;  
    }
    
    wdReset();
    return 0xFFFFFFFFFFFFFFFF;
}


void* SafeVoidPtr_Get( SafeVoidPtr* const me )
{
    uint32 value;
    uint32 valueInv;

    IOs_EnterCritical();

    value = (uint32) (me->attribute);
    valueInv = (uint32) (me->attributeInverted);

    IOs_ExitCritical();

    if ( (value^valueInv) == 0xFFFFFFFF )
    {
      return (void*)value;
    }

    wdReset();
    return NULL;
}


void SafeBool_Set( SafeBool* me, bool argument )
{
    const uint8 data = argument ? SAFE_TRUE : SAFE_FALSE;
    const SafeBool copy = { .attribute = data, .attributeInverted = ~data };

    me->atom = copy.atom;
}


void SafeSint8_Set( SafeSint8* me, sint8 argument )
{
    const SafeSint8 copy = { .attribute = argument, .attributeInverted = ~argument };
    me->atom = copy.atom;
}


void SafeSint16_Set( SafeSint16* me, sint16 argument )
{
    const SafeSint16 copy = { .attribute = argument, .attributeInverted = ~argument };
    me->atom = copy.atom;
}


void SafeSint32_Set( SafeSint32* me, sint32 argument )
{
    IOs_EnterCritical();
    
    me->attribute = argument;
    me->attributeInverted = ~me->attribute; 
    
    IOs_ExitCritical();
}


void SafeSint64_Set( SafeSint64* me, sint64 argument )
{
    IOs_EnterCritical();

    me->attribute = argument;
    me->attributeInverted = ~me->attribute;

    IOs_ExitCritical();
}


void SafeUint8_Set( SafeUint8* me, uint8 argument )
{
    const SafeUint8 copy = { .attribute = argument, .attributeInverted = ~argument };
    me->atom = copy.atom;
}


void SafeUint16_Set( SafeUint16* me, uint16 argument )
{
    const SafeUint16 copy = { .attribute = argument, .attributeInverted = ~argument };
    me->atom = copy.atom;
}


void SafeUint32_Set( SafeUint32* me, uint32 argument )
{
    IOs_EnterCritical();

    me->attribute = argument;
    me->attributeInverted = ~me->attribute;

    IOs_ExitCritical();
}


void SafeUint64_Set( SafeUint64* me, uint64 argument )
{
    IOs_EnterCritical();

    me->attribute = argument;
    me->attributeInverted = ~me->attribute;

    IOs_ExitCritical();
}


void SafeVoidPtr_Set( SafeVoidPtr* me, void* argument )
{
    IOs_EnterCritical();

    me->attribute = argument;
    me->attributeInverted = (void*)(~((uint32)(me->attribute)));
 
    IOs_ExitCritical();
}


/*
-------------------------
   Private functions
-------------------------
*/
void wdReset( void )
{
    #ifndef UT_WIN
    SOFTWARE_EXCEPTION();
    #endif
}
