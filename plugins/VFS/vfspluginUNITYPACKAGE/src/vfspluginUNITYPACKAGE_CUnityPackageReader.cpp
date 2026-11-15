/*
 *  vfspluginUNITYPACKAGE: GUCEF VFS plugin presenting a Unity package as the project tree it contains
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

#include "archive.h"
#include "archive_entry.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "vfspluginUNITYPACKAGE_CUnityPackageReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace UNITYPACKAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 SourceBufferSize = 64 * 1024;

/*-------------------------------------------------------------------------*/

static la_ssize_t
ReadSourceCallback( struct archive* a, void* clientData, const void** buffer )
{GUCEF_TRACE;

    CUnityPackageReader* reader = static_cast< CUnityPackageReader* >( clientData );
    return (la_ssize_t) reader->ReadSource( buffer );
}

/*-------------------------------------------------------------------------*/

/**
 *  Makes a private copy of the given access object, positioned at the start
 */
static CORE::IOAccessPtr
CloneAccess( const CORE::IOAccessPtr& source )
{GUCEF_TRACE;

    if ( source.IsNULL() )
        return CORE::IOAccessPtr();

    MT::CICloneable* clone = source->Clone();
    if ( GUCEF_NULL == clone )
        return CORE::IOAccessPtr();

    CORE::CIOAccess* access = dynamic_cast< CORE::CIOAccess* >( clone );
    if ( GUCEF_NULL == access )
    {
        GUCEF_DELETE clone;
        return CORE::IOAccessPtr();
    }

    CORE::IOAccessPtr result( access );
    if ( !result->Opened() )
        result->Open();
    if ( !result->IsValid() || 0 != result->Seek( 0, SEEK_SET ) )
        return CORE::IOAccessPtr();
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnityPackageReader::CUnityPackageReader( void )
    : m_archive( GUCEF_NULL )
    , m_access()
    , m_sourceBuffer()
    , m_entryCount( 0 )
    , m_failed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnityPackageReader::~CUnityPackageReader()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageReader::Open( const CORE::CString& path             ,
                           const CORE::IOAccessPtr& sourceAccess )
{GUCEF_TRACE;

    Close();

    m_failed = false;
    m_entryCount = 0;

    m_archive = archive_read_new();
    if ( GUCEF_NULL == m_archive )
        return false;

    // A Unity package is a gzip compressed tar. We do not accept anything else, no format sniffing
    // of other archive formats is wanted here
    archive_read_support_filter_gzip( m_archive );
    archive_read_support_format_tar( m_archive );

    int result = ARCHIVE_FATAL;
    if ( !sourceAccess.IsNULL() )
    {
        m_access = CloneAccess( sourceAccess );
        if ( m_access.IsNULL() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UnityPackageReader: Unable to obtain a separate access object to the package data" );
            Close();
            m_failed = true;
            return false;
        }
        m_sourceBuffer.SetDataSize( SourceBufferSize );
        result = archive_read_open2( m_archive, this, GUCEF_NULL, ReadSourceCallback, GUCEF_NULL, GUCEF_NULL );
    }
    else
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        std::wstring widePath = CORE::ToWString( path );
        result = archive_read_open_filename_w( m_archive, widePath.c_str(), SourceBufferSize );
        #else
        result = archive_read_open_filename( m_archive, path.C_String(), SourceBufferSize );
        #endif
    }

    if ( ARCHIVE_OK != result )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UnityPackageReader: Unable to open package \"" << path << "\": " << GetErrorString() );
        Close();
        m_failed = true;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CUnityPackageReader::Close( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_archive )
    {
        archive_read_free( m_archive );
        m_archive = GUCEF_NULL;
    }
    m_access.Unlink();
    m_sourceBuffer.Clear( true );
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageReader::IsOpen( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_archive;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnityPackageReader::ReadSource( const void** data )
{GUCEF_TRACE;

    if ( m_access.IsNULL() )
        return 0;

    *data = m_sourceBuffer.GetBufferPtr();
    return m_access->Read( m_sourceBuffer.GetBufferPtr(), 1, SourceBufferSize );
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageReader::NextEntry( struct archive_entry** entry )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_archive )
        return false;

    int result = archive_read_next_header( m_archive, entry );
    if ( ARCHIVE_EOF == result )
        return false;
    if ( ARCHIVE_OK != result && ARCHIVE_WARN != result )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UnityPackageReader: Error reading package: " << GetErrorString() );
        m_failed = true;
        return false;
    }

    ++m_entryCount;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageReader::SeekToEntry( UInt32 entryIndex, struct archive_entry** entry )
{GUCEF_TRACE;

    // Note that the entries are counted including the ones the caller did not index,
    // the index of an entry is its position in the tar
    while ( m_entryCount <= entryIndex )
    {
        if ( m_entryCount > 0 )
            SkipEntryData();
        if ( !NextEntry( entry ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnityPackageReader::GetEntryCount( void ) const
{GUCEF_TRACE;

    return m_entryCount;
}

/*-------------------------------------------------------------------------*/

void
CUnityPackageReader::SkipEntryData( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_archive )
        archive_read_data_skip( m_archive );
}

/*-------------------------------------------------------------------------*/

UInt64
CUnityPackageReader::ReadEntryData( void* dest, UInt64 bytes )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_archive )
        return 0;

    char* out = static_cast< char* >( dest );
    UInt64 total = 0;
    while ( total < bytes )
    {
        la_ssize_t bytesRead = archive_read_data( m_archive, out + total, (size_t) ( bytes - total ) );
        if ( bytesRead <= 0 )
            break;
        total += (UInt64) bytesRead;
    }
    return total;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageReader::ReadWholeEntry( const CORE::CString& path             ,
                                     const CORE::IOAccessPtr& sourceAccess ,
                                     UInt32 entryIndex                     ,
                                     UInt32 entrySize                      ,
                                     CORE::CDynamicBuffer& buffer          )
{GUCEF_TRACE;

    buffer.Clear();
    if ( !Open( path, sourceAccess ) )
        return false;

    struct archive_entry* entry = GUCEF_NULL;
    if ( !SeekToEntry( entryIndex, &entry ) )
        return false;

    buffer.SetDataSize( entrySize );
    UInt64 bytesRead = ReadEntryData( buffer.GetBufferPtr(), entrySize );
    if ( bytesRead != entrySize )
    {
        buffer.SetDataSize( (UInt32) bytesRead );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnityPackageReader::HasFailed( void ) const
{GUCEF_TRACE;

    return m_failed;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnityPackageReader::GetErrorString( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_archive )
    {
        const char* error = archive_error_string( m_archive );
        if ( GUCEF_NULL != error )
            return CORE::CString( error );
    }
    return CORE::CString();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITYPACKAGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
