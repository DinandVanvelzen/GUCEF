/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#include <string.h>

#include "gucefDRN_CDRNMemoryResourceSink.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNMemoryResourceSink::CDRNMemoryResourceSink( UInt32 maxBytes )
    : CIDRNResourceSink()
    , m_maxBytes( maxBytes )
    , m_windowOffset( 0 )
    , m_data()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNMemoryResourceSink::~CDRNMemoryResourceSink()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CDynamicBuffer&
CDRNMemoryResourceSink::GetData( void ) const
{GUCEF_TRACE;

    return m_data;
}

/*-------------------------------------------------------------------------*/

void
CDRNMemoryResourceSink::TakeData( CORE::CDynamicBuffer& destination )
{GUCEF_TRACE;

    destination = GUCEF_MOVE( m_data );
    m_data.Clear( false );
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNMemoryResourceSink::Begin( const CORE::CString& provider       ,
                               const CDRNResourceInfo& info        ,
                               UInt64 windowOffset                 ,
                               UInt64 windowLength                 ,
                               bool /* resume */                   ,
                               CDRNResourcePartialState& state     )
{GUCEF_TRACE;

    if ( windowLength > m_maxBytes )
        return DRN_RESSTATUS_TOO_LARGE;
    m_windowOffset = windowOffset;
    if ( !m_data.SetBufferSize( (UInt32) windowLength, false, true ) )
        return DRN_RESSTATUS_TOO_LARGE;
    m_data.SetDataSize( (UInt32) windowLength );
    if ( windowLength > 0 )
        memset( m_data.GetBufferPtr(), 0, (size_t) windowLength );
    return state.Reset( provider, info.name, info ) ? DRN_RESSTATUS_OK : DRN_RESSTATUS_TOO_LARGE;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNMemoryResourceSink::Write( UInt64 offset, const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( offset < m_windowOffset || offset - m_windowOffset + size > m_data.GetDataSize() )
        return DRN_RESSTATUS_LOCAL_IO_ERROR;
    memcpy( static_cast< UInt8* >( m_data.GetBufferPtr() ) + ( offset - m_windowOffset ), data, size );
    return DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNMemoryResourceSink::ReadBack( UInt64 offset, UInt32 size, void* destination )
{GUCEF_TRACE;

    if ( offset < m_windowOffset || offset - m_windowOffset + size > m_data.GetDataSize() )
        return DRN_RESSTATUS_LOCAL_IO_ERROR;
    memcpy( destination, static_cast< const UInt8* >( m_data.GetConstBufferPtr() ) + ( offset - m_windowOffset ), size );
    return DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNMemoryResourceSink::Finish( void )
{GUCEF_TRACE;

    return DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CDRNMemoryResourceSink::Abort( const CDRNResourcePartialState& /* state */, bool /* discard */ )
{GUCEF_TRACE;

    m_data.Clear( false );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
