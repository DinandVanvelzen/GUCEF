/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_CRpcFrameEventData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRpcFrameEventData::CRpcFrameEventData( const void* frame, UInt32 size, TRpcProtectionClass protection )
    : CORE::CICloneable()
    , m_frame( static_cast< const UInt8* >( frame ) )
    , m_size( size )
    , m_protection( protection )
    , m_ownedCopy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcFrameEventData::CRpcFrameEventData( const CRpcFrameEventData& src )
    : CORE::CICloneable( src )
    , m_frame( GUCEF_NULL )
    , m_size( src.m_size )
    , m_protection( src.m_protection )
    , m_ownedCopy()
{GUCEF_TRACE;

    if ( src.m_size > 0 )
    {
        m_ownedCopy.CopyFrom( src.m_size, src.m_frame );
        m_frame = static_cast< const UInt8* >( m_ownedCopy.GetConstBufferPtr() );
    }
}

/*-------------------------------------------------------------------------*/

CRpcFrameEventData::~CRpcFrameEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const UInt8*
CRpcFrameEventData::GetFrame( void ) const
{GUCEF_TRACE;

    return m_frame;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcFrameEventData::GetSize( void ) const
{GUCEF_TRACE;

    return m_size;
}

/*-------------------------------------------------------------------------*/

TRpcProtectionClass
CRpcFrameEventData::GetProtection( void ) const
{GUCEF_TRACE;

    return m_protection;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRpcFrameEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRpcFrameEventData( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
