/**
 ******************************************************************************
 * @file      DrawManager.c
 *
 * @brief     DrawManager module implementation.
 ******************************************************************************
 */
/*
 ------------------------------------------------------------------------------
 Include files
 ------------------------------------------------------------------------------
 */
#include "DrawManager.h"
#include "IDrawManagerCfg.h"

#include "CircularBuffer.h"
#include "HmiLog.h"
#include "IDraw.h"
#include "IOs.h"
#include "IScheduler.h"
#include "ISoftwareException.h"
#include "RoboticTypes.h"

/*
 ------------------------------------------------------------------------------
 Local defines
 ------------------------------------------------------------------------------
 */

#define DRAWMANAGER_SET_BUFFER_SIZE  ( (1+DRAWMANAGERCFG_MAX_SET_SIZE) * sizeof(SetElement) )

/*
-------------------------------------------------------------------------------
    Local types
-------------------------------------------------------------------------------
*/

/**
 * Packs of parameters for all drawing commands
 * e.q. DrawImage, DrawRectangle
 */
typedef union
{
    struct
    {
        tIImageDB_Id    id;
        tIDraw_Position pos;
    } drawImage;
    struct
    {
        tIDraw_Rectangle rect;
        tIDraw_Colour    col;
    } drawRectangle,
      drawFilledRectangle;
    struct
    {
        tIDraw_Rectangle rect;
        tIDraw_Colour    col;
        uint16           borderThickness;
    } drawThickRectangle;
} tDrawManager_CommandParams;

/**
 * Drawing command.
 */
typedef bool ( *tDrawManager_Command )( const tDrawManager_CommandParams* );

/**
 * Drawing command object capable of capturing parameters in scope.
 */
typedef struct
{
    tDrawManager_Command       command;
    tDrawManager_CommandParams params;
} tDrawManager_LambdaCommand;


typedef CircularBuffer              SetContainer;
typedef SetContainer*               SetHandler;
typedef tDrawManager_LambdaCommand  SetElement;
typedef uint8 SetBuffer[ DRAWMANAGER_SET_BUFFER_SIZE ];


typedef struct
{
    bool fInDrawing; //!< inDrawing Set is being drawn now

    struct
    {
        SetContainer inDrawing;
        SetContainer closed;
        SetContainer open;
    } container; /**!< All Sets */

    struct
    {
        SetHandler inDrawing;
        SetHandler closed;
        SetHandler open;
    } set; /**!< All Set handlers */

    struct
    {
        SetBuffer inDrawing;
        SetBuffer closed;
        SetBuffer open;
    } buffer; /**!< All Set buffers */

    tIOs_MutexId mutexId;
    tEventCallback redrawCallbacks[ MAX_NUM_REDRAW_CALLBACKS ]; /* Pointer to registered callback function */
} tDrawManager_Vars;

/*
 ------------------------------------------------------------------------------
 Private data
 ------------------------------------------------------------------------------
 */

static tDrawManager_Vars vars =
    {
     .fInDrawing = false,

     .set =
     {
      .inDrawing = &vars.container.inDrawing,
      .closed = &vars.container.closed,
      .open = &vars.container.open
     },

     .mutexId = 0
    };

/*
 ------------------------------------------------------------------------------
 Private function prototypes
 ------------------------------------------------------------------------------
 */

static void DrawSet( tEvent event );
static void OnDrawSetFinished( tEvent event );
static void OnDrawElementFinished( tEvent event );

static bool ExecuteLambda( const tDrawManager_LambdaCommand* lambda );
static bool DrawImage( const tDrawManager_CommandParams* commandParams );
static bool DrawRectangle( const tDrawManager_CommandParams* commandParams );
static bool DrawThickRectangle( const tDrawManager_CommandParams* commandParams );
static bool DrawFilledRectangle( const tDrawManager_CommandParams* commandParams );

static void SetInit( SetHandler set, SetBuffer buffer );
static void SetPushElem( SetHandler set, const SetElement* element );
static bool SetPopElem( SetHandler set, SetElement* element );
static void SetClear( SetHandler set );
static bool SetEmpty( const SetHandler set );
static void SetSwap( SetHandler* lhs, SetHandler* rhs );


