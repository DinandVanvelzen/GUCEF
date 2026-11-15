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

#ifndef GUCEF_GUI_CBREADCRUMBBAR_H
#define GUCEF_GUI_CBREADCRUMBBAR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CDRAWSURFACE_H
#include "gucefGUI_CDrawSurface.h"
#define GUCEF_GUI_CDRAWSURFACE_H
#endif /* GUCEF_GUI_CDRAWSURFACE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CPopupMenu;

/**
 *  A path of places (a breadcrumb trail: "Holdings > Positions > ACME"), each item with an id of the application.
 *  The last item is where the user is and is shown in the text colour; the items before it are shown in the link
 *  colour (SelectionColour) and a click on one raises ItemClickedEvent with its index and id. While the bar has the
 *  focus Left and Right move between the items and Enter clicks the one focused.
 *
 *  When the items do not fit, the items after the first collapse, from the front, into a "..." item that opens a menu
 *  of the collapsed items; the first and the last item stay. When even that does not fit, the last item is cut with "...".
 *
 *  The bar is a draw surface: it draws itself with commands, so every driver that shows a draw surface shows it, and it
 *  lays itself out again at every size. Widget type "BreadcrumbBar". Properties: Font, TextColour, SelectionColour,
 *  HoverColour, BackgroundColour.
 */
class GUCEF_GUI_PUBLIC_CPP CBreadcrumbBar : public CDrawSurface
{
    public:

    static const CORE::CEvent ItemClickedEvent;     /**< CItemEventData (not routed): the index of the item and its id as the item */

    static void RegisterEvents( void );

    struct GUCEF_GUI_PUBLIC_CPP SItem
    {
        CString id;
        CString text;

        SItem( void );
        SItem( const CString& itemId, const CString& itemText );
    };
    typedef struct SItem TItem;
    typedef GUCEF::vector< TItem > TItemVector;

    /** The room around the text of an item, and the room of the separator between two items */
    enum ELayout
    {
        ITEM_PADDING = 6        ,
        SEPARATOR_ROOM = 16
    };

    /** What GetItemAt() says for the "..." item of the collapsed items */
    static const Int32 OverflowItem;

    CBreadcrumbBar( void );

    virtual ~CBreadcrumbBar();

    void SetItems( const TItemVector& items );

    void AddItem( const CString& id, const CString& text );

    void ClearItems( void );

    UInt32 GetItemCount( void ) const;

    /** The item at an index; an empty item for an index that is not there */
    const TItem& GetItem( UInt32 index ) const;

    const TItemVector& GetItems( void ) const;

    /** Does what a click on the item does: ItemClickedEvent. False for the last item (where the user is) and for an index that is not there */
    bool ClickItem( UInt32 index );

    /** Lays the items out for a size and draws them; the bar does this itself when its size or items change */
    void Layout( Int32 width, Int32 height );

    /** Where an item is drawn, relative to the bar, after the last layout; empty for a collapsed item */
    CRect GetItemRect( UInt32 index ) const;

    /** Whether the item is in the menu of the "..." item */
    bool IsItemCollapsed( UInt32 index ) const;

    /** The text drawn for an item: its text, or for the last item when it does not fit, its start with "..." */
    CString GetShownText( UInt32 index ) const;

    /** The "..." item, empty when no item is collapsed */
    CRect GetOverflowRect( void ) const;

    /** The item at a point relative to the bar: its index, OverflowItem for the "..." item, -1 for none */
    Int32 GetItemAt( Int32 localX, Int32 localY ) const;

    /** Opens the menu of the collapsed items below the "..." item; a choice clicks the item. False when nothing is collapsed or without a context */
    bool OpenOverflowMenu( void );

    bool IsOverflowMenuOpen( void ) const;

    /** The item under the pointer, OverflowItem, or -1 */
    Int32 GetHoveredItem( void ) const;

    /** The item the keys are on while the bar has the focus, OverflowItem, or -1 */
    Int32 GetFocusedItem( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "BreadcrumbBar"; }

    static CWidget* Create( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool NeedsUpdate( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnUpdate( UInt32 deltaInMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void AdjustPlacement( CRect& rect, const CRect& area, bool sizeFromArea ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CBreadcrumbBar > TEventCallback;
    typedef GUCEF::vector< CRect > TRectVector;
    typedef GUCEF::vector< Int32 > TInt32Vector;

    Int32 TextWidth( const CString& text ) const;

    Int32 LineHeight( void ) const;

    /** The items and the "..." item in the order they are shown, for the keys */
    TInt32Vector GetFocusOrder( void ) const;

    void Draw( void );

    void OnOverflowMenuItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CBreadcrumbBar( const CBreadcrumbBar& src );
    CBreadcrumbBar& operator=( const CBreadcrumbBar& src );

    private:

    TItemVector m_items;
    TRectVector m_itemRects;
    TStringVector m_shownTexts;
    TInt32Vector m_collapsed;
    CRect m_overflowRect;
    Int32 m_hovered;
    Int32 m_focused;
    bool m_dirty;
    bool m_drawnFocus;
    Int32 m_builtWidth;
    Int32 m_builtHeight;
    CPopupMenu* m_menu;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CBREADCRUMBBAR_H ? */
