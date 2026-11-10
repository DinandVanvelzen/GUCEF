/*
 *  inputdriverWAYLAND: GUCEF input driver for Wayland: the pointer and keyboard of the seat for the windows of guidriverWAYLAND
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_INPUT_WAYLAND_CWAYLANDINPUTCONTEXT_H
#include "inputdriverWAYLAND_CWaylandInputContext.h"
#define GUCEF_INPUT_WAYLAND_CWAYLANDINPUTCONTEXT_H
#endif /* GUCEF_INPUT_WAYLAND_CWAYLANDINPUTCONTEXT_H ? */

#include "inputdriverWAYLAND.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace INPUT {
namespace WAYLAND {
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const TVersion version = { 1, 0, 0, 0 };
#define DRIVER_NAME       "WAYLAND\0"
#define DRIVER_COPYRIGHT  "Copyright (C) Dinand Vanvelzen. 1998 - 2026. License: Apache 2.0.\0"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The value after a key in the arguments the input controller makes from the parameters of the context ("-WINDOW", "7") */
const char*
GetArgListItem( const int argc    ,
                const char** argv ,
                const char* key   )
{
    for ( int i=0; i<argc-1; ++i )
        if ( 0 == strcmp( argv[ i ], key ) )
            return argv[ i+1 ];
    return GUCEF_NULL;
}

}

/*--------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Init( void** plugdata   ,
                      const int argc    ,
                      const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    // the contexts keep their own state; nothing is connected until one is made
    *plugdata = GUCEF_NULL;
    return 1;
}

/*--------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Shutdown( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return 1;
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Name( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return DRIVER_NAME;
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Copyright( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return DRIVER_COPYRIGHT;
}

/*--------------------------------------------------------------------------*/

const TVersion* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Version( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return &version;
}

/*--------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Update( void* plugdata    ,
                        void* contextdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    // we are updated by the pulses of the application
    return 1;
}

/*--------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_CreateContext( void* plugdata                   ,
                               void** contextdata               ,
                               int argc                         ,
                               const char** argv                ,
                               const TInputCallbacks* callbacks ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( GUCEF_NULL == contextdata || GUCEF_NULL == callbacks )
        return 0;

    // the window context of guidriverWAYLAND gives the id of its surface as the WINDOW property
    const char* windowArg = GetArgListItem( argc, argv, "-WINDOW" );
    const UInt32 surfaceId = GUCEF_NULL != windowArg ? CORE::StringToUInt32( windowArg, 0 ) : 0;
    if ( 0 == surfaceId )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "inputdriverWAYLAND: The context needs the WINDOW parameter of a window of the Wayland window driver" );
        return 0;
    }

    CORE::CString problem;
    PLUGINGLUE::WAYLAND::CWaylandConnection* connection = PLUGINGLUE::WAYLAND::CWaylandConnection::Acquire( &problem );
    if ( GUCEF_NULL == connection )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "inputdriverWAYLAND: No connection to the compositor: " << problem );
        return 0;
    }
    if ( GUCEF_NULL == connection->FindSurface( surfaceId ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "inputdriverWAYLAND: There is no window with the surface " << surfaceId );
        PLUGINGLUE::WAYLAND::CWaylandConnection::Release( connection );
        return 0;
    }

    CWaylandInputContext* context = new CWaylandInputContext( *connection, surfaceId, *callbacks );
    context->SetReleaseConnectionOnDestroy( true );
    *contextdata = context;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Created Wayland input context for window surface " << surfaceId );
    return 1;
}

/*--------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_DestroyContext( void* plugdata    ,
                                void* contextdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    delete static_cast< CWaylandInputContext* >( contextdata );
    return 1;
}

/*--------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetMousePos( void* plugdata    ,
                             void* contextdata ,
                             UInt32* xpos      ,
                             UInt32* ypos      ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    CWaylandInputContext* context = static_cast< CWaylandInputContext* >( contextdata );
    Int32 x = 0, y = 0;
    if ( GUCEF_NULL == context || GUCEF_NULL == xpos || GUCEF_NULL == ypos || !context->GetMousePosition( x, y ) )
        return 0;
    *xpos = x > 0 ? (UInt32) x : 0;
    *ypos = y > 0 ? (UInt32) y : 0;
    return 1;
}

/*--------------------------------------------------------------------------*/

UInt8* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetKeyBoardKeyStates( void* plugdata    ,
                                      void* contextdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    CWaylandInputContext* context = static_cast< CWaylandInputContext* >( contextdata );
    return GUCEF_NULL != context ? context->GetKeyStates() : GUCEF_NULL;
}

/*--------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetMouseButtonPressedState( void* plugdata           ,
                                            void* contextdata        ,
                                            const UInt32 buttonindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    CWaylandInputContext* context = static_cast< CWaylandInputContext* >( contextdata );
    return GUCEF_NULL != context && context->IsButtonDown( buttonindex ) ? 1 : 0;
}

/*--------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetKeyboardKeyPressedState( void* plugdata        ,
                                            void* contextdata     ,
                                            const UInt32 keyindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    CWaylandInputContext* context = static_cast< CWaylandInputContext* >( contextdata );
    return GUCEF_NULL != context && context->IsKeyDown( (KeyCode) keyindex ) ? 1 : 0;
}

/*--------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetDeviceBoolState( void* plugdata          ,
                                    void* contextdata       ,
                                    const UInt32 deviceid   ,
                                    const UInt32 stateindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    /* not supported */
    return 0;
}

/*--------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetDeviceVarState( void* plugdata          ,
                                   void* contextdata       ,
                                   const UInt32 deviceid   ,
                                   const UInt32 stateindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    /* not supported */
    return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace WAYLAND */
}; /* namespace INPUT */
}; /* namespace GUCEF */
#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/
