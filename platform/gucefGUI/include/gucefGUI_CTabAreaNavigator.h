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

#ifndef GUCEF_GUI_CTABAREANAVIGATOR_H
#define GUCEF_GUI_CTABAREANAVIGATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

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

class CTabControl;
class CKeyboardShortcutMap;

/**
 *  Two levels of tabs as one navigation: an area tab control whose pages hold, per area, a tab control of sub-views.
 *  Every page the user can be on has an id of the application (a stable name), registered with its area and sub-view;
 *  the navigator selects a page by its id on both levels at once, returns to the sub-view last shown when an area is
 *  chosen again, cycles the sub-views of an area, and raises PageChangedEvent whenever the page shown changes, by a click
 *  on a tab or by a member. With BindDefaultShortcuts() Ctrl+1 to Ctrl+9 choose the first nine areas and Ctrl+Tab,
 *  Ctrl+Shift+Tab the sub-views.
 *
 *  The tab controls are not owned: they must outlive the navigator, or be set to NULL before they go.
 */
class GUCEF_GUI_PUBLIC_CPP CTabAreaNavigator : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent PageChangedEvent;         /**< CORE::TCloneableString: the id of the page shown now */

    static const char* ActionAreaPrefix;                /**< "area.": the actions "area.1" .. "area.9" choose an area */
    static const char* ActionNextSubView;               /**< "subview.next" */
    static const char* ActionPreviousSubView;           /**< "subview.previous" */

    static void RegisterEvents( void );

    CTabAreaNavigator( void );

    virtual ~CTabAreaNavigator();

    /** The tab control of the areas; NULL lets go of it */
    bool SetAreaControl( CTabControl* areas );

    CTabControl* GetAreaControl( void ) const;

    /** The tab control of the sub-views of an area (on a page of the area control); NULL for an area without sub-views */
    bool SetSubViewControl( UInt32 area, CTabControl* subViews );

    CTabControl* GetSubViewControl( UInt32 area ) const;

    /** Registers a page: the area and the sub-view (-1 for an area without sub-views) it is on. False for an empty id; an id registered before moves */
    bool AddPage( const CString& pageId, UInt32 area, Int32 subView );

    bool RemovePage( const CString& pageId );

    bool FindPage( const CString& pageId, UInt32& area, Int32& subView ) const;

    /** The id of the page at a place, empty for none */
    CString GetPageAt( UInt32 area, Int32 subView ) const;

    /** Shows the page: its area and its sub-view. False for an id that is not registered or without an area control */
    bool SelectPage( const CString& pageId );

    /** Shows an area at the sub-view last shown in it. False for an area that is not there */
    bool SelectArea( UInt32 area );

    /** The next (or previous) sub-view of the area shown, round. False when the area has no sub-views */
    bool NextSubView( bool backwards );

    /** The id of the page shown, empty when the place shown has no page */
    CString GetCurrentPage( void ) const;

    /** Binds Ctrl+1 .. Ctrl+9 to "area.1" .. "area.9", Ctrl+Tab to "subview.next" and Ctrl+Shift+Tab to "subview.previous" */
    void BindDefaultShortcuts( CKeyboardShortcutMap& map ) const;

    /** Does an action of BindDefaultShortcuts(); false for another action or one that could not be done */
    bool HandleShortcut( const CString& actionId );

    private:

    typedef CORE::CTEventHandlerFunctor< CTabAreaNavigator > TEventCallback;

    struct SPage
    {
        CString id;
        UInt32 area;
        Int32 subView;
    };
    typedef struct SPage TPage;
    typedef GUCEF::vector< TPage > TPageVector;
    typedef GUCEF::map< UInt32, CTabControl* > TControlMap;
    typedef GUCEF::map< UInt32, Int32 > TSubViewMap;

    void OnActiveTabChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    /** Finds the page shown and raises PageChangedEvent when it is another one */
    void UpdateCurrentPage( void );

    void Subscribe( CTabControl* control );

    void Unsubscribe( CTabControl* control );

    CTabAreaNavigator( const CTabAreaNavigator& src );
    CTabAreaNavigator& operator=( const CTabAreaNavigator& src );

    private:

    CTabControl* m_areas;
    TControlMap m_subViews;
    TSubViewMap m_lastSubView;
    TPageVector m_pages;
    CString m_currentPage;
    bool m_selecting;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTABAREANAVIGATOR_H ? */
