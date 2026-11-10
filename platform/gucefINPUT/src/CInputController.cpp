/*
 *  gucefINPUT: GUCEF module providing input device interaction
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#include <assert.h>

#ifndef GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H
#include "CNotificationIDRegistry.h"
#define GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H
#endif /* GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"           /* C++ string utils */
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_INPUT_CINPUTDRIVER_H
#include "gucefINPUT_CInputDriver.h"
#define GUCEF_INPUT_CINPUTDRIVER_H
#endif /* GUCEF_INPUT_CINPUTDRIVER_H ? */

#ifndef CINPUTDRIVERPLUGIN_H
#include "CInputDriverPlugin.h"
#define CINPUTDRIVERPLUGIN_H
#endif /* CINPUTDRIVERPLUGIN_H ? */

#include "CInputController.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CInputController::InputDriverLoadedEvent = "GUCEF::INPUT::InputDriverLoadedEvent";
const CORE::CEvent CInputController::InputDriverUnloadedEvent = "GUCEF::INPUT::InputDriverUnloadedEvent";
const CORE::CEvent CInputController::MouseAttachedEvent = "GUCEF::INPUT::MouseAttachedEvent";
const CORE::CEvent CInputController::MouseDetachedEvent = "GUCEF::INPUT::MouseDetachedEvent";
const CORE::CEvent CInputController::KeyboardAttachedEvent = "GUCEF::INPUT::KeyboardAttachedEvent";
const CORE::CEvent CInputController::KeyboardDetachedEvent = "GUCEF::INPUT::KeyboardDetachedEvent";
const CORE::CEvent CInputController::JoystickAttachedEvent = "GUCEF::INPUT::JoystickAttachedEvent";
const CORE::CEvent CInputController::JoystickDetachedEvent = "GUCEF::INPUT::JoystickDetachedEvent";
CInputController* CInputController::m_instance = NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_IMPLEMENT_MSGEXCEPTION( CInputController, EInvalidIndex );

/*-------------------------------------------------------------------------*/

CInputController::CInputController( void )
        : CORE::CTSGNotifier()       ,
          m_idGenerator()            ,
          m_keyboardMap()            ,
          m_mouseMap()               ,
          m_pulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() ),
          #ifdef GUCEF_MSWIN_BUILD
          m_hinstance(0UL),
          #endif
          m_inputDriverPluginManager() ,
          m_defaultDriver()            ,
          m_inputdriverMap()
{GUCEF_TRACE;

    m_idGenerator = TIdGenerator::CreateSharedObj();

    RegisterEvents();

    TEventCallback callback( this, &CInputController::OnPulse );
    SubscribeTo( m_pulseGenerator.GetPointerAlways() ,
                 CORE::CPulseGenerator::PulseEvent   ,
                 callback                            );
}

/*-------------------------------------------------------------------------*/

CInputController::~CInputController()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CInputController::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "GUCEF::INPUT::CInputController";
    return typeName;
}

/*-------------------------------------------------------------------------*/

CInputContext*
CInputController::CreateContext( const CORE::CValueList& params )
{GUCEF_TRACE;

    return CreateContext( m_defaultDriver, params );
}

/*-------------------------------------------------------------------------*/

CInputContext*
CInputController::CreateContext( const CString& driverName      ,
                                 const CORE::CValueList& params )
{GUCEF_TRACE;

    TInputDriverMap::iterator i = m_inputdriverMap.find( driverName );
    if ( i != m_inputdriverMap.end() )
    {
        TInputDriverPtr driver = (*i).second;

        #ifdef GUCEF_MSWIN_BUILD

        CORE::CValueList extraparams( params );
        CORE::CString hinststr = CORE::ToString( m_hinstance );
        extraparams.Set( "HINSTANCE", hinststr );

        CInputContext* context = driver->CreateContext( extraparams );

        #else

        CInputContext* context = driver->CreateContext( params );

        #endif

        if ( NULL != context )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Created input context using driver " + driverName );
            TIdGenerator::TNumericID id = m_idGenerator->GenerateID();
            context->SetID( id );
            m_contextSet.insert( context );
            return context;
        }
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to created input context using driver " + driverName );

    return NULL;
}

/*-------------------------------------------------------------------------*/

void
CInputController::DestroyContext( CInputContext* context )
{GUCEF_TRACE;

    m_contextSet.erase( context );
    context->GetDriver()->DeleteContext( context );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Destroyed input context" );
}

