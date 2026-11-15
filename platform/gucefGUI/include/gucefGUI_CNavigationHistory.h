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

#ifndef GUCEF_GUI_CNAVIGATIONHISTORY_H
#define GUCEF_GUI_CNAVIGATIONHISTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

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

/**
 *  The places a user visited in an application, for Back and Forward as a web browser has them. A place is an opaque
 *  location string of the application (a page with its scope, a path of a drill-down) with a title for menus and
 *  breadcrumbs. A visit after going back drops the places that were ahead; a visit of the place shown adds nothing (its
 *  title is updated); at most a number of places is kept, the oldest go first (0: no maximum).
 *
 *  Events: NavigatedEvent when the history moved by Back(), Forward() or GoToIndex(): the application shows the place
 *  it carries. Visit() and Replace() do not raise it, since the application is there already. HistoryChangedEvent after
 *  every change, for the state of Back and Forward buttons.
 */
class GUCEF_GUI_PUBLIC_CPP CNavigationHistory : public CORE::CNotifier
{
    public:

    static const CORE::CEvent NavigatedEvent;           /**< TNavigatedEventData: the place to show */
    static const CORE::CEvent HistoryChangedEvent;      /**< no data */

    static void RegisterEvents( void );

    /** How the place shown was reached */
    enum ENavigationKind
    {
        NAVIGATION_VISIT = 0    ,
        NAVIGATION_BACK         ,
        NAVIGATION_FORWARD      ,
        NAVIGATION_JUMP             /**< GoToIndex() */
    };

    struct GUCEF_GUI_PUBLIC_CPP SEntry
    {
        CString location;
        CString title;

        SEntry( void );
        SEntry( const CString& entryLocation, const CString& entryTitle );
    };
    typedef struct SEntry TEntry;
    typedef GUCEF::vector< TEntry > TEntryVector;

    struct GUCEF_GUI_PUBLIC_CPP SNavigated
    {
        CString location;
        CString title;
        Int32 index;                /**< the index of the place in the history */
        ENavigationKind kind;

        SNavigated( void );
    };
    typedef struct SNavigated TNavigated;
    typedef CORE::CTCloneableObj< TNavigated > TNavigatedEventData;

    enum EDefaults
    {
        DefaultMaximumEntries = 100
    };

    explicit CNavigationHistory( UInt32 maximumEntries = DefaultMaximumEntries );

    virtual ~CNavigationHistory();

    /** The place is shown now: it becomes the current one and the places ahead are dropped; the same location again only updates its title */
    void Visit( const CString& location, const CString& title = CString::Empty );

    /** The current place becomes another one, without a new step (a change of scope that Back should not undo); false when there is none */
    bool Replace( const CString& location, const CString& title = CString::Empty );

    bool CanGoBack( void ) const;

    bool CanGoForward( void ) const;

    /** One place back; raises NavigatedEvent. False when there is none */
    bool Back( void );

    /** One place forward; raises NavigatedEvent. False when there is none */
    bool Forward( void );

    /** To the place at the index (a choice in a menu of the history); raises NavigatedEvent. False for an index that is not there or is the current one */
    bool GoToIndex( UInt32 index );

    bool HasCurrent( void ) const;

    /** The place shown; an empty entry when nothing was visited */
    const TEntry& GetCurrent( void ) const;

    /** -1 when nothing was visited */
    Int32 GetCurrentIndex( void ) const;

    UInt32 GetCount( void ) const;

    /** The place at an index, the oldest at 0; an empty entry for an index that is not there */
    const TEntry& GetEntry( UInt32 index ) const;

    /** The places behind the current one, the nearest first */
    TEntryVector GetBackEntries( void ) const;

    /** The places ahead of the current one, the nearest first */
    TEntryVector GetForwardEntries( void ) const;

    void Clear( void );

    /** At most this many places (0: no maximum); the oldest go at once when it is lowered */
    void SetMaximumEntries( UInt32 maximumEntries );

    UInt32 GetMaximumEntries( void ) const;

    private:

    /** Moves to the index and raises the events */
    bool MoveTo( Int32 index, ENavigationKind kind );

    void CutToMaximum( void );

    CNavigationHistory( const CNavigationHistory& src );
    CNavigationHistory& operator=( const CNavigationHistory& src );

    private:

    TEntryVector m_entries;
    Int32 m_index;
    UInt32 m_maximumEntries;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CNAVIGATIONHISTORY_H ? */
