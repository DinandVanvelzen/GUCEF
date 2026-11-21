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

#include "gucefVFS_CVfsPrefetchWork.h"

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

CVfsPrefetchWork::CVfsPrefetchWork( const CORE::CString::StringVector& files )
    : m_lock()
    , m_files( files )
    , m_next( 0 )
    , m_helpersRunning( 0 )
    , m_succeeded( 0 )
    , m_failed( 0 )
    , m_cancelled( false )
    , m_firstFailure()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CVfsPrefetchWork::TakeNext( CORE::CString& file )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_next >= m_files.size() )
        return false;
    file = m_files[ m_next++ ];
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVfsPrefetchWork::NoteEnded( const CVfsCallResult& result )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( result.IsSuccess() )
    {
        ++m_succeeded;
    }
    else
    {
        ++m_failed;
        if ( CORE::FILESYSTEMACCESS_CANCELLED == result.result )
            m_cancelled = true;
        else
        if ( m_firstFailure.IsSuccess() )
            m_firstFailure = result;
    }
}

/*-------------------------------------------------------------------------*/

void
CVfsPrefetchWork::HelperStarted( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    ++m_helpersRunning;
}

/*-------------------------------------------------------------------------*/

void
CVfsPrefetchWork::HelperEnded( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_helpersRunning > 0 )
        --m_helpersRunning;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsPrefetchWork::GetHelpersRunning( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_helpersRunning;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsPrefetchWork::GetFilesTaken( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_next;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsPrefetchWork::GetFilesSucceeded( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_succeeded;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsPrefetchWork::GetFilesFailed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_failed;
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsPrefetchWork::GetFirstFailure( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_firstFailure;
}

/*-------------------------------------------------------------------------*/

bool
CVfsPrefetchWork::HasCancelled( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_cancelled;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
