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

#ifndef GUCEF_GUI_CGRIDPREFERENCES_H
#define GUCEF_GUI_CGRIDPREFERENCES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

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
 *  What a user chose for the columns of one grid: which columns are shown, in which order, how wide, which column
 *  the rows are sorted by, and where the last CSV export of the grid was saved. Columns are named by their key
 *  (CGridView::GetColumnKey()), never by their index, so a record survives columns being added, removed or moved by
 *  another version of the application: keys the grid does not have are ignored when the record is applied, and
 *  columns the record does not name keep their defaults.
 *
 *  Kept by a CIGridPreferenceStore, written as a CDataNode tree (SaveTo / LoadFrom):
 *
 *      GridPreferences version=1 gridId=.. hasWidths=.. sortKey=.. sortAscending=.. exportRoot=.. exportDirectory=.. defaultsRevision=..
 *          Column key=.. shown=.. width=..
 *
 *  Reading is tolerant: unknown attributes and nodes are ignored, a missing attribute takes its default; a record
 *  without a version (or version 0) is refused; a record of a higher version is read for what this version knows.
 */
class GUCEF_GUI_PUBLIC_CPP CGridPreferences
{
    public:

    /** The version written; records of other versions are read for the members this version knows */
    static const UInt32 CurrentVersion;

    /** The name of the node of a record */
    static const CString RecordNodeName;

    /** A column: its key, whether it is shown, its width in the ColumnWidths of the grid (0: not set by the user) */
    struct GUCEF_GUI_PUBLIC_CPP SColumn
    {
        CString key;
        bool shown;
        Int32 width;

        SColumn( void );
        SColumn( const CString& columnKey, bool isShown, Int32 columnWidth );
        bool operator==( const SColumn& other ) const;
    };
    typedef struct SColumn TColumn;
    typedef GUCEF::vector< TColumn > TColumnVector;

    UInt32 version;
    CString gridId;                 /**< the id the record was saved under, for people reading the file */
    TColumnVector columns;          /**< every column of the grid in display order, hidden columns at their place */
    bool hasWidths;                 /**< the widths are the user's (the grid had ColumnWidths set) */
    bool hasSort;
    CString sortKey;
    bool sortAscending;
    CString exportRoot;             /**< the root of the file save dialog the last export went to */
    CString exportDirectory;        /**< the directory in that root */
    CString defaultsRevision;       /**< the revision of the default columns of the application the record was made against (CGridView::SetDefaultsRevision()); empty when not known */

    CGridPreferences( void );

    /** Back to an empty record of the current version */
    void Clear( void );

    /** No columns, no sort, no export location */
    bool IsEmpty( void ) const;

    /** The index of a column in 'columns', -1 when the record does not name it */
    Int32 FindColumn( const CString& key ) const;

    /** Adds a node named RecordNodeName below 'parent' with the record */
    bool SaveTo( CORE::CDataNode& parent ) const;

    /**
     *  Reads the record from 'node' when it is named RecordNodeName, else from its child of that name. False, with the
     *  record cleared, when there is none or it has no version
     */
    bool LoadFrom( const CORE::CDataNode& node );

    bool operator==( const CGridPreferences& other ) const;

    bool operator!=( const CGridPreferences& other ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDPREFERENCES_H ? */
