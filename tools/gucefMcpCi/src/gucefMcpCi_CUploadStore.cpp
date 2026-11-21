/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CUploadStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
ToHex( const UInt8* bytes, UInt32 count )
{
    static const char* digits = "0123456789abcdef";
    CORE::CString text;
    for ( UInt32 i=0; i<count; ++i )
    {
        text += digits[ bytes[ i ] >> 4 ];
        text += digits[ bytes[ i ] & 0x0F ];
    }
    return text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUploadStore::CUpload::CUpload( void )
    : id()
    , ciId()
    , machineId()
    , size( 0 )
    , sha256Hex()
    , received( 0 )
    , path()
    , lastActivityInMs( 0 )
    , busy( false )
    , hasher()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUploadStore::CUploadStore( void )
    : m_directory()
    , m_maxUploads( 4 )
    , m_maxUploadBytes( (UInt64) 500 * 1024 * 1024 )
    , m_chunkBytes( MaxChunkBytes )
    , m_idleTimeoutSecs( 600 )
    , m_uploads()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUploadStore::~CUploadStore()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

UInt64
CUploadStore::NowInMs( void )
{GUCEF_TRACE;

    return CCiUtils::MonotonicMs();
}

/*-------------------------------------------------------------------------*/

void
CUploadStore::SetDirectory( const CORE::CString& directory )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_directory = directory;
}

/*-------------------------------------------------------------------------*/

