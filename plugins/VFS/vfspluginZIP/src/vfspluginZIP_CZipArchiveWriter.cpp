/*
 *  vfspluginZIP: Generic GUCEF VFS plugin for ZIP archives
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#include <string.h>
#include <vector>

#include "zlib.h"
#include "zip.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFSPLUGIN_ZIP_CZIPARCHIVE_H
#include "vfspluginZIP_CZIPArchive.h"
#define GUCEF_VFSPLUGIN_ZIP_CZIPARCHIVE_H
#endif /* GUCEF_VFSPLUGIN_ZIP_CZIPARCHIVE_H ? */

#include "vfspluginZIP_CZipArchiveWriter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace ZIP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const uLong ZipUtf8NameFlag = 0x0800;            /* general purpose bit 11: name and comment are UTF-8 */
static const int ZipVersionMadeByUnix = ( 3 << 8 ) | 63; /* host system unix, specification version 6.3 */
static const uLong UnixFileMode = 0100644;
static const uLong UnixDirMode = 040755;
static const uLong DosDirAttribute = 0x10;
static const UInt64 Zip64Threshold = 0xF0000000;        /* below 4 GiB with room for data that does not compress */
static const UInt32 CopyBufferSize = 1024 * 1024;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  minizip I/O on top of a CIOAccess. The stream handle is the CIOAccess itself
 */

static voidpf ZCALLBACK
IoOpen( voidpf opaque, const void* filename, int mode )
{
    return opaque;
}

/*-------------------------------------------------------------------------*/

static uLong ZCALLBACK
IoRead( voidpf opaque, voidpf stream, void* buf, uLong size )
{
    return (uLong) static_cast< CORE::CIOAccess* >( stream )->Read( buf, 1, (UInt32) size );
}

/*-------------------------------------------------------------------------*/

static uLong ZCALLBACK
IoWrite( voidpf opaque, voidpf stream, const void* buf, uLong size )
{
    return (uLong) static_cast< CORE::CIOAccess* >( stream )->Write( buf, 1, (UInt32) size );
}

/*-------------------------------------------------------------------------*/

static ZPOS64_T ZCALLBACK
IoTell( voidpf opaque, voidpf stream )
{
    return (ZPOS64_T) static_cast< CORE::CIOAccess* >( stream )->Tell();
}

/*-------------------------------------------------------------------------*/

static long ZCALLBACK
IoSeek( voidpf opaque, voidpf stream, ZPOS64_T offset, int origin )
{
    Int32 seekOrigin = SEEK_SET;
    switch ( origin )
    {
        case ZLIB_FILEFUNC_SEEK_CUR: seekOrigin = SEEK_CUR; break;
        case ZLIB_FILEFUNC_SEEK_END: seekOrigin = SEEK_END; break;
        case ZLIB_FILEFUNC_SEEK_SET: seekOrigin = SEEK_SET; break;
        default: return -1;
    }
    return 0 == static_cast< CORE::CIOAccess* >( stream )->Seek( (Int64) offset, seekOrigin ) ? 0 : -1;
}

/*-------------------------------------------------------------------------*/

static int ZCALLBACK
IoClose( voidpf opaque, voidpf stream )
{
    // The output belongs to the caller, it is not closed here
    return 0;
}

/*-------------------------------------------------------------------------*/

