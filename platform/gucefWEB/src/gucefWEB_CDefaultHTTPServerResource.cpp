/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_WEB_CHTTPMIMETYPES_H
#include "gucefWEB_CHttpMimeTypes.h"
#define GUCEF_WEB_CHTTPMIMETYPES_H
#endif /* GUCEF_WEB_CHTTPMIMETYPES_H ? */

#include "gucefWEB_CDefaultHTTPServerResource.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDefaultHTTPServerResource::CDefaultHTTPServerResource( void )
    : CIHTTPServerResource()
    , m_cacheabilityProperty()
    , m_lastModifiedProperty()
    , m_resourceVersionProperty()
    , m_encodingProperty()
    , m_urlSegment()
    , m_deserializationReps()
    , m_serializationReps()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CDefaultHTTPServerResource::CDefaultHTTPServerResource( const CDefaultHTTPServerResource& src )
    : CIHTTPServerResource( src )
    , m_cacheabilityProperty( src.m_cacheabilityProperty )
    , m_lastModifiedProperty( src.m_lastModifiedProperty )
    , m_resourceVersionProperty( src.m_resourceVersionProperty )
    , m_encodingProperty( src.m_encodingProperty )
    , m_urlSegment( src.m_urlSegment )
    , m_deserializationReps( src.m_deserializationReps )
    , m_serializationReps( src.m_serializationReps )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CDefaultHTTPServerResource::~CDefaultHTTPServerResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CDefaultHTTPServerResource&
