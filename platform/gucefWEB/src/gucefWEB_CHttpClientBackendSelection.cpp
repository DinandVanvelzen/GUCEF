/*
 *  gucefWEB: GUCEF module providing WEB application functionality
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_WEB_CNATIVEHTTPCLIENTBACKEND_H
#include "gucefWEB_CNativeHttpClientBackend.h"
#define GUCEF_WEB_CNATIVEHTTPCLIENTBACKEND_H
#endif /* GUCEF_WEB_CNATIVEHTTPCLIENTBACKEND_H ? */

#include "gucefWEB_CHttpClientBackendSelection.h"

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

CHttpClientBackendSelection::TStringVector
CHttpClientBackendSelection::GetDefaultPreferredBackends( void )
{GUCEF_TRACE;

    TStringVector defaults;
    defaults.push_back( "curl" );
    defaults.push_back( "winhttp" );
    defaults.push_back( "native" );
    return defaults;
}

/*-------------------------------------------------------------------------*/

CHttpClientBackendSelection::CHttpClientBackendSelection( void )
    : m_preferredBackends( GetDefaultPreferredBackends() )
    , m_backends()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpClientBackendSelection::~CHttpClientBackendSelection()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CHttpClientBackendSelection::SetPreferredBackends( const TStringVector& backendNames )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_preferredBackends = backendNames;

    // The backends are created again on demand. This also releases backends which are no longer wanted,
    // a plugin which provided one can be unloaded once no request is using it
    m_backends.clear();
}

/*-------------------------------------------------------------------------*/

CHttpClientBackendSelection::TStringVector
CHttpClientBackendSelection::GetPreferredBackends( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_preferredBackends;
}

/*-------------------------------------------------------------------------*/

CIHttpClientBackendPtr
CHttpClientBackendSelection::GetBackendForScheme( const CString& scheme )
{GUCEF_TRACE;

    return FindBackendForScheme( scheme, true );
}

/*-------------------------------------------------------------------------*/

bool
CHttpClientBackendSelection::IsSchemeSupported( const CString& scheme )
{GUCEF_TRACE;

    return !FindBackendForScheme( scheme.Lowercase(), false ).IsNULL();
}

/*-------------------------------------------------------------------------*/

CString
CHttpClientBackendSelection::DescribeAvailableBackends( void )
{GUCEF_TRACE;

    THttpClientBackendFactory& factory = CWebGlobal::Instance()->GetHttpClientBackendFactory();

    CString description;
    MT::CScopeMutex lock( m_lock );
    for ( TStringVector::const_reverse_iterator i = m_preferredBackends.rbegin(); i != m_preferredBackends.rend(); ++i )
    {
        const CString& backendName = (*i);
        TBackendMap::iterator existing = m_backends.find( backendName );
        CIHttpClientBackendPtr backend;
        if ( existing != m_backends.end() )
        {
            backend = (*existing).second;
        }
        else
        if ( factory.IsConstructible( backendName ) )
        {
            backend = factory.Create( backendName );
            if ( !backend.IsNULL() )
                m_backends[ backendName ] = backend;
        }
        if ( backend.IsNULL() )
            continue;

        if ( !description.IsNULLOrEmpty() )
            description += ", ";
        description += backendName;
        if ( backend->SupportsScheme( "https" ) )
            description += " (https)";
    }
    return description;
}

/*-------------------------------------------------------------------------*/

CIHttpClientBackendPtr
CHttpClientBackendSelection::FindBackendForScheme( const CString& scheme, bool logWhenNone )
{GUCEF_TRACE;

    THttpClientBackendFactory& factory = CWebGlobal::Instance()->GetHttpClientBackendFactory();

    MT::CScopeMutex lock( m_lock );
    for ( TStringVector::const_iterator i = m_preferredBackends.begin(); i != m_preferredBackends.end(); ++i )
    {
        const CString& backendName = (*i);
        TBackendMap::iterator existing = m_backends.find( backendName );
        CIHttpClientBackendPtr backend;
        if ( existing != m_backends.end() )
        {
            backend = (*existing).second;
        }
        else
        if ( factory.IsConstructible( backendName ) )
        {
            backend = factory.Create( backendName );
            if ( !backend.IsNULL() )
                m_backends[ backendName ] = backend;
        }

        if ( !backend.IsNULL() && backend->SupportsScheme( scheme ) )
            return backend;
    }

    if ( logWhenNone )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpClientBackendSelection: None of the preferred HTTP client backends is available for \"" << scheme <<
            "\": " << DescribeUnsupportedScheme( scheme ) << ". For https install what it says is missing, or load a plugin that provides a backend for it, such as httpclientpluginCURL" );
    return CIHttpClientBackendPtr();
}

/*-------------------------------------------------------------------------*/

CString
CHttpClientBackendSelection::DescribeUnsupportedScheme( const CString& scheme )
{GUCEF_TRACE;

    if ( scheme.Equals( "https", false ) )
    {
        CString whyNot;
        if ( !CNativeHttpClientBackend::IsHttpsAvailable( whyNot ) )
            return "no HTTPS support: " + whyNot;
        return "no HTTP client backend that is preferred carries https";
    }
    return "the scheme \"" + scheme + "\" is not supported";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
