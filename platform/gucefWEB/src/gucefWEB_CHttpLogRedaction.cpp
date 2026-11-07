/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#ifndef GUCEF_STL_H
#include "gucef_stl.h"
#define GUCEF_STL_H
#endif /* GUCEF_STL_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#include "gucefWEB_CHttpLogRedaction.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The flags that the opt-out for developers turns off: what shows the path, the query and the fragment as they are */
static const CORE::TRedactionFlags fullUrlFlags = CORE::REDACT_URL_QUERY_VALUES | CORE::REDACT_URL_FRAGMENT | CORE::REDACT_URL_PATH_IDS;

/*-------------------------------------------------------------------------*/

/** The redactors for the log of the HTTP stack, the secrets that are registered and whether the opt-out is on */
class CState
{
    public:

    CState( void )
        : m_lock()
        , m_secretCounts()
        , m_url( CORE::CRedactor::ForUrl() )
        , m_headers( CORE::CRedactor::ForHeaders() )
        , m_text( CORE::CRedactor::ForLogLine() )
        , m_fullUrls( false )
    {
        m_url.AddFlags( CORE::REDACT_URL_PATH_IDS | CORE::REDACT_NAME_WORDS );
        m_headers.AddFlags( CORE::REDACT_URL_PATH_IDS );
        m_text.AddFlags( CORE::REDACT_URL_PATH_IDS );

        CORE::CString fromEnvironment = CORE::GetEnv( GUCEF_WEB_LOG_FULL_URLS_ENVVAR );
        if ( fromEnvironment == "1" || fromEnvironment.Equals( "true", false ) || fromEnvironment.Equals( "yes", false ) )
            SetFullUrls( true );
    }

    void SetFullUrls( bool enabled )
    {
        MT::CScopeMutex lock( m_lock );
        m_fullUrls = enabled;
    }

    bool GetFullUrls( void )
    {
        MT::CScopeMutex lock( m_lock );
        return IsFullUrls();
    }

    CORE::CString RedactUrl( const CORE::CString& url )
    {
        MT::CScopeMutex lock( m_lock );
        return Choose( m_url ).RedactUrl( url );
    }

    CORE::CString RedactHeaders( const CORE::CString& block )
    {
        MT::CScopeMutex lock( m_lock );
        return Choose( m_headers ).RedactHeaders( block );
    }

    CORE::CString RedactHeaderValue( const CORE::CString& name, const CORE::CString& value )
    {
        MT::CScopeMutex lock( m_lock );
        return Choose( m_headers ).RedactHeaderValue( name, value );
    }

    CORE::CString RedactText( const CORE::CString& text )
    {
        MT::CScopeMutex lock( m_lock );
        return Choose( m_text ).RedactText( text );
    }

    bool AddSecret( const CORE::CString& secret )
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_url.AddSecret( secret ) )
            return false;
        m_headers.AddSecret( secret );
        m_text.AddSecret( secret );
        ++m_secretCounts[ secret ];
        return true;
    }

    bool RemoveSecret( const CORE::CString& secret )
    {
        MT::CScopeMutex lock( m_lock );
        TSecretCountMap::iterator i = m_secretCounts.find( secret );
        if ( i == m_secretCounts.end() )
            return false;
        if ( (*i).second > 1 )
        {
            --(*i).second;
            return true;
        }
        m_secretCounts.erase( i );
        m_url.RemoveSecret( secret );
        m_headers.RemoveSecret( secret );
        m_text.RemoveSecret( secret );
        return true;
    }

    void ClearSecrets( void )
    {
        MT::CScopeMutex lock( m_lock );
        m_url.ClearSecrets();
        m_headers.ClearSecrets();
        m_text.ClearSecrets();
        m_secretCounts.clear();
    }

    private:

    CState( const CState& src );
    CState& operator=( const CState& src );

    /** Release builds always redact, whatever was asked for */
    bool IsFullUrls( void ) const
    {
        #ifdef GUCEF_DEBUG_MODE
        return m_fullUrls;
        #else
        return false;
        #endif
    }

    /** The redactor to use: with the opt-out on one that leaves the path, the query and the fragment as they are */
    const CORE::CRedactor& Choose( const CORE::CRedactor& redactor )
    {
        if ( !IsFullUrls() )
            return redactor;

        // the opt-out is rare and for developers, so the redactor that serves it is made when it is needed
        m_full = redactor;
        m_full.RemoveFlags( fullUrlFlags );
        return m_full;
    }

    typedef GUCEF::map< CORE::CString, UInt32 > TSecretCountMap;

    MT::CMutex m_lock;
    TSecretCountMap m_secretCounts;         /**< how many times each secret was registered */
    CORE::CRedactor m_url;
    CORE::CRedactor m_headers;
    CORE::CRedactor m_text;
    CORE::CRedactor m_full;
    bool m_fullUrls;
};

/*-------------------------------------------------------------------------*/

CState&
GetState( void )
{
    static CState state;
    return state;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CHttpLogRedaction::RedactUrl( const CORE::CString& url )
{GUCEF_TRACE;

    return GetState().RedactUrl( url );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CHttpLogRedaction::RedactHeaders( const CORE::CString& headerBlock )
{GUCEF_TRACE;

    return GetState().RedactHeaders( headerBlock );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CHttpLogRedaction::RedactHeaderValue( const CORE::CString& name  ,
                                      const CORE::CString& value )
{GUCEF_TRACE;

    return GetState().RedactHeaderValue( name, value );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CHttpLogRedaction::RedactText( const CORE::CString& text )
{GUCEF_TRACE;

    return GetState().RedactText( text );
}

/*-------------------------------------------------------------------------*/

bool
CHttpLogRedaction::AddSecret( const CORE::CString& secret )
{GUCEF_TRACE;

    return GetState().AddSecret( secret );
}

/*-------------------------------------------------------------------------*/

bool
CHttpLogRedaction::RemoveSecret( const CORE::CString& secret )
{GUCEF_TRACE;

    return GetState().RemoveSecret( secret );
}

/*-------------------------------------------------------------------------*/

void
CHttpLogRedaction::ClearSecrets( void )
{GUCEF_TRACE;

    GetState().ClearSecrets();
}

/*-------------------------------------------------------------------------*/

void
CHttpLogRedaction::SetFullUrlsInDebugLogs( bool enabled )
{GUCEF_TRACE;

    GetState().SetFullUrls( enabled );
}

/*-------------------------------------------------------------------------*/

bool
CHttpLogRedaction::GetFullUrlsInDebugLogs( void )
{GUCEF_TRACE;

    return GetState().GetFullUrls();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
