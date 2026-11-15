/*
 *  inputdriverOIS: GUCEF plugin module proving an input driver backend
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
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
#include <stdio.h>
#include <vector>

#ifndef OIS_OISALL_H
#include "OIS.h"
#define OIS_OISALL_H
#endif /* OIS_OISALL_H ? */

#ifndef GUCEF_INPUT_OISMOUSELISTENER_H
#include "gucefINPUT_OISMouseListener.h"
#define GUCEF_INPUT_OISMOUSELISTENER_H
#endif /* GUCEF_INPUT_OISMOUSELISTENER_H ? */

#ifndef GUCEF_INPUT_OISKEYBOARDLISTENER_H
#include "gucefINPUT_OISKeyboardListener.h"
#define GUCEF_INPUT_OISKEYBOARDLISTENER_H
#endif /* GUCEF_INPUT_OISKEYBOARDLISTENER_H ? */

#ifndef GUCEF_INPUT_OISJOYSTICKLISTENER_H
#include "gucefINPUT_OISJoystickListener.h"
#define GUCEF_INPUT_OISJOYSTICKLISTENER_H
#endif /* GUCEF_INPUT_OISJOYSTICKLISTENER_H ? */

#include "DLLMainInputDriverOIS.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const TVersion version = { 1, 0, 0, 0 };
#define DRIVER_NAME       "OIS\0"
#define DRIVER_COPYRIGHT  "Copyright (C) Dinand Vanvelzen. 2002 - 2008. LGPL.\0"

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SContextData
{
    TInputCallbacks callbacks;
    OIS::InputManager* inputManager;
    std::vector< OIS::Keyboard* > keyboards;
    std::vector< OIS::Mouse* > mice;
    std::vector< OIS::JoyStick* > joysticks;
    OIS::JoyStickListener* joystickListener;
    OIS::MouseListener* mouseListener;
    OIS::KeyListener* keyboardListener;
};
typedef struct SContextData TContextData;

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
        sscanf( value, "%d", &result );
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

/*---------------------------------------------------------------------------*/

