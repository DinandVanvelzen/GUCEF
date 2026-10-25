/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_INPUT_CINPUTGLOBAL_H
#include "gucefINPUT_CInputGlobal.h"
#define GUCEF_INPUT_CINPUTGLOBAL_H
#endif /* GUCEF_INPUT_CINPUTGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_INPUT_CINPUTCONTROLLER_H
#include "CInputController.h"
#define GUCEF_INPUT_CINPUTCONTROLLER_H
#endif /* GUCEF_INPUT_CINPUTCONTROLLER_H ? */

#ifndef GUCEF_INPUT_CMOUSEBUTTONEVENTDATA_H
#include "gucefINPUT_CMouseButtonEventData.h"
#define GUCEF_INPUT_CMOUSEBUTTONEVENTDATA_H
#endif /* GUCEF_INPUT_CMOUSEBUTTONEVENTDATA_H ? */

#ifndef GUCEF_INPUT_CMOUSEMOVEDEVENTDATA_H
#include "gucefINPUT_CMouseMovedEventData.h"
#define GUCEF_INPUT_CMOUSEMOVEDEVENTDATA_H
#endif /* GUCEF_INPUT_CMOUSEMOVEDEVENTDATA_H ? */

#ifndef GUCEF_INPUT_CMOUSEVARIABLECHANGEDEVENTDATA_H
#include "gucefINPUT_CMouseVariableChangedEventData.h"
#define GUCEF_INPUT_CMOUSEVARIABLECHANGEDEVENTDATA_H
#endif /* GUCEF_INPUT_CMOUSEVARIABLECHANGEDEVENTDATA_H ? */

#ifndef GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H
#include "gucefINPUT_CKeyStateChangedEventData.h"
#define GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H
#endif /* GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H ? */

