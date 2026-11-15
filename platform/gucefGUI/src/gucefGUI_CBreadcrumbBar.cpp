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

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CPOPUPMENU_H
#include "gucefGUI_CPopupMenu.h"
#define GUCEF_GUI_CPOPUPMENU_H
#endif /* GUCEF_GUI_CPOPUPMENU_H ? */

#ifndef GUCEF_GUI_CWIDGETPAINTER_H
#include "gucefGUI_CWidgetPainter.h"
#define GUCEF_GUI_CWIDGETPAINTER_H
#endif /* GUCEF_GUI_CWIDGETPAINTER_H ? */

#include "gucefGUI_CBreadcrumbBar.h"

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

const CORE::CEvent CBreadcrumbBar::ItemClickedEvent = "GUCEF::GUI::CBreadcrumbBar::ItemClickedEvent";

const Int32 CBreadcrumbBar::OverflowItem = -2;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Without a context or a font: a guess of the width of a character and of the height of a line */
const Int32 FallbackCharWidth = 8;
const Int32 FallbackLineHeight = 16;

const char* const OverflowText = "...";
const char* const SeparatorText = ">";

CColour
ColourOr( const CWidget& widget, EWidgetProperty property, const CColour& fallback )
{
    CColour colour;
    if ( !widget.TryGetColourProperty( property, colour ) )
        return fallback;
    return colour;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CBreadcrumbBar::SItem::SItem( void )
    : id()
    , text()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBreadcrumbBar::SItem::SItem( const CString& itemId, const CString& itemText )
    : id( itemId )
    , text( itemText )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CBreadcrumbBar::RegisterEvents( void )
{GUCEF_TRACE;

    ItemClickedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CBreadcrumbBar::CBreadcrumbBar( void )
    : CDrawSurface( GetRegisteredTypeName() )
    , m_items()
    , m_itemRects()
    , m_shownTexts()
    , m_collapsed()
    , m_overflowRect()
    , m_hovered( -1 )
    , m_focused( -1 )
    , m_dirty( true )
    , m_drawnFocus( false )
    , m_builtWidth( -1 )
    , m_builtHeight( -1 )
    , m_menu( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CBreadcrumbBar::~CBreadcrumbBar()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();

    // the menu is a root widget of the context; when the context goes first it has deleted it already, and
    // DestroyWidget() ignores a widget it does not have
    CGUIContext* context = GetContext();
    if ( GUCEF_NULL != m_menu && GUCEF_NULL != context )
    {
        if ( context->GetPopup() == m_menu )
            context->ClosePopup();
        context->DestroyWidget( m_menu );
    }
    m_menu = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CWidget*
CBreadcrumbBar::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CBreadcrumbBar();
}

/*-------------------------------------------------------------------------*/

const CString&
CBreadcrumbBar::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CBreadcrumbBar";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CBreadcrumbBar::SetItems( const TItemVector& items )
{GUCEF_TRACE;

    m_items = items;
    m_hovered = -1;
    m_focused = -1;
    m_dirty = true;
    const CRect& rect = GetRect();
    Layout( rect.width, rect.height );
}

/*-------------------------------------------------------------------------*/

void
CBreadcrumbBar::AddItem( const CString& id, const CString& text )
{GUCEF_TRACE;

    m_items.push_back( TItem( id, text ) );
    m_dirty = true;
    const CRect& rect = GetRect();
    Layout( rect.width, rect.height );
}

/*-------------------------------------------------------------------------*/

void
CBreadcrumbBar::ClearItems( void )
{GUCEF_TRACE;

    SetItems( TItemVector() );
}

/*-------------------------------------------------------------------------*/

UInt32
CBreadcrumbBar::GetItemCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_items.size();
}

/*-------------------------------------------------------------------------*/

const CBreadcrumbBar::TItem&
CBreadcrumbBar::GetItem( UInt32 index ) const
{GUCEF_TRACE;

    static const TItem empty;
    return index < (UInt32) m_items.size() ? m_items[ index ] : empty;
}

/*-------------------------------------------------------------------------*/

const CBreadcrumbBar::TItemVector&
CBreadcrumbBar::GetItems( void ) const
{GUCEF_TRACE;

    return m_items;
}

/*-------------------------------------------------------------------------*/

bool
CBreadcrumbBar::ClickItem( UInt32 index )
{GUCEF_TRACE;

    // the last item is where the user is: nothing to go to
    if ( index + 1 >= (UInt32) m_items.size() )
        return false;
    CItemEventData data;
    data.source = this;
    data.index = (Int32) index;
    data.item = m_items[ index ].id;
    NotifyObservers( ItemClickedEvent, &data );
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
CBreadcrumbBar::TextWidth( const CString& text ) const
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
        return 0;
    CGUIContext* context = GetContext();
    Int32 width = 0, height = 0;
    if ( GUCEF_NULL != context && context->MeasureText( GetPropertyRef( EWidgetProperty_Font ), text, 0, width, height ) )
        return width;
    return (Int32) text.Length() * FallbackCharWidth;
}

/*-------------------------------------------------------------------------*/

Int32
CBreadcrumbBar::LineHeight( void ) const
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    if ( GUCEF_NULL == context )
        return FallbackLineHeight;
    const Int32 height = context->GetLineHeight( GetPropertyRef( EWidgetProperty_Font ) );
    return height > 0 ? height : FallbackLineHeight;
}

