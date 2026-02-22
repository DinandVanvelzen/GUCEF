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

    MT::CScopeMutex lock( m_swapLock );
    CVariantStreamPtr temp = m_frontBuffer;
    m_frontBuffer = m_backBuffer;
    m_backBuffer = temp;
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
