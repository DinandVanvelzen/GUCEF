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

#include "gucefGUI_CIHelpSource.h"

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

CIHelpSource::STopicEntry::STopicEntry( void )
    : id()
    , title()
    , group()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIHelpSource::STopicEntry::STopicEntry( const CString& topicId, const CString& topicTitle, const CString& topicGroup )
    : id( topicId )
    , title( topicTitle )
    , group( topicGroup )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIHelpSource::CIHelpSource( void )
    : CORE::CTSharedPtrCreator< CIHelpSource, MT::CMutex >( this )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIHelpSource::~CIHelpSource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIHelpSource::HasTopic( const CString& topicId ) const
{GUCEF_TRACE;

    CHelpDocument document;
    return GetTopic( topicId, document );
}

/*-------------------------------------------------------------------------*/

CString
CIHelpSource::GetTopicTitle( const CString& topicId ) const
{GUCEF_TRACE;

    CHelpDocument document;
    if ( GetTopic( topicId, document ) && !document.title.IsNULLOrEmpty() )
        return document.title;
    return topicId;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