#ifndef GUCEF_INPUT_CKEYBOARDTEXTEVENTDATA_H
#include "gucefINPUT_CKeyboardTextEventData.h"
#define GUCEF_INPUT_CKEYBOARDTEXTEVENTDATA_H
#endif /* GUCEF_INPUT_CKEYBOARDTEXTEVENTDATA_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CGUIInputAdapter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const Int32 CGUIInputAdapter::AnyInputContext = -1;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGUIInputAdapter::CGUIInputAdapter( CGUIContext& guiContext, Int32 inputContextId )
    : CORE::CObserver()                ,
      m_guiContext( &guiContext )      ,
      m_inputContextId( inputContextId ) ,
      m_attached( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGUIInputAdapter::~CGUIInputAdapter()
{GUCEF_TRACE;

    Detach();
}

/*-------------------------------------------------------------------------*/

void
CGUIInputAdapter::Attach( void )
{GUCEF_TRACE;

    if ( m_attached )
        return;
    m_attached = true;

    INPUT::CInputController& controller = INPUT::CInputGlobal::Instance()->GetInputController();
    SubscribeTo( &controller, INPUT::CInputController::MouseAttachedEvent );
    SubscribeTo( &controller, INPUT::CInputController::KeyboardAttachedEvent );
    controller.SubscribeToAllMice( this );
    controller.SubscribeToAllKeyboards( this );
}

/*-------------------------------------------------------------------------*/

void
CGUIInputAdapter::Detach( void )
{GUCEF_TRACE;

    if ( !m_attached )
        return;
    m_attached = false;
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CGUIInputAdapter::IsAttached( void ) const
{GUCEF_TRACE;

    return m_attached;
}

/*-------------------------------------------------------------------------*/

void
CGUIInputAdapter::SetInputContextId( Int32 inputContextId )
{GUCEF_TRACE;

    m_inputContextId = inputContextId;
}

/*-------------------------------------------------------------------------*/

Int32
CGUIInputAdapter::GetInputContextId( void ) const
{GUCEF_TRACE;

    return m_inputContextId;
}

/*-------------------------------------------------------------------------*/

TMouseButton
CGUIInputAdapter::MapMouseButton( UInt32 buttonIndex )
{GUCEF_TRACE;

    switch ( buttonIndex )
    {
        case 0: return MOUSEBUTTON_LEFT;
        case 1: return MOUSEBUTTON_RIGHT;
        case 2: return MOUSEBUTTON_MIDDLE;
        case 3: return MOUSEBUTTON_X1;
        case 4: return MOUSEBUTTON_X2;
        default: return MOUSEBUTTON_NONE;
    }
}

/*-------------------------------------------------------------------------*/

bool
CGUIInputAdapter::IsForUs( Int32 inputContextId ) const
{GUCEF_TRACE;

    // input of our window, unless a GUI layer above ours in the window shows something and takes it
    if ( AnyInputContext != m_inputContextId && inputContextId != m_inputContextId )
        return false;
    return !m_guiContext->HasHigherLayerTakingInput();
}

/*-------------------------------------------------------------------------*/

void
CGUIInputAdapter::OnNotify( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;

    if ( INPUT::CMouse::MouseMovedEvent == eventId )
    {
        const INPUT::CMouseMovedEventData* data = static_cast< INPUT::CMouseMovedEventData* >( eventData );
        if ( IsForUs( data->GetContextId() ) )
            m_guiContext->ProcessWindowMouseMove( data->GetXPos(), data->GetYPos() );
    }
    else
    if ( INPUT::CMouse::MouseButtonEvent == eventId )
    {
        const INPUT::CMouseButtonEventData* data = static_cast< INPUT::CMouseButtonEventData* >( eventData );
        TMouseButton button = MapMouseButton( data->GetButtonIndex() );
        if ( IsForUs( data->GetContextId() ) && MOUSEBUTTON_NONE != button )
            m_guiContext->InjectMouseButton( button, data->GetPressedState() );
    }
    else
    if ( INPUT::CMouse::MouseVariableChangedEvent == eventId )
    {
        const INPUT::CMouseVariableChangedEventData* data = static_cast< INPUT::CMouseVariableChangedEventData* >( eventData );
        if ( INPUT::CMouse::MOUSEVARIABLE_WHEEL == data->GetVariableIndex() && 0 != data->GetValueDelta() )
        {
            if ( IsForUs( data->GetContextId() ) )
                m_guiContext->InjectMouseWheel( data->GetValueDelta() );
            else
            {
                CORE::CString reason = "a layer above takes the input";
                if ( m_inputContextId != data->GetContextId() && AnyInputContext != m_inputContextId )
                    reason = "it listens to input context " + CORE::Int32ToString( m_inputContextId );
                GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "GUI layer " << m_guiContext->GetLayer() << ": wheel " << data->GetValueDelta() << " of input context " << data->GetContextId()
                                << " is not for this GUI (" << reason << ")" );
            }
        }
    }
    else
    if ( INPUT::CKeyboard::KeyStateChangedEvent == eventId )
    {
        const INPUT::CKeyStateChangedEventData* data = static_cast< INPUT::CKeyStateChangedEventData* >( eventData );
        if ( IsForUs( data->GetContextId() ) )
            m_guiContext->InjectKey( data->GetKeyCode(), data->GetKeyPressedState(), data->GetKeyModPressedStates() );
    }
    else
    if ( INPUT::CKeyboard::TextInputEvent == eventId )
    {
        const INPUT::CKeyboardTextEventData* data = static_cast< INPUT::CKeyboardTextEventData* >( eventData );
        if ( IsForUs( data->GetContextId() ) )
            m_guiContext->InjectText( data->GetText() );
    }
    else
    if ( INPUT::CInputController::MouseAttachedEvent == eventId )
    {
        Int32 deviceId = static_cast< INPUT::CInputController::TMouseAttachedEventData* >( eventData )->GetData();
        INPUT::CInputGlobal::Instance()->GetInputController().GetMouse( deviceId ).Subscribe( this );
    }
    else
    if ( INPUT::CInputController::KeyboardAttachedEvent == eventId )
    {
        Int32 deviceId = static_cast< INPUT::CInputController::TKeyboardAttachedEventData* >( eventData )->GetData();
        INPUT::CInputGlobal::Instance()->GetInputController().GetKeyboard( deviceId ).Subscribe( this );
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CGUIInputAdapter::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CGUIInputAdapter";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
