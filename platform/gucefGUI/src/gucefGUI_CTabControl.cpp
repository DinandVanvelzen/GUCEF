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

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CTABCONTENTPANE_H
#include "gucefGUI_CTabContentPane.h"
#define GUCEF_GUI_CTABCONTENTPANE_H
#endif /* GUCEF_GUI_CTABCONTENTPANE_H ? */

#ifndef GUCEF_GUI_CPOPUPMENU_H
#include "gucefGUI_CPopupMenu.h"
#define GUCEF_GUI_CPOPUPMENU_H
#endif /* GUCEF_GUI_CPOPUPMENU_H ? */

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
    , m_firstTab( 0 )
    , m_overflowMenu( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CTabControl::~CTabControl()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();

    // the menu is a root widget of the context; when the context goes first it has deleted it already, and
    // DestroyWidget() ignores a widget it does not have
    CGUIContext* context = GetContext();
    if ( GUCEF_NULL != m_overflowMenu && GUCEF_NULL != context )
    {
        if ( context->GetPopup() == m_overflowMenu )
            context->ClosePopup();
        context->DestroyWidget( m_overflowMenu );
    }
    m_overflowMenu = GUCEF_NULL;
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
    if ( IsContentSized() )
    {
        TRectVector rects;
        bool overflow = false;
        ComputeContentLayout( rects, overflow );
        return tabIndex < (UInt32) rects.size() ? rects[ tabIndex ] : CRect();
    }
    Int32 width = GetRect().width / (Int32) count;
    return CRect( (Int32) tabIndex * width, 0, width, GetIntProperty( EWidgetProperty_TabHeight ) );
}

/*-------------------------------------------------------------------------*/

bool
CTabControl::IsContentSized( void ) const
{GUCEF_TRACE;

    return "Content" == GetPropertyRef( EWidgetProperty_TabSizing );
}

/*-------------------------------------------------------------------------*/

CString
CTabControl::GetTabTitle( UInt32 tabIndex ) const
{GUCEF_TRACE;

    const TStringVector titles = GetStringListProperty( EWidgetProperty_Tabs );
    return tabIndex < (UInt32) titles.size() ? titles[ tabIndex ] : CString::Empty;
}

/*-------------------------------------------------------------------------*/

Int32
CTabControl::MeasureWidth( const CString& text ) const
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
        return 0;
    CGUIContext* context = GetContext();
    Int32 width = 0, height = 0;
    if ( GUCEF_NULL != context && context->MeasureText( GetPropertyRef( EWidgetProperty_Font ), text, 0, width, height ) )
        return width;
    return (Int32) text.Length() * 8;
}

/*-------------------------------------------------------------------------*/

Int32
CTabControl::GetTabBadgeRoom( UInt32 tabIndex ) const
{GUCEF_TRACE;

    const CString badge = GetTabBadge( tabIndex );
    if ( badge.IsNULLOrEmpty() )
        return 0;
    if ( "*" == badge )
        return (Int32) BADGE_DOT_SIZE + (Int32) BADGE_GAP;
    return MeasureWidth( badge ) + 2 * (Int32) BADGE_PADDING + (Int32) BADGE_GAP;
}

/*-------------------------------------------------------------------------*/

Int32
CTabControl::GetTabWidth( UInt32 tabIndex ) const
{GUCEF_TRACE;

    if ( tabIndex >= GetTabCount() )
        return 0;
    const Int32 width = MeasureWidth( GetTabTitle( tabIndex ) ) + 2 * (Int32) TAB_TEXT_PADDING + GetTabBadgeRoom( tabIndex );
    return width > (Int32) MIN_TAB_WIDTH ? width : (Int32) MIN_TAB_WIDTH;
}

/*-------------------------------------------------------------------------*/

