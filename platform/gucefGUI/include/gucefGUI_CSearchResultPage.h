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
#ifndef GUCEF_GUI_CSEARCHRESULTPAGE_H
#define GUCEF_GUI_CSEARCHRESULTPAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  A page of the results of a search: its rows, how many results there are over all pages, the page it is, and the facets
 *  (for every filter group and value the number of results that have it, over all pages, not only this one). The result
 *  data of CISearchModel::SearchCompletedEvent.
 */
class GUCEF_GUI_PUBLIC_CPP CSearchResultPage
{
    public:

    /** One result */
    struct GUCEF_GUI_PUBLIC_CPP SRow
    {
        CString key;                    /**< what the application knows the result by: an id or a path; the form hands it back in its events */
        CString title;
        CString subtitle;
        CString thumbnailKey;           /**< what a CIThumbnailProvider is asked a picture of for the thumbnail view; empty for a result without a picture */
        CString typeName;               /**< shown in a tile without a picture */
        CString tooltip;
        GUCEF::map< CString, CString > values;      /**< the text of the cells of the columns of the model by column key */
        bool available;                 /**< false for a result that cannot be opened now: it is drawn dimmed */

        SRow( void );
    };
    typedef struct SRow TRow;
    typedef GUCEF::vector< TRow > TRowVector;
    typedef GUCEF::map< CString, UInt64 > TValueCountMap;
    typedef GUCEF::map< CString, TValueCountMap > TFacetMap;

    TRowVector rows;
    UInt64 total;                       /**< the number of results over all pages */
    UInt32 pageIndex;
    UInt32 pageSize;
    TFacetMap facets;                   /**< the counts by id of the filter group, then by value */

    CSearchResultPage( void );

    void Clear( void );

    /** The count of a value of a group, false when the page has no count for it */
    bool GetFacetCount( const CString& groupId, const CString& value, UInt64& count ) const;

    /** The number of pages the total makes at the page size, 0 for no results */
    UInt32 GetPageCount( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CSEARCHRESULTPAGE_H ? */