void
SetWindowSizeForMice( TContextData* data ,
                      UInt32 width       ,
                      UInt32 height      )
{
    UInt32 miceCount = (UInt32) data->mice.size();
    std::vector< OIS::Mouse* >::iterator i = data->mice.begin();
    while ( i != data->mice.end() )
    {
        const OIS::MouseState& mouseState = (*i)->getMouseState();
        mouseState.width = width;
        mouseState.height = height;
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Init( void** plugdata   ,
                      const int argc    ,
                      const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    // no special driver wide initialization needed
    return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Shutdown( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    // no special driver wide shutdown needed
    return 1;
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Name( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return DRIVER_NAME;
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Copyright( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return DRIVER_COPYRIGHT;
}

/*---------------------------------------------------------------------------*/

const TVersion* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Version( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return &version;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_Update( void* plugdata    ,
                        void* contextdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TContextData* data = static_cast< TContextData* >( contextdata );

    UInt32 miceCount = (UInt32) data->mice.size();
    for ( UInt32 i=0; i<miceCount; ++i )
    {
        data->mice[ i ]->capture();
    }
    UInt32 keyboardCount = (UInt32) data->keyboards.size();
    for ( UInt32 i=0; i<keyboardCount; ++i )
    {
        data->keyboards[ i ]->capture();
    }
    UInt32 joystickCount = (UInt32) data->joysticks.size();
    for ( UInt32 i=0; i<joystickCount; ++i )
    {
        data->joysticks[ i ]->capture();
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_CreateContext( void* plugdata                   ,
                               void** contextdata               ,
                               int argc                         ,
                               const char** argv                ,
                               const TInputCallbacks* callbacks ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    size_t handle = (size_t) ParseArgListItemUInt32( argc, argv, "-WINDOW" );
    if ( 0 != handle )
    {
        TContextData* data = new TContextData;
        data->inputManager = GUCEF_NULL;
        data->callbacks = *callbacks;

        try
        {
            data->inputManager = OIS::InputManager::createInputSystem( handle );
        }
        catch ( OIS::Exception& )
        {
            delete data;
            return 0;
        }

        data->joystickListener = new CJoyStickListener( *callbacks );
        data->mouseListener = new CMouseListener( *callbacks );
        data->keyboardListener = new CKeyboardListener( *callbacks );

        UInt32 keyboardCount = (UInt32) data->inputManager->getNumberOfDevices( OIS::OISKeyboard );
        for ( UInt32 i=0; i<keyboardCount; ++i )
        {
            try
            {
                OIS::Keyboard* keyboard = static_cast< OIS::Keyboard* >( data->inputManager->createInputObject( OIS::OISKeyboard, true ) );
                keyboard->setEventCallback( data->keyboardListener );
                data->keyboards.push_back( keyboard );
                data->callbacks.onKeyboardAttached( data->callbacks.userData, keyboard->getID() );
            }
            catch ( OIS::Exception& )
            {
            }
        }
        UInt32 windowWidth = ParseArgListItemUInt32( argc, argv, "WINDOW_WIDTH" );
        UInt32 windowHeight = ParseArgListItemUInt32( argc, argv, "WINDOW_HEIGHT" );
        UInt32 miceCount = (UInt32) data->inputManager->getNumberOfDevices( OIS::OISMouse );
        for ( UInt32 i=0; i<miceCount; ++i )
        {
            try
            {
                OIS::Mouse* mouse = static_cast< OIS::Mouse* >( data->inputManager->createInputObject( OIS::OISMouse, true ) );
                const OIS::MouseState& mouseState = mouse->getMouseState();

                mouse->setEventCallback( data->mouseListener );
                mouseState.width = windowWidth;
                mouseState.height = windowHeight;
                data->mice.push_back( mouse );
                data->callbacks.onMouseAttached( data->callbacks.userData, mouse->getID() );
            }
            catch ( OIS::Exception& )
            {
            }
        }
        UInt32 joystickCount = (UInt32) data->inputManager->getNumberOfDevices( OIS::OISJoyStick );
        for ( UInt32 i=0; i<joystickCount; ++i )
        {
            try
            {
                OIS::JoyStick* joystick = static_cast< OIS::JoyStick* >( data->inputManager->createInputObject( OIS::OISJoyStick, true ) );
                joystick->setEventCallback( data->joystickListener );
                data->joysticks.push_back( joystick );
                data->callbacks.onDeviceAttached( data->callbacks.userData, joystick->getID() );
            }
            catch ( OIS::Exception& )
            {
            }
        }

        *contextdata = data;
        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVERPLUG_DestroyContext( void* plugdata    ,
                                void* contextData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TContextData* data = (TContextData*) contextData;
    if ( GUCEF_NULL != data )
    {
        if ( GUCEF_NULL != data->inputManager )
        {
            for ( UInt32 i=0; i<data->keyboards.size(); ++i )
            {
                OIS::Keyboard* keyboard = data->keyboards[ i ];
                data->callbacks.onKeyboardDetached( data->callbacks.userData, keyboard->getID() );
                data->inputManager->destroyInputObject( keyboard );
            }
            data->keyboards.clear();

            for ( UInt32 i=0; i<data->mice.size(); ++i )
            {
                OIS::Mouse* mouse = data->mice[ i ];
                data->callbacks.onMouseDetached( data->callbacks.userData, mouse->getID() );
                data->inputManager->destroyInputObject( mouse );
            }
            data->mice.clear();

            for ( UInt32 i=0; i<data->joysticks.size(); ++i )
            {
                OIS::JoyStick* joystick = data->joysticks[ i ];
                data->callbacks.onDeviceDetached( data->callbacks.userData, joystick->getID() );
                data->inputManager->destroyInputObject( joystick );
            }
            data->joysticks.clear();

            delete data->joystickListener;
            data->joystickListener = GUCEF_NULL;
            delete data->mouseListener;
            data->mouseListener = GUCEF_NULL;
            delete data->keyboardListener;
            data->keyboardListener = GUCEF_NULL;

            OIS::InputManager::destroyInputSystem( data->inputManager );
            data->inputManager = GUCEF_NULL;
        }

        delete data;
    }
    return 1;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*---------------------------------------------------------------------------*/
