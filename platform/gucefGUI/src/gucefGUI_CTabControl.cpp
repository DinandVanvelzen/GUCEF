/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CTABCONTENTPANE_H
#include "gucefGUI_CTabContentPane.h"
#define GUCEF_GUI_CTABCONTENTPANE_H
#endif /* GUCEF_GUI_CTABCONTENTPANE_H ? */

#include "gucefGUI_CTabControl.h"

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

const CORE::CEvent CTabControl::ActiveTabChangedEvent = "GUCEF::GUI::CTabControl::ActiveTabChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CTabControl::RegisterEvents( void )
{GUCEF_TRACE;

    ActiveTabChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CTabControl::CTabControl( void )
    : CWidget( "TabControl" )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CTabControl::~CTabControl()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CWidget*
CTabControl::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CTabControl();
}

/*-------------------------------------------------------------------------*/

bool
CTabControl::SetActiveTab( const UInt32 tabIndex )
{GUCEF_TRACE;

    return SetIntProperty( EWidgetProperty_ActiveTab, (Int32) tabIndex );
}

/*-------------------------------------------------------------------------*/

Int32
CTabControl::GetActiveTab( void ) const
{GUCEF_TRACE;

    return GetIntProperty( EWidgetProperty_ActiveTab );
}

/*-------------------------------------------------------------------------*/

CTabContentPane*
CTabControl::GetTabContentPane( const UInt32 tabIndex )
{GUCEF_TRACE;

    UInt32 index = 0;
    const TWidgetVector& children = GetChildren();
    for ( UInt32 i=0; i<(UInt32)children.size(); ++i )
    {
        CTabContentPane* pane = CWidget::As< CTabContentPane >( children[ i ] );
        if ( GUCEF_NULL == pane )
            continue;
        if ( index == tabIndex )
            return pane;
        ++index;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CTabControl::GetTabCount( void ) const
{GUCEF_TRACE;

    return (UInt32) GetStringListProperty( EWidgetProperty_Tabs ).size();
}

/*-------------------------------------------------------------------------*/

CRect
CTabControl::GetTabRect( UInt32 tabIndex ) const
{GUCEF_TRACE;

    UInt32 count = GetTabCount();
    if ( 0 == count || tabIndex >= count )
        return CRect();
    Int32 width = GetRect().width / (Int32) count;
    return CRect( (Int32) tabIndex * width, 0, width, GetIntProperty( EWidgetProperty_TabHeight ) );
}

/*-------------------------------------------------------------------------*/

void
CTabControl::ShowActivePane( void )
{GUCEF_TRACE;

    Int32 active = GetActiveTab();
    UInt32 index = 0;
    const TWidgetVector& children = GetChildren();
    for ( UInt32 i=0; i<(UInt32)children.size(); ++i )
    {
        CTabContentPane* pane = CWidget::As< CTabContentPane >( children[ i ] );
        if ( GUCEF_NULL == pane )
            continue;
        pane->SetVisibility( (Int32) index == active );
        ++index;
    }
}

/*-------------------------------------------------------------------------*/

void
CTabControl::OnChildrenChanged( void )
{GUCEF_TRACE;

    ShowActivePane();
}

/*-------------------------------------------------------------------------*/

void
CTabControl::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    if ( property == EWidgetProperty_ActiveTab )
    {
        ShowActivePane();
        TActiveTabChangedEventData eventData( (UInt32) GetActiveTab() );
        NotifyObservers( ActiveTabChangedEvent, &eventData );
    }
}

/*-------------------------------------------------------------------------*/

bool
CTabControl::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    UInt32 count = GetTabCount();
    if ( 0 == count )
        return false;

    if ( MouseClickEvent == eventId )
    {
        CPointerEventData* pointer = static_cast< CPointerEventData* >( &eventData );
        if ( MOUSEBUTTON_LEFT != pointer->button )
            return false;
        for ( UInt32 i=0; i<count; ++i )
        {
            if ( GetTabRect( i ).Contains( pointer->localX, pointer->localY ) )
            {
                ChangeValueByUser( EWidgetProperty_ActiveTab, CORE::Int32ToString( (Int32) i ) );
                return true;
            }
        }
        return false;
    }
    if ( NavigationEvent == eventId )
    {
        CNavigationEventData* nav = static_cast< CNavigationEventData* >( &eventData );
        Int32 active = GetActiveTab();
        if ( NAVIGATION_LEFT == nav->direction && active > 0 )
            return ChangeValueByUser( EWidgetProperty_ActiveTab, CORE::Int32ToString( active - 1 ) );
        if ( NAVIGATION_RIGHT == nav->direction && active + 1 < (Int32) count )
            return ChangeValueByUser( EWidgetProperty_ActiveTab, CORE::Int32ToString( active + 1 ) );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CString&
CTabControl::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CTabControl";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
