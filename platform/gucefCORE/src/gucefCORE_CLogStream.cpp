/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CLOGSTREAM_H
#include "gucefCORE_CLogStream.h"
#define GUCEF_CORE_CLOGSTREAM_H
#endif /* GUCEF_CORE_CLOGSTREAM_H ? */

#ifndef GUCEF_CORE_CLOGGINGGLOBAL_H
#include "gucefCORE_CLoggingGlobal.h"
#define GUCEF_CORE_CLOGGINGGLOBAL_H
#endif /* GUCEF_CORE_CLOGGINGGLOBAL_H ? */

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

CLogStream::CLogStream( void )
    : CTSharedObjCreator< CLogStream, MT::CMutex >( this )
    , m_variantStream()
    , m_isPulseGeneratorThread( false )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CLogStream::CLogStream( bool isPulseGeneratorThread )
    : CTSharedObjCreator< CLogStream, MT::CMutex >( this )
    , m_variantStream()
    , m_isPulseGeneratorThread( isPulseGeneratorThread )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CLogStream::~CLogStream()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CVariantStream&
CLogStream::GetStream( void )
{GUCEF_TRACE;

    return m_variantStream;
}

/*-------------------------------------------------------------------------*/

const CVariantStream&
CLogStream::GetStream( void ) const
{GUCEF_TRACE;

    return m_variantStream;
}

/*-------------------------------------------------------------------------*/

void
CLogStream::WriteSegmentEnd( void )
{GUCEF_TRACE;

    m_variantStream.WriteSegmentEnd();

    if ( m_isPulseGeneratorThread )
    {
        CLoggingGlobal* loggingGlobal = CLoggingGlobal::Instance();
        if ( GUCEF_NULL != loggingGlobal )
        {
            loggingGlobal->FlushThreadStreamBuffer( CreateSharedPtr() );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CLogStream::GetIsPulseGeneratorThread( void ) const
{GUCEF_TRACE;

    return m_isPulseGeneratorThread;
}

/*-------------------------------------------------------------------------*/

void
CLogStream::SetIsPulseGeneratorThread( bool isPulseGeneratorThread )
{GUCEF_TRACE;

    m_isPulseGeneratorThread = isPulseGeneratorThread;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
