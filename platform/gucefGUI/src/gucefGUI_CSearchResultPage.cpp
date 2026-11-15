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

#ifndef GUCEF_GUI_CSEARCHRESULTPAGE_H
#include "gucefGUI_CSearchResultPage.h"
#define GUCEF_GUI_CSEARCHRESULTPAGE_H
#endif /* GUCEF_GUI_CSEARCHRESULTPAGE_H ? */

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


CSearchResultPage::SRow::SRow( void )
    : key()
    , title()
    , subtitle()
    , thumbnailKey()
    , typeName()
    , tooltip()
    , values()
    , available( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSearchResultPage::CSearchResultPage( void )
    : rows()
    , total( 0 )
    , pageIndex( 0 )
    , pageSize( 0 )
    , facets()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CSearchResultPage::Clear( void )
{GUCEF_TRACE;

    rows.clear();
    total = 0;
    pageIndex = 0;
    pageSize = 0;
    facets.clear();
}

/*-------------------------------------------------------------------------*/

bool
CSearchResultPage::GetFacetCount( const CString& groupId, const CString& value, UInt64& count ) const
{GUCEF_TRACE;

    TFacetMap::const_iterator g = facets.find( groupId );
    if ( g == facets.end() )
        return false;
    TValueCountMap::const_iterator v = (*g).second.find( value );
    if ( v == (*g).second.end() )
        return false;
    count = (*v).second;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CSearchResultPage::GetPageCount( void ) const
{GUCEF_TRACE;

    if ( 0 == total )
        return 0;
    if ( 0 == pageSize )
        return 1;
    return (UInt32) ( ( total + pageSize - 1 ) / pageSize );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
