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

#ifdef __APPLE__
#include <stdlib.h>
#else
#include <malloc.h>     /* needed for malloc/realloc */
#endif
#include <string.h>     /* needed for memset */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H 
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_INPUT_CKEYBOARD_H
#include "gucefINPUT_CKeyboard.h"
#define GUCEF_INPUT_CKEYBOARD_H
#endif /* GUCEF_INPUT_CKEYBOARD_H ? */

#ifndef GUCEF_INPUT_CMOUSE_H
#include "gucefINPUT_CMouse.h"
#define GUCEF_INPUT_CMOUSE_H
#endif /* GUCEF_INPUT_CMOUSE_H ? */

#ifndef GUCEF_INPUT_ACTIONEVENTDATA_H
#include "gucefINPUT_ActionEventData.h"
#define GUCEF_INPUT_ACTIONEVENTDATA_H
#endif /* GUCEF_INPUT_ACTIONEVENTDATA_H ? */

#include "CInputActionMap.h"

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

const CORE::CEvent CInputActionMap::ActionEvent = "GUCEF::INPUT::CInputActionMap::ActionEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CInputActionMap::RegisterEvents( void )
{
    ActionEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CInputActionMap::CInputActionMap( const CORE::CString& name )
    : CORE::CObservingNotifier() ,
      m_keyUpMap()               ,
      m_keyDownMap()             ,
      m_mouseButtonUpMap()       ,
      m_mouseButtonDownMap()     ,
      m_deviceBoolOnMap()        ,
      m_deviceBoolOffMap()       ,
      m_deviceVarChangeMap()     ,
      m_name( name )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CInputActionMap::CInputActionMap( const CInputActionMap& src )
    : CORE::CObservingNotifier( src )                  ,
      m_keyUpMap( src.m_keyUpMap )                     ,
      m_keyDownMap( src.m_keyDownMap )                 ,
      m_mouseButtonUpMap( src.m_mouseButtonUpMap )     ,
      m_mouseButtonDownMap( src.m_mouseButtonDownMap ) ,
      m_deviceBoolOnMap( src.m_deviceBoolOnMap )       ,
      m_deviceBoolOffMap( src.m_deviceBoolOffMap )     ,
      m_deviceVarChangeMap( src.m_deviceVarChangeMap ) ,
      m_name( src.m_name )
{GUCEF_TRACE;
      
}

/*-------------------------------------------------------------------------*/

CInputActionMap::~CInputActionMap()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CInputActionMap&
CInputActionMap::operator=( const CInputActionMap& src )
{GUCEF_TRACE;

    if ( &src != this )
    {              
        m_name = src.m_name;
        m_keyUpMap = src.m_keyUpMap;
        m_keyDownMap = src.m_keyDownMap;
        m_mouseButtonUpMap = src.m_mouseButtonUpMap;
        m_mouseButtonDownMap = src.m_mouseButtonDownMap;
        m_deviceBoolOnMap = src.m_deviceBoolOnMap;
        m_deviceBoolOffMap = src.m_deviceBoolOffMap;
        m_deviceVarChangeMap = src.m_deviceVarChangeMap;
        
        CORE::CObservingNotifier::operator=( src );       
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CInputActionMap::TKeyStateActionMap&
CInputActionMap::GetKeyboardKeyDownMap( void )
{GUCEF_TRACE;

    return m_keyDownMap;
}

/*-------------------------------------------------------------------------*/

CInputActionMap::TKeyStateActionMap&
CInputActionMap::GetKeyboardKeyUpMap( void )
{GUCEF_TRACE;

    return m_keyUpMap;
}

/*-------------------------------------------------------------------------*/
    
CInputActionMap::TStateActionMap&
CInputActionMap::GetMouseButtonDownMap( const UInt32 mouseIndex )
{GUCEF_TRACE;

    return m_mouseButtonDownMap[ mouseIndex ];
}

/*-------------------------------------------------------------------------*/

CInputActionMap::TStateActionMap&
CInputActionMap::GetMouseButtonUpMap( const UInt32 mouseIndex )
{GUCEF_TRACE;

    return m_mouseButtonUpMap[ mouseIndex ];
}

/*-------------------------------------------------------------------------*/
    
CInputActionMap::TStateActionMap&
CInputActionMap::GetDeviceBoolOnMap( const UInt32 deviceIndex )
{GUCEF_TRACE;

    return m_deviceBoolOnMap[ deviceIndex ];
}

/*-------------------------------------------------------------------------*/
    
CInputActionMap::TStateActionMap&
CInputActionMap::GetDeviceBoolOffMap( const UInt32 deviceIndex )
{GUCEF_TRACE;

    return m_deviceBoolOffMap[ deviceIndex ];
}

/*-------------------------------------------------------------------------*/
    
CInputActionMap::TStateActionMap&
CInputActionMap::GetDeviceVarChanged( const UInt32 deviceIndex )
{GUCEF_TRACE;
    
    return m_deviceVarChangeMap[ deviceIndex ];
}

/*-------------------------------------------------------------------------*/

void
CInputActionMap::Clear( void )
{GUCEF_TRACE;

    m_keyUpMap.clear();
    m_keyDownMap.clear();
    m_mouseButtonUpMap.clear();
    m_mouseButtonDownMap.clear();
    m_deviceBoolOnMap.clear();
    m_deviceBoolOffMap.clear();
    m_deviceVarChangeMap.clear();
    m_name = GUCEF_NULL;      
}

/*-------------------------------------------------------------------------*/
                                            
bool 
CInputActionMap::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;
/*


        CORE::CString intstr;
        CORE::CDataNode newnode( "InputActionMap" );
        
        intstr.SetInt( m_mousemapcount );
        newnode.SetAttribute( "MouseButtonMapCount" ,
                              intstr                );

        intstr.SetInt( m_keyboardmapcount );
        newnode.SetAttribute( "KeyboardKeyMapCount" ,
                              intstr                );
                           
        intstr.SetInt( m_deviceboolmapcount );
        newnode.SetAttribute( "DeviceBoolMapCount" ,
                              intstr               );                           

        intstr.SetInt( m_devicevarmapcount );
        newnode.SetAttribute( "DeviceVarMapCount" ,
                              intstr              );

        CORE::CDataNode* listRoot = tree.AddChild( newnode );

        CORE::CString actionIndexStr( "ActionIndex" );
        CORE::CString actionNameStr( "ActionName" );
        CORE::CString buttonIndexStr( "ButtonIndex" );
        CORE::CString keyIndexStr( "KeyIndex" );

        newnode.SetName( "MouseButtonUp" );
        for ( UInt32 i=0; i<m_mousemapcount; ++i )
        {
                newnode.SetAttribute( buttonIndexStr, CORE::Int32ToString( i ) );
                newnode.SetAttribute( actionIndexStr, CORE::Int32ToString( m_mousebupmap[ i ] ) );
                newnode.SetAttribute( actionNameStr, m_mouseBUpActionNames[ i ] );
                
                listRoot->AddChild( newnode );
        }
        newnode.SetName( "MouseButtonDown" );
        for ( UInt32 i=0; i<m_mousemapcount; ++i )
        {
                newnode.SetAttribute( buttonIndexStr, CORE::Int32ToString( i ) );
                newnode.SetAttribute( actionIndexStr, CORE::Int32ToString( m_mousebdownmap[ i ] ) );
                newnode.SetAttribute( actionNameStr, m_mouseBDownActionNames[ i ] );
                
                listRoot->AddChild( newnode );
        }        
        newnode.SetName( "KeyboardKeyUp" );
        for ( UInt32 i=0; i<m_keyboardmapcount; ++i )
        {
                newnode.SetAttribute( keyIndexStr, CORE::Int32ToString( i ) );
                newnode.SetAttribute( actionIndexStr, CORE::Int32ToString( m_keyboardkupmap[ i ] ) );
                newnode.SetAttribute( actionNameStr, m_keyboardKUpActionNames[ i ] );
                
                listRoot->AddChild( newnode );
        }
        newnode.SetName( "KeyboardKeyDown" );
        for ( UInt32 i=0; i<m_keyboardmapcount; ++i )
        {
                newnode.SetAttribute( keyIndexStr, CORE::Int32ToString( i ) );
                newnode.SetAttribute( actionIndexStr, CORE::Int32ToString( m_keyboardkdownmap[ i ] ) );
                newnode.SetAttribute( actionNameStr, m_keyboardKDownActionNames[ i ] );
                
                listRoot->AddChild( newnode );
        }            */    
        return true;                                    
}

/*-------------------------------------------------------------------------*/
                                                               
bool
CInputActionMap::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;
/*

        Clear();
        
        if ( treeroot.GetName() == "InputActionMap" )
        {
                CORE::CString intstr;
                const CORE::CDataNode::TNodeAtt* att;
                UInt32 mouseButtonCount( 0UL );
                UInt32 keyboardKeyCount( 0UL );
                UInt32 deviceBoolCount( 0UL );
                UInt32 deviceVarCount( 0UL );
                
                att = treeroot.GetAttribute( "name" );
                if ( att )
                {
                        m_name = att->value;                                             
                }
                                
                CORE::CString actionIndexStr( "ActionIndex" );
                CORE::CString actionNameStr( "ActionName" );
                CORE::CString buttonIndexStr( "ButtonIndex" );                
                CORE::CString mouseButtonUpStr( "MouseButtonUp" );
                CORE::CString mouseButtonDownStr( "MouseButtonDown" );
                CORE::CString keyIndexStr( "KeyIndex" );
                CORE::CString keyboardKeyUpStr( "KeyboardKeyUp" );
                CORE::CString keyboardKeyDownStr( "KeyboardKeyDown" );
                                
                UInt32 inputIndex;
                UInt32 actionIndex;
                CORE::CString actionName;
                const CORE::CDataNode *cn;
                CORE::CDataNode::const_iterator i( treeroot.ConstBegin() );
                
                while ( i != treeroot.ConstEnd() )
                {
                        cn = (*i);
                        
                        if ( cn->GetName() == mouseButtonUpStr )
                        {
                                att = cn->GetAttribute( buttonIndexStr );
                                if ( att )
                                {
                                        inputIndex = att->value.GetInt();
                                        att = cn->GetAttribute( actionIndexStr );
                                        if ( att )
                                        {
                                                actionIndex = att->value.GetInt();
                                                att = cn->GetAttribute( actionNameStr );
                                                if ( att )
                                                {
                                                        actionName = att->value;
                                                        if ( actionName.Length() > 0 )
                                                        {
                                                                SetMouseButtonUpMap( inputIndex  ,
                                                                                     actionIndex ,
                                                                                     actionName  );
                                                        }        
                                                }        
                                        }
                                }
                        }
                        else
                        if ( cn->GetName() == mouseButtonDownStr )
                        {
                                att = cn->GetAttribute( buttonIndexStr );
                                if ( att )
                                {
                                        inputIndex = att->value.GetInt();
                                        att = cn->GetAttribute( actionIndexStr );
                                        if ( att )
                                        {
                                                actionIndex = att->value.GetInt();
                                                att = cn->GetAttribute( actionNameStr );
                                                if ( att )
                                                {
                                                        actionName = att->value;
                                                        if ( actionName.Length() > 0 )
                                                        {
                                                                SetMouseButtonDownMap( inputIndex  ,
                                                                                       actionIndex ,
                                                                                       actionName  );
                                                        }        
                                                }        
                                        }
                                }
                        }
                        else
                        if ( cn->GetName() == keyboardKeyUpStr )
                        {
                                att = cn->GetAttribute( keyIndexStr );
                                if ( att )
                                {
                                        inputIndex = att->value.GetInt();
                                        att = cn->GetAttribute( actionIndexStr );
                                        if ( att )
                                        {
                                                actionIndex = att->value.GetInt();
                                                att = cn->GetAttribute( actionNameStr );
                                                if ( att )
                                                {
                                                        actionName = att->value;
                                                        if ( actionName.Length() > 0 )
                                                        {
                                                                SetKeyboardKeyUpMap( inputIndex  ,
                                                                                     actionIndex ,
                                                                                     actionName  );
                                                        }        
                                                }        
                                        }
                                }
                        }
                        else
                        if ( cn->GetName() == keyboardKeyDownStr )
                        {
                                att = cn->GetAttribute( keyIndexStr );
                                if ( att )
                                {
                                        inputIndex = att->value.GetInt();
                                        att = cn->GetAttribute( actionIndexStr );
                                        if ( att )
                                        {
                                                actionIndex = att->value.GetInt();
                                                att = cn->GetAttribute( actionNameStr );
                                                if ( att )
                                                {
                                                        actionName = att->value;
                                                        if ( actionName.Length() > 0 )
                                                        {
                                                                SetKeyboardKeyDownMap( inputIndex  ,
                                                                                       actionIndex ,
                                                                                       actionName  );
                                                        }        
                                                }        
                                        }
                                }
                        }                                                                              
                        ++i;
                }                
           
        }
        */
        
        return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CInputActionMap::GetName( void ) const
{GUCEF_TRACE;
    
    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CInputActionMap::OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( CKeyboard::KeyStateChangedEvent == eventid )
    {
        CKeyStateChangedEventData* eData = static_cast< CKeyStateChangedEventData* >( eventdata );
        TKeyStateActionMap& map = eData->GetKeyPressedState() ? m_keyDownMap : m_keyUpMap;
        TKeyStateActionMap::iterator n = map.find( eData->GetKeyCode() );
        if ( n != map.end() )
        {
            TKeyStateModMapActionMap& keyModMap = (*n).second;
            TKeyStateModMapActionMap::iterator i = keyModMap.find( eData->GetKeyModPressedStates() );
            if ( i != keyModMap.end() )
            {
                TKeyStateChangedActionEventData actionData( (*i).second ,  // action ID
                                                            eventid     ,  // the trigger event
                                                            *eData      ); // the input event data
                NotifyObservers( ActionEvent, &actionData );
            }
        }
        return;
    } /*
    else
    if ( CKeyboard::KeyModStateChangedEvent == eventid )
    {
        CKeyModStateChangedEventData* eData = static_cast< CKeyModStateChangedEventData* >( eventdata );
        TKeyStateActionMap& map = eData->GetKeyPressedState() ? m_keyDownMap : m_keyUpMap;
        TKeyStateActionMap::iterator i = map.find( eData->GetModifier() );
        if ( i != map.end() )
        {
            TKeyModStateChangedActionEventData actionData( (*i).second ,  // action ID
                                                           eventid     ,  // the trigger event
                                                           *eData      ); // the input event data
            NotifyObservers( ActionEvent, &actionData );
        }
        return;
    } */
    else
    if ( CMouse::MouseButtonEvent == eventid )
    {
        CMouseButtonEventData* eData = static_cast< CMouseButtonEventData* >( eventdata );
        TStateActionMap& map = eData->GetPressedState() ? m_mouseButtonDownMap[ eData->GetDeviceID() ] : 
                                                          m_mouseButtonUpMap[ eData->GetDeviceID() ];
                
        TStateActionMap::iterator i = map.find( eData->GetButtonIndex() );
        if ( i != map.end() )
        {
            TMouseButtonActionEventData actionData( (*i).second ,  // action ID
                                                    eventid     ,  // the trigger event
                                                    *eData      ); // the input event data
            NotifyObservers( ActionEvent, &actionData );
        }
        return;        
    } /*
    else
    if ( CMouse::MouseMoveEvent == eventid )
    {
        CMouseMovedEventData* eData = static_cast< CMouseMovedEventData* >( eventdata );
        TStateActionMap& map = eData->GetPressedState() ? m_mouseButtonDownMap[ eData->GetDeviceID() ] : 
                                                          m_mouseButtonUpMap[ eData->GetDeviceID() ];
                
        TStateActionMap::iterator i = map.find( eData->GetButtonIndex() );
        if ( i != map.end() )
        {
            TMouseButtonActionEventData actionData( (*i).second ,  // action ID
                                                    eventid     ,  // the trigger event
                                                    *eData      ); // the input event data
            NotifyObservers( ActionEvent, &actionData );
        }
        return;    
    } */   
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
