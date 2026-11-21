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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "vfspluginDRN_CDrnBridgeBlockSource.h"

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

CDrnBridgeBlockSource::CDrnBridgeBlockSource( CDrnBridgeFileAccess* access, MT::CTAtomic< UInt64 >* fetchCalls, MT::CTAtomic< UInt64 >* bytesFetched )
    : VFS::CIVfsBlockSource()
    , m_access( access )
    , m_fetchCalls( fetchCalls )
    , m_bytesFetched( bytesFetched )
    , m_lastBegin( 0 )
    , m_lastEnd( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDrnBridgeBlockSource::~CDrnBridgeBlockSource()
{GUCEF_TRACE;

    GUCEF_DELETE m_access;
    m_access = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

VFS::CIVfsBlockSourcePtr
CDrnBridgeBlockSource::Open( const CDrnBridgeHandlePoolPtr& pool   ,
                             const CORE::CString& path             ,
                             const CORE::CString& expectedRevision ,
                             VFS::CVfsCallResult& result           ,
                             CDrnBridgeResult* bridgeResult        ,
                             MT::CTAtomic< UInt64 >* fetchCalls    ,
                             MT::CTAtomic< UInt64 >* bytesFetched  )
{GUCEF_TRACE;

    CDrnBridgeResult opened;
    CDrnBridgeFileAccess* access = CDrnBridgeHandlePool::OpenFile( pool, path, opened, expectedRevision );
    if ( GUCEF_NULL != bridgeResult )
        *bridgeResult = opened;
    if ( GUCEF_NULL == access )
    {
        result = opened.ToCallResult();
        if ( VFS::CVfsCallResult( opened.fileSystemResult ).IsSuccess() )
            result = VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "\"" + path + "\" could not be opened" );
        return VFS::CIVfsBlockSourcePtr();
    }

    result = VFS::CVfsCallResult();
    return VFS::CIVfsBlockSourcePtr( GUCEF_NEW CDrnBridgeBlockSource( access, fetchCalls, bytesFetched ) );
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeBlockSource::GetFileSize( void ) const
{GUCEF_TRACE;

    return m_access->GetSize();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDrnBridgeBlockSource::GetPinnedRevision( void ) const
{GUCEF_TRACE;

    return m_access->GetPinnedRevision();
}

/*-------------------------------------------------------------------------*/

VFS::CVfsCallResult
CDrnBridgeBlockSource::FetchRange( UInt64 offset, UInt32 length, void* buffer, UInt32& bytesRead )
{GUCEF_TRACE;

    bytesRead = 0;
    if ( 0 == length )
        return VFS::CVfsCallResult();
    if ( GUCEF_NULL == buffer )
        return VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "no buffer to read into" );
    if ( offset >= m_access->GetSize() )
        return VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "the range starts past the end of the file" );

    if ( GUCEF_NULL != m_fetchCalls )
        m_fetchCalls->FetchAdd( 1 );

    // a range that is asked for again (what an earlier run of it gave was found wrong by the layer and thrown away) is asked of the share again: the chunks
    // that the handle remembers of it are the ones that were wrong
    if ( offset < m_lastEnd && offset + length > m_lastBegin )
        m_access->ReleaseHandle();
    m_lastBegin = offset;
    m_lastEnd = offset + length;

    m_access->Setpos( offset );
    const UInt32 got = m_access->Read( buffer, 1, length );
    bytesRead = got;
    if ( GUCEF_NULL != m_bytesFetched && got > 0 )
        m_bytesFetched->FetchAdd( got );
    if ( got >= length )
        return VFS::CVfsCallResult();

    // the file access says why it came short; a file that changed on the serving side stays failed for good
    const CDrnBridgeResult failure = m_access->GetLastResult();

    // what the handle holds of the file (its chunks) is in doubt after a failure: the next run asks the share again, through another handle
    if ( !failure.IsChanged() )
        m_access->ReleaseHandle();
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DrnBridgeBlockSource: The read of \"" << m_access->GetPath() << "\" at " << offset << " for " << length << " bytes gave " << got << ": " << failure.ToString() );
    if ( failure.IsOk() )
        return VFS::CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "the read of \"" + m_access->GetPath() + "\" gave less than was asked for" );
    return failure.ToCallResult();
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
