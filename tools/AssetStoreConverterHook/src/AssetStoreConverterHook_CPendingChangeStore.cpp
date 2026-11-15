/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "AssetStoreConverterHook_CPendingChangeStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 TokenBytes = 16;

UInt64
NowMs( void )
{GUCEF_TRACE;

    return MT::PrecisionTickCount() * 1000 / MT::PrecisionTimerResolution();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPendingChangeStore::CPendingChangeStore( void )
    : m_lock()
    , m_changes()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPendingChangeStore::ExpireLocked( UInt64 nowMs ) const
{GUCEF_TRACE;

    for ( size_t i=m_changes.size(); i>0; --i )
        if ( m_changes[ i-1 ].expiresAtMs <= nowMs )
            m_changes.erase( m_changes.begin() + ( i-1 ) );
}

/*-------------------------------------------------------------------------*/

bool
CPendingChangeStore::Add( CPendingChange& change, UInt32 ttlMs, UInt32 maxPerPrincipal )
{GUCEF_TRACE;

    UInt8 bytes[ TokenBytes ];
    if ( !CORE::CRandom::GetRandomBytes( bytes, TokenBytes ) || !CORE::Base64UrlEncode( bytes, TokenBytes, change.token, false ) )
        return false;
    const UInt64 now = NowMs();
    change.expiresAtMs = now + ttlMs;
    change.expiresUtcMs = (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() + (Int64) ttlMs;

    MT::CScopeMutex lock( m_lock );
    ExpireLocked( now );
    UInt32 held = 0;
    for ( size_t i=0; i<m_changes.size(); ++i )
        if ( m_changes[ i ].principal == change.principal )
            ++held;
    // the oldest of the principal go first: the list is in the order of their making
    for ( size_t i=0; i<m_changes.size() && held>=maxPerPrincipal; )
    {
        if ( m_changes[ i ].principal == change.principal )
        {
            m_changes.erase( m_changes.begin() + i );
            --held;
            continue;
        }
        ++i;
    }
    m_changes.push_back( change );
    return true;
}

/*-------------------------------------------------------------------------*/

CPendingChangeStore::ETakeResult
CPendingChangeStore::Take( const CORE::CString& token, const CORE::CString& principal, const CORE::CString& operation,
                           const CORE::CString& argumentsDigest, CPendingChange& change )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    ExpireLocked( NowMs() );
    for ( size_t i=0; i<m_changes.size(); ++i )
    {
        if ( m_changes[ i ].token != token )
            continue;
        if ( m_changes[ i ].principal != principal || m_changes[ i ].operation != operation || m_changes[ i ].argumentsDigest != argumentsDigest )
            return TAKE_MISMATCH;
        change = m_changes[ i ];
        m_changes.erase( m_changes.begin() + i );
        return TAKE_OK;
    }
    return TAKE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

UInt32
CPendingChangeStore::GetCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    ExpireLocked( NowMs() );
    return (UInt32) m_changes.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