void
CTabControl::ComputeContentLayout( TRectVector& rects, bool& overflow ) const
{GUCEF_TRACE;

    const UInt32 count = GetTabCount();
    rects.assign( count, CRect() );
    overflow = false;
    if ( 0 == count )
        return;
    const Int32 height = GetIntProperty( EWidgetProperty_TabHeight );
    const Int32 controlWidth = GetRect().width;
    GUCEF::vector< Int32 > widths( count, 0 );
    Int32 total = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        widths[ i ] = GetTabWidth( i );
        total += widths[ i ];
    }

    Int32 first = 0;
    Int32 last = (Int32) count - 1;
    if ( total > controlWidth )
    {
        // the overflow button takes a square at the right end; the tabs shown follow the active one
        overflow = true;
        const Int32 room = controlWidth - height;
        const Int32 active = GetActiveTab();
        first = m_firstTab < 0 ? 0 : ( m_firstTab >= (Int32) count ? (Int32) count - 1 : m_firstTab );
        if ( active >= 0 && active < (Int32) count )
        {
            if ( active < first )
                first = active;
            Int32 span = 0;
            for ( Int32 i=first; i<=active; ++i )
                span += widths[ i ];
            while ( first < active && span > room )
            {
                span -= widths[ first ];
                ++first;
            }
        }
        // as many tabs after the first as fit, then as many before it as still fit
        last = first;
        Int32 used = widths[ first ];
        while ( last + 1 < (Int32) count && used + widths[ last + 1 ] <= room )
        {
            ++last;
            used += widths[ last ];
        }
        while ( first > 0 && used + widths[ first - 1 ] <= room )
        {
            --first;
            used += widths[ first ];
        }
        m_firstTab = first;
    }
    else
        m_firstTab = 0;

    Int32 x = 0;
    for ( Int32 i=first; i<=last; ++i )
    {
        rects[ i ] = CRect( x, 0, widths[ i ], height );
        x += widths[ i ];
    }
}

/*-------------------------------------------------------------------------*/

bool
CTabControl::HasOverflow( void ) const
{GUCEF_TRACE;

    if ( !IsContentSized() )
        return false;
    TRectVector rects;
    bool overflow = false;
    ComputeContentLayout( rects, overflow );
    return overflow;
}

/*-------------------------------------------------------------------------*/

CRect
CTabControl::GetOverflowButtonRect( void ) const
{GUCEF_TRACE;

    if ( !HasOverflow() )
        return CRect();
    const Int32 height = GetIntProperty( EWidgetProperty_TabHeight );
    return CRect( GetRect().width - height, 0, height, height );
}

/*-------------------------------------------------------------------------*/

Int32
CTabControl::GetFirstVisibleTab( void ) const
{GUCEF_TRACE;

    if ( !IsContentSized() )
        return 0;
    TRectVector rects;
    bool overflow = false;
    ComputeContentLayout( rects, overflow );
    return m_firstTab;
}

/*-------------------------------------------------------------------------*/

bool
CTabControl::IsTabVisible( UInt32 tabIndex ) const
{GUCEF_TRACE;

    return !GetTabRect( tabIndex ).IsEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CTabControl::SetTabBadge( UInt32 tabIndex, const CString& badge )
{GUCEF_TRACE;

    if ( tabIndex >= GetTabCount() )
        return false;
    TStringVector badges = GetStringListProperty( EWidgetProperty_TabBadges );
    if ( tabIndex >= (UInt32) badges.size() )
    {
        if ( badge.IsNULLOrEmpty() )
            return true;
        badges.resize( tabIndex + 1 );
    }
    badges[ tabIndex ] = badge;

    // no empty entries at the end
    while ( !badges.empty() && badges.back().IsNULLOrEmpty() )
        badges.pop_back();
    return SetStringListProperty( EWidgetProperty_TabBadges, badges );
}

/*-------------------------------------------------------------------------*/

CString
CTabControl::GetTabBadge( UInt32 tabIndex ) const
{GUCEF_TRACE;

    const TStringVector badges = GetStringListProperty( EWidgetProperty_TabBadges );
    return tabIndex < (UInt32) badges.size() ? badges[ tabIndex ] : CString::Empty;
}

/*-------------------------------------------------------------------------*/

CRect
CTabControl::GetTabBadgeRect( UInt32 tabIndex ) const
{GUCEF_TRACE;

    const CString badge = GetTabBadge( tabIndex );
    if ( badge.IsNULLOrEmpty() )
        return CRect();
    const CRect tab = GetTabRect( tabIndex );
    if ( tab.IsEmpty() )
        return CRect();
    if ( "*" == badge )
        return CRect( tab.Right() - (Int32) TAB_TEXT_PADDING - (Int32) BADGE_DOT_SIZE, tab.top + ( tab.height - (Int32) BADGE_DOT_SIZE ) / 2, (Int32) BADGE_DOT_SIZE, (Int32) BADGE_DOT_SIZE );
    const Int32 width = MeasureWidth( badge ) + 2 * (Int32) BADGE_PADDING;
    CGUIContext* context = GetContext();
    Int32 lineHeight = GUCEF_NULL != context ? context->GetLineHeight( GetPropertyRef( EWidgetProperty_Font ) ) : 16;
    Int32 height = lineHeight + 2;
    if ( height > tab.height - 2 )
        height = tab.height - 2;
    if ( height < 1 )
        height = 1;
    return CRect( tab.Right() - (Int32) TAB_TEXT_PADDING - width, tab.top + ( tab.height - height ) / 2, width, height );
}

/*-------------------------------------------------------------------------*/

bool
CTabControl::OpenOverflowMenu( void )
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    const UInt32 count = GetTabCount();
    if ( GUCEF_NULL == context || 0 == count )
        return false;
    if ( GUCEF_NULL == m_overflowMenu )
    {
        m_overflowMenu = CWidget::As< CPopupMenu >( context->CreateWidget( CPopupMenu::GetRegisteredTypeName(), GetName() + "#TabMenu", GUCEF_NULL ) );
        if ( GUCEF_NULL == m_overflowMenu )
            return false;
        m_overflowMenu->SetProperty( EWidgetProperty_Font, GetPropertyRef( EWidgetProperty_Font ) );
        TEventCallback callback( this, &CTabControl::OnOverflowMenuItemActivated );
        SubscribeTo( m_overflowMenu, CWidget::ItemActivatedEvent, callback );
    }

    // every tab, the active one marked, with its badge
    const Int32 active = GetActiveTab();
    CPopupMenu::TStringVector items;
    for ( UInt32 i=0; i<count; ++i )
    {
        CString item = ( (Int32) i == active ? "* " : "  " ) + GetTabTitle( i );
        const CString badge = GetTabBadge( i );
        if ( !badge.IsNULLOrEmpty() )
            item += "*" == badge ? CString( "  (new)" ) : "  (" + badge + ")";
        items.push_back( item );
    }
    m_overflowMenu->SetItems( items );
    CRect button = GetOverflowButtonRect();
    if ( button.IsEmpty() )
        button = CRect( GetRect().width - GetIntProperty( EWidgetProperty_TabHeight ), 0, GetIntProperty( EWidgetProperty_TabHeight ), GetIntProperty( EWidgetProperty_TabHeight ) );
    return m_overflowMenu->ShowAt( GetRect().left + button.left, GetRect().top + button.Bottom() );
}

