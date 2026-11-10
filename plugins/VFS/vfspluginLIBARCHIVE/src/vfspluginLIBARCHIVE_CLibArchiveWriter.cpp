/*
 *  vfspluginLIBARCHIVE: Generic GUCEF VFS plugin for archives handled by libarchive
 *  Copyright (C) 2026.  Dinand Vanvelzen
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

#include <vector>

#include "archive.h"
#include "archive_entry.h"

#include "vfspluginLIBARCHIVE_CUtf8LocaleScope.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#include "vfspluginLIBARCHIVE_CLibArchiveWriter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace LIBARCHIVE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 CopyBufferSize = 1024 * 1024;
static const int UnixFileMode = 0644;
static const int UnixDirMode = 0755;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static la_ssize_t
WriteToOutput( struct archive* a, void* clientData, const void* buffer, size_t length )
{
    CORE::CIOAccess* output = static_cast< CORE::CIOAccess* >( clientData );
    UInt32 written = output->Write( buffer, 1, (UInt32) length );
    if ( written != (UInt32) length )
    {
        archive_set_error( a, EIO, "Failed to write to the output" );
        return -1;
    }
    return (la_ssize_t) written;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
NormalizeEntryName( const CORE::CString& nameInArchive )
{
    CORE::CString name = nameInArchive.ReplaceChar( '\\', '/' );
    while ( name.Length() > 0 && '/' == name[ 0 ] )
        name = name.CutChars( 1, true );
    return name;
}

/*-------------------------------------------------------------------------*/

