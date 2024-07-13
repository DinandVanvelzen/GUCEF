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

#ifndef GUCEF_CORE_DVMD5UTILS_H
#include "dvmd5utils.h"
#define GUCEF_CORE_DVMD5UTILS_H
#endif /* GUCEF_CORE_DVMD5UTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CSUBFILEACCESS_H
#include "gucefCORE_CSubFileAccess.h"
#define GUCEF_CORE_CSUBFILEACCESS_H
#endif /* GUCEF_CORE_CSUBFILEACCESS_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef VFSPLUGIN_ZIP_CZIPIOACCESSS_H
#include "vfspluginZIP_CZipIOAccess.h"
#define VFSPLUGIN_ZIP_CZIPIOACCESSS_H
#endif /* VFSPLUGIN_ZIP_CZIPIOACCESSS_H ? */

#include "vfspluginZIP_CZIPArchive.h"

// Include the zzip header last because it performs some funky type
// magic which will screw up types in other headers
#ifndef _ZZIP_ZZIP_H
#include "zzip.h"
#define _ZZIP_ZZIP_H
#endif /* _ZZIP_ZZIP_H ? */

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const VFS::CString CZIPArchive::ZipArchiveTypeName = "zip";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/// Utility method to format out zzip errors
VFS::CString
getZzipErrorDescription( zzip_error_t zzipError )
{GUCEF_TRACE;

    VFS::CString errorMsg;
    switch ( zzipError )
    {
    case ZZIP_NO_ERROR:
        break;
    case ZZIP_OUTOFMEM:
        errorMsg = "Out of memory.";
        break;
    case ZZIP_DIR_OPEN:
    case ZZIP_DIR_STAT:
    case ZZIP_DIR_SEEK:
    case ZZIP_DIR_READ:
        errorMsg = "Unable to read zip file.";
        break;
    case ZZIP_UNSUPP_COMPR:
        errorMsg = "Unsupported compression format.";
        break;
    case ZZIP_CORRUPTED:
        errorMsg = "Corrupted archive.";
        break;
    default:
        errorMsg = "Unknown error.";
        break;
    };

    return errorMsg;
}

/*-------------------------------------------------------------------------*/

CZIPArchive::CZIPArchive( void )
    : CArchive()        ,
      m_archiveName()   ,
      m_archivePath()   ,
      m_zipRoot( NULL ) ,
      m_fileList()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CZIPArchive::~CZIPArchive()
{GUCEF_TRACE;

    UnloadArchive();
}

/*-------------------------------------------------------------------------*/

VFS::TBasicVfsResourcePtr
CZIPArchive::GetFile( const VFS::CString& file      ,
                      const char* mode              ,
                      const VFS::UInt32 memLoadSize ,
                      const bool overwrite          )
{GUCEF_TRACE;

    // We only support read only modes
    if ( *mode != 'r' ) 
        return VFS::TBasicVfsResourcePtr();

    CORE::IOAccessPtr fileAccess = LoadFile( file, memLoadSize );
    if ( !fileAccess.IsNULL() )
    {
        // Create path to file
        CORE::CString filePath = m_archivePath;                
        CORE::AppendToPath( filePath, file );

        // Construct & return handle
        return VFS::TVfsResourcePtr( GUCEF_NEW VFS::CVFSHandle( fileAccess, file, filePath ), this );
    }
    return VFS::TBasicVfsResourcePtr();
}

/*-------------------------------------------------------------------------*/

VFS::TBasicVfsResourcePtr 
CZIPArchive::GetFileAs( const VFS::CString& file                ,
                        const CORE::CResourceMetaData& metaData ,
                        const char* mode                        ,
                        const UInt32 memLoadSize                ,
                        const bool overwrite                    )
{GUCEF_TRACE;

    // We dont currently support applying meta-data
    return VFS::TBasicVfsResourcePtr();
}

/*-------------------------------------------------------------------------*/