static int ZCALLBACK
IoError( voidpf opaque, voidpf stream )
{
    return 0;
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
PutUInt16( std::vector< unsigned char >& out, UInt16 value )
{
    out.push_back( (unsigned char) ( value & 0xFF ) );
    out.push_back( (unsigned char) ( ( value >> 8 ) & 0xFF ) );
}

/*-------------------------------------------------------------------------*/

static void
PutUInt32( std::vector< unsigned char >& out, UInt32 value )
{
    PutUInt16( out, (UInt16) ( value & 0xFFFF ) );
    PutUInt16( out, (UInt16) ( ( value >> 16 ) & 0xFFFF ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CZipArchiveWriter::CZipArchiveWriter( void )
    : VFS::CIArchiveWriter()
    , m_zipFile( GUCEF_NULL )
    , m_output( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CZipArchiveWriter::~CZipArchiveWriter()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

bool
CZipArchiveWriter::Open( CORE::CIOAccess& output )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_zipFile )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ZipArchiveWriter: An archive is already open, close it first" );
        return false;
    }

    zlib_filefunc64_def ioFunctions;
    memset( &ioFunctions, 0, sizeof( ioFunctions ) );
    ioFunctions.zopen64_file = IoOpen;
    ioFunctions.zread_file = IoRead;
    ioFunctions.zwrite_file = IoWrite;
    ioFunctions.ztell64_file = IoTell;
    ioFunctions.zseek64_file = IoSeek;
    ioFunctions.zclose_file = IoClose;
    ioFunctions.zerror_file = IoError;
    ioFunctions.opaque = &output;

    // minizip asks for a path but our I/O functions ignore it
    m_zipFile = zipOpen2_64( "output", APPEND_STATUS_CREATE, GUCEF_NULL, &ioFunctions );
    if ( GUCEF_NULL == m_zipFile )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ZipArchiveWriter: Unable to start a new archive on the given output" );
        return false;
    }
    m_output = &output;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CZipArchiveWriter::OpenEntry( const CORE::CString& nameInArchive      ,
                              const CORE::CDateTime& modificationTime ,
                              bool isDirectory                        ,
                              int method                              ,
                              int level                               ,
                              bool needsZip64                         )
{GUCEF_TRACE;

    zip_fileinfo info;
    memset( &info, 0, sizeof( info ) );

    // DOS dates start in 1980 and use the local time
    Int16 year = modificationTime.GetYear();
    if ( year < 1980 )
    {
        info.tmz_date.tm_year = 1980;
        info.tmz_date.tm_mon = 0;
        info.tmz_date.tm_mday = 1;
    }
    else
    {
        info.tmz_date.tm_year = year;
        info.tmz_date.tm_mon = modificationTime.GetMonth() - 1;
        info.tmz_date.tm_mday = modificationTime.GetDay();
        info.tmz_date.tm_hour = modificationTime.GetHours();
        info.tmz_date.tm_min = modificationTime.GetMinutes();
        info.tmz_date.tm_sec = modificationTime.GetSeconds();
    }
    info.external_fa = isDirectory ? ( ( UnixDirMode << 16 ) | DosDirAttribute ) : ( UnixFileMode << 16 );

    // Extended timestamp field with the modification time in UTC, the same in the local and the central header
    std::vector< unsigned char > extraField;
    UInt64 unixTimeInSecs = modificationTime.ToUTC().ToUnixEpochBasedTicksInMillisecs() / 1000;
    PutUInt16( extraField, 0x5455 );
    PutUInt16( extraField, 5 );
    extraField.push_back( 1 );     /* flags: modification time present */
    PutUInt32( extraField, (UInt32) unixTimeInSecs );

    int result = zipOpenNewFileInZip4_64( m_zipFile                         ,
                                          nameInArchive.C_String()          ,
                                          &info                             ,
                                          &extraField[ 0 ]                  ,
                                          (uInt) extraField.size()          ,
                                          &extraField[ 0 ]                  ,
                                          (uInt) extraField.size()          ,
                                          GUCEF_NULL                        ,
                                          method                            ,
                                          level                             ,
                                          0                                 ,
                                          -MAX_WBITS                        ,
                                          DEF_MEM_LEVEL                     ,
                                          Z_DEFAULT_STRATEGY                ,
                                          GUCEF_NULL                        ,
                                          0                                 ,
                                          ZipVersionMadeByUnix              ,
                                          ZipUtf8NameFlag                   ,
                                          needsZip64 ? 1 : 0                );
    if ( ZIP_OK != result )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ZipArchiveWriter: Unable to add entry \"" + nameInArchive + "\", error " + CORE::ToString( result ) );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CZipArchiveWriter::AddFile( const CORE::CString& nameInArchive                ,
                            CORE::CIOAccess& source                           ,
                            const VFS::CArchiveWriterEntrySettings& settings )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_zipFile )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ZipArchiveWriter: Cannot add \"" + nameInArchive + "\", no archive is open" );
        return false;
    }

    CORE::CString name = NormalizeEntryName( nameInArchive );
    if ( name.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ZipArchiveWriter: Cannot add an entry without a name" );
        return false;
    }

    // ZIP64 extra fields have to be in the local header up front, so use them unless the size is known to be small
    UInt64 sourceSize = source.GetSize();
    UInt64 sourcePos = source.Tell();
    bool sizeKnown = sourceSize > 0 && sourceSize >= sourcePos;
    bool needsZip64 = !sizeKnown || ( sourceSize - sourcePos ) >= Zip64Threshold;

    bool deflate = VFS::CArchiveWriterEntrySettings::COMPRESSION_DEFLATE == settings.compression;
    int method = deflate ? Z_DEFLATED : 0;
    int level = deflate ? ( settings.compressionLevel < 0 ? Z_DEFAULT_COMPRESSION : settings.compressionLevel ) : 0;

    if ( !OpenEntry( name, settings.modificationTime, false, method, level, needsZip64 ) )
        return false;

    bool success = true;
    std::vector< char > buffer( CopyBufferSize );
    while ( success )
    {
        UInt32 bytesRead = source.Read( &buffer[ 0 ], 1, CopyBufferSize );
        if ( 0 == bytesRead )
            break;
        success = ZIP_OK == zipWriteInFileInZip( m_zipFile, &buffer[ 0 ], (unsigned) bytesRead );
    }

    int closeResult = zipCloseFileInZip( m_zipFile );
    if ( !success || ZIP_OK != closeResult )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ZipArchiveWriter: Failed to write the content of entry \"" + name + "\"" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CZipArchiveWriter::AddDirectory( const CORE::CString& nameInArchive      ,
                                 const CORE::CDateTime& modificationTime )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_zipFile )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ZipArchiveWriter: Cannot add \"" + nameInArchive + "\", no archive is open" );
        return false;
    }

    CORE::CString name = NormalizeEntryName( nameInArchive );
    if ( name.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ZipArchiveWriter: Cannot add a directory without a name" );
        return false;
    }
    if ( '/' != name[ name.Length()-1 ] )
        name += '/';

    if ( !OpenEntry( name, modificationTime, true, 0, 0, false ) )
        return false;
    return ZIP_OK == zipCloseFileInZip( m_zipFile );
}

/*-------------------------------------------------------------------------*/

bool
CZipArchiveWriter::Close( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_zipFile )
        return true;

    int result = zipClose( m_zipFile, GUCEF_NULL );
    m_zipFile = GUCEF_NULL;
    m_output = GUCEF_NULL;
    if ( ZIP_OK != result )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ZipArchiveWriter: Failed to finish the archive, error " + CORE::ToString( result ) );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CZipArchiveWriter::IsOpen( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_zipFile;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CZipArchiveWriter::GetArchiveType( void ) const
{GUCEF_TRACE;

    return CZIPArchive::ZipArchiveTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ZIP */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
