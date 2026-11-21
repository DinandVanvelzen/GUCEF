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

#include "gucefDRN_CDRNIOAccessResourceSink.h"

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

CDRNIOAccessResourceSink::CDRNIOAccessResourceSink( CORE::CIOAccess& access )
    : CIDRNResourceSink()
    , m_access( &access )
    , m_nextOffset( 0 )
    , m_bytesWritten( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNIOAccessResourceSink::~CDRNIOAccessResourceSink()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt64
CDRNIOAccessResourceSink::GetBytesWritten( void ) const
{GUCEF_TRACE;

    return m_bytesWritten;
}

/*-------------------------------------------------------------------------*/

bool
CDRNIOAccessResourceSink::IsInOrder( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNIOAccessResourceSink::Begin( const CORE::CString& provider       ,
                                 const CDRNResourceInfo& info        ,
                                 UInt64 windowOffset                 ,
                                 UInt64 /* windowLength */           ,
                                 bool /* resume */                   ,
                                 CDRNResourcePartialState& state     )
{GUCEF_TRACE;

    if ( !m_access->Opened() )
        m_access->Open();
    if ( !m_access->Opened() || !m_access->IsWriteable() )
        return DRN_RESSTATUS_LOCAL_IO_ERROR;
    m_nextOffset = windowOffset;
    m_bytesWritten = 0;
    return state.Reset( provider, info.name, info ) ? DRN_RESSTATUS_OK : DRN_RESSTATUS_TOO_LARGE;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNIOAccessResourceSink::Write( UInt64 offset, const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( offset != m_nextOffset )
        return DRN_RESSTATUS_LOCAL_IO_ERROR;
    if ( size != m_access->Write( data, 1, size ) )
        return DRN_RESSTATUS_LOCAL_IO_ERROR;
    m_nextOffset += size;
    m_bytesWritten += size;
    return DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNIOAccessResourceSink::Finish( void )
{GUCEF_TRACE;

    m_access->Flush();
    return DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CDRNIOAccessResourceSink::Abort( const CDRNResourcePartialState& /* state */, bool /* discard */ )
{GUCEF_TRACE;

    m_access->Flush();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