/*-------------------------------------------------------------------------*/

void
CBreadcrumbBar::Layout( Int32 width, Int32 height )
{GUCEF_TRACE;

    m_builtWidth = width;
    m_builtHeight = height;
    m_dirty = false;
    const UInt32 count = (UInt32) m_items.size();
    m_itemRects.assign( count, CRect() );
    m_shownTexts.assign( count, CString() );
    m_collapsed.clear();
    m_overflowRect = CRect();
    if ( 0 == count || width <= 0 || height <= 0 )
    {
        Draw();
        return;
    }

    const Int32 padding = 2 * (Int32) ITEM_PADDING;
    const Int32 separator = (Int32) SEPARATOR_ROOM;
    TInt32Vector widths( count, 0 );
    Int32 total = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        widths[ i ] = TextWidth( m_items[ i ].text ) + padding;
        total += widths[ i ] + ( i > 0 ? separator : 0 );
        m_shownTexts[ i ] = m_items[ i ].text;
    }

    // too wide: the items after the first go into the "..." item, from the front, until the rest fits
    UInt32 collapsedUpTo = 0;          // items 1 .. collapsedUpTo are collapsed
    const Int32 overflowWidth = TextWidth( OverflowText ) + padding;
    if ( total > width && count > 2 )
    {
        Int32 shown = total;
        for ( UInt32 k=1; k+1<count; ++k )
        {
            // the first collapse puts the "..." item in the place of item 1, the next ones only take items away
            shown -= widths[ k ] + separator;
            if ( 1 == k )
                shown += overflowWidth + separator;
            collapsedUpTo = k;
            if ( shown <= width )
                break;
        }
        total = shown;
        for ( UInt32 i=1; i<=collapsedUpTo; ++i )
            m_collapsed.push_back( (Int32) i );
    }

    // still too wide: the last item is cut with "..."
    if ( total > width )
    {
        const UInt32 last = count - 1;
        const Int32 room = widths[ last ] - ( total - width );
        bool cut = false;
        CGUIContext* context = GetContext();
        if ( GUCEF_NULL != context )
            m_shownTexts[ last ] = CWidgetPainter::FitText( *context, GetPropertyRef( EWidgetProperty_Font ), m_items[ last ].text, room - padding, cut );
        widths[ last ] = room > 0 ? room : 0;
    }

    // the places, left to right
    Int32 x = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( i > 0 && i <= collapsedUpTo )
        {
            if ( 1 == i )
            {
                x += separator;
                m_overflowRect = CRect( x, 0, overflowWidth, height );
                x += overflowWidth;
            }
            continue;
        }
        if ( i > 0 )
            x += separator;
        m_itemRects[ i ] = CRect( x, 0, widths[ i ], height );
        x += widths[ i ];
    }
    Draw();
}

/*-------------------------------------------------------------------------*/

void
CBreadcrumbBar::Draw( void )
{GUCEF_TRACE;

    Clear();
    m_drawnFocus = HasFocus();
    const CString& font = GetPropertyRef( EWidgetProperty_Font );
    const CColour textColour = ColourOr( *this, EWidgetProperty_TextColour, CColour( 240, 239, 236, 255 ) );
    const CColour linkColour = ColourOr( *this, EWidgetProperty_SelectionColour, CColour( 120, 170, 255, 255 ) );
    const CColour hoverColour = ColourOr( *this, EWidgetProperty_HoverColour, CColour( 255, 255, 255, 30 ) );
    CColour separatorColour = textColour;
    separatorColour.a = (UInt8) ( (UInt32) separatorColour.a * 160 / 255 );
    const Int32 lineHeight = LineHeight();
    const Int32 count = (Int32) m_items.size();

    for ( Int32 i=0; i<count; ++i )
    {
        const CRect& rect = m_itemRects[ (UInt32) i ];
        if ( rect.IsEmpty() && !( 1 == i && !m_overflowRect.IsEmpty() ) )
            continue;

        // the "..." item stands in the place of the collapsed items
        const bool overflow = rect.IsEmpty();
        const CRect& place = overflow ? m_overflowRect : rect;
        const Int32 key = overflow ? OverflowItem : i;
        const Int32 textY = place.top + ( place.height - lineHeight ) / 2;
        if ( i > 0 )
        {
            const Int32 separatorWidth = TextWidth( SeparatorText );
            DrawString( font, SeparatorText, place.left - (Int32) SEPARATOR_ROOM + ( (Int32) SEPARATOR_ROOM - separatorWidth ) / 2, textY, separatorColour );
        }
        const bool clickable = overflow || i + 1 < count;
        if ( clickable && key == m_hovered )
            FillRect( place, hoverColour );
        DrawString( font, overflow ? CString( OverflowText ) : m_shownTexts[ (UInt32) i ], place.left + (Int32) ITEM_PADDING, textY, clickable ? linkColour : textColour );
        if ( m_drawnFocus && key == m_focused )
            StrokeRect( place, linkColour, 1 );
    }
    Commit();
}

