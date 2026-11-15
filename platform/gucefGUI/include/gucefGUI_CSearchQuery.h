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
#ifndef GUCEF_GUI_CSEARCHQUERY_H
#define GUCEF_GUI_CSEARCHQUERY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
 *  What a CAssetSearchForm asks a CISearchModel for: a text, the choices made per filter group, the ranges per filter
 *  group, a sort order and the page wanted. It knows nothing of what is searched: the groups are named by the filter
 *  description of the model (CSearchFilterDescription). A choice group matches any of its chosen values; a range
 *  with only a minimum or only a maximum is open on the other side.
 *
 *  'extra' holds what the model keeps in a query that the form does not edit (the model fills it and takes it back from a
 *  saved search; the form carries it from query to query unchanged).
 */
class GUCEF_GUI_PUBLIC_CPP CSearchQuery
{
    public:

    /** A range of numbers of one filter group */
    struct GUCEF_GUI_PUBLIC_CPP SRange
    {
        bool hasMinimum;
        bool hasMaximum;
        Float64 minimum;
        Float64 maximum;

        SRange( void );

        bool IsSet( void ) const;

        bool operator==( const SRange& other ) const;
    };
    typedef struct SRange TRange;
    typedef GUCEF::map< CString, CString::StringSet > TChoiceMap;
    typedef GUCEF::map< CString, TRange > TRangeMap;

    CString text;
    TChoiceMap choices;             /**< the chosen values by id of a choice group */
    TRangeMap ranges;               /**< the ranges by id of a range group, only groups that have a bound */
    CString sort;                   /**< an id of the sorts of the model, empty for its default */
    UInt32 pageIndex;               /**< from 0 */
    UInt32 pageSize;                /**< 0 for the page size of the model */
    CORE::CDataNode extra;

    CSearchQuery( void );

    CSearchQuery( const CSearchQuery& src );

    CSearchQuery& operator=( const CSearchQuery& src );

    /** Everything back to its default */
    void Clear( void );

    /** Whether no text, choice or range is set (the sort and the page do not count) */
    bool IsEmpty( void ) const;

    /** Adds the value to the chosen values of the group */
    void AddChoice( const CString& groupId, const CString& value );

    /** Takes the value out of the chosen values of the group; the group goes when it has no value left */
    void RemoveChoice( const CString& groupId, const CString& value );

    bool HasChoice( const CString& groupId, const CString& value ) const;

    /** Sets the range of the group; a range without a bound takes the group out */
    void SetRange( const CString& groupId, const TRange& range );

    /** The range of the group, a range without bounds when it has none */
    TRange GetRange( const CString& groupId ) const;

    /** Whether the two ask the same thing: text, choices, ranges, sort and extra (the page is not compared) */
    bool IsSameSearch( const CSearchQuery& other ) const;

    /** Whether the two are equal in everything including the page */
    bool operator==( const CSearchQuery& other ) const;

    bool operator!=( const CSearchQuery& other ) const;

    /** Writes the query as a node: the text, the choices, the ranges, the sort, the page size and the extra node (the page is not kept) */
    void Save( CORE::CDataNode& node ) const;

    /** Reads what Save() wrote; false for a node that is not a query */
    bool Load( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CSEARCHQUERY_H ? */
