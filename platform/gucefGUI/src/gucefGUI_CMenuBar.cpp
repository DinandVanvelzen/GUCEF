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

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_UTILS_H
#include "gucefGUI_Utils.h"
#define GUCEF_GUI_UTILS_H
#endif /* GUCEF_GUI_UTILS_H ? */

#ifndef GUCEF_GUI_CPOPUPMENU_H
#include "gucefGUI_CPopupMenu.h"
#define GUCEF_GUI_CPOPUPMENU_H
#endif /* GUCEF_GUI_CPOPUPMENU_H ? */

#include "gucefGUI_CMenuBar.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


namespace {

Int32
RowHeightOf( const CWidget& widget )
{
    Int32 rowHeight = widget.GetIntProperty( EWidgetProperty_RowHeight );
    if ( rowHeight > 0 )
        return rowHeight;
    if ( GUCEF_NULL != widget.GetContext() )
        return widget.GetContext()->GetLineHeight( widget.GetProperty( EWidgetProperty_Font ) );
    return 16;
}

Int32
TextWidthOf( const CWidget& widget, const CString& text )
{
    Int32 width = 0, height = 0;
    if ( GUCEF_NULL != widget.GetContext() )
        widget.GetContext()->MeasureText( widget.GetProperty( EWidgetProperty_Font ), text, 0, width, height );
    return width;
}

}

CMenuBar::CMenuBar( void )
    : CWidget( "MenuBar" )
    , m_popup( GUCEF_NULL )
    , m_openMenuName()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CMenuBar::~CMenuBar()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    if ( GUCEF_NULL != m_popup && GUCEF_NULL != GetContext() )
        GetContext()->DestroyWidget( m_popup );
    m_popup = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CWidget*
CMenuBar::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CMenuBar();
}

/*-------------------------------------------------------------------------*/

CMenuBar::TStringVector
CMenuBar::GetMenus( void ) const
{GUCEF_TRACE;

    TStringVector items = GetStringListProperty( EWidgetProperty_Items );
    TStringVector menus;
    for ( UInt32 i=0; i<(UInt32)items.size(); ++i )
    {
        CString menu = items[ i ].SubstrToChar( '/', true );
        bool known = false;
        for ( UInt32 m=0; m<(UInt32)menus.size(); ++m )
            if ( menus[ m ] == menu )
                known = true;
        if ( !known )
            menus.push_back( menu );
    }
    return menus;
}

/*-------------------------------------------------------------------------*/

CMenuBar::TStringVector
CMenuBar::GetMenuItems( const CString& menu ) const
{GUCEF_TRACE;

    TStringVector items = GetStringListProperty( EWidgetProperty_Items );
    TStringVector result;
    CString prefix = menu + '/';
    for ( UInt32 i=0; i<(UInt32)items.size(); ++i )
        if ( items[ i ].Length() > prefix.Length() && items[ i ].SubstrFromRange( 0, prefix.Length() ) == prefix )
            result.push_back( items[ i ].SubstrFromRange( prefix.Length(), items[ i ].Length() ) );
    return result;
}

/*-------------------------------------------------------------------------*/

CRect
CMenuBar::GetMenuRect( UInt32 menuIndex ) const
{GUCEF_TRACE;

    TStringVector menus = GetMenus();
    Int32 x = 0;
    for ( UInt32 i=0; i<(UInt32)menus.size(); ++i )
    {
        Int32 width = TextWidthOf( *this, menus[ i ] ) + 16;
        if ( i == menuIndex )
            return CRect( x, 0, width, GetRect().height );
        x += width;
    }
    return CRect();
}

/*-------------------------------------------------------------------------*/