void
CUploadStore::SetLimits( UInt32 maxUploads, UInt64 maxUploadBytes, UInt32 chunkBytes, UInt32 idleTimeoutSecs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_maxUploads = maxUploads > 0 ? maxUploads : 1;
    m_maxUploadBytes = maxUploadBytes;
    m_chunkBytes = chunkBytes > 0 && chunkBytes <= MaxChunkBytes ? chunkBytes : MaxChunkBytes;
    m_idleTimeoutSecs = idleTimeoutSecs > 0 ? idleTimeoutSecs : 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CUploadStore::GetChunkBytes( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_chunkBytes;
}

/*-------------------------------------------------------------------------*/

UInt64
CUploadStore::GetMaxUploadBytes( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_maxUploadBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CUploadStore::GetCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_uploads.size();
}

/*-------------------------------------------------------------------------*/

bool
CUploadStore::Start( CORE::CString& problem )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_directory.IsNULLOrEmpty() )
    {
        problem = "no directory for the uploads";
        return false;
    }
    // what an earlier run left is of no use: its uploads are not known any more
    if ( CORE::DirExists( m_directory ) )
        CORE::DeleteDir( m_directory, true );
    if ( !CORE::CreateDirs( m_directory ) || !CORE::DirExists( m_directory ) )
    {
        problem = "the directory for the uploads cannot be made: " + m_directory;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CUploadStore::Stop( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( TUploadMap::iterator i = m_uploads.begin(); i != m_uploads.end(); ++i )
        Delete( (*i).second );
    m_uploads.clear();
}

/*-------------------------------------------------------------------------*/

void
CUploadStore::Delete( CUpload* upload )
{GUCEF_TRACE;

    if ( GUCEF_NULL == upload )
        return;
    if ( CORE::FileExists( upload->path ) )
        CORE::DeleteFile( upload->path );
    GUCEF_DELETE upload;
}

/*-------------------------------------------------------------------------*/

CUploadStore::CUpload*
CUploadStore::Find( const CORE::CString& uploadId, const CORE::CString& machineId )
{GUCEF_TRACE;

    TUploadMap::iterator i = m_uploads.find( uploadId );
    if ( i == m_uploads.end() || (*i).second->machineId != machineId )
        return GUCEF_NULL;
    return (*i).second;
}

/*-------------------------------------------------------------------------*/

CUploadStore::EResult
CUploadStore::Create( const CORE::CString& ciId        ,
                      const CORE::CString& machineId   ,
                      UInt64 size                      ,
                      const CORE::CString& sha256Hex   ,
                      CORE::CString& uploadId          ,
                      CORE::CString& problem           )
{GUCEF_TRACE;

    if ( 0 == size || 64 != sha256Hex.Length() || !CCiUtils::IsHex( sha256Hex, 64 ) )
    {
        problem = "an upload needs its size (more than 0) and its SHA-256 as 64 hexadecimal digits";
        return UPLOAD_BAD_REQUEST;
    }
    UInt8 random[ 16 ];
    if ( !CORE::CRandom::GetRandomBytes( random, sizeof( random ) ) )
    {
        problem = "no random id could be made";
        return UPLOAD_IO_ERROR;
    }

    CUpload* upload = GUCEF_NEW CUpload();
    upload->id = ToHex( random, sizeof( random ) );
    upload->ciId = ciId;
    upload->machineId = machineId;
    upload->size = size;
    upload->sha256Hex = sha256Hex.Lowercase();
    upload->lastActivityInMs = NowInMs();
    {
        MT::CScopeMutex lock( m_lock );
        if ( size > m_maxUploadBytes )
        {
            GUCEF_DELETE upload;
            problem = "the snapshot is " + CCiUtils::FormatBytes( size ) + ", more than the limit of the gateway of " + CCiUtils::FormatBytes( m_maxUploadBytes ) +
                      " (--max-snapshot-mb). Large files that are not meant for the CI belong in .gitignore.";
            return UPLOAD_TOO_LARGE;
        }
        if ( m_uploads.size() >= m_maxUploads )
        {
            GUCEF_DELETE upload;
            problem = "the gateway takes " + CORE::UInt32ToString( m_maxUploads ) + " snapshots at the same time and has them; try again shortly";
            return UPLOAD_FULL;
        }
        upload->path = CORE::CombinePath( m_directory, upload->id + ".bundle" );
        upload->busy = true;
        m_uploads[ upload->id ] = upload;
    }

    // the file exists from the start, empty: chunks are written into it at their offset
    CORE::CFileAccess file;
    const bool made = file.Open( upload->path, "wb" ) && file.Opened();
    file.Close();

    MT::CScopeMutex lock( m_lock );
    upload->busy = false;
    if ( !made )
    {
        m_uploads.erase( upload->id );
        Delete( upload );
        problem = "the gateway cannot write the upload";
        return UPLOAD_IO_ERROR;
    }
    uploadId = upload->id;
    return UPLOAD_OK;
}

/*-------------------------------------------------------------------------*/

CUploadStore::EResult
CUploadStore::Write( const CORE::CString& uploadId     ,
                     const CORE::CString& machineId    ,
                     UInt64 offset                     ,
                     const CORE::CDynamicBuffer& data  ,
                     UInt64& received                  ,
                     CORE::CString& problem            )
{GUCEF_TRACE;

    CUpload* upload = GUCEF_NULL;
    CORE::CString path;
    {
        MT::CScopeMutex lock( m_lock );
        upload = Find( uploadId, machineId );
        if ( GUCEF_NULL == upload )
        {
            problem = "no upload \"" + uploadId + "\": it ended, was removed after being idle, or the gateway restarted; start the upload again";
            return UPLOAD_NOT_FOUND;
        }
        received = upload->received;
        if ( upload->busy )
        {
            problem = "another call works on the upload";
            return UPLOAD_BUSY;
        }
        if ( offset != upload->received )
        {
            problem = "the chunk starts at " + CORE::UInt64ToString( offset ) + " but the upload has " + CORE::UInt64ToString( upload->received ) + " bytes";
            return UPLOAD_OFFSET_MISMATCH;
        }
        if ( 0 == data.GetDataSize() || data.GetDataSize() > m_chunkBytes || offset + data.GetDataSize() > upload->size )
        {
            problem = "a chunk has 1 to " + CORE::UInt32ToString( m_chunkBytes ) + " bytes and ends within the size of the upload";
            return UPLOAD_TOO_LARGE;
        }
        upload->busy = true;
        upload->lastActivityInMs = NowInMs();
        path = upload->path;
    }

    // written at its offset: a chunk that failed before is overwritten by the next try
    CORE::CFileAccess file;
    bool ok = file.Open( path, "r+b" ) && file.Opened() && 0 == file.Setpos( offset );
    if ( ok )
        ok = data.GetDataSize() == file.Write( data.GetConstBufferPtr(), 1, data.GetDataSize() );
    file.Close();
    if ( ok )
        upload->hasher.Update( data.GetConstBufferPtr(), data.GetDataSize() );

    MT::CScopeMutex lock( m_lock );
    upload->busy = false;
    upload->lastActivityInMs = NowInMs();
    if ( !ok )
    {
        received = upload->received;
        problem = "the gateway could not write the chunk";
        return UPLOAD_IO_ERROR;
    }
    upload->received += data.GetDataSize();
    received = upload->received;
    return UPLOAD_OK;
}

/*-------------------------------------------------------------------------*/

CUploadStore::EResult
CUploadStore::GetState( const CORE::CString& uploadId     ,
                        const CORE::CString& machineId    ,
                        UInt64& received                  ,
                        UInt64& size                      ,
                        CORE::CString& problem            )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CUpload* upload = Find( uploadId, machineId );
    if ( GUCEF_NULL == upload )
    {
        problem = "no upload \"" + uploadId + "\"";
        return UPLOAD_NOT_FOUND;
    }
    received = upload->received;
    size = upload->size;
    upload->lastActivityInMs = NowInMs();
    return UPLOAD_OK;
}

