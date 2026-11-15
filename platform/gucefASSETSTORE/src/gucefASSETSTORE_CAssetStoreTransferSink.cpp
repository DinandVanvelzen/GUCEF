/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#include "gucefASSETSTORE_CAssetStoreTransferSink.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetStoreTransferSink::CAssetStoreTransferSink( CAssetStoreTransferContext& context, const WEB::CHttpResponseData& response )
    : CORE::CIOAccess()
    , m_context( context )
    , m_response( response )
    , m_file()
    , m_access()
    , m_fileBytes( context.GetResumeOffset() )
    , m_opened( false )
    , m_stopped( false )
    , m_writeError( false )
    , m_restarted( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreTransferSink::~CAssetStoreTransferSink()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferSink::OpenFile( bool append )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    m_file = vfs.GetFile( m_context.GetPartFilePath(), append ? "ab" : "wb", !append );
    if ( m_file.IsNULL() )
        return false;
    m_access = m_file->GetAccess();
    m_opened = !m_access.IsNULL();
    return m_opened;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreTransferSink::Write( const void* srcdata, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    const UInt32 byteCount = esize * elements;
    if ( m_context.IsStopRequested() )
    {
        m_stopped = true;
        return 0;
    }

    const int status = m_response.statusCode;
    const bool keep = 0 == status || ( status >= 200 && status < 300 );
    if ( !keep )
        return elements;            // an error page: not part of the file

    if ( !m_opened )
    {
        const bool append = 206 == status && m_context.GetResumeOffset() > 0;
        if ( !append )
        {
            m_restarted = m_context.GetResumeOffset() > 0;
            m_fileBytes = 0;
        }
        if ( !OpenFile( append ) )
        {
            m_writeError = true;
            return 0;
        }
    }

    // in slices, so a throttle and a request to stop take effect within a block of the body
    const UInt8* bytes = static_cast< const UInt8* >( srcdata );
    UInt32 offset = 0;
    while ( offset < byteCount )
    {
        const UInt32 slice = ( byteCount - offset ) > SliceSize ? SliceSize : ( byteCount - offset );
        const UInt32 written = m_access->Write( bytes + offset, 1, slice );
        if ( written != slice )
        {
            m_writeError = true;
            return 0;
        }
        offset += slice;
        m_fileBytes += slice;
        m_context.ReportFileBytes( m_fileBytes );
        m_context.Throttle( m_fileBytes );
        if ( m_context.IsStopRequested() )
        {
            // what was written stays in the part file, the rest of the body is not taken
            m_stopped = true;
            return 0;
        }
    }
    return elements;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreTransferSink::Open( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetStoreTransferSink::Close( void )
{GUCEF_TRACE;

    if ( !m_access.IsNULL() )
    {
        m_access->Flush();
        m_access->Close();
    }
    m_access.Unlink();
    m_file.Unlink();
    m_opened = false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferSink::Opened( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreTransferSink::Read( void* dest, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetStoreTransferSink::Tell( void ) const
{GUCEF_TRACE;

    return m_fileBytes;
}

/*-------------------------------------------------------------------------*/

Int32
CAssetStoreTransferSink::Seek( Int64 offset, Int32 origin )
{GUCEF_TRACE;

    return -1;
}

/*-------------------------------------------------------------------------*/

char
CAssetStoreTransferSink::GetChar( void )
{GUCEF_TRACE;

    return '\0';
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferSink::Eof( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferSink::IsReadable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferSink::IsWriteable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferSink::IsValid( void )
{GUCEF_TRACE;

    return !m_writeError;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreTransferSink::Flush( void )
{GUCEF_TRACE;

    if ( !m_access.IsNULL() )
        m_access->Flush();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetStoreTransferSink::Clone( void ) const
{GUCEF_TRACE;

    // a sink writes one file for one request: it is not copied
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetStoreTransferSink::GetFileBytes( void ) const
{GUCEF_TRACE;

    return m_fileBytes;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferSink::WasStopped( void ) const
{GUCEF_TRACE;

    return m_stopped;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferSink::HadWriteError( void ) const
{GUCEF_TRACE;

    return m_writeError;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreTransferSink::WasRestarted( void ) const
{GUCEF_TRACE;

    return m_restarted;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
