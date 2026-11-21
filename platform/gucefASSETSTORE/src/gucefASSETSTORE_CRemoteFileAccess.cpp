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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefASSETSTORE_CRemoteFileAccess.h"

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

CRemoteFileAccess::CRemoteFileAccess( const CORE::CString& fileSystemId, const CORE::CString& servicePath )
    : CORE::CIOAccess()
    , m_fileSystemId( fileSystemId )
    , m_path( servicePath )
    , m_file()
    , m_position( 0 )
    , m_opened( false )
    , m_failureLogged( false )
    , m_openMessage()
    , m_lock()
{GUCEF_TRACE;

    CRemoteAssetFileSystem* system = CRemoteAssetFileSystem::FindById( m_fileSystemId );
    if ( GUCEF_NULL == system )
    {
        m_openMessage = "The files of the store service are not mounted any more";
        return;
    }
    m_opened = system->OpenFile( m_path, m_file, m_openMessage );
}

/*-------------------------------------------------------------------------*/

CRemoteFileAccess::~CRemoteFileAccess()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

void
CRemoteFileAccess::Open( void )
{GUCEF_TRACE;

    // a file that was closed is opened again at the start
    MT::CScopeMutex lock( m_lock );
    if ( m_opened )
        return;
    CRemoteAssetFileSystem* system = CRemoteAssetFileSystem::FindById( m_fileSystemId );
    if ( GUCEF_NULL != system )
        m_opened = system->OpenFile( m_path, m_file, m_openMessage );
    m_position = 0;
}

/*-------------------------------------------------------------------------*/

void
CRemoteFileAccess::Close( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_opened )
        return;
    CRemoteAssetFileSystem* system = CRemoteAssetFileSystem::FindById( m_fileSystemId );
    if ( GUCEF_NULL != system )
        system->CloseFile( m_file );
    m_opened = false;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteFileAccess::Opened( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_opened;
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteFileAccess::Read( void *dest, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_opened || 0 == esize || 0 == elements )
        return 0;
    CRemoteAssetFileSystem* system = CRemoteAssetFileSystem::FindById( m_fileSystemId );
    if ( GUCEF_NULL == system )
    {
        m_file.status = ASSETSTORE_STATUS_UNAVAILABLE;
        m_file.message = "The files of the store service are not mounted any more";
        return 0;
    }
    UInt64 wanted = (UInt64) esize * elements;
    if ( wanted > GUCEF_UINT32MAX )
        wanted = ( GUCEF_UINT32MAX / esize ) * esize;
    UInt32 total = 0;
    UInt8* out = static_cast< UInt8* >( dest );
    // the file system reads at most a block per piece and a run per call; the loop gives the whole request
    const UInt32 got = system->ReadFile( m_file, m_position, out, (UInt32) wanted );
    total = got;
    m_position += got;
    if ( ASSETSTORE_STATUS_OK != m_file.status && !m_failureLogged )
    {
        m_failureLogged = true;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CRemoteFileAccess: " << m_file.message );
    }
    else
    if ( ASSETSTORE_STATUS_OK == m_file.status )
        m_failureLogged = false;
    return total / esize;
}

/*-------------------------------------------------------------------------*/

UInt64
CRemoteFileAccess::Tell( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_position;
}

/*-------------------------------------------------------------------------*/

UInt64
CRemoteFileAccess::GetSize( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_file.size;
}

/*-------------------------------------------------------------------------*/

Int32
CRemoteFileAccess::Seek( Int64 offset, Int32 origin )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    Int64 target = 0;
    switch ( origin )
    {
        case SEEK_SET : target = offset; break;
        case SEEK_CUR : target = (Int64) m_position + offset; break;
        case SEEK_END : target = (Int64) m_file.size + offset; break;
        default : return -1;
    }
    if ( target < 0 )
        return -1;
    m_position = (UInt64) target;
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteFileAccess::Setpos( UInt64 position )
{GUCEF_TRACE;

    // as fsetpos(): 0 is success
    MT::CScopeMutex lock( m_lock );
    m_position = position;
    return 0;
}

/*-------------------------------------------------------------------------*/

char
CRemoteFileAccess::GetChar( void )
{GUCEF_TRACE;

    char value = 0;
    return 1 == Read( &value, 1, 1 ) ? value : 0;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteFileAccess::Eof( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_position >= m_file.size;
}

/*-------------------------------------------------------------------------*/

bool CRemoteFileAccess::IsReadable( void ) const { return true; }
bool CRemoteFileAccess::IsWriteable( void ) const { return false; }

/*-------------------------------------------------------------------------*/

bool
CRemoteFileAccess::IsValid( void )
{
    MT::CScopeMutex lock( m_lock );
    return m_opened;
}

/*-------------------------------------------------------------------------*/

void
CRemoteFileAccess::Flush( void )
{
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRemoteFileAccess::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRemoteFileAccess( m_fileSystemId, m_path );
}

/*-------------------------------------------------------------------------*/

void
CRemoteFileAccess::Cancel( void )
{GUCEF_TRACE;

    // not under the lock of a read that waits: the flag is looked at between two pieces of a read
    m_file.cancelled = true;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteFileAccess::GetLastStatus( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_opened ? m_file.status : ASSETSTORE_STATUS_NOT_FOUND;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteFileAccess::GetLastMessage( void ) const
{
    return m_opened ? m_file.message : m_openMessage;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
