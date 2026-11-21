/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#include "gucefVFS_CVfsLastResults.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsLastResults::CVfsLastResults( void )
    : m_lock()
    , m_results()
    , m_entryCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsLastResults::~CVfsLastResults()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CVfsLastResults::Set( const CVfsCallResult& result ) const
{GUCEF_TRACE;

    if ( result.IsSuccess() )
    {
        Clear();
        return;
    }

    const TThreadId threadId = MT::GetCurrentThreadID();
    MT::CScopeMutex lock( m_lock );
    if ( m_results.size() >= MaxThreads && m_results.find( threadId ) == m_results.end() )
        m_results.clear();
    m_results[ threadId ] = result;
    m_entryCount.Store( (UInt32) m_results.size() );
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsLastResults::Get( void ) const
{GUCEF_TRACE;

    if ( 0 == m_entryCount.Load() )
        return CVfsCallResult();

    const TThreadId threadId = MT::GetCurrentThreadID();
    MT::CScopeMutex lock( m_lock );
    TResultMap::const_iterator i = m_results.find( threadId );
    return i != m_results.end() ? i->second : CVfsCallResult();
}

/*-------------------------------------------------------------------------*/

void
CVfsLastResults::Clear( void ) const
{GUCEF_TRACE;

    if ( 0 == m_entryCount.Load() )
        return;

    const TThreadId threadId = MT::GetCurrentThreadID();
    MT::CScopeMutex lock( m_lock );
    if ( 0 != m_results.erase( threadId ) )
        m_entryCount.Store( (UInt32) m_results.size() );
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsLastResults::GetEntryCount( void ) const
{GUCEF_TRACE;

    return m_entryCount.Load();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