/*-------------------------------------------------------------------------*/

CUploadStore::EResult
CUploadStore::BeginCommit( const CORE::CString& uploadId     ,
                           const CORE::CString& machineId    ,
                           CORE::CString& path               ,
                           CORE::CString& ciId               ,
                           CORE::CString& problem            )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CUpload* upload = Find( uploadId, machineId );
    if ( GUCEF_NULL == upload )
    {
        problem = "no upload \"" + uploadId + "\"";
        return UPLOAD_NOT_FOUND;
    }
    if ( upload->busy )
    {
        problem = "another call works on the upload";
        return UPLOAD_BUSY;
    }
    if ( upload->received != upload->size )
    {
        problem = "the upload is not complete: " + CORE::UInt64ToString( upload->received ) + " of " + CORE::UInt64ToString( upload->size ) + " bytes arrived";
        return UPLOAD_INCOMPLETE;
    }
    if ( CORE::FileSize( upload->path ) != upload->size )
    {
        problem = "the file of the upload does not have the size of the upload";
        m_uploads.erase( upload->id );
        Delete( upload );
        return UPLOAD_IO_ERROR;
    }
    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256 hasher = upload->hasher;
    hasher.Final( digest );
    if ( ToHex( digest, sizeof( digest ) ) != upload->sha256Hex )
    {
        // useless now: removed, the client starts again
        problem = "the snapshot that arrived does not have the SHA-256 it was announced with; it was dropped, send it again";
        m_uploads.erase( upload->id );
        Delete( upload );
        return UPLOAD_HASH_MISMATCH;
    }
    upload->busy = true;
    upload->lastActivityInMs = NowInMs();
    path = upload->path;
    ciId = upload->ciId;
    return UPLOAD_OK;
}

/*-------------------------------------------------------------------------*/

void
CUploadStore::EndCommit( const CORE::CString& uploadId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TUploadMap::iterator i = m_uploads.find( uploadId );
    if ( i == m_uploads.end() )
        return;
    CUpload* upload = (*i).second;
    m_uploads.erase( i );
    Delete( upload );
}

/*-------------------------------------------------------------------------*/

CUploadStore::EResult
CUploadStore::Remove( const CORE::CString& uploadId, const CORE::CString& machineId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CUpload* upload = Find( uploadId, machineId );
    if ( GUCEF_NULL == upload )
        return UPLOAD_NOT_FOUND;
    if ( upload->busy )
        return UPLOAD_BUSY;
    m_uploads.erase( upload->id );
    Delete( upload );
    return UPLOAD_OK;
}

/*-------------------------------------------------------------------------*/

UInt32
CUploadStore::RemoveAbandoned( UInt64 nowInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt32 removed = 0;
    const UInt64 idleMs = (UInt64) m_idleTimeoutSecs * 1000;
    TUploadMap::iterator i = m_uploads.begin();
    while ( i != m_uploads.end() )
    {
        CUpload* upload = (*i).second;
        if ( !upload->busy && nowInMs > upload->lastActivityInMs && nowInMs - upload->lastActivityInMs >= idleMs )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Gateway: The upload " << upload->id << " of " << upload->machineId <<
                              " was idle for " << m_idleTimeoutSecs << " s and is removed (" << upload->received << " of " << upload->size << " bytes)" );
            m_uploads.erase( i++ );
            Delete( upload );
            ++removed;
        }
        else
            ++i;
    }
    return removed;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