bool
CMenuBar::OpenMenu( Int32 menuIndex )
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    TStringVector menus = GetMenus();
    if ( GUCEF_NULL == context || menuIndex < 0 || menuIndex >= (Int32) menus.size() )
        return false;
    if ( GUCEF_NULL == m_popup )
    {
        m_popup = static_cast< CPopupMenu* >( context->CreateWidget( "PopupMenu", GetName() + "#Menu", GUCEF_NULL ) );
        if ( GUCEF_NULL == m_popup )
            return false;
        TEventCallback itemActivatedCallback( this, &CMenuBar::OnPopupItemActivated );
        SubscribeTo( m_popup, CWidget::ItemActivatedEvent, itemActivatedCallback );
        TEventCallback formHiddenCallback( this, &CMenuBar::OnFormHidden );
        SubscribeTo( context, CGUIContext::FormHiddenEvent, formHiddenCallback );
    }
    m_popup->SetItems( GetMenuItems( menus[ menuIndex ] ) );
    m_popup->SetProperty( EWidgetProperty_Font, GetProperty( EWidgetProperty_Font ) );
    m_popup->SetProperty( EWidgetProperty_TextColour, GetProperty( EWidgetProperty_TextColour ) );
    m_popup->SetProperty( EWidgetProperty_BackgroundColour, GetProperty( EWidgetProperty_BackgroundColour ) );
    m_popup->SetProperty( EWidgetProperty_HoverColour, GetProperty( EWidgetProperty_HoverColour ) );
    CRect title = GetMenuRect( (UInt32) menuIndex );
    m_openMenuName = menus[ menuIndex ];
    SetIntProperty( EWidgetProperty_OpenMenu, menuIndex );
    return m_popup->ShowAt( GetRect().left + title.left, GetRect().Bottom() );
}

/*-------------------------------------------------------------------------*/

bool
CMenuBar::CloseMenu( void )
{GUCEF_TRACE;

    SetIntProperty( EWidgetProperty_OpenMenu, -1 );
    if ( GUCEF_NULL == m_popup )
        return false;
    return m_popup->Close();
}

/*-------------------------------------------------------------------------*/

bool
CMenuBar::GetNaturalSize( Int32& width, Int32& height ) const
{GUCEF_TRACE;

    TStringVector menus = GetMenus();
    width = 0;
    for ( UInt32 i=0; i<(UInt32)menus.size(); ++i )
        width += TextWidthOf( *this, menus[ i ] ) + 16;
    height = RowHeightOf( *this ) + 4;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMenuBar::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    TStringVector menus = GetMenus();
    if ( MouseMoveEvent == eventId )
    {
        CPointerEventData* pointer = static_cast< CPointerEventData* >( &eventData );
        Int32 hover = -1;
        for ( UInt32 i=0; i<(UInt32)menus.size(); ++i )
            if ( GetMenuRect( i ).Contains( pointer->localX, pointer->localY ) )
                hover = (Int32) i;
        SetIntProperty( EWidgetProperty_HoverIndex, hover );
        return true;
    }
    if ( MouseFocusLostEvent == eventId )
    {
        SetIntProperty( EWidgetProperty_HoverIndex, -1 );
        return true;
    }
    if ( MouseClickEvent == eventId && MOUSEBUTTON_LEFT == static_cast< CPointerEventData* >( &eventData )->button )
    {
        CPointerEventData* pointer = static_cast< CPointerEventData* >( &eventData );
        for ( UInt32 i=0; i<(UInt32)menus.size(); ++i )
        {
            if ( GetMenuRect( i ).Contains( pointer->localX, pointer->localY ) )
            {
                if ( GetIntProperty( EWidgetProperty_OpenMenu ) == (Int32) i )
                    CloseMenu();
                else
                    OpenMenu( (Int32) i );
                return true;
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CMenuBar::OnPopupItemActivated( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the popup closes before it reports the item, so the menu was remembered when it opened
    CItemEventData* item = static_cast< CItemEventData* >( eventData );
    SetIntProperty( EWidgetProperty_OpenMenu, -1 );
    if ( GUCEF_NULL != item )
        ActivateItem( item->index, m_openMenuName + '/' + item->item );
}

/*-------------------------------------------------------------------------*/

void
CMenuBar::OnFormHidden( CORE::CNotifier* notifier    ,
                        const CORE::CEvent& eventId  ,
                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_popup && GUCEF_NULL != GetContext() && GetContext()->GetPopup() != m_popup )
        SetIntProperty( EWidgetProperty_OpenMenu, -1 );
}

/*-------------------------------------------------------------------------*/

const CString&
CMenuBar::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CMenuBar";
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
