/*
 *  guidriverMyGUI: glue module for the MyGUI GUI backend
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#include <assert.h>

#include <MyGUI.h>

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_INPUT_CINPUTGLOBAL_H
#include "gucefINPUT_CInputGlobal.h"
#define GUCEF_INPUT_CINPUTGLOBAL_H
#endif /* GUCEF_INPUT_CINPUTGLOBAL_H ? */

#ifndef GUCEF_INPUT_CINPUTCONTROLLER_H
#include "CInputController.h"
#define GUCEF_INPUT_CINPUTCONTROLLER_H
#endif /* GUCEF_INPUT_CINPUTCONTROLLER_H ? */

#ifndef GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H
#include "gucefINPUT_CKeyStateChangedEventData.h"
#define GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H
#endif /* GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H ? */

#ifndef GUCEF_INPUT_CMOUSEBUTTONEVENTDATA_H
#include "gucefINPUT_CMouseButtonEventData.h"
#define GUCEF_INPUT_CMOUSEBUTTONEVENTDATA_H
#endif /* GUCEF_INPUT_CMOUSEBUTTONEVENTDATA_H ? */

#ifndef GUCEF_INPUT_CMOUSEMOVEDEVENTDATA_H
#include "gucefINPUT_CMouseMovedEventData.h"
#define GUCEF_INPUT_CMOUSEMOVEDEVENTDATA_H
#endif /* GUCEF_INPUT_CMOUSEMOVEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

#include "guceMyGUI_CMyGUIInputAdapter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MYGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CMyGUIInputAdapter::CMyGUIInputAdapter( void )
    : GUCEF::CORE::CObserver()  ,
      m_inputContext( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMyGUIInputAdapter::~CMyGUIInputAdapter()
{GUCEF_TRACE;

    m_inputContext = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CMyGUIInputAdapter::StartListningForInputEvents( void )
{GUCEF_TRACE;

    StopListningForInputEvents();

    INPUT::CInputController& inputController = INPUT::CInputGlobal::Instance()->GetInputController();
    inputController.SubscribeToAllKeyboards( this );
    inputController.SubscribeToAllMice( this );
}

/*-------------------------------------------------------------------------*/

void
CMyGUIInputAdapter::StopListningForInputEvents( void )
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CMyGUIInputAdapter::SetInputContext( INPUT::CInputContext* inputContext )
{GUCEF_TRACE;

    m_inputContext = inputContext;
}

/*-------------------------------------------------------------------------*/

INPUT::CInputContext*
CMyGUIInputAdapter::GetInputContext( void ) const
{GUCEF_TRACE;

    return m_inputContext;
}

/*-------------------------------------------------------------------------*/

const CString&
CMyGUIInputAdapter::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::MYGUI::CMyGUIInputAdapter";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

MyGUI::MouseButton
CMyGUIInputAdapter::ConvertMouseButtonIndex( const UInt32 buttonIndex )
{GUCEF_TRACE;

    switch ( buttonIndex )
    {
        case 0: return MyGUI::MouseButton( MyGUI::MouseButton::Button0 );
        case 1: return MyGUI::MouseButton( MyGUI::MouseButton::Button1 );
        case 2: return MyGUI::MouseButton( MyGUI::MouseButton::Button2 );
        case 3: return MyGUI::MouseButton( MyGUI::MouseButton::Button3 );
        case 4: return MyGUI::MouseButton( MyGUI::MouseButton::Button4 );
        case 5: return MyGUI::MouseButton( MyGUI::MouseButton::Button5 );
        case 6: return MyGUI::MouseButton( MyGUI::MouseButton::Button6 );
        case 7: return MyGUI::MouseButton( MyGUI::MouseButton::Button7 );
        default:
        {
            GUCEF_ERROR_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, "CMyGUIInputAdapter(" + GUCEF::CORE::PointerToString( this ) + "): Unable to convert mouse button with index " + GUCEF::CORE::UInt32ToString( buttonIndex ) + " to a MyGUI equivalant" );
            return MyGUI::MouseButton( MyGUI::MouseButton::None );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMyGUIInputAdapter::OnNotify( GUCEF::CORE::CNotifier* notifier                 ,
                              const GUCEF::CORE::CEvent& eventid               ,
                              GUCEF::CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    MyGUI::InputManager* inputManager = MyGUI::InputManager::getInstancePtr();
    if ( GUCEF_NULL != inputManager )
    {
        if ( GUCEF::INPUT::CMouse::MouseButtonEvent == eventid )
        {
            GUCEF::INPUT::CMouseButtonEventData* eData = static_cast< GUCEF::INPUT::CMouseButtonEventData* >( eventdata );
            if ( m_inputContext->GetID() == eData->GetContextId() )
            {
                if ( eData->GetPressedState() )
                {
                    inputManager->injectMousePress( (int)eData->GetXPos()                              ,
                                                    (int)eData->GetYPos()                              ,
                                                    ConvertMouseButtonIndex( eData->GetButtonIndex() ) );

                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "GUIDRIVERMYGUI::CMyGUIInputAdapter: injected mouse button down. buttonIndex=" + CORE::Int32ToString( eData->GetButtonIndex() ) );
                }
                else
                {
                    inputManager->injectMouseRelease( (int)eData->GetXPos()                            ,
                                                      (int)eData->GetYPos()                            ,
                                                      ConvertMouseButtonIndex( eData->GetButtonIndex() ) );

                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "GUIDRIVERMYGUI::CMyGUIInputAdapter: injected mouse button up. buttonIndex=" + CORE::Int32ToString( eData->GetButtonIndex() ) );
                }
            }
            return;
        }
        if ( GUCEF::INPUT::CMouse::MouseMovedEvent == eventid )
        {
            GUCEF::INPUT::CMouseMovedEventData* eData = static_cast< GUCEF::INPUT::CMouseMovedEventData* >( eventdata );
            if ( m_inputContext->GetID() == eData->GetContextId() )
            {
                inputManager->injectMouseMove( (int) eData->GetXPos() ,
                                               (int) eData->GetYPos() ,
                                               0                      );

                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "GUIDRIVERMYGUI::CMyGUIInputAdapter: injected mouse move x=" + CORE::Int32ToString( eData->GetXPos() ) +
                                                                                                                     " y=" + CORE::Int32ToString( eData->GetYPos() ) );
            }
            return;
        }
        if ( GUCEF::INPUT::CKeyboard::KeyStateChangedEvent == eventid )
        {
            GUCEF::INPUT::CKeyStateChangedEventData* eData = static_cast< GUCEF::INPUT::CKeyStateChangedEventData* >( eventdata );
            if ( m_inputContext->GetID() == eData->GetContextId() )
            {
                if ( eData->GetKeyPressedState() )
                {
                    // The MyGUI scancode values are the same as those used by gucefINPUT
                    // as such a simple cast is sufficient to perform the translation
                    inputManager->injectKeyPress( (MyGUI::KeyCode::Enum) eData->GetKeyCode() );
                }
                else
                {
                    // The MyGUI scancode values are the same as those used by gucefINPUT
                    // as such a simple cast is sufficient to perform the translation
                    inputManager->injectKeyRelease( (MyGUI::KeyCode::Enum) eData->GetKeyCode() );
                }
            }
            return;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MYGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
