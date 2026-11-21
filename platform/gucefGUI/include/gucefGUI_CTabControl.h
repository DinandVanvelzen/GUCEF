/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
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

#ifndef GUCEF_GUI_CTABCONTROL_H
#define GUCEF_GUI_CTABCONTROL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

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

class CTabContentPane;
class CPopupMenu;

/**
 *  Tabs along the top (the Tabs property, one entry per tab) above pages: the children of type
 *  TabContentPane, in order. The page of ActiveTab is shown. Properties: Tabs, ActiveTab, TabHeight,
 *  Font, TextColour, TabColour, ActiveTabColour, BackgroundColour, TextOverflow (a title wider than its
 *  tab: Clip, or Ellipsis with the whole title as the tooltip of the tab). A click on a tab or Left /
 *  Right while focused switches the tab (ValueChangedEvent and ActiveTabChangedEvent).
 *
 *  TabSizing Content gives every tab the width of its title (and badge) instead of a share of the width; when the
 *  tabs do not fit, an overflow button at the right end opens a menu of every tab, and the tabs shown follow the
 *  active one, so a title is never cut. TabBadges puts a badge at the right end of a tab: a dot ("*") or a pill with
 *  a text (a count); its room is kept free of the title. A page can hold another tab control: CTabAreaNavigator
 *  ties two levels (areas and their sub-views) together.
 */
class GUCEF_GUI_PUBLIC_CPP CTabControl : public CWidget
{
    public:

    static const CORE::CEvent ActiveTabChangedEvent;
    typedef CORE::TCloneableUInt32 TActiveTabChangedEventData;

    static void RegisterEvents( void );

    public:

    CTabControl( void );

    virtual ~CTabControl();

    virtual bool SetActiveTab( const UInt32 tabIndex );

    virtual Int32 GetActiveTab( void ) const;

    virtual CTabContentPane* GetTabContentPane( const UInt32 tabIndex );

    UInt32 GetTabCount( void ) const;

    /** Where the header of a tab is, relative to the control; tabs share the width equally (TabSizing Equal) or have the width of their title (Content). Empty for a tab that is not shown (Content, overflow) */
    /** The height of the tab strip: TabHeight, and at least a line of the text of the tabs with a little room */
    Int32 GetTabHeight( void ) const;

    CRect GetTabRect( UInt32 tabIndex ) const;

    /** The tab at a position relative to the control, -1 when none */
    Int32 GetTabAt( Int32 localX, Int32 localY ) const;

    /** The room around the title of a tab, the smallest tab of Content sizing, the room of a badge */
    enum ETabLayout
    {
        TAB_TEXT_PADDING = 4    ,
        MIN_TAB_WIDTH = 48      ,
        BADGE_PADDING = 4       ,   /**< left and right of the text of a pill */
        BADGE_DOT_SIZE = 8      ,
        BADGE_GAP = 4               /**< between the title and the badge */
    };

    /** The pixels the title of a tab has: the width of its header without the padding on both sides and without the room of its badge */
    Int32 GetTabTextRoom( UInt32 tabIndex ) const;

    /** The title of a tab, empty for an index that is not a tab */
    CString GetTabTitle( UInt32 tabIndex ) const;

    /** TabSizing is Content */
    bool IsContentSized( void ) const;

    /** The width a tab has with TabSizing Content: its title, the padding and its badge, at least MIN_TAB_WIDTH */
    Int32 GetTabWidth( UInt32 tabIndex ) const;

    /** Whether the tabs are wider than the control (TabSizing Content): some are not shown and the overflow button is there */
    bool HasOverflow( void ) const;

    /** The overflow button at the right end of the tabs, relative to the control; empty without overflow */
    CRect GetOverflowButtonRect( void ) const;

    /** Opens the menu of every tab below the overflow button (what the button does); a choice activates the tab. False without a context or tabs */
    bool OpenOverflowMenu( void );

    /** Whether the overflow menu is open */
    bool IsOverflowMenuOpen( void ) const;

    /** The first tab shown: 0 unless the tabs overflow and the active tab is further on */
    Int32 GetFirstVisibleTab( void ) const;

    bool IsTabVisible( UInt32 tabIndex ) const;

    /** Sets the badge of a tab (TabBadges): empty for none, "*" for a dot, any other text in a pill. False for an index that is not a tab */
    bool SetTabBadge( UInt32 tabIndex, const CString& badge );

    CString GetTabBadge( UInt32 tabIndex ) const;

    /** Where the badge of a tab is drawn, relative to the control; empty without a badge or for a tab that is not shown */
    CRect GetTabBadgeRect( UInt32 tabIndex ) const;

    /** The pixels a badge takes in its tab, the gap to the title included; 0 without a badge */
    Int32 GetTabBadgeRoom( UInt32 tabIndex ) const;

    /** Whether the title of a tab is wider than its room: with TextOverflow Ellipsis it is drawn cut with "..." and its tooltip is the whole title */
    bool IsTabTitleCut( UInt32 tabIndex ) const;

    /** Over a tab whose title is cut (TextOverflow Ellipsis): the whole title; else the Tooltip property */
    virtual CString GetTooltipAt( Int32 localX, Int32 localY ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "TabControl"; }

    static CWidget* Create( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    /** A page added or taken away: the active one is shown, the others not */
    virtual void OnChildrenChanged( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CTabControl > TEventCallback;
    typedef GUCEF::vector< CRect > TRectVector;

    void ShowActivePane( void );

    /** The headers of the tabs with TabSizing Content (empty for the tabs not shown), and whether they overflow */
    void ComputeContentLayout( TRectVector& rects, bool& overflow ) const;

    /** The width of a text in the font of the control; a guess of 8 pixels a character without a context */
    Int32 MeasureWidth( const CString& text ) const;

    void OnOverflowMenuItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CTabControl( const CTabControl& src );
    CTabControl& operator=( const CTabControl& src );

    private:

    mutable Int32 m_firstTab;           /**< the first tab shown when the tabs overflow; follows the active tab */
    CPopupMenu* m_overflowMenu;         /**< made in the context when first opened; destroyed with the control */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTABCONTROL_H ? */