/*-------------------------------------------------------------------------*/

bool
CTabControl::IsOverflowMenuOpen( void ) const
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    return GUCEF_NULL != context && GUCEF_NULL != m_overflowMenu && context->GetPopup() == m_overflowMenu;
}

/*-------------------------------------------------------------------------*/

void
CTabControl::OnOverflowMenuItemActivated( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    CItemEventData* item = static_cast< CItemEventData* >( eventData );
    if ( EVENTPHASE_TUNNEL == item->phase )
        return;
    CGUIContext* context = GetContext();
    if ( GUCEF_NULL != context && context->GetPopup() == m_overflowMenu )
        context->ClosePopup();
    if ( item->index >= 0 && item->index < (Int32) GetTabCount() )
        ChangeValueByUser( EWidgetProperty_ActiveTab, CORE::Int32ToString( item->index ) );
}

/*-------------------------------------------------------------------------*/

Int32
CTabControl::GetTabAt( Int32 localX, Int32 localY ) const
{GUCEF_TRACE;

    UInt32 count = GetTabCount();
    for ( UInt32 i=0; i<count; ++i )
        if ( GetTabRect( i ).Contains( localX, localY ) )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CTabControl::GetTabTextRoom( UInt32 tabIndex ) const
{GUCEF_TRACE;

    Int32 room = GetTabRect( tabIndex ).width - 2 * TAB_TEXT_PADDING - GetTabBadgeRoom( tabIndex );
    return room > 0 ? room : 0;
}

/*-------------------------------------------------------------------------*/

bool
CTabControl::IsTabTitleCut( UInt32 tabIndex ) const
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    const TStringVector titles = GetStringListProperty( EWidgetProperty_Tabs );
    if ( GUCEF_NULL == context || tabIndex >= (UInt32) titles.size() || titles[ tabIndex ].IsNULLOrEmpty() )
        return false;
    Int32 width = 0, height = 0;
    context->MeasureText( GetPropertyRef( EWidgetProperty_Font ), titles[ tabIndex ], 0, width, height );
    return width > GetTabTextRoom( tabIndex );
}

/*-------------------------------------------------------------------------*/

CString
CTabControl::GetTooltipAt( Int32 localX, Int32 localY ) const
{GUCEF_TRACE;

    if ( "Ellipsis" == GetPropertyRef( EWidgetProperty_TextOverflow ) )
    {
        const Int32 tab = GetTabAt( localX, localY );
        if ( tab >= 0 && IsTabTitleCut( (UInt32) tab ) )
            return GetStringListProperty( EWidgetProperty_Tabs )[ (UInt32) tab ];
    }
    return CWidget::GetTooltipAt( localX, localY );
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
        if ( IsContentSized() && GetOverflowButtonRect().Contains( pointer->localX, pointer->localY ) )
            return OpenOverflowMenu();
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
