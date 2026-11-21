/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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

#include "vfspluginDRN_CDrnBridgeLastResults.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDrnBridgeLastResults::CDrnBridgeLastResults( void )
    : m_lock()
    , m_results()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDrnBridgeLastResults::~CDrnBridgeLastResults()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CDrnBridgeResult&
CDrnBridgeLastResults::Set( const CDrnBridgeResult& result ) const
{GUCEF_TRACE;

    const TThreadId threadId = MT::GetCurrentThreadID();
    MT::CScopeMutex lock( m_lock );
    if ( result.IsOk() )
    {
        TResultMap::iterator i = m_results.find( threadId );
        if ( i != m_results.end() )
            m_results.erase( i );
    }
    else
    {
        if ( m_results.size() >= MaxThreads && m_results.find( threadId ) == m_results.end() )
            m_results.clear();
        m_results[ threadId ] = result;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CDrnBridgeResult
CDrnBridgeLastResults::Get( void ) const
{GUCEF_TRACE;

    const TThreadId threadId = MT::GetCurrentThreadID();
    MT::CScopeMutex lock( m_lock );
    TResultMap::const_iterator i = m_results.find( threadId );
    return i != m_results.end() ? (*i).second : CDrnBridgeResult();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
