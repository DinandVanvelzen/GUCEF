/*
 *  gucefPUBSUB: GUCEF module providing pub-sub communication facilities
 *
 *  Copyright (C) 1998 - 2022.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefPUBSUB_CPubSubBookmark.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPubSubBookmark::CPubSubBookmark( void )
    : m_bmType( BOOKMARK_TYPE_NOT_INITIALIZED )
    , m_bmData()
    , m_bmDt()
    , m_bmKeyField()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubBookmark::CPubSubBookmark( TBookmarkType bmType )
    : m_bmType( bmType )
    , m_bmData()
    , m_bmDt()
    , m_bmKeyField()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubBookmark::CPubSubBookmark( TBookmarkType bmType         ,
                                  const CORE::CVariant& bmData )
    : m_bmType( bmType )
    , m_bmData( bmData )
    , m_bmDt()
    , m_bmKeyField()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubBookmark::CPubSubBookmark( TBookmarkType bmType         ,
                                  const CORE::CVariant& bmData ,
                                  const CORE::CDateTime& bmDt  )
    : m_bmType( bmType )
    , m_bmData( bmData )
    , m_bmDt( bmDt )
    , m_bmKeyField()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubBookmark::CPubSubBookmark( const CPubSubBookmark& src  ,
                                  bool linkIfPossible         ,
                                  const CORE::CDateTime& bmDt )
    : m_bmType( src.m_bmType )
    , m_bmData( src.m_bmData, linkIfPossible )
    , m_bmDt( bmDt )
    , m_bmKeyField( src.m_bmKeyField )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubBookmark::~CPubSubBookmark()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubBookmark&
CPubSubBookmark::operator=( const CPubSubBookmark& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_bmType     = src.m_bmType;
        m_bmData     = src.m_bmData;
        m_bmDt       = src.m_bmDt;
        m_bmKeyField = src.m_bmKeyField;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CPubSubBookmark::SetBookmarkType( TBookmarkType bmType )
{GUCEF_TRACE;

    m_bmType = bmType;
}

/*-------------------------------------------------------------------------*/

CPubSubBookmark::TBookmarkType
CPubSubBookmark::GetBookmarkType( void ) const
{GUCEF_TRACE;

    return m_bmType;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPubSubBookmark::GetBookmarkTypeName( void ) const
{GUCEF_TRACE;

    switch ( m_bmType )        
    {        
        case BOOKMARK_TYPE_NOT_AVAILABLE:       return "BOOKMARK_TYPE_NOT_AVAILABLE";
        case BOOKMARK_TYPE_NOT_APPLICABLE:      return "BOOKMARK_TYPE_NOT_APPLICABLE";
        
        case BOOKMARK_TYPE_MSG_ID:              return "BOOKMARK_TYPE_MSG_ID";
        case BOOKMARK_TYPE_MSG_INDEX:           return "BOOKMARK_TYPE_MSG_INDEX";
        case BOOKMARK_TYPE_MSG_DATETIME:        return "BOOKMARK_TYPE_MSG_DATETIME";
        case BOOKMARK_TYPE_TOPIC_INDEX:         return "BOOKMARK_TYPE_TOPIC_INDEX";
        case BOOKMARK_TYPE_INDEX_KEY_VALUE:     return "BOOKMARK_TYPE_INDEX_KEY_VALUE";

        default:
        case BOOKMARK_TYPE_NOT_INITIALIZED:     return "BOOKMARK_TYPE_NOT_INITIALIZED";
    }
 
}

/*-------------------------------------------------------------------------*/

void
CPubSubBookmark::SetBookmarkData( const CORE::CVariant& bmData )
{GUCEF_TRACE;

    m_bmData = bmData;
}
/*-------------------------------------------------------------------------*/

const CORE::CVariant&
CPubSubBookmark::GetBookmarkData( void ) const
{GUCEF_TRACE;

    return m_bmData;
}

/*-------------------------------------------------------------------------*/

CORE::CVariant&
CPubSubBookmark::GetBookmarkData( void )
{GUCEF_TRACE;

    return m_bmData;
}

/*-------------------------------------------------------------------------*/

const CORE::CDateTime&
CPubSubBookmark::GetBookmarkDateTime( void ) const
{GUCEF_TRACE;
    
    return m_bmDt;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime&
CPubSubBookmark::GetBookmarkDateTime( void )
{GUCEF_TRACE;

    return m_bmDt;
}

/*-------------------------------------------------------------------------*/

CORE::CString 
CPubSubBookmark::ToString( void ) const
{GUCEF_TRACE;

    // Since a bookmark is a simple combo of a few basic fields we can just use a CSV approach
    // Keep in mind this also relies on Base64 encoding of BLOBs and such by the variant

    if ( m_bmType == BOOKMARK_TYPE_INDEX_KEY_VALUE )
        return m_bmDt.ToIso8601DateTimeString( false, true ) + ',' + CORE::ToString( m_bmType ) + ',' + m_bmKeyField + '=' + m_bmData.AsString();
    return m_bmDt.ToIso8601DateTimeString( false, true ) + ',' + CORE::ToString( m_bmType ) + ',' + m_bmData.AsString();
}

/*-------------------------------------------------------------------------*/

CPubSubBookmark& 
CPubSubBookmark::LinkTo( const CPubSubBookmark& srcBookmark )
{GUCEF_TRACE;

    if ( this != &srcBookmark )
    {
        m_bmType     = srcBookmark.m_bmType;
        m_bmData.LinkTo( srcBookmark.m_bmData );
        m_bmDt       = srcBookmark.m_bmDt;
        m_bmKeyField = srcBookmark.m_bmKeyField;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CPubSubBookmark::SetBookmarkKeyField( const CORE::CString& keyField )
{GUCEF_TRACE;

    m_bmKeyField = keyField;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPubSubBookmark::GetBookmarkKeyField( void ) const
{GUCEF_TRACE;

    return m_bmKeyField;
}

/*-------------------------------------------------------------------------*/

CPubSubBookmark
CPubSubBookmark::MakeIndexKeyValueBookmark( const CORE::CString& keyField ,
                                            const CORE::CVariant& value   )
{GUCEF_TRACE;

    CPubSubBookmark bm( BOOKMARK_TYPE_INDEX_KEY_VALUE, value );
    bm.m_bmKeyField = keyField;
    return bm;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
