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

#include "gucefVFS_CVfsFetchProgress.h"

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

CVfsFetchProgress::CVfsFetchProgress( void )
    : bytesTotal( 0 )
    , bytesDone( 0 )
    , filesTotal( 0 )
    , filesDone( 0 )
    , filesFailed( 0 )
    , cancel( false )
    , m_lock()
    , m_lastFailure()
    , m_cancelSource( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsFetchProgress::GetLastFailure( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastFailure;
}

/*-------------------------------------------------------------------------*/

void
CVfsFetchProgress::SetLastFailure( const CVfsCallResult& failure )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_lastFailure = failure;
}

/*-------------------------------------------------------------------------*/

bool
CVfsFetchProgress::IsCancelRequested( void ) const
{GUCEF_TRACE;

    if ( cancel.Load() )
        return true;

    // asked under the lock: the call that set the source takes it away under the same lock, before the source is gone
    MT::CScopeMutex lock( m_lock );
    return GUCEF_NULL != m_cancelSource && m_cancelSource->IsCancelRequested();
}

/*-------------------------------------------------------------------------*/

void
CVfsFetchProgress::SetCancelSource( const CIVfsCancelSource* source )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_cancelSource = source;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
