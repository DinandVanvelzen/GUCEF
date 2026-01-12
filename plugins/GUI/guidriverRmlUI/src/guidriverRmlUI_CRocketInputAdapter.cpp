///*
// *  guceMyGUIOgre: glue module for the MyGUI+Ogre GUI backend
// *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
// *
// *  This library is free software; you can redistribute it and/or
// *  modify it under the terms of the GNU Lesser General Public
// *  License as published by the Free Software Foundation; either
// *  version 2.1 of the License, or (at your option) any later version.
// *
// *  This library is distributed in the hope that it will be useful,
// *  but WITHOUT ANY WARRANTY; without even the implied warranty of
// *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// *  Lesser General Public License for more details.
// *
// *  You should have received a copy of the GNU Lesser General Public
// *  License along with this library; if not, write to the Free Software
// *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
// */
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      INCLUDES                                                           //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//#include <assert.h>
//
//#include <MyGUI.h>
//
//#ifndef GUCEF_CORE_CLOGMANAGER_H
//#include "CLogManager.h"
//#define GUCEF_CORE_CLOGMANAGER_H
//#endif /* GUCEF_CORE_CLOGMANAGER_H ? */
//
//#ifndef GUCEF_INPUT_CINPUTCONTROLLER_H
//#include "CInputController.h"
//#define GUCEF_INPUT_CINPUTCONTROLLER_H
//#endif /* GUCEF_INPUT_CINPUTCONTROLLER_H ? */
//
//#ifndef GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H
//#include "gucefINPUT_CKeyStateChangedEventData.h"
//#define GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H
//#endif /* GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H ? */
//
//#ifndef GUCEF_INPUT_CMOUSEBUTTONEVENTDATA_H
//#include "gucefINPUT_CMouseButtonEventData.h"
//#define GUCEF_INPUT_CMOUSEBUTTONEVENTDATA_H
//#endif /* GUCEF_INPUT_CMOUSEBUTTONEVENTDATA_H ? */
//
//#ifndef GUCEF_INPUT_CMOUSEMOVEDEVENTDATA_H
//#include "gucefINPUT_CMouseMovedEventData.h"
//#define GUCEF_INPUT_CMOUSEMOVEDEVENTDATA_H
//#endif /* GUCEF_INPUT_CMOUSEMOVEDEVENTDATA_H ? */
//
//#ifndef GUCE_CORE_CGUCEAPPLICATION_H
//#include "CGUCEApplication.h"
//#define GUCE_CORE_CGUCEAPPLICATION_H
//#endif /* GUCE_CORE_CGUCEAPPLICATION_H ? */
//
//#ifndef GUCE_GUI_MACROS_H
//#include "guceGUI_macros.h"
//#define GUCE_GUI_MACROS_H
//#endif /* GUCE_GUI_MACROS_H ? */
//
//#include "guceMyGUIOgre_CMyGUIInputAdapter.h"
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      NAMESPACE                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//namespace GUCE {
//namespace MYGUIOGRE {
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      UTILITIES                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//CMyGUIInputAdapter::CMyGUIInputAdapter( MyGUI::Gui* guiSystem )
//    : GUCEF::CORE::CObserver()  ,
//      m_guiSystem( guiSystem )
//{GUCE_TRACE;
//
//    assert( NULL != m_guiSystem );
//    
//    // Add this input event consumer to the GUI input observer group
//    // This allows external agents to redirect input elsewhere when needed
//    GUCE::CORE::CGUCEApplication* app = GUCE::CORE::CGUCEApplication::Instance();
//    app->GetInputObserverSwitch().AddObserverToGroup( "GUI", *this );
//    
//    // Subscribe to all input events the input switch can forward
//    SubscribeTo( &app->GetInputObserverSwitch() );
//}
//
///*-------------------------------------------------------------------------*/
//
//CMyGUIInputAdapter::~CMyGUIInputAdapter()
//{GUCE_TRACE;
//
//    m_guiSystem = NULL;
//}
//
///*-------------------------------------------------------------------------*/
//    
//const CString&
//CMyGUIInputAdapter::GetClassTypeName( void ) const
//{GUCE_TRACE;
//
//    static const CString classTypeName = "GUCE::MYGUIOGRE::CMyGUIInputAdapter";
//    return classTypeName;
//}
//
///*-------------------------------------------------------------------------*/
//
//MyGUI::MouseButton
//CMyGUIInputAdapter::ConvertMouseButtonIdex( const UInt32 buttonIndex )
//{GUCE_TRACE;
//
//    switch ( buttonIndex )
//    {
//        case 0: return MyGUI::MouseButton( MyGUI::MouseButton::Button0 );
//        case 1: return MyGUI::MouseButton( MyGUI::MouseButton::Button1 );
//        case 2: return MyGUI::MouseButton( MyGUI::MouseButton::Button2 );
//        case 3: return MyGUI::MouseButton( MyGUI::MouseButton::Button3 );
//        case 4: return MyGUI::MouseButton( MyGUI::MouseButton::Button4 );
//        case 5: return MyGUI::MouseButton( MyGUI::MouseButton::Button5 );
//        case 6: return MyGUI::MouseButton( MyGUI::MouseButton::Button6 );
//        case 7: return MyGUI::MouseButton( MyGUI::MouseButton::Button7 );
//        default:
//        {
//            GUCEF_ERROR_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, "CMyGUIInputAdapter(" + GUCEF::CORE::PointerToString( this ) + "): Unable to convert mouse button with index " + GUCEF::CORE::UInt32ToString( buttonIndex ) + " to a MyGUI equivalant" );
//            return MyGUI::MouseButton( MyGUI::MouseButton::None );
//        }
//    }
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CMyGUIInputAdapter::OnNotify( GUCEF::CORE::CNotifier* notifier                 ,
//                              const GUCEF::CORE::CEvent& eventid               ,
//                              GUCEF::CORE::CICloneable* eventdata /* = NULL */ )
//{GUCE_TRACE;
//
//    if ( NULL != m_guiSystem )
//    {
//        if ( GUCEF::INPUT::CMouse::MouseButtonEvent == eventid )
//        {
//            GUCEF::INPUT::CMouseButtonEventData* eData = static_cast< GUCEF::INPUT::CMouseButtonEventData* >( eventdata );
//            if ( eData->GetPressedState() )
//            {
//                m_guiSystem->injectMousePress( (int)eData->GetXPos()                             , 
//                                               (int)eData->GetYPos()                             , 
//                                               ConvertMouseButtonIdex( eData->GetButtonIndex() ) );
//            }
//            else
//            {
//                m_guiSystem->injectMouseRelease( (int)eData->GetXPos()                           ,
//                                                 (int)eData->GetYPos()                           ,
//                                                 ConvertMouseButtonIdex( eData->GetButtonIndex() ) );
//            }
//            return;
//        }
//        if ( GUCEF::INPUT::CMouse::MouseMovedEvent == eventid )
//        {
//            GUCEF::INPUT::CMouseMovedEventData* eData = static_cast< GUCEF::INPUT::CMouseMovedEventData* >( eventdata );
//            m_guiSystem->injectMouseMove( (int) eData->GetXPos() ,
//                                          (int) eData->GetYPos() ,
//                                          0                      );
//            return;
//        }
//        if ( GUCEF::INPUT::CKeyboard::KeyStateChangedEvent == eventid )
//        {
//            GUCEF::INPUT::CKeyStateChangedEventData* eData = static_cast< GUCEF::INPUT::CKeyStateChangedEventData* >( eventdata );
//            if ( eData->GetKeyPressedState() )
//            {   
//                // The MyGUI scancode values are the same as those used by gucefINPUT
//                // as such a simple cast is sufficient to perform the translation
//                m_guiSystem->injectKeyPress( (MyGUI::KeyCode::Enum) eData->GetKeyCode() );
//            }
//            else
//            {
//                // The MyGUI scancode values are the same as those used by gucefINPUT
//                // as such a simple cast is sufficient to perform the translation
//                m_guiSystem->injectKeyRelease( (MyGUI::KeyCode::Enum) eData->GetKeyCode() );
//            }                
//            return;
//        }
//    }
//}
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      NAMESPACE                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//}; /* namespace MYGUIOGRE */
//}; /* namespace GUCE */
//
///*-------------------------------------------------------------------------*/

