/*
 *  inputdriverXWINMSG: GUCEF input driver module for X11
 *  Copyright (C) 2002 - 2012.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <set>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_INPUT_XWINMSG_CEVENTLISTNER_H
#include "inputdriverXWINMSG_CEventListner.h"
#define GUCEF_INPUT_XWINMSG_CEVENTLISTNER_H
#endif /* GUCEF_INPUT_XWINMSG_CEVENTLISTNER_H ? */

#include "inputdriverXWINMSG.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace INPUT {
namespace XWINMSG {
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const TVersion version = { 1, 0, 0, 0 };
#define DRIVER_NAME       "XWINMSG\0"
#define DRIVER_COPYRIGHT  "Copyright (C) Dinand Vanvelzen. 2002 - 2012. License: LGPLv3.\0"

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef std::set< CEventListner* > TEventListnerSet;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
GetArgListItem( const int argc    ,
                const char** argv ,
                const char* key   )
{
    int i;
    for ( i=0; i<argc-1; ++i )
    {
        if ( strcmp( argv[ i ], key ) == 0 )
        {
            return argv[ i+1 ];
        }
        ++i;
    }
    return 0;
}

/*--------------------------------------------------------------------------*/

UInt32
ParseArgListItemUInt32( const int argc    ,
                        const char** argv ,
                        const char* key   )
{
    const char* value = GetArgListItem( argc, argv, key );
    if ( GUCEF_NULL != value )
    {
        UInt32 result = 0UL;
        sscanf( value, "%ul", &result );
        return result;
    }
    return 0;
}

/*--------------------------------------------------------------------------*/

void*
ParseArgListItemPointer( const int argc    ,
                         const char** argv ,
                         const char* key   )
{
    const char* value = GetArgListItem( argc, argv, key );
    if ( GUCEF_NULL != value )
    {
        void* result = GUCEF_NULL;
        sscanf( value, "%p", &result );
        return result;
    }
    return 0;
}

/*--------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Init( void** plugdata   ,
                      const int argc    ,
                      const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    *plugdata = new TEventListnerSet();
    return 1;
}

/*--------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Shutdown( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TEventListnerSet* eventListnerSet = (TEventListnerSet*) plugdata;
    delete eventListnerSet;

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
    // we are updated automatically
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
    // input sanity check
    if ( GUCEF_NULL == plugdata ) return 0;

    ::Window window = ParseArgListItemUInt32( argc, argv, "-WINDOW" );
    if ( 0 == window ) return 0;

    TEventListnerSet* eventListnerSet = (TEventListnerSet*) plugdata;
    CEventListner* eventListner = new CEventListner( window, callbacks );
    eventListnerSet->insert( eventListner );

    *contextdata = eventListner;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Created X11 input context for window " + CORE::UInt32ToString( window ) );

    return 1;
}

/*--------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_DestroyContext( void* plugdata    ,
                                void* contextdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    // input sanity check
    if ( GUCEF_NULL == plugdata ) return 0;

    TEventListnerSet* eventListnerSet = (TEventListnerSet*) plugdata;
    CEventListner* eventListner = (CEventListner*) contextdata;
    eventListnerSet->erase( eventListner );
    delete eventListner;

    return 1;
}

/*--------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetMousePos( void* plugdata    ,
                             void* contextdata ,
                             UInt32* xpos      ,
                             UInt32* ypos      ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return 0;
}

/*--------------------------------------------------------------------------*/

UInt8* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetKeyBoardKeyStates( void* plugdata    ,
                                      void* contextdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return 0;
}

/*--------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetMouseButtonPressedState( void* plugdata           ,
                                            void* contextdata        ,
                                            const UInt32 buttonindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return 0;
}

/*--------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_GetKeyboardKeyPressedState( void* plugdata        ,
                                            void* contextdata     ,
                                            const UInt32 keyindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return 0;
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
}; /* namespace XWINMSG */
}; /* namespace INPUT */
}; /* namespace GUCEF */
#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/