/*
 ------------------------------------------------------------------------------
 Implementation of interface functions
 ------------------------------------------------------------------------------
 */

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IDrawManager_Init( void )
{
    static bool initialized = false;
    if ( initialized )
        return;
    initialized = true;

    IDraw_Init();
    HmiLog_Init();
    IOs_Init();
    IScheduler_Init();
    ISoftwareException_Init();

    for (uint8 i = 0; i < MAX_NUM_REDRAW_CALLBACKS; i++)
    {
        vars.redrawCallbacks[ i ] = NULL;
    }

    SetInit( vars.set.inDrawing, vars.buffer.inDrawing );
    SetInit( vars.set.closed,    vars.buffer.closed );
    SetInit( vars.set.open,      vars.buffer.open );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IDrawManager_Start( void )
{
    static bool started = false;
    if ( started )
        return;
    started = true;

    IDraw_Start();
    HmiLog_Start();
    IOs_Start();
    IScheduler_Start();
    ISoftwareException_Start();

    /* Register DrawSet to LOW priority thread. */
    IScheduler_SetCallbackPriority( DrawSet, IOS_PRIO_MED_LOW );

    bool mutexCreated = IOs_MutexCreate( &vars.mutexId );
    SOFTWARE_EXCEPTION_ASSERT( true == mutexCreated );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IDrawManager_BeginSet( void )
{
    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IDrawManager_EndSet( void )
{
    if ( !IOs_MutexLock( vars.mutexId, IOS_TIMEOUT_FOREVER ) )
    {
        ILOGIF( DOLOG( ILOG_LEVEL_ERROR ), ILOG_LEVEL_ERROR, "Unable to lock mutex", "^" );
        return false;
    }

    // Closing open Set and discarding Set which is waiting for drawing.
    SetClear( vars.set.closed );
    SetSwap( &vars.set.closed, &vars.set.open );

    // inDrawing Set is not in drawing
    if ( !vars.fInDrawing )
    {
        // Start drawing Set which is waiting for drawing
        vars.fInDrawing = true;
        SetSwap( &vars.set.inDrawing, &vars.set.closed );
        tEvent event = { .id = IDRAW_EVENT_DONE };
        IScheduler_PushEvent( event, DrawSet );
    }

    if ( !IOs_MutexUnlock( vars.mutexId ) )
    {
        ILOGIF( DOLOG( ILOG_LEVEL_ERROR ), ILOG_LEVEL_ERROR, "Unable to unlock mutex", "^" );
        return false;
    }
    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IDrawManager_DrawImage( tIImageDB_Id id, tIDraw_Position* pPos )
{
    tDrawManager_LambdaCommand lambda =
    {
     .command = DrawImage,
     .params  =
     {
      .drawImage =
      {
       .id = id,
       .pos =
       {
        .x = pPos->x,
        .y = pPos->y
       }
      }
     }
    };

    // Postpone drawing until Set is open
    SetPushElem( vars.set.open, &lambda );
    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IDrawManager_DrawRectangle( tIDraw_Rectangle* pRect, tIDraw_Colour* pCol )
{
    tDrawManager_LambdaCommand lambda =
    {
     .command = DrawRectangle,
     .params  =
     {
      .drawRectangle =
      {
       .rect =
       {
        .topLeft =
        {
         .x = pRect->topLeft.x,
         .y = pRect->topLeft.y
        },
        .bottomRight =
        {
         .x = pRect->bottomRight.x,
         .y = pRect->bottomRight.y
        }
       },
       .col =
       {
        .r = pCol->r,
        .g = pCol->g,
        .b = pCol->b,
        .a = pCol->a
       }
      }
     }
    };

    // Postpone drawing until Set is open
    SetPushElem( vars.set.open, &lambda );

    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IDrawManager_DrawThickRectangle(
        tIDraw_Rectangle* pRect, tIDraw_Colour* pCol, uint16 borderThickness )
{
    tDrawManager_LambdaCommand lambda =
    {
     .command = DrawThickRectangle,
     .params  =
     {
      .drawThickRectangle =
      {
       .rect =
       {
        .topLeft =
        {
         .x = pRect->topLeft.x,
         .y = pRect->topLeft.y
        },
        .bottomRight =
        {
         .x = pRect->bottomRight.x,
         .y = pRect->bottomRight.y
        }
       },
       .col =
       {
        .r = pCol->r,
        .g = pCol->g,
        .b = pCol->b,
        .a = pCol->a
       },
       .borderThickness = borderThickness
      }
     }
    };

    // Postpone drawing until Set is open
    SetPushElem( vars.set.open, &lambda );

    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool IDrawManager_DrawFilledRectangle( tIDraw_Rectangle* pRect, tIDraw_Colour* pCol )
{
    tDrawManager_LambdaCommand lambda =
    {
     .command = DrawFilledRectangle,
     .params  =
     {
      .drawFilledRectangle =
      {
       .rect =
       {
        .topLeft =
        {
         .x = pRect->topLeft.x,
         .y = pRect->topLeft.y
        },
        .bottomRight =
        {
         .x = pRect->bottomRight.x,
         .y = pRect->bottomRight.y
        }
       },
       .col =
       {
        .r = pCol->r,
        .g = pCol->g,
        .b = pCol->b,
        .a = pCol->a
       }
      }
     }
    };

    // Postpone drawing until Set is open
    SetPushElem( vars.set.open, &lambda );

    return true;
}


/*
 ------------------------------------------------------------------------------
 Implementation of private functions
 ------------------------------------------------------------------------------
 */

/*
 ******************************************************************************
 * Pop element from inDrawing Set for drawing.
 * Close function and wait for callback on drawing process finish.
 ******************************************************************************
 */
static void DrawSet( tEvent event )
{
    SetElement element;

    // get Set element for drawing
    if ( SetPopElem( vars.set.inDrawing, &element ) )
    {
        if ( !ExecuteLambda( &element ) )
        {
            // failed to draw element, draw next one
            event.id = IDRAW_EVENT_FAILED;
            IScheduler_PushEvent( event, DrawSet );
        }
    }
    else // no element for drawing left
    {
        // should be protected by mutex if assigning bool value is not atomic
        vars.fInDrawing = false;
        // drawing of inDrawing Set completed
        event.id = IDRAW_EVENT_DONE;
        IScheduler_PushEvent( event, OnDrawSetFinished );
    }
}

tReturn IDrawManager_RegisterRedrawCallback( tEventCallback eventCallback )
{
    uint8 i;
    if ( NULL == eventCallback )
    {
        /* NULL Pointer */
        return E_UNDEFINED;
    }

    for (i = 0; i < MAX_NUM_REDRAW_CALLBACKS; i++)
    {
        if(vars.redrawCallbacks[i] == eventCallback)
        {
            return OK; // already registered
        }
        else if(vars.redrawCallbacks[i] == NULL)
        {
            break; // found a free slot
        }
    }
    // If no slots were available i will point to last occupied slot
    if (NULL != vars.redrawCallbacks[i])
    {
        /* Return Warning Instead */
        return E_UNDEFINED;
    }

    /* Assign */
    vars.redrawCallbacks[i] = eventCallback;

    return OK;
}

static void SendEvent( tIDrawManagerEvents event )
{
    tEvent ev;
    ev.id = event;
    for (uint8 i = 0; i < MAX_NUM_REDRAW_CALLBACKS; i++)
    {
        if (NULL != vars.redrawCallbacks[i])
        {
            vars.redrawCallbacks[i](ev);
        }
    }
}
/*
 ******************************************************************************
 * When inDrawing Set was drawn, start drawing closed Set.
 ******************************************************************************
 */
static void OnDrawSetFinished( tEvent event )
{
    if ( !IOs_MutexLock( vars.mutexId, IOS_TIMEOUT_FOREVER ) )
    {
        ILOGIF( DOLOG( ILOG_LEVEL_ERROR ), ILOG_LEVEL_ERROR, "Unable to lock mutex", "^" );
        return;
    }

    //When everything is drawn
    IDraw_FrameCompleted();

    // inDrawing Set is not in drawing
    // and there is closed Set to draw
    if ( !vars.fInDrawing && !SetEmpty( vars.set.closed ) )
    {
        // Start drawing Set which is waiting for drawing
        vars.fInDrawing = true;
        SetSwap( &vars.set.inDrawing, &vars.set.closed );
        IScheduler_PushEvent( event, DrawSet );
        //
    }

    if ( !IOs_MutexUnlock( vars.mutexId ) )
    {
        ILOGIF( DOLOG( ILOG_LEVEL_DEBUG ), ILOG_LEVEL_ERROR, "Unable to unlock mutex", "^" );
    }

    SendEvent(IDRAWMANAGER_EVENT_REDRAW);
}

/*
 ******************************************************************************
 * When element from inDrawing Set was drawn, start drawing another one.
 ******************************************************************************
 */
void OnDrawElementFinished( tEvent event )
{
    IScheduler_PushEvent( event, DrawSet );
}


/*
 ******************************************************************************
 * Call command with parameters captured in lambda
 ******************************************************************************
 */
static bool ExecuteLambda( const tDrawManager_LambdaCommand* lambda )
{
    return lambda->command( &lambda->params );
}

/*
 ******************************************************************************
 * Call Image draw with command parameters
 ******************************************************************************
 */
static bool DrawImage( const tDrawManager_CommandParams* commandParams )
{
    return IDraw_Image(
            commandParams->drawImage.id,
            (tIDraw_Position*) &commandParams->drawImage.pos,
            OnDrawElementFinished );
}

/*
 ******************************************************************************
 * Call Rectangle draw with command parameters
 ******************************************************************************
 */
static bool DrawRectangle( const tDrawManager_CommandParams* commandParams )
{
    return IDraw_Rectangle(
            (tIDraw_Rectangle*) &commandParams->drawRectangle.rect,
            (tIDraw_Colour*) &commandParams->drawRectangle.col,
            OnDrawElementFinished );
}

/*
 ******************************************************************************
 * Call thick framed Rectangle draw with command parameters
 ******************************************************************************
 */
static bool DrawThickRectangle( const tDrawManager_CommandParams* commandParams )
{
    return IDraw_ThickRectangle(
            (tIDraw_Rectangle*) &commandParams->drawThickRectangle.rect,
            (tIDraw_Colour*) &commandParams->drawThickRectangle.col,
            commandParams->drawThickRectangle.borderThickness,
            OnDrawElementFinished );
}

/*
 ******************************************************************************
 * Call Filled Rectangle draw with command parameters
 ******************************************************************************
 */
static bool DrawFilledRectangle( const tDrawManager_CommandParams* commandParams )
{
    return IDraw_DrawFilledRect(
            (tIDraw_Rectangle*) &commandParams->drawFilledRectangle.rect,
            (tIDraw_Colour*) &commandParams->drawFilledRectangle.col,
            OnDrawElementFinished );
}


/*
 ******************************************************************************
 * Initialize Set. Assign buffer to Set.
 ******************************************************************************
 */
static void SetInit( SetHandler set, SetBuffer buffer )
{
    CircularBuffer_Init( set, buffer, DRAWMANAGER_SET_BUFFER_SIZE );
}

/*
 ******************************************************************************
 * Push element into Set.
 ******************************************************************************
 */
static void SetPushElem( SetHandler set, const SetElement* element )
{
    const uint32 pushedSize =
            CircularBuffer_Push( set, (const uint8*) element, sizeof(SetElement) );

    // DrawManager Set Buffer to small!
    // extend DRAWMANAGERCFG_MAX_SET_SIZE
    SOFTWARE_EXCEPTION_ASSERT( sizeof(SetElement) == pushedSize );
}

/*
 ******************************************************************************
 * Pop element from Set.
 * Returns false if buffer empty.
 ******************************************************************************
 */
static bool SetPopElem( SetHandler set, SetElement* element )
{
    return CircularBuffer_Pop( set, (uint8*) element, sizeof(SetElement) );
}

/*
 ******************************************************************************
 * Remove all elements from Set.
 ******************************************************************************
 */
static void SetClear( SetHandler set )
{
    CircularBuffer_Clear( set );
}

/*
 ******************************************************************************
 * Check if Set is empty.
 ******************************************************************************
 */
static bool SetEmpty( const SetHandler set )
{
    return 0 == CircularBuffer_GetCount( set );
}

/*
 ******************************************************************************
 * Swap Sets between two Set handlers.
 ******************************************************************************
 */
static void SetSwap( SetHandler* lhs, SetHandler* rhs )
{
    SetHandler tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}


/*
 * needed for unittests
 */
uint32 DrawManager_GetSetSize(uint32 index)
{
    switch(index)
    {
        case 0:
        {
            return CircularBuffer_GetCount(vars.set.open);
        }
        case 1:
        {
            return CircularBuffer_GetCount(vars.set.closed);
        }
        case 2:
        {
            return CircularBuffer_GetCount(vars.set.inDrawing);
        }

    }

    return 0;

}

void DrawManager_AddToSet(uint32 index)
{
    tDrawManager_LambdaCommand testLambda =
        {
         .command = DrawImage,
         .params  =
         {
          .drawImage =
          {
           .id = 0,
           .pos =
           {
            .x = 0,
            .y = 0
           }
          }
         }
        };

    switch(index)
    {
        case 0:
        {
            CircularBuffer_Push( vars.set.open, (const uint8*) &testLambda, sizeof(testLambda) );
            break;
        }
        case 1:
        {
            CircularBuffer_Push( vars.set.closed, (const uint8*) &testLambda, sizeof(testLambda) );
            break;
        }
        case 2:
        {
            CircularBuffer_Push( vars.set.inDrawing, (const uint8*) &testLambda, sizeof(testLambda) );
            break;
        }

    }

    return;
}

void DrawManager_SetDrawing(bool isDrawing)
{
    vars.fInDrawing = isDrawing;
}

