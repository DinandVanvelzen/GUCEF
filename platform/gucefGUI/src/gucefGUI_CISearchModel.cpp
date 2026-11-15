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

#ifndef GUCEF_GUI_CISEARCHMODEL_H
#include "gucefGUI_CISearchModel.h"
#define GUCEF_GUI_CISEARCHMODEL_H
#endif /* GUCEF_GUI_CISEARCHMODEL_H ? */

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

const CORE::CEvent CISearchModel::SearchCompletedEvent = "GUCEF::GUI::CISearchModel::SearchCompletedEvent";
const CORE::CEvent CISearchModel::FilterDescriptionChangedEvent = "GUCEF::GUI::CISearchModel::FilterDescriptionChangedEvent";
const CORE::CEvent CISearchModel::SavedSearchesChangedEvent = "GUCEF::GUI::CISearchModel::SavedSearchesChangedEvent";
const CORE::CEvent CISearchModel::SuggestionsChangedEvent = "GUCEF::GUI::CISearchModel::SuggestionsChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


void
CISearchModel::RegisterEvents( void )
{GUCEF_TRACE;

    SearchCompletedEvent.Initialize();
    FilterDescriptionChangedEvent.Initialize();
    SavedSearchesChangedEvent.Initialize();
    SuggestionsChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CISearchModel::SSearchCompletion::SSearchCompletion( void )
    : requestId( 0 )
    , succeeded( false )
    , message()
    , page()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CISearchModel::SColumn::SColumn( void )
    : key()
    , title()
    , width( 1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CISearchModel::SColumn::SColumn( const CString& columnKey, const CString& columnTitle, Int32 columnWidth )
    : key( columnKey )
    , title( columnTitle )
    , width( columnWidth )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CISearchModel::SSortChoice::SSortChoice( void )
    : id()
    , title()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CISearchModel::SSortChoice::SSortChoice( const CString& sortId, const CString& sortTitle )
    : id( sortId )
    , title( sortTitle )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CISearchModel::CISearchModel( void )
    : CORE::CObservingNotifier()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CISearchModel::~CISearchModel()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CISearchModel::GetResultColumns( TColumnVector& columns ) const
{GUCEF_TRACE;

    columns.clear();
    columns.push_back( TColumn( "title", "Name", 60 ) );
    columns.push_back( TColumn( "subtitle", "Details", 40 ) );
}

/*-------------------------------------------------------------------------*/

void
CISearchModel::GetSortChoices( TSortChoiceVector& choices ) const
{GUCEF_TRACE;

    choices.clear();
}

/*-------------------------------------------------------------------------*/

void
CISearchModel::GetSuggestions( const CString& prefix, UInt32 maxCount, CString::StringVector& suggestions ) const
{GUCEF_TRACE;

    suggestions.clear();
}

/*-------------------------------------------------------------------------*/

bool
CISearchModel::SupportsSavedSearches( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CISearchModel::GetSavedSearches( TSavedSearchVector& searches ) const
{GUCEF_TRACE;

    searches.clear();
}

/*-------------------------------------------------------------------------*/

bool
CISearchModel::SaveSearch( const CSavedSearch& search )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CISearchModel::DeleteSavedSearch( const CString& searchId )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

const CString&
CISearchModel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CISearchModel";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