/*-------------------------------------------------------------------------*/

CRect
CBreadcrumbBar::GetItemRect( UInt32 index ) const
{GUCEF_TRACE;

    return index < (UInt32) m_itemRects.size() ? m_itemRects[ index ] : CRect();
}

/*-------------------------------------------------------------------------*/

bool
CBreadcrumbBar::IsItemCollapsed( UInt32 index ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_collapsed.size(); ++i )
        if ( m_collapsed[ i ] == (Int32) index )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

CString
CBreadcrumbBar::GetShownText( UInt32 index ) const
{GUCEF_TRACE;

    return index < (UInt32) m_shownTexts.size() ? m_shownTexts[ index ] : CString::Empty;
}

/*-------------------------------------------------------------------------*/

CRect
CBreadcrumbBar::GetOverflowRect( void ) const
{GUCEF_TRACE;

    return m_overflowRect;
}

/*-------------------------------------------------------------------------*/

Int32
CBreadcrumbBar::GetItemAt( Int32 localX, Int32 localY ) const
{GUCEF_TRACE;

    if ( !m_overflowRect.IsEmpty() && m_overflowRect.Contains( localX, localY ) )
        return OverflowItem;
    for ( UInt32 i=0; i<(UInt32)m_itemRects.size(); ++i )
        if ( !m_itemRects[ i ].IsEmpty() && m_itemRects[ i ].Contains( localX, localY ) )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CBreadcrumbBar::GetHoveredItem( void ) const
{GUCEF_TRACE;

    return m_hovered;
}

/*-------------------------------------------------------------------------*/

Int32
CBreadcrumbBar::GetFocusedItem( void ) const
{GUCEF_TRACE;

    return m_focused;
}

/*-------------------------------------------------------------------------*/

CBreadcrumbBar::TInt32Vector
CBreadcrumbBar::GetFocusOrder( void ) const
{GUCEF_TRACE;

    TInt32Vector order;
    for ( UInt32 i=0; i<(UInt32)m_itemRects.size(); ++i )
    {
        if ( 1 == i && !m_overflowRect.IsEmpty() )
            order.push_back( OverflowItem );
        if ( !m_itemRects[ i ].IsEmpty() && i + 1 < (UInt32) m_itemRects.size() )
            order.push_back( (Int32) i );
    }
    return order;
}

/*-------------------------------------------------------------------------*/

bool
CBreadcrumbBar::OpenOverflowMenu( void )
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    if ( GUCEF_NULL == context || m_collapsed.empty() )
        return false;
    if ( GUCEF_NULL == m_menu )
    {
        m_menu = CWidget::As< CPopupMenu >( context->CreateWidget( CPopupMenu::GetRegisteredTypeName(), GetName() + "#Menu", GUCEF_NULL ) );
        if ( GUCEF_NULL == m_menu )
            return false;
        m_menu->SetProperty( EWidgetProperty_Font, GetPropertyRef( EWidgetProperty_Font ) );
        TEventCallback callback( this, &CBreadcrumbBar::OnOverflowMenuItemActivated );
        SubscribeTo( m_menu, CWidget::ItemActivatedEvent, callback );
    }
    CPopupMenu::TStringVector texts;
    for ( UInt32 i=0; i<(UInt32)m_collapsed.size(); ++i )
        texts.push_back( m_items[ (UInt32) m_collapsed[ i ] ].text );
    m_menu->SetItems( texts );
    return m_menu->ShowAt( GetRect().left + m_overflowRect.left, GetRect().top + m_overflowRect.Bottom() );
}

/*-------------------------------------------------------------------------*/

bool
CBreadcrumbBar::IsOverflowMenuOpen( void ) const
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    return GUCEF_NULL != context && GUCEF_NULL != m_menu && context->GetPopup() == m_menu;
}

/*-------------------------------------------------------------------------*/

