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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "gucefVFS_CVfsAsyncCallGate.h"

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

CVfsAsyncCallGate::CVfsAsyncCallGate( void )
    : CIVfsCancelSource()
    , m_lock()
    , m_closed( false )
    , m_inside( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsAsyncCallGate::~CVfsAsyncCallGate()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CVfsAsyncCallGate::TryEnter( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_closed )
        return false;
    ++m_inside;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVfsAsyncCallGate::Leave( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_inside > 0 )
        --m_inside;
}

/*-------------------------------------------------------------------------*/

bool
CVfsAsyncCallGate::CloseAndWait( UInt32 timeoutInMs )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_closed = true;
    }

    UInt32 waited = 0;
    for ( ;; )
    {
        {
            MT::CScopeMutex lock( m_lock );
            if ( 0 == m_inside )
                return true;
        }
        if ( waited >= timeoutInMs )
            return false;
        MT::ThreadDelay( 1 );
        ++waited;
    }
}

/*-------------------------------------------------------------------------*/

bool
CVfsAsyncCallGate::IsClosed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_closed;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsAsyncCallGate::GetInside( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_inside;
}

/*-------------------------------------------------------------------------*/

bool
CVfsAsyncCallGate::IsCancelRequested( void ) const
{GUCEF_TRACE;

    return IsClosed();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