bool 
CZIPArchive::DeleteFile( const VFS::CString& filePath )
{GUCEF_TRACE;

    // Not implemented / supported at this time
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CZIPArchive::MoveFile( const VFS::CString& oldFilePath ,
                       const VFS::CString& newFilePath ,
                       const bool overwrite            )
{GUCEF_TRACE;

    // Not implemented / supported at this time
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CZIPArchive::StoreAsFile( const CORE::CString& filepath    ,
                          const CORE::CDynamicBuffer& data ,
                          const CORE::UInt64 offset        ,
                          const bool overwrite             )
{GUCEF_TRACE;

    // Not implemented / supported at this time
    return false;
}

/*-------------------------------------------------------------------------*/

void
CZIPArchive::GetFileList( TStringVector& outputList                   ,
                          const CORE::CString& mountLocation          , 
                          const CORE::CString& archiveLocation        ,
                          bool recursive                              ,
                          bool includePathInFilename                  ,
                          const CORE::CString::StringSet& nameFilters ,
                          UInt32 maxListEntries                       ) const
{GUCEF_TRACE;

    // @TODO
}

/*-------------------------------------------------------------------------*/

void
CZIPArchive::GetDirList( TStringVector& outputList                   ,
                         const CORE::CString& mountLocation          , 
                         const CORE::CString& archiveLocation        ,
                         bool recursive                              ,
                         bool includeParentDirInName                 ,
                         const CORE::CString::StringSet& nameFilters ,
                         UInt32 maxListEntries                       ) const
{GUCEF_TRACE;

    // @TODO
}

/*-------------------------------------------------------------------------*/

bool 
CZIPArchive::GetFileMetaData( const VFS::CString& filePath      ,
                              CORE::CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    metaData.Clear();
    
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CZIPArchive: request for file size for file: " +  filePath );
    
    if ( NULL != m_zipRoot )
    {
        // Get uncompressed size too
        ZZIP_STAT zstat;
        zzip_dir_stat( m_zipRoot, filePath.C_String(), &zstat, ZZIP_CASEINSENSITIVE );

        metaData.resourceExists = true;
        metaData.resourceSizeInBytes = zstat.st_size;
        metaData.hasResourceSizeInBytes = true;
        metaData.name = zstat.d_name;
        metaData.hasName = true;

        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CZIPArchive::FileExists( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    ZZIP_STAT zstat;
    int res = zzip_dir_stat( m_zipRoot, filePath.C_String(), &zstat, ZZIP_CASEINSENSITIVE );

    return (res == ZZIP_NO_ERROR);
}

/*-------------------------------------------------------------------------*/

bool 
CZIPArchive::DirExists( const VFS::CString& dirPath ) const
{GUCEF_TRACE;

    ZZIP_STAT zstat;
    int res = zzip_dir_stat( m_zipRoot, dirPath.C_String(), &zstat, ZZIP_CASEINSENSITIVE );

    return (res == ZZIP_NO_ERROR);
}

/*-------------------------------------------------------------------------*/

VFS::UInt64
CZIPArchive::GetFileSize( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    if ( NULL != m_zipRoot )
    {
        // Get uncompressed size too
        ZZIP_STAT zstat;
        zzip_dir_stat( m_zipRoot, filePath.C_String(), &zstat, ZZIP_CASEINSENSITIVE );

        return zstat.st_size;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CZIPArchive::GetFileModificationTime( const VFS::CString& filePath ) const
{
    // @todo: makeme
    return CORE::CDateTime();
}

/*-------------------------------------------------------------------------*/

CORE::IOAccessPtr
CZIPArchive::LoadFile( const VFS::CString& file      ,
                       const VFS::UInt32 memLoadSize ) const
{GUCEF_TRACE;

    // Check for failed archive
    if ( NULL == m_zipRoot ) 
        return CORE::IOAccessPtr();

    // Get non const access, no worries, it wont be changed
    ZZIP_DIR* zipRoot = const_cast< ZZIP_DIR* >( m_zipRoot );

    // Format not used here (always binary)
    ZZIP_FILE* zipFile = zzip_file_open( zipRoot, file.C_String(), ZZIP_ONLYZIP | ZZIP_CASELESS );
    if ( NULL == zipFile )
    {
        int zerr = zzip_error( zipRoot );
        VFS::CString zzDesc = getZzipErrorDescription( (zzip_error_t)zerr );
        GUCEF_SYSTEM_LOG( 0, m_archiveName + ": - Unable to open file " + file + ", error was '" + zzDesc + "'" );

        // return null pointer
        return CORE::IOAccessPtr();
    }

    // Get uncompressed size too
    ZZIP_STAT zstat;
    zzip_dir_stat( zipRoot, file.C_String(), &zstat, ZZIP_CASEINSENSITIVE );

    CORE::IOAccessPtr fileAccess( GUCEF_NEW CZipIOAccess( zipFile, zstat.st_size ) );

    // We will not load anything in memory for now (ignore memLoadSize)
    return fileAccess;
}

/*-------------------------------------------------------------------------*/

VFS::CString
CZIPArchive::GetFileHash( const VFS::CString& file ) const
{GUCEF_TRACE;

    CORE::IOAccessPtr fileAccess = LoadFile( file, 102400 );
    if ( !fileAccess.IsNULL() )
    {
        VFS::UInt8 digest[ 16 ];
        if ( 0 != CORE::md5fromfile( fileAccess->CStyleAccess() ,
                                     digest                     ) )
        {
            fileAccess.Unlink();

            char md5_str[ 48 ];
            CORE::md5tostring( digest, md5_str );
            VFS::CString md5Str;
            md5Str.Set( md5_str, 48 );
            return md5Str;
        }

        fileAccess.Unlink();
    }
    return VFS::CString();
}

/*-------------------------------------------------------------------------*/

const VFS::CString&
CZIPArchive::GetArchiveName( void ) const
{GUCEF_TRACE;

    return m_archiveName;
}

/*-------------------------------------------------------------------------*/

bool
CZIPArchive::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CZIPArchive::CheckZzipError( int zzipError                 ,
                             const VFS::CString& operation ) const
{GUCEF_TRACE;

    if ( zzipError != ZZIP_NO_ERROR )
    {
        VFS::CString errorMsg = getZzipErrorDescription( static_cast<zzip_error_t>( zzipError ) );
        GUCEF_ERROR_LOG( 0, "Archive " + m_archiveName + ": - error whilst " + operation + ": " + errorMsg );
    }
}

/*-------------------------------------------------------------------------*/

bool
CZIPArchive::LoadArchive( const VFS::CArchiveSettings& settings )
{GUCEF_TRACE;

    // We do not support writable ZIP archives
    if ( settings.GetWriteableRequested() ) 
        return false;

    if ( NULL == m_zipRoot )
    {
        zzip_error_t zzipError;
        m_zipRoot = zzip_dir_open( settings.GetActualArchivePath().C_String(), &zzipError );
        CheckZzipError( zzipError, "opening archive" );

        if ( NULL == m_zipRoot )
        {
            return false;
        }

        // Cache names
        ZZIP_DIRENT zzipEntry;
        while ( zzip_dir_read( m_zipRoot, &zzipEntry ) )
        {
            TZipEntry info;

            // Get the entry name
            info.path = zzipEntry.d_name;

            // Get sizes
            info.compressedSize = static_cast<size_t>(zzipEntry.d_csize);
            info.uncompressedSize = static_cast<size_t>(zzipEntry.st_size);

            // Check for folder entries
            VFS::CString filename = CORE::ExtractFilename( info.path );
            if ( filename.Length() == 0 )
            {
                info.compressedSize = 0;
                info.isDir = true;
            }
            else
            {
                info.isDir = false;
            }

            m_fileList.push_back( info );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CZIPArchive::LoadArchive( const VFS::CString& archiveName       ,
                          VFS::TBasicVfsResourcePtr vfsResource ,
                          const bool writeableRequest           )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CZIPArchive::UnloadArchive( void )
{GUCEF_TRACE;

    if ( NULL != m_zipRoot )
    {
        zzip_dir_close( m_zipRoot );
        m_zipRoot = NULL;

        m_fileList.clear();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const VFS::CString& 
CZIPArchive::GetType( void ) const
{GUCEF_TRACE;

    return ZipArchiveTypeName;
}

/*-------------------------------------------------------------------------*/

void
CZIPArchive::DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const
{GUCEF_TRACE;

    delete objectToBeDestroyed;
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
