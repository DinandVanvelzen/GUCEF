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

#include <stdio.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "vfspluginDRN_CDrnBridgeFileAccess.h"

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

CDrnBridgeFileAccess::CDrnBridgeFileAccess( const CDrnBridgeHandlePoolPtr& pool, const CORE::CString& path )
    : CORE::CIOAccess()
    , m_pool( pool )
    , m_path( path )
    , m_pinnedRevision()
    , m_expectedRevision()
    , m_reportsChange( false )
    , m_pinnedSize( 0 )
    , m_pinnedModifiedUtcMs( 0 )
    , m_pinned( false )
    , m_position( 0 )
    , m_closed( false )
    , m_failed( false )
    , m_lastResult()
    , m_inner( GUCEF_NULL )
    , m_busy( false )
    , m_opening( false )
    , m_lastUsed( 0 )
{GUCEF_TRACE;

    m_pool->Attach( *this );
}

/*-------------------------------------------------------------------------*/

CDrnBridgeFileAccess::CDrnBridgeFileAccess( const CDrnBridgeFileAccess& src )
    : CORE::CIOAccess( src )
    , m_pool( src.m_pool )
    , m_path( src.m_path )
    , m_pinnedRevision( src.m_pinnedRevision )
    , m_expectedRevision( src.m_expectedRevision )
    , m_reportsChange( src.m_reportsChange )
    , m_pinnedSize( src.m_pinnedSize )
    , m_pinnedModifiedUtcMs( src.m_pinnedModifiedUtcMs )
    , m_pinned( src.m_pinned )
    , m_position( src.m_position )
    , m_closed( src.m_closed )
    , m_failed( src.m_failed )
    , m_lastResult( src.m_lastResult )
    , m_inner( GUCEF_NULL )
    , m_busy( false )
    , m_opening( false )
    , m_lastUsed( 0 )
{GUCEF_TRACE;

    m_pool->Attach( *this );
}

/*-------------------------------------------------------------------------*/

CDrnBridgeFileAccess::~CDrnBridgeFileAccess()
{GUCEF_TRACE;

    m_pool->Detach( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDrnBridgeFileAccess::GetPath( void ) const
{GUCEF_TRACE;

    return m_path;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDrnBridgeFileAccess::GetPinnedRevision( void ) const
{GUCEF_TRACE;

    return m_pinnedRevision;
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeFileAccess::GetPinnedModifiedUtcMs( void ) const
{GUCEF_TRACE;

    return m_pinnedModifiedUtcMs;
}

/*-------------------------------------------------------------------------*/

const CDrnBridgeResult&
CDrnBridgeFileAccess::GetLastResult( void ) const
{GUCEF_TRACE;

    return m_lastResult;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeFileAccess::ReleaseHandle( void )
{GUCEF_TRACE;

    m_pool->DropHandle( *this );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeFileAccess::HasHandle( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_pool->m_lock );
    return GUCEF_NULL != m_inner;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeFileAccess::Fail( const CDrnBridgeResult& result )
{GUCEF_TRACE;

    m_lastResult = result;
    m_pool->RecordResult( result );
    if ( DRN::DRN_NSRESULT_UNAVAILABLE == result.code )
    {
        // the handle is dead (its link went): it is not kept, the next read opens another
        m_pool->DropHandle( *this );
    }
    if ( result.IsChanged() )
    {
        // the file is another revision now: this access ends, the next open of the path gets the new one
        m_failed = true;
        m_pool->NoteChanged( m_path, m_pinnedRevision, m_reportsChange );
        m_pool->DropHandle( *this );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DrnBridgeFileAccess: \"" << m_path << "\" changed on the serving side since it was opened, this access to it ends: " << result.message );
    }
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeFileAccess::Open( void )
{GUCEF_TRACE;

    m_closed = false;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeFileAccess::Close( void )
{GUCEF_TRACE;

    m_closed = true;
    m_pool->DropHandle( *this );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeFileAccess::Opened( void ) const
{GUCEF_TRACE;

    return !m_closed;
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnBridgeFileAccess::Read( void* dest, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    if ( m_closed || m_failed || GUCEF_NULL == dest || 0 == esize || 0 == elements || m_position >= m_pinnedSize )
        return 0;

    // a read whose handle was lost (the link went) opens a handle again once, at the revision the file was opened at, when the link is back
    const UInt64 positionBefore = m_position;
    for ( UInt32 attempt = 0; attempt < 2; ++attempt )
    {
        CDrnBridgeResult result;
        DRN::CDRNResourceIOAccess* inner = m_pool->BeginUse( *this, result );
        if ( GUCEF_NULL == inner )
        {
            Fail( result );
            return 0;
        }

        // the handle keeps its own position: it is where the last read left it, or new
        if ( inner->Tell() != positionBefore )
            inner->Setpos( positionBefore );
        const UInt32 got = inner->Read( dest, esize, elements );
        m_position = positionBefore + (UInt64) got * esize;

        // fewer elements than the file holds from here, as far as were asked for: the handle failed (the file changed, the link went, ...)
        const UInt64 available = ( m_pinnedSize - positionBefore ) / esize;
        const UInt32 expected = available < elements ? (UInt32) available : elements;
        if ( got >= expected )
        {
            m_pool->EndUse( *this );
            return got;
        }

        CDrnBridgeResult failure = CDrnBridgeResult::FromTransferResult( inner->GetLastResult() );
        if ( failure.IsOk() )
            failure = CDrnBridgeResult( DRN::DRN_NSRESULT_IO_ERROR, DRN::DRN_RESSTATUS_IO_ERROR, "the read of the file gave less than the file holds" );
        m_pool->EndUse( *this );
        if ( 0 == got && 0 == attempt && DRN::DRN_NSRESULT_UNAVAILABLE == failure.code )
        {
            m_pool->DropHandle( *this );
            continue;
        }
        Fail( failure );
        return got;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeFileAccess::Tell( void ) const
{GUCEF_TRACE;

    return m_position;
}

/*-------------------------------------------------------------------------*/

Int32
CDrnBridgeFileAccess::Seek( Int64 offset, Int32 origin )
{GUCEF_TRACE;

    Int64 base = 0;
    switch ( origin )
    {
        case SEEK_SET: { base = 0; break; }
        case SEEK_CUR: { base = (Int64) m_position; break; }
        case SEEK_END: { base = (Int64) m_pinnedSize; break; }
        default: return -1;
    }
    const Int64 target = base + offset;
    if ( target < 0 )
        return -1;
    m_position = (UInt64) target;
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnBridgeFileAccess::Setpos( UInt64 position )
{GUCEF_TRACE;

    m_position = position;
    return 0;
}

/*-------------------------------------------------------------------------*/

char
CDrnBridgeFileAccess::GetChar( void )
{GUCEF_TRACE;

    char value = 0;
    if ( 1 != Read( &value, 1, 1 ) )
        return 0;
    return value;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeFileAccess::Eof( void ) const
{GUCEF_TRACE;

    return m_position >= m_pinnedSize;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeFileAccess::IsReadable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeFileAccess::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeFileAccess::IsValid( void )
{GUCEF_TRACE;

    return !m_failed && !m_closed && !m_pool->IsShutdown();
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeFileAccess::GetSize( void ) const
{GUCEF_TRACE;

    return m_pinnedSize;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeFileAccess::Flush( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CDrnBridgeFileAccess::Write( const void* srcdata, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    // read only: nothing is ever written to the share
    return 0;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CDrnBridgeFileAccess::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDrnBridgeFileAccess( *this );
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