void
CBreadcrumbBar::OnOverflowMenuItemActivated( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventId  ,
                                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    CItemEventData* item = static_cast< CItemEventData* >( eventData );
    if ( EVENTPHASE_TUNNEL == item->phase )
        return;
    CGUIContext* context = GetContext();
    if ( GUCEF_NULL != context && context->GetPopup() == m_menu )
        context->ClosePopup();
    if ( item->index >= 0 && item->index < (Int32) m_collapsed.size() )
        ClickItem( (UInt32) m_collapsed[ (UInt32) item->index ] );
}

/*-------------------------------------------------------------------------*/

bool
CBreadcrumbBar::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( MouseMoveEvent == eventId )
    {
        const CPointerEventData* pointer = static_cast< const CPointerEventData* >( &eventData );
        const Int32 hovered = GetItemAt( pointer->localX, pointer->localY );
        if ( hovered != m_hovered )
        {
            m_hovered = hovered;
            SetIntProperty( EWidgetProperty_HoverIndex, hovered );
            Draw();
        }
        return false;
    }
    if ( MouseFocusLostEvent == eventId )
    {
        if ( -1 != m_hovered )
        {
            m_hovered = -1;
            SetIntProperty( EWidgetProperty_HoverIndex, -1 );
            Draw();
        }
        return false;
    }
    if ( MouseClickEvent == eventId )
    {
        const CPointerEventData* pointer = static_cast< const CPointerEventData* >( &eventData );
        if ( MOUSEBUTTON_LEFT != pointer->button )
            return false;
        const Int32 item = GetItemAt( pointer->localX, pointer->localY );
        if ( OverflowItem == item )
            return OpenOverflowMenu();
        if ( item >= 0 )
            return ClickItem( (UInt32) item );
        return false;
    }
    if ( KeyDownEvent == eventId )
    {
        const CKeyEventData* key = static_cast< const CKeyEventData* >( &eventData );
        const TInt32Vector order = GetFocusOrder();
        if ( order.empty() )
            return false;
        Int32 place = -1;
        for ( UInt32 i=0; i<(UInt32)order.size(); ++i )
            if ( order[ i ] == m_focused )
                place = (Int32) i;
        if ( INPUT::KEYCODE_LEFT == key->keyCode || INPUT::KEYCODE_RIGHT == key->keyCode )
        {
            if ( place < 0 )
                place = INPUT::KEYCODE_LEFT == key->keyCode ? (Int32) order.size() - 1 : 0;
            else
            if ( INPUT::KEYCODE_LEFT == key->keyCode && place > 0 )
                --place;
            else
            if ( INPUT::KEYCODE_RIGHT == key->keyCode && place + 1 < (Int32) order.size() )
                ++place;
            m_focused = order[ (UInt32) place ];
            Draw();
            return true;
        }
        if ( ( INPUT::KEYCODE_RETURN == key->keyCode || INPUT::KEYCODE_NUMPADENTER == key->keyCode ) && place >= 0 )
        {
            if ( OverflowItem == m_focused )
                return OpenOverflowMenu();
            return ClickItem( (UInt32) m_focused );
        }
        return false;
    }
    return CDrawSurface::OnDefaultAction( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CBreadcrumbBar::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    CDrawSurface::OnPropertyChanged( property, oldValue, newValue );
    switch ( property )
    {
        case EWidgetProperty_Font:
        case EWidgetProperty_TextColour:
        case EWidgetProperty_SelectionColour:
        case EWidgetProperty_HoverColour:
        case EWidgetProperty_BackgroundColour:
        {
            m_dirty = true;
            break;
        }
        default:
            break;
    }
}

/*-------------------------------------------------------------------------*/

bool
CBreadcrumbBar::NeedsUpdate( void ) const
{GUCEF_TRACE;

    return IsEffectivelyVisible();
}

/*-------------------------------------------------------------------------*/

void
CBreadcrumbBar::OnUpdate( UInt32 deltaInMs )
{GUCEF_TRACE;

    CDrawSurface::OnUpdate( deltaInMs );
    const CRect& rect = GetRect();
    if ( rect.IsEmpty() )
        return;
    if ( m_dirty || rect.width != m_builtWidth || rect.height != m_builtHeight )
        Layout( rect.width, rect.height );
    else
    if ( HasFocus() != m_drawnFocus )
        Draw();
}

/*-------------------------------------------------------------------------*/

void
CBreadcrumbBar::AdjustPlacement( CRect& rect, const CRect& area, bool sizeFromArea )
{GUCEF_TRACE;

    CDrawSurface::AdjustPlacement( rect, area, sizeFromArea );
    // a new size is laid out in the same pass, before the frame shows the old drawing
    if ( !rect.IsEmpty() && ( rect.width != m_builtWidth || rect.height != m_builtHeight ) )
        Layout( rect.width, rect.height );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
