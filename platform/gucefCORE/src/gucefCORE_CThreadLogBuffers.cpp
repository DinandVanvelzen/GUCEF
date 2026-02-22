/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CTHREADLOGBUFFERS_H
#include "gucefCORE_CThreadLogBuffers.h"
#define GUCEF_CORE_CTHREADLOGBUFFERS_H
#endif /* GUCEF_CORE_CTHREADLOGBUFFERS_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CThreadLogBuffers::CThreadLogBuffers( void )
    : m_frontBuffer( GUCEF_NEW CVariantStream() )
    , m_backBuffer( GUCEF_NEW CVariantStream() )
    , m_swapLock()
    , m_threadId( 0 )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CThreadLogBuffers::~CThreadLogBuffers()
{GUCEF_TRACE;

    m_frontBuffer.Unlink();
    m_backBuffer.Unlink();
}

/*-------------------------------------------------------------------------*/

void
CThreadLogBuffers::Swap( void )
{GUCEF_TRACE;

    CVariantStreamTypedPtr frontBufferPtr = m_frontBuffer;
    m_frontBuffer = m_backBuffer;
    m_backBuffer = frontBufferPtr;
}

/*-------------------------------------------------------------------------*/

CVariantStreamPtr
CThreadLogBuffers::GetFrontBuffer( void )
{GUCEF_TRACE;

    return m_frontBuffer;
}

/*-------------------------------------------------------------------------*/

CVariantStreamPtr
CThreadLogBuffers::GetBackBuffer( void )
{GUCEF_TRACE;

    return m_backBuffer;
}

/*-------------------------------------------------------------------------*/

UInt32
CThreadLogBuffers::GetThreadId( void ) const
{GUCEF_TRACE;

    return m_threadId;
}

/*-------------------------------------------------------------------------*/

void
CThreadLogBuffers::SetThreadId( UInt32 threadId )
{GUCEF_TRACE;

    m_threadId = threadId;
}

/*-------------------------------------------------------------------------*/

MT::CMutex&
CThreadLogBuffers::GetSwapLock( void )
{GUCEF_TRACE;

    return m_swapLock;
}

/*-------------------------------------------------------------------------*/

bool
CThreadLogBuffers::IsFrontBufferInUse( void ) const
{GUCEF_TRACE;

    // If ref count > 1, someone else (CLogStreamScope) holds a reference
    return m_frontBuffer.GetReferenceCount() > 1;
}

/*-------------------------------------------------------------------------*/

CVariantStreamPtr
CThreadLogBuffers::TrySwapAndGetBackBuffer( void )
{GUCEF_TRACE;

    try
    {
        // we lock the shared pointer itself to ensure that the
        // reference count check and swap are atomic with respect to other threads
        // this thus prevents the front buffer from being acquired by the logger thread
        // while we are in the middle of swapping
        MT::CObjectScopeLock lock( m_frontBuffer );
    
        // Check if front buffer is in use
        if ( m_frontBuffer.GetReferenceCount() > 1 )
        {
            // Front buffer is in use, cannot swap
            return CVariantStreamPtr();
        }
    
        // Safe to swap - no external references to front buffer
        CVariantStreamTypedPtr frontBufferPtr = m_frontBuffer;
        m_frontBuffer = m_backBuffer;
        m_backBuffer = frontBufferPtr;
    
        // Return the back buffer (which was the front before swap) for draining
        // The caller will process this and then clear it
        return m_backBuffer;
    }
    catch ( const timeout_exception& )
    {
        // Simply return a null pointer if we fail to acquire the lock within the timeout
        // This is a non-critical operation, so we can just skip this flush cycle if we are contended
        return CVariantStreamPtr();
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
