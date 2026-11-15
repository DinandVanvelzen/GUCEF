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

#ifndef GUCEF_GUI_CRECENTLOCATIONS_H
#define GUCEF_GUI_CRECENTLOCATIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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
 *  The places an application opened lately, the newest first: a list of paths of the file system of the computer that keeps
 *  at most a number of them (12 unless told otherwise) and has each path once (a path that is opened again moves to the front).
 *  The list is a notifier: ChangedEvent says that its content changed, for the forms that show it (CFileSystemDialogForm places,
 *  COpenActionStripForm) and for the application that keeps it in its settings.
 *
 *  Paths are kept with '/' as separator and without a '/' at the end, so the same place in other spellings is one entry.
 *  Prune() takes out the places that are no longer there; Load() does that as well, so a settings file that outlived a folder
 *  does not offer it. Save() and Load() use a node of the settings: an array of paths in a child node of a section.
 */
class GUCEF_GUI_PUBLIC_CPP CRecentLocations : public CORE::CNotifier
{
    public:

    static const CORE::CEvent ChangedEvent;

    static void RegisterEvents( void );

    enum EDefaults
    {
        DEFAULT_MAX_COUNT = 12
    };

    explicit CRecentLocations( UInt32 maxCount = DEFAULT_MAX_COUNT );

    virtual ~CRecentLocations();

    /** At most this many places are kept (at least 1); the oldest go when the limit is lowered */
    void SetMaxCount( UInt32 maxCount );

    UInt32 GetMaxCount( void ) const;

    /** Puts the place at the front; an empty path is refused. Notifies ChangedEvent when the list changed */
    bool Add( const CString& path );

    /** Takes the place out of the list; false when it was not there */
    bool Remove( const CString& path );

    void Clear( void );

    /** Takes out the places that are no longer there (a folder or a file of the file system of the computer); the number taken out */
    UInt32 Prune( void );

    UInt32 GetCount( void ) const;

    /** The path at the index, 0 is the newest; empty for an index outside the list */
    const CString& Get( UInt32 index ) const;

    const CString::StringVector& GetAll( void ) const;

    bool Contains( const CString& path ) const;

    /** The spelling the list keeps: '/' separators, no '/' at the end (the path "/" stays) */
    static CString Normalize( const CString& path );

    /** The name of the place for a list: the last part of the path, the whole path for a root */
    static CString GetShortName( const CString& path );

    /** Writes the places as the array 'name' of the section, replacing what it had */
    bool Save( CORE::CDataNode& section, const CString& name = "RecentLocations" ) const;

    /** Reads the places from the array 'name' of the section, then prunes them; what the list held before is replaced. False when the section has none */
    bool Load( const CORE::CDataNode& section, const CString& name = "RecentLocations" );

    private:

    CRecentLocations( const CRecentLocations& src );
    CRecentLocations& operator=( const CRecentLocations& src );

    void CutToMaxCount( void );

    private:

    CString::StringVector m_locations;
    UInt32 m_maxCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CRECENTLOCATIONS_H ? */