CDefaultHTTPServerResource::operator=( const CDefaultHTTPServerResource& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_cacheabilityProperty = src.m_cacheabilityProperty;
        m_lastModifiedProperty = src.m_lastModifiedProperty;
        m_resourceVersionProperty = src.m_resourceVersionProperty;
        m_encodingProperty = src.m_encodingProperty;
        m_urlSegment = src.m_urlSegment;
        m_deserializationReps = src.m_deserializationReps;
        m_serializationReps = src.m_serializationReps;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void 
CDefaultHTTPServerResource::SetURL( const CString& url )
{GUCEF_TRACE;

    m_urlSegment = url;
}

/*-------------------------------------------------------------------------*/

const CString& 
CDefaultHTTPServerResource::GetURL( void ) const
{GUCEF_TRACE;

    return m_urlSegment;
}

/*-------------------------------------------------------------------------*/

const CString& 
CDefaultHTTPServerResource::GetEncoding( const CORE::CString& resourcePath ) const
{GUCEF_TRACE;

    return m_encodingProperty;
}

/*-------------------------------------------------------------------------*/
                    
const CDefaultHTTPServerResource::TStringVector& 
CDefaultHTTPServerResource::GetSupportedSerializationRepresentations( const CORE::CString& resourcePath )
{GUCEF_TRACE;

    return m_serializationReps;
}

/*-------------------------------------------------------------------------*/

CString 
CDefaultHTTPServerResource::GetBestMatchedSerializationRepresentation( const CORE::CString& resourcePath                                ,
                                                                       const CDefaultHTTPServerResource::TStringVector& representations )
{GUCEF_TRACE;

    if ( m_serializationReps.empty() )
        return CString::Empty; 
    
    // Does the client accept the "any" representation?
    TStringVector::const_iterator n = representations.begin();
    while ( n != representations.end() )
    { 
        if ( CHttpMimeTypes::MimeTypeAny == (*n) )
        {
            // Since "any" is supported we will prefer the highest priority server side 
            // representation as the negotiated representation
            return *m_serializationReps.begin();
        }
        ++n;
    }
    
    // The default assumption is that representations have been stored in order of most desireable
    // As such we look for a match between our list in order against the options given
    TStringVector::iterator i = m_serializationReps.begin();
    while ( i != m_serializationReps.end() )
    {
        // Does the client support this representation?
        TStringVector::const_iterator n = representations.begin();
        while ( n != representations.end() )
        { 
            if ( (*i) == (*n) )
            {
                // We found the most desireable representation based on the server-side
                // priority ordered list
                return (*i);
            }
            ++n;
        }
        ++i;
    }

    // No supported agreeable representation found that both sides can agree on 
    // and support
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

const CDefaultHTTPServerResource::TStringVector& 
CDefaultHTTPServerResource::GetSupportedDeserializationRepresentations( const CORE::CString& resourcePath )
{GUCEF_TRACE;

    return m_deserializationReps;
}

/*-------------------------------------------------------------------------*/

CString 
CDefaultHTTPServerResource::GetBestSupportedDeserializationRepresentation( const CORE::CString& resourcePath    ,
                                                                           const TStringVector& representations )
{GUCEF_TRACE;

    // The default assumption is that representations have been stored in order of most desireable
    // As such we look for a match between our list in order against the options given
    TStringVector::iterator i = m_deserializationReps.begin();
    while ( i != m_deserializationReps.end() )
    {
        // Does the client support this representation?
        TStringVector::const_iterator n = representations.begin();
        while ( n != representations.end() )
        { 
            if ( (*i) == (*n) )
            {
                // We found the most desireable representation based on the server-side
                // priority ordered list
                return (*i);
            }
            ++n;
        }
        ++i;
    }

    // No supported agreeable representation found that both sides can agree on 
    // and support
    return CString();
}

/*-------------------------------------------------------------------------*/

CDefaultHTTPServerResource::TCreateState 
CDefaultHTTPServerResource::CreateResource( const CORE::CString& resourcePath             ,
                                            const CString& transactionID                  ,
                                            const CORE::CDynamicBuffer& inputBuffer       ,
                                            const CString& representation                 ,
                                            const CString& params                         ,
                                            THTTPServerResourcePtr& resourceOutput        ,
                                            TStringVector& supportedRepresentationsOutput )
{GUCEF_TRACE;

    // By default we don't allow or implement more complex interaction patterns
    // such as dynamic resources or collections
    return CIHTTPServerResource::CREATESTATE_FAILED;
}

/*-------------------------------------------------------------------------*/
    
bool 
CDefaultHTTPServerResource::DeleteResource( const CORE::CString& resourcePath )
{GUCEF_TRACE;

    // By default we don't allow or implement more complex interaction patterns
    // such as dynamic resources or collections
    return false;
}

/*-------------------------------------------------------------------------*/

const CString& 
CDefaultHTTPServerResource::GetResourceVersion( const CORE::CString& resourcePath )
{GUCEF_TRACE;

    return m_resourceVersionProperty;
}

/*-------------------------------------------------------------------------*/
    
const CString& 
CDefaultHTTPServerResource::GetLastModifiedTime( const CORE::CString& resourcePath )
{GUCEF_TRACE;

    return m_lastModifiedProperty;
}

/*-------------------------------------------------------------------------*/

const CString& 
CDefaultHTTPServerResource::GetCacheability( const CORE::CString& resourcePath )
{GUCEF_TRACE;

    return m_cacheabilityProperty;
}

/*-------------------------------------------------------------------------*/

bool 
CDefaultHTTPServerResource::Serialize( const CORE::CString& resourcePath  ,
                                       CORE::CDynamicBuffer& outputBuffer ,
                                       const CString& representation      ,
                                       const CString& params              )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

CDefaultHTTPServerResource::TDeserializeState 
CDefaultHTTPServerResource::Deserialize( const CORE::CString& resourcePath       ,
                                         const CORE::CDynamicBuffer& inputBuffer ,
                                         const CString& representation           ,
                                         bool isDeltaUpdateOnly                  )
{GUCEF_TRACE;

    return CIHTTPServerResource::DESERIALIZESTATE_UNABLETOUPDATE;
}

/*-------------------------------------------------------------------------*/

bool 
CDefaultHTTPServerResource::IsCollection( const CORE::CString& resourcePath ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CDefaultHTTPServerResource::IsCreateSupported( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CDefaultHTTPServerResource::IsSerializeSupported( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CDefaultHTTPServerResource::IsDeserializeSupported( bool deltaOnly ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
