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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CTABCONTROL_H
#include "gucefGUI_CTabControl.h"
#define GUCEF_GUI_CTABCONTROL_H
#endif /* GUCEF_GUI_CTABCONTROL_H ? */

#ifndef GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H
#include "gucefGUI_CKeyboardShortcutMap.h"
#define GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H
#endif /* GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H ? */

#include "gucefGUI_CTabAreaNavigator.h"

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

const CORE::CEvent CTabAreaNavigator::PageChangedEvent = "GUCEF::GUI::CTabAreaNavigator::PageChangedEvent";

const char* CTabAreaNavigator::ActionAreaPrefix = "area.";
const char* CTabAreaNavigator::ActionNextSubView = "subview.next";
const char* CTabAreaNavigator::ActionPreviousSubView = "subview.previous";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CTabAreaNavigator::RegisterEvents( void )
{GUCEF_TRACE;

    PageChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CTabAreaNavigator::CTabAreaNavigator( void )
    : CORE::CObservingNotifier()
    , m_areas( GUCEF_NULL )
    , m_subViews()
    , m_lastSubView()
    , m_pages()
    , m_currentPage()
    , m_selecting( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CTabAreaNavigator::~CTabAreaNavigator()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CTabAreaNavigator::Subscribe( CTabControl* control )
{GUCEF_TRACE;

    if ( GUCEF_NULL == control )
        return;
    TEventCallback callback( this, &CTabAreaNavigator::OnActiveTabChanged );
    SubscribeTo( control, CTabControl::ActiveTabChangedEvent, callback );
}

/*-------------------------------------------------------------------------*/

void
CTabAreaNavigator::Unsubscribe( CTabControl* control )
{GUCEF_TRACE;

    if ( GUCEF_NULL == control )
        return;

    // a control may serve as the areas and as the sub-views of an area at once: it stays subscribed while it has a role
    if ( control == m_areas )
        return;
    for ( TControlMap::const_iterator i=m_subViews.begin(); i!=m_subViews.end(); ++i )
        if ( (*i).second == control )
            return;
    UnsubscribeFrom( control );
}

/*-------------------------------------------------------------------------*/

bool
CTabAreaNavigator::SetAreaControl( CTabControl* areas )
{GUCEF_TRACE;

    CTabControl* previous = m_areas;
    m_areas = areas;
    Unsubscribe( previous );
    Subscribe( areas );
    UpdateCurrentPage();
    return true;
}

/*-------------------------------------------------------------------------*/

CTabControl*
CTabAreaNavigator::GetAreaControl( void ) const
{GUCEF_TRACE;

    return m_areas;
}

/*-------------------------------------------------------------------------*/

bool
CTabAreaNavigator::SetSubViewControl( UInt32 area, CTabControl* subViews )
{GUCEF_TRACE;

    CTabControl* previous = GetSubViewControl( area );
    if ( GUCEF_NULL == subViews )
        m_subViews.erase( area );
    else
        m_subViews[ area ] = subViews;
    Unsubscribe( previous );
    Subscribe( subViews );
    UpdateCurrentPage();
    return true;
}

/*-------------------------------------------------------------------------*/

CTabControl*
CTabAreaNavigator::GetSubViewControl( UInt32 area ) const
{GUCEF_TRACE;

    TControlMap::const_iterator i = m_subViews.find( area );
    return i != m_subViews.end() ? (*i).second : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CTabAreaNavigator::AddPage( const CString& pageId, UInt32 area, Int32 subView )
{GUCEF_TRACE;

    if ( pageId.IsNULLOrEmpty() )
        return false;
    RemovePage( pageId );
    TPage page;
    page.id = pageId;
    page.area = area;
    page.subView = subView < 0 ? -1 : subView;
    m_pages.push_back( page );
    UpdateCurrentPage();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTabAreaNavigator::RemovePage( const CString& pageId )
{GUCEF_TRACE;

    for ( TPageVector::iterator i=m_pages.begin(); i!=m_pages.end(); ++i )
    {
        if ( (*i).id == pageId )
        {
            m_pages.erase( i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTabAreaNavigator::FindPage( const CString& pageId, UInt32& area, Int32& subView ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_pages.size(); ++i )
    {
        if ( m_pages[ i ].id == pageId )
        {
            area = m_pages[ i ].area;
            subView = m_pages[ i ].subView;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CString
CTabAreaNavigator::GetPageAt( UInt32 area, Int32 subView ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_pages.size(); ++i )
        if ( m_pages[ i ].area == area && m_pages[ i ].subView == ( subView < 0 ? -1 : subView ) )
            return m_pages[ i ].id;
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString
CTabAreaNavigator::GetCurrentPage( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_areas || m_areas->GetActiveTab() < 0 )
        return CString::Empty;
    const UInt32 area = (UInt32) m_areas->GetActiveTab();
    const CTabControl* subViews = GetSubViewControl( area );
    if ( GUCEF_NULL != subViews && subViews->GetActiveTab() >= 0 )
    {
        const CString page = GetPageAt( area, subViews->GetActiveTab() );
        if ( !page.IsNULLOrEmpty() )
            return page;
    }
    return GetPageAt( area, -1 );
}

/*-------------------------------------------------------------------------*/

void
CTabAreaNavigator::UpdateCurrentPage( void )
{GUCEF_TRACE;

    if ( m_selecting )
        return;
    const CString page = GetCurrentPage();
    if ( page == m_currentPage )
        return;
    m_currentPage = page;
    if ( page.IsNULLOrEmpty() )
        return;
    CORE::TCloneableString data( page );
    NotifyObservers( PageChangedEvent, &data );
}

/*-------------------------------------------------------------------------*/

bool
CTabAreaNavigator::SelectPage( const CString& pageId )
{GUCEF_TRACE;

    UInt32 area = 0;
    Int32 subView = -1;
    if ( GUCEF_NULL == m_areas || !FindPage( pageId, area, subView ) )
        return false;
    m_selecting = true;
    CTabControl* subViews = GetSubViewControl( area );
    if ( GUCEF_NULL != subViews && subView >= 0 )
    {
        subViews->SetActiveTab( (UInt32) subView );
        m_lastSubView[ area ] = subView;
    }
    m_areas->SetActiveTab( area );
    m_selecting = false;
    UpdateCurrentPage();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTabAreaNavigator::SelectArea( UInt32 area )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_areas || area >= m_areas->GetTabCount() )
        return false;
    m_selecting = true;
    CTabControl* subViews = GetSubViewControl( area );
    TSubViewMap::const_iterator last = m_lastSubView.find( area );
    if ( GUCEF_NULL != subViews && last != m_lastSubView.end() && (*last).second >= 0 && (UInt32) (*last).second < subViews->GetTabCount() )
        subViews->SetActiveTab( (UInt32) (*last).second );
    m_areas->SetActiveTab( area );
    m_selecting = false;
    UpdateCurrentPage();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTabAreaNavigator::NextSubView( bool backwards )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_areas || m_areas->GetActiveTab() < 0 )
        return false;
    const UInt32 area = (UInt32) m_areas->GetActiveTab();
    CTabControl* subViews = GetSubViewControl( area );
    if ( GUCEF_NULL == subViews || 0 == subViews->GetTabCount() )
        return false;
    const Int32 count = (Int32) subViews->GetTabCount();
    const Int32 active = subViews->GetActiveTab() < 0 ? 0 : subViews->GetActiveTab();
    const Int32 next = ( active + ( backwards ? count - 1 : 1 ) ) % count;
    m_selecting = true;
    subViews->SetActiveTab( (UInt32) next );
    m_lastSubView[ area ] = next;
    m_selecting = false;
    UpdateCurrentPage();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTabAreaNavigator::OnActiveTabChanged( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_selecting )
        return;

    // a sub-view chosen by the user is the one the area returns to
    for ( TControlMap::const_iterator i=m_subViews.begin(); i!=m_subViews.end(); ++i )
    {
        if ( static_cast< CORE::CNotifier* >( (*i).second ) == notifier )
            m_lastSubView[ (*i).first ] = (*i).second->GetActiveTab();
    }
    UpdateCurrentPage();
}

/*-------------------------------------------------------------------------*/

void
CTabAreaNavigator::BindDefaultShortcuts( CKeyboardShortcutMap& map ) const
{GUCEF_TRACE;

    for ( UInt32 i=1; i<=9; ++i )
        map.Bind( CString( ActionAreaPrefix ) + CORE::UInt32ToString( i ), (INPUT::KeyCode) ( (UInt32) INPUT::KEYCODE_1 + i - 1 ), INPUT::KEYMOD_CTRL );
    map.Bind( ActionNextSubView, INPUT::KEYCODE_TAB, INPUT::KEYMOD_CTRL );
    map.Bind( ActionPreviousSubView, INPUT::KEYCODE_TAB, (UInt32) INPUT::KEYMOD_CTRL | (UInt32) INPUT::KEYMOD_SHIFT );
}

/*-------------------------------------------------------------------------*/

bool
CTabAreaNavigator::HandleShortcut( const CString& actionId )
{GUCEF_TRACE;

    if ( actionId == ActionNextSubView )
        return NextSubView( false );
    if ( actionId == ActionPreviousSubView )
        return NextSubView( true );
    const CString prefix( ActionAreaPrefix );
    if ( actionId.Length() > prefix.Length() && actionId.SubstrToIndex( prefix.Length(), true ) == prefix )
    {
        const Int32 number = CORE::StringToInt32( actionId.CutChars( prefix.Length(), true ), 0 );
        if ( number >= 1 )
            return SelectArea( (UInt32) ( number - 1 ) );
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
