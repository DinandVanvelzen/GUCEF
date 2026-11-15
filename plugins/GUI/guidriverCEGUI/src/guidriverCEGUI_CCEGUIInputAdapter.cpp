/*
 *  guidriverCEGUI: glue module for the CEGUI GUI backend
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

#ifndef _CEGUI_h_
#include "CEGUI/CEGUI.h"
#define _CEGUI_h_
#endif /* _CEGUI_h_ ? */

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

#include "guidriverCEGUI_CCEGUIInputAdapter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERCEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

inline CEGUI::MouseButton 
ConvertButtonIndexToCEGui( const UInt32 buttonindex )
{GUCEF_TRACE;

    switch ( buttonindex )
    {
        case 0:
        {
            return CEGUI::LeftButton;
        }                        
        case 1:
        {
            return CEGUI::RightButton;
        }                        
        case 2:
        {
            return CEGUI::MiddleButton;
        }        
        case 3:
        {
            return CEGUI::X1Button;
        }                        
        case 4:
        {
            return CEGUI::X2Button;
        }                
        default:
        {
            return CEGUI::LeftButton;
        }                        
    }
}

/*-------------------------------------------------------------------------*/

CCEGUIInputAdapter::CCEGUIInputAdapter( void )
    : GUCEF::CORE::CObserver()  ,
      m_guisystem( CEGUI::System::getSingletonPtr() )  ,
      m_inputContext( GUCEF_NULL )                           ,
      m_ceGuiContext( GUCEF_NULL )                           ,
      m_inputController( &INPUT::CInputGlobal::Instance()->GetInputController() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCEGUIInputAdapter::~CCEGUIInputAdapter()
{GUCEF_TRACE;

    m_inputContext = GUCEF_NULL;
    m_ceGuiContext = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CCEGUIInputAdapter::StartListningForInputEvents( void )
{GUCEF_TRACE;

    StopListningForInputEvents();

    INPUT::CInputController& inputController = INPUT::CInputGlobal::Instance()->GetInputController();
    inputController.SubscribeToAllKeyboards( this );
    inputController.SubscribeToAllMice( this );
}

/*-------------------------------------------------------------------------*/

void
CCEGUIInputAdapter::StopListningForInputEvents( void )
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CCEGUIInputAdapter::SetCEGUIContext( CEGUI::GUIContext* ceGuiContext )
{GUCEF_TRACE;

    m_ceGuiContext = ceGuiContext;
}

/*-------------------------------------------------------------------------*/

void
CCEGUIInputAdapter::SetInputContext( INPUT::CInputContext* inputContext )
{GUCEF_TRACE;

    m_inputContext = inputContext;
}

/*-------------------------------------------------------------------------*/

INPUT::CInputContext*
CCEGUIInputAdapter::GetInputContext( void ) const
{GUCEF_TRACE;

    return m_inputContext;
}

/*-------------------------------------------------------------------------*/

const CString&
CCEGUIInputAdapter::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUIDRIVERCEGUI::CCEGUIInputAdapter";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CCEGUIInputAdapter::OnNotify( GUCEF::CORE::CNotifier* notifier                 ,
                              const GUCEF::CORE::CEvent& eventid               ,
                              GUCEF::CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_ceGuiContext )
    {
        if ( GUCEF::INPUT::CMouse::MouseButtonEvent == eventid )
        {
            GUCEF::INPUT::CMouseButtonEventData* eData = static_cast< GUCEF::INPUT::CMouseButtonEventData* >( eventdata );
            if ( eData->GetPressedState() )
            {
                m_ceGuiContext->injectMouseButtonDown( ConvertButtonIndexToCEGui( eData->GetButtonIndex() ) );
            }
            else
            {
                m_ceGuiContext->injectMouseButtonUp( ConvertButtonIndexToCEGui( eData->GetButtonIndex() ) );
            }
            return;
        }
        if ( GUCEF::INPUT::CMouse::MouseMovedEvent == eventid )
        {
            GUCEF::INPUT::CMouseMovedEventData* eData = static_cast< GUCEF::INPUT::CMouseMovedEventData* >( eventdata );
            m_ceGuiContext->injectMousePosition( (float) eData->GetXPos() ,
                                                 (float) eData->GetYPos() );
            return;
        }
        if ( GUCEF::INPUT::CKeyboard::KeyStateChangedEvent == eventid )
        {
            GUCEF::INPUT::CKeyStateChangedEventData* eData = static_cast< GUCEF::INPUT::CKeyStateChangedEventData* >( eventdata );
            if ( eData->GetKeyPressedState() )
            {
                UInt32 unicode = 0;
                GUCEF::INPUT::CKeyboard& keyboard = m_inputController->GetKeyboard( eData->GetDeviceID() );
                if ( keyboard.GetUnicodeForKeyCode( eData->GetKeyCode()             ,
                                                    eData->GetKeyModPressedStates() ,
                                                    unicode                         ) )
                {
                    m_ceGuiContext->injectChar( unicode );
                }
                
                // The CEGUI scancode values are the same as those used by gucefINPUT
                // as such a simple cast is sufficient to perform the translation
                m_ceGuiContext->injectKeyDown( (CEGUI::Key::Scan) eData->GetKeyCode() );
            }
            else
            {
                // The CEGUI scancode values are the same as those used by gucefINPUT
                // as such a simple cast is sufficient to perform the translation
                m_ceGuiContext->injectKeyUp( (CEGUI::Key::Scan) eData->GetKeyCode() );
            }                
            return;
        }
    }
    
    if ( GUCEF::INPUT::CInputController::MouseAttachedEvent == eventid )
    {
        Int32 deviceID = static_cast< GUCEF::INPUT::CInputController::TMouseAttachedEventData* >( eventdata )->GetData();
        GUCEF::INPUT::CMouse& mouse = m_inputController->GetMouse( deviceID );
        SubscribeTo( &mouse );
    }
    if ( GUCEF::INPUT::CInputController::KeyboardAttachedEvent == eventid )
    {
        Int32 deviceID = static_cast< GUCEF::INPUT::CInputController::TKeyboardAttachedEventData* >( eventdata )->GetData();
        GUCEF::INPUT::CKeyboard& keyboard = m_inputController->GetKeyboard( deviceID );
        SubscribeTo( &keyboard );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERCEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