/*-------------------------------------------------------------------------*/

UInt32
CInputController::GetContextCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_contextSet.size();
}

/*-------------------------------------------------------------------------*/

void
CInputController::OnPulse( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    CORE::CPulseData* pulseData = static_cast< CORE::CPulseData* >( eventdata );
    UInt64 tickcount = pulseData->GetTickCount();
    Float64 updateDeltaInMilliSecs = pulseData->GetUpdateDeltaInMilliSecs();

    TContextSet::iterator i = m_contextSet.begin();
    while( i != m_contextSet.end() )
    {
        #ifdef GUCEF_INPUT_DEBUG_MODE

        if ( !(*i)->GetDriver()->OnUpdate( tickcount              ,
                                           updateDeltaInMilliSecs ,
                                           (*i)                   ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to perform an update cycle on the input driver" );
        }

        #else

        (*i)->GetDriver()->OnUpdate( tickcount              ,
                                     updateDeltaInMilliSecs ,
                                     (*i)                   );

        #endif

        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CInputController::OnPumpedNotify( CORE::CNotifier* notifier                 ,
                                  const CORE::CEvent& eventid               ,
                                  CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( eventid == CORE::CGUCEFApplication::AppInitEvent )
    {
        #ifdef GUCEF_MSWIN_BUILD
        CORE::CGUCEFApplication::TAppInitEventData* initData = static_cast< CORE::CGUCEFApplication::TAppInitEventData* >( eventdata );
        #pragma warning( disable: 4311 ) // pointer truncation warning
        m_hinstance = reinterpret_cast<UInt64>( initData->GetData().hinstance );
        #endif /* GUCEF_MSWIN_BUILD ? */
    }
}

/*-------------------------------------------------------------------------*/

void
CInputController::RegisterEvents( void )
{GUCEF_TRACE;

    InputDriverLoadedEvent.Initialize();
    InputDriverUnloadedEvent.Initialize();
    MouseAttachedEvent.Initialize();
    MouseDetachedEvent.Initialize();
    KeyboardAttachedEvent.Initialize();
    KeyboardDetachedEvent.Initialize();
    JoystickAttachedEvent.Initialize();
    JoystickDetachedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

void
CInputController::SetMouseButtonState( const Int32 contextId    ,
                                       const Int32 deviceID     ,
                                       const UInt32 buttonIndex ,
                                       const bool pressedState  )
{GUCEF_TRACE;

    TMouseMap::iterator i = m_mouseMap.find( deviceID );
    if ( i != m_mouseMap.end() )
    {
        (*i).second->SetButtonState( contextId    ,
                                     buttonIndex  ,
                                     pressedState );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Invalid input device ID given: " + CORE::Int32ToString( deviceID ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CInputController::SetMousePos( const Int32 contextId ,
                               const Int32 deviceID  ,
                               const UInt32 xPos     ,
                               const UInt32 yPos     ,
                               const Int32 xDelta    ,
                               const Int32 yDelta    )
{GUCEF_TRACE;

    TMouseMap::iterator i = m_mouseMap.find( deviceID );
    if ( i != m_mouseMap.end() )
    {
        (*i).second->SetMousePos( contextId ,
                                  xPos      ,
                                  yPos      );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Invalid input device ID given: " + CORE::Int32ToString( deviceID ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CInputController::ResetMouseStates( const Int32 deviceID )
{GUCEF_TRACE;

    TMouseMap::iterator i = m_mouseMap.find( deviceID );
    if ( i != m_mouseMap.end() )
    {
        (*i).second->ResetMouseStates();
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Invalid input device ID given: " + CORE::Int32ToString( deviceID ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CInputController::SetKeyboardKeyState( const Int32 contextId ,
                                       const Int32 deviceID  ,
                                       const KeyCode keyCode ,
                                       const bool keyPressed )
{GUCEF_TRACE;

    TKeyboardMap::iterator i = m_keyboardMap.find( deviceID );
    if ( i != m_keyboardMap.end() )
    {
        (*i).second->SetKeyState( contextId  ,
                                  keyCode    ,
                                  keyPressed );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Invalid input device ID given: " + CORE::Int32ToString( deviceID ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CInputController::SetMouseVariable( const Int32 contextId      ,
                                    const Int32 deviceID       ,
                                    const UInt32 variableIndex ,
                                    const Int32 value          ,
                                    const Int32 valueDelta     )
{GUCEF_TRACE;

    TMouseMap::iterator i = m_mouseMap.find( deviceID );
    if ( i != m_mouseMap.end() )
    {
        (*i).second->SetVariable( contextId, variableIndex, value, valueDelta );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Invalid input device ID given: " + CORE::Int32ToString( deviceID ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CInputController::SetKeyboardText( const Int32 contextId ,
                                   const Int32 deviceID  ,
                                   const CString& text   )
{GUCEF_TRACE;

    TKeyboardMap::iterator i = m_keyboardMap.find( deviceID );
    if ( i != m_keyboardMap.end() )
    {
        (*i).second->SetText( contextId, text );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Invalid input device ID given: " + CORE::Int32ToString( deviceID ) );
    }
}

/*-------------------------------------------------------------------------*/

CKeyboard&
CInputController::GetKeyboard( const Int32 deviceID )
{GUCEF_TRACE;

    TKeyboardMap::iterator i = m_keyboardMap.find( deviceID );
    if ( i != m_keyboardMap.end() )
    {
        return *(*i).second;
    }

    GUCEF_EMSGTHROW( EInvalidIndex, "CInputController::GetKeyboard(): Invalid device ID given" );
}

/*-------------------------------------------------------------------------*/

UInt32
CInputController::GetKeyboardCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_keyboardMap.size();
}

/*-------------------------------------------------------------------------*/

const CInputController::TKeyboardMap&
CInputController::GetKeyboardMap( void ) const
{GUCEF_TRACE;

    return m_keyboardMap;
}

/*-------------------------------------------------------------------------*/

CMouse&
CInputController::GetMouse( const Int32 deviceID )
{GUCEF_TRACE;

    TMouseMap::iterator i = m_mouseMap.find( deviceID );
    if ( i != m_mouseMap.end() )
    {
        return *(*i).second;
    }
    GUCEF_EMSGTHROW( EInvalidIndex, "CInputController::GetMouse(): Invalid device ID given" );
}

/*-------------------------------------------------------------------------*/

UInt32
CInputController::GetMouseCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_mouseMap.size();
}

/*-------------------------------------------------------------------------*/

const CInputController::TMouseMap&
CInputController::GetMouseMap( void ) const
{GUCEF_TRACE;

    return m_mouseMap;
}

/*-------------------------------------------------------------------------*/

void
CInputController::AddMouse( CInputDriver* inputDriver, const Int32 deviceID )
{GUCEF_TRACE;

    TMouseMap::iterator i = m_mouseMap.find( deviceID );
    if ( i == m_mouseMap.end() )
    {
        CMouse* mouse = new CMouse( *inputDriver, deviceID );
        m_mouseMap.insert( std::pair< UInt32, CMouse* >( deviceID, mouse ) );

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Mouse input device added with device ID " + CORE::Int32ToString( deviceID ) );
        
        TMouseAttachedEventData eData( deviceID );
        NotifyObservers( MouseAttachedEvent, &eData );
    }
}

/*-------------------------------------------------------------------------*/

void
CInputController::RemoveMouse( const Int32 deviceID )
{GUCEF_TRACE;

    TMouseMap::iterator i = m_mouseMap.find( deviceID );
    if ( i != m_mouseMap.end() )
    {
        CMouse* mouse = (*i).second;
        delete mouse;
        m_mouseMap.erase( i );

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Mouse input device removed with device ID " + CORE::Int32ToString( deviceID ) );
        
        TMouseDetachedEventData eData( deviceID );
        NotifyObservers( MouseDetachedEvent, &eData );
    }
}

/*-------------------------------------------------------------------------*/

void
CInputController::AddKeyboard( CInputDriver* inputDriver, const Int32 deviceID )
{GUCEF_TRACE;

    TKeyboardMap::iterator i = m_keyboardMap.find( deviceID );
    if ( i == m_keyboardMap.end() )
    {
        CKeyboard* keyboard = new CKeyboard( *inputDriver, deviceID );
        m_keyboardMap.insert( std::pair< UInt32, CKeyboard* >( deviceID, keyboard ) );

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Keyboard input device added with device ID " + CORE::Int32ToString( deviceID ) );
        
        TKeyboardAttachedEventData eData( deviceID );
        NotifyObservers( KeyboardAttachedEvent, &eData );
    }
}

/*-------------------------------------------------------------------------*/

void
CInputController::RemoveKeyboard( const Int32 deviceID )
{GUCEF_TRACE;

    TKeyboardMap::iterator i = m_keyboardMap.find( deviceID );
    if ( i != m_keyboardMap.end() )
    {
        CKeyboard* keyboard = (*i).second;
        delete keyboard;
        m_keyboardMap.erase( i );

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Keyboard input device removed with device ID " + CORE::Int32ToString( deviceID ) );
        
        TKeyboardDetachedEventData eData( deviceID );
        NotifyObservers( KeyboardDetachedEvent, &eData );
    }
}

/*-------------------------------------------------------------------------*/

void
CInputController::AddDevice( CInputDriver* inputDriver, const Int32 deviceID )
{GUCEF_TRACE;

    // @TODO
}

/*-------------------------------------------------------------------------*/

void
CInputController::RemoveDevice( const Int32 deviceID )
{GUCEF_TRACE;

    // @TODO
}

/*-------------------------------------------------------------------------*/

void
CInputController::SubscribeToAllKeyboards( CORE::CObserver* keyboardObserver )
{GUCEF_TRACE;

    TKeyboardMap::iterator i = m_keyboardMap.begin();
    while ( i != m_keyboardMap.end() )
    {
        (*i).second->Subscribe( keyboardObserver );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CInputController::SubscribeToAllMice( CORE::CObserver* mouseObserver )
{GUCEF_TRACE;

    TMouseMap::iterator i = m_mouseMap.begin();
    while ( i != m_mouseMap.end() )
    {
        (*i).second->Subscribe( mouseObserver );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

CInputDriverPluginManager&
CInputController::GetInputDriverPluginManager( void )
{GUCEF_TRACE;

    return m_inputDriverPluginManager;
}

/*-------------------------------------------------------------------------*/

void
CInputController::RegisterDriver( TInputDriverPtr driver )
{GUCEF_TRACE;

    if ( driver )
    {
        CString driverName = driver->GetName();

        m_inputdriverMap[ driverName ] = driver;
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "InputController: A new driver has been registered with name " + driverName );

        if ( m_defaultDriver.IsNULLOrEmpty() )
        {
            m_defaultDriver = driverName;
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "InputController: No default driver was defined so the newly registed driver " + driverName + " will be set as the default driver" );
        }
        NotifyObservers( InputDriverLoadedEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CInputController::UnregisterDriver( const CString& driverName )
{GUCEF_TRACE;

    // the contexts of the driver go with it: their code and data belong to the driver
    TInputDriverMap::iterator d = m_inputdriverMap.find( driverName );
    if ( d != m_inputdriverMap.end() )
    {
        CInputDriver* driver = (*d).second.GetPointerAlways();
        TContextSet contexts = m_contextSet;
        for ( TContextSet::iterator i=contexts.begin(); i!=contexts.end(); ++i )
            if ( (*i)->GetDriver() == driver )
                DestroyContext( *i );
    }
    m_inputdriverMap.erase( driverName );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "InputController: Unregistered driver with name " + driverName );
    NotifyObservers( InputDriverUnloadedEvent );
}

/*-------------------------------------------------------------------------*/

void
CInputController::SetDefaultDriver( const CString& driverName )
{GUCEF_TRACE;

    m_defaultDriver = driverName;
}

/*-------------------------------------------------------------------------*/

const CString&
CInputController::GetDefaultDriver( void ) const
{GUCEF_TRACE;

    return m_defaultDriver;
}

/*-------------------------------------------------------------------------*/

CInputController::TStringSet
CInputController::GetListOfAvailableDriversByName( void ) const
{GUCEF_TRACE;

    CInputController::TStringSet resultSet;
    TInputDriverMap::const_iterator i = m_inputdriverMap.begin();
    while ( i != m_inputdriverMap.end() )
    {
        resultSet.insert( (*i).first );
        ++i;
    }
    return resultSet;
}

/*-------------------------------------------------------------------------*/

TInputDriverPtr
CInputController::GetDriverByName( const CString& driverName )
{GUCEF_TRACE;

    TInputDriverMap::iterator i = m_inputdriverMap.find( driverName );
    if ( i != m_inputdriverMap.end() )
    {
        return (*i).second;
    }
    return TInputDriverPtr();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