static void
SetModificationTime( struct archive_entry* entry, const CORE::CDateTime& modificationTime )
{
    UInt64 ticks = modificationTime.ToUnixEpochBasedTicksInMillisecs();
    archive_entry_set_mtime( entry, (time_t) ( ticks / 1000 ), (long) ( ( ticks % 1000 ) * 1000000 ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLibArchiveWriter::CLibArchiveWriter( EFormat format )
    : VFS::CIArchiveWriter()
    , m_format( format )
    , m_archive( GUCEF_NULL )
    , m_output( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLibArchiveWriter::~CLibArchiveWriter()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLibArchiveWriter::GetFormatTypeName( EFormat format )
{GUCEF_TRACE;

    switch ( format )
    {
        case FORMAT_TAR: return "tar";
        case FORMAT_TAR_GZIP: return "tgz";
        case FORMAT_ZIP: return "zip";
        case FORMAT_7ZIP: return "7z";
        case FORMAT_CPIO: return "cpio";
        default: return CORE::CString::Empty;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLibArchiveWriter::GetArchiveType( void ) const
{GUCEF_TRACE;

    return GetFormatTypeName( m_format );
}

/*-------------------------------------------------------------------------*/

void
CLibArchiveWriter::LogError( const CORE::CString& operation ) const
{GUCEF_TRACE;

    const char* error = GUCEF_NULL != m_archive ? archive_error_string( m_archive ) : GUCEF_NULL;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LibArchiveWriter(" + GetArchiveType() + "): " + operation + ": " + CORE::CString( GUCEF_NULL != error ? error : "unknown error" ) );
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveWriter::Open( CORE::CIOAccess& output )
{GUCEF_TRACE;

    CUtf8LocaleScope utf8Locale;

    if ( GUCEF_NULL != m_archive )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LibArchiveWriter: An archive is already open, close it first" );
        return false;
    }

    m_archive = archive_write_new();
    if ( GUCEF_NULL == m_archive )
        return false;

    int result = ARCHIVE_OK;
    switch ( m_format )
    {
        case FORMAT_TAR:
            result = archive_write_set_format_pax_restricted( m_archive );
            break;
        case FORMAT_TAR_GZIP:
            result = archive_write_set_format_pax_restricted( m_archive );
            if ( ARCHIVE_OK == result )
                result = archive_write_add_filter_gzip( m_archive );
            break;
        case FORMAT_ZIP:
            result = archive_write_set_format_zip( m_archive );
            break;
        case FORMAT_7ZIP:
            result = archive_write_set_format_7zip( m_archive );
            if ( ARCHIVE_OK == result )
                result = archive_write_set_format_option( m_archive, "7zip", "compression", "deflate" );
            break;
        case FORMAT_CPIO:
            result = archive_write_set_format_cpio_newc( m_archive );
            break;
        default:
            result = ARCHIVE_FATAL;
    }

    // Note: names are passed as UTF-8 via archive_entry_set_pathname_utf8(), the formats store them as UTF-8 by default.
    // The "hdrcharset" option is not used, it relies on the locale and fails in the "C" locale

    // libarchive pads the output to the tar block size by default, which only makes sense for uncompressed tar
    if ( ARCHIVE_OK == result && FORMAT_TAR != m_format )
        result = archive_write_set_bytes_in_last_block( m_archive, 1 );

    if ( ARCHIVE_OK == result )
        result = archive_write_open2( m_archive, &output, GUCEF_NULL, WriteToOutput, GUCEF_NULL, GUCEF_NULL );

    if ( ARCHIVE_OK != result )
    {
        LogError( "Unable to start a new archive" );
        archive_write_free( m_archive );
        m_archive = GUCEF_NULL;
        return false;
    }
    m_output = &output;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveWriter::AddFileOfKnownSize( const CORE::CString& name                        ,
                                       CORE::CIOAccess& source                          ,
                                       UInt64 size                                      ,
                                       const VFS::CArchiveWriterEntrySettings& settings )
{GUCEF_TRACE;

    CUtf8LocaleScope utf8Locale;

    if ( FORMAT_ZIP == m_format )
    {
        // Note: libarchive only allows the compression level to be set before the archive is opened,
        // as such the level of the settings is not used and the default level applies
        if ( VFS::CArchiveWriterEntrySettings::COMPRESSION_DEFLATE == settings.compression )
            archive_write_zip_set_compression_deflate( m_archive );
        else
            archive_write_zip_set_compression_store( m_archive );
    }

    struct archive_entry* entry = archive_entry_new();
    archive_entry_set_pathname_utf8( entry, name.C_String() );
    archive_entry_set_filetype( entry, AE_IFREG );
    archive_entry_set_perm( entry, UnixFileMode );
    archive_entry_set_size( entry, (la_int64_t) size );
    SetModificationTime( entry, settings.modificationTime );

    bool success = ARCHIVE_OK == archive_write_header( m_archive, entry );
    archive_entry_free( entry );
    if ( !success )
    {
        LogError( "Unable to add entry \"" + name + "\"" );
        return false;
    }

    std::vector< char > buffer( CopyBufferSize );
    UInt64 remaining = size;
    while ( remaining > 0 && success )
    {
        UInt32 chunk = remaining > CopyBufferSize ? CopyBufferSize : (UInt32) remaining;
        UInt32 bytesRead = source.Read( &buffer[ 0 ], 1, chunk );
        if ( 0 == bytesRead )
            break;
        success = (la_ssize_t) bytesRead == archive_write_data( m_archive, &buffer[ 0 ], bytesRead );
        remaining -= bytesRead;
    }
    if ( success && 0 != remaining )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LibArchiveWriter: The source of entry \"" + name + "\" ended before its stated size" );
        success = false;
    }

    if ( ARCHIVE_OK != archive_write_finish_entry( m_archive ) )
        success = false;
    if ( !success )
        LogError( "Failed to write the content of entry \"" + name + "\"" );
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveWriter::AddFile( const CORE::CString& nameInArchive                ,
                            CORE::CIOAccess& source                           ,
                            const VFS::CArchiveWriterEntrySettings& settings )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_archive )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LibArchiveWriter: Cannot add \"" + nameInArchive + "\", no archive is open" );
        return false;
    }

    CORE::CString name = NormalizeEntryName( nameInArchive );
    if ( name.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LibArchiveWriter: Cannot add an entry without a name" );
        return false;
    }

    UInt64 sourceSize = source.GetSize();
    UInt64 sourcePos = source.Tell();
    if ( sourceSize > 0 && sourceSize >= sourcePos )
        return AddFileOfKnownSize( name, source, sourceSize - sourcePos, settings );

    // The size is not known, spool the content to a temporary file to learn it
    CORE::CString spoolPath = CORE::CombinePath( CORE::TempDir(), "vfspluginLIBARCHIVE_" + CORE::ToString( CORE::CRandom::GetRandomUInt32() ) + ".tmp" );
    bool success = false;
    {
        CORE::CFileAccess spool( spoolPath, "w+b" );
        if ( !spool.IsValid() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LibArchiveWriter: Unable to create the temporary file \"" + spoolPath + "\" for entry \"" + name + "\"" );
            return false;
        }
        UInt64 spooled = spool.Write( source );
        spool.Flush();
        if ( 0 == spool.Seek( 0, SEEK_SET ) )
            success = AddFileOfKnownSize( name, spool, spooled, settings );
    }
    CORE::DeleteFile( spoolPath );
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveWriter::AddDirectory( const CORE::CString& nameInArchive      ,
                                 const CORE::CDateTime& modificationTime )
{GUCEF_TRACE;

    CUtf8LocaleScope utf8Locale;

    if ( GUCEF_NULL == m_archive )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LibArchiveWriter: Cannot add \"" + nameInArchive + "\", no archive is open" );
        return false;
    }

    CORE::CString name = NormalizeEntryName( nameInArchive );
    if ( name.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LibArchiveWriter: Cannot add a directory without a name" );
        return false;
    }
    if ( '/' != name[ name.Length()-1 ] )
        name += '/';

    struct archive_entry* entry = archive_entry_new();
    archive_entry_set_pathname_utf8( entry, name.C_String() );
    archive_entry_set_filetype( entry, AE_IFDIR );
    archive_entry_set_perm( entry, UnixDirMode );
    archive_entry_set_size( entry, 0 );
    SetModificationTime( entry, modificationTime );

    bool success = ARCHIVE_OK == archive_write_header( m_archive, entry );
    archive_entry_free( entry );
    if ( success )
        success = ARCHIVE_OK == archive_write_finish_entry( m_archive );
    if ( !success )
        LogError( "Unable to add directory \"" + name + "\"" );
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveWriter::Close( void )
{GUCEF_TRACE;

    CUtf8LocaleScope utf8Locale;

    if ( GUCEF_NULL == m_archive )
        return true;

    bool success = ARCHIVE_OK == archive_write_close( m_archive );
    if ( !success )
        LogError( "Failed to finish the archive" );
    archive_write_free( m_archive );
    m_archive = GUCEF_NULL;
    m_output = GUCEF_NULL;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CLibArchiveWriter::IsOpen( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_archive;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LIBARCHIVE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
