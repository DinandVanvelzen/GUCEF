/*
 *  vfspluginVP: Generic GUCEF VFS plugin for "Violation Pack" archives
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "vfspluginVP_CVPArchive.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace VP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SVPFileIndexEntry
{
    VFS::UInt32 offset;
    VFS::UInt32 size;
    char filename[ 32 ];
    VFS::Int32 timestamp;
};
typedef struct SVPFileIndexEntry TVPFileIndexEntry;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define VP_HEADER_SIZE          16
#define VP_INDEX_ENTRY_SIZE     44

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const VFS::CString CVPArchive::VPArchiveTypeName = "vp";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CVPArchive::CVPArchive( void )
    : CArchive()      ,
      m_header()      ,
      m_index()       ,
      m_archiveName() ,
      m_archivePath()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVPArchive::~CVPArchive()
{GUCEF_TRACE;

    UnloadArchive();
}

/*-------------------------------------------------------------------------*/

VFS::TBasicVfsResourcePtr
CVPArchive::GetFile( const VFS::CString& file      ,
                     const char* mode              ,
                     const VFS::UInt32 memLoadSize ,
                     const bool overwrite          )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CVPArchive: request to get file: " +  file );

    // We only support read only modes
    if ( *mode != 'r' )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CVPArchive: Unable to support requested file access mode for file: " + file );
        return VFS::TBasicVfsResourcePtr();
    }

    // load the file
    CORE::IOAccessPtr fileAccess = LoadFile( file, memLoadSize );
    if ( !fileAccess.IsNULL() )
    {
        // create a handle for the file
        VFS::CString filePath = m_archivePath;
        CORE::AppendToPath( filePath, file );

        VFS::TVfsResourcePtr fileHandle( GUCEF_NEW VFS::CVFSHandle( fileAccess ,
                                                                    file       ,
                                                                    filePath   ), this );

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CVPArchive: providing access to file: " + file );
        return fileHandle;
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CVPArchive: Unable to provide access to file: " + file );
    return VFS::TBasicVfsResourcePtr();
}

/*-------------------------------------------------------------------------*/

bool 
CVPArchive::DeleteFile( const VFS::CString& filePath )
{GUCEF_TRACE;

    // Not implemented / supported at this time
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CVPArchive::MoveFile( const VFS::CString& oldFilePath ,
                      const VFS::CString& newFilePath ,
                      const bool overwrite            )
{GUCEF_TRACE;

    // Not implemented / supported at this time
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CVPArchive::StoreAsFile( const CORE::CString& filepath    ,
                         const CORE::CDynamicBuffer& data ,
                         const CORE::UInt64 offset        ,
                         const bool overwrite             )
{GUCEF_TRACE;

    // Not implemented / supported at this time
    return false;
}

/*-------------------------------------------------------------------------*/

void
CVPArchive::GetFileList( TStringVector& outputList                   ,
                         const CORE::CString& mountLocation          , 
                         const CORE::CString& archiveLocation        ,
                         bool recursive                              ,
                         bool includePathInFilename                  ,
                         const CORE::CString::StringSet& nameFilters ,
                         UInt32 maxListEntries                       ) const
{GUCEF_TRACE;

    TFileIndexMap::const_iterator i = m_index.begin();
    while ( i != m_index.end() )
    {
        // Check if the starting path matches
        const VFS::CString& filePath = (*i).first;

        if ( filePath == archiveLocation )
        {
            // Don't add the location itself to the list
            ++i;
            continue;
        }

        if ( 0 == filePath.HasSubstr( archiveLocation, true ) )
        {
            const TVPIndexEntry& indexEntry = (*i).second;

            // Check if the entry is a file
            if ( indexEntry.size != 0 && indexEntry.offset != 0 )
            {
                if ( !recursive )
                {
                    // Check if we have multiple subdirs beyond the "location" to get to
                    // the archive. If so then we cannot add this archive because recursive
                    // searching is not allowed.
                    if ( !CORE::IsFileInDir( archiveLocation, filePath ) )
                    {
                        // The directory seperator was not the last character so we have multiple
                        // sub-dirs which is not allowed, we cannot add this item
                        ++i;
                        continue;
                    }
                }

                VFS::CString filename = CORE::ExtractFilename( filePath );
                if ( filename != ".." )
                {
                    if ( includePathInFilename )
                    {
                        outputList.push_back( filePath );
                    }
                    else
                    {
                        outputList.push_back( filename );
                    }
                }
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CVPArchive::GetDirList( TStringVector& outputList                   ,
                        const CORE::CString& mountLocation          , 
                        const CORE::CString& archiveLocation        ,
                        bool recursive                              ,
                        bool includeParentDirInName                 ,
                        const CORE::CString::StringSet& nameFilters ,
                        UInt32 maxListEntries                       ) const
{GUCEF_TRACE;

    TFileIndexMap::const_iterator i = m_index.begin();
    while ( i != m_index.end() )
    {
        // Check if the starting path matches
        const VFS::CString& filePath = (*i).first;

        if ( filePath == archiveLocation )
        {
            // Don't add the location itself to the list
            ++i;
            continue;
        }

        if ( 0 == filePath.HasSubstr( archiveLocation, true ) )
        {
            const TVPIndexEntry& indexEntry = (*i).second;

            // Check if the entry is a directory
            if ( indexEntry.size == 0 || indexEntry.offset == 0 )
            {
                if ( !recursive )
                {
                    // Check if we have multiple subdirs beyond the "location" to get to
                    // the archive. If so then we cannot add this archive because recursive
                    // searching is not allowed.
                    if ( !CORE::IsFileInDir( archiveLocation, filePath ) )
                    {
                        // The directory seperator was not the last character so we have multiple
                        // sub-dirs which is not allowed, we cannot add this item
                        ++i;
                        continue;
                    }
                }

                VFS::CString filename = CORE::ExtractFilename( filePath );
                if ( filename != ".." )
                {
                    if ( includeParentDirInName )
                    {
                        outputList.push_back( filePath );
                    }
                    else
                    {
                        outputList.push_back( filename );
                    }
                }
            }
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CVPArchive::GetFileMetaData( const VFS::CString& filePath      ,
                             CORE::CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    metaData.Clear();
    
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CDVPArchive: request for file size for file: " +  filePath );
    
    TFileIndexMap::const_iterator i = m_index.find( filePath.Lowercase().ReplaceChar( '/', '\\' ) );
    if ( i != m_index.end() )
    {
        const TVPIndexEntry& entry = (*i).second;
        metaData.resourceExists = true;
        metaData.resourceSizeInBytes = entry.size;
        metaData.hasResourceSizeInBytes = true;
        metaData.creationDateTime.FromUnixEpochBasedTicksInMillisecs( entry.timestamp );
        metaData.hasCreationDateTime = true;
        metaData.modifiedDateTime = metaData.creationDateTime;
        metaData.hasModifiedDateTime = true;
        metaData.name = (*i).first;
        metaData.hasName = true;

        return true;
    }
    
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVPArchive::FileExists( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CVPArchive: request to check if file exists: " +  filePath );
    return m_index.find( filePath.Lowercase().ReplaceChar( '/', '\\' ) ) != m_index.end();
}

/*-------------------------------------------------------------------------*/

bool
CVPArchive::DirExists( const VFS::CString& dirPath ) const
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CVPArchive: request to check if dir exists: " +  dirPath );

    // this container does not support sub-dirs
    return dirPath.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

VFS::UInt64
CVPArchive::GetFileSize( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    TFileIndexMap::const_iterator i = m_index.find( filePath.Lowercase().ReplaceChar( '/', '\\' ) );
    if ( i != m_index.end() )
    {
        return (*i).second.size;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

CORE::IOAccessPtr
CVPArchive::LoadFile( const VFS::CString& file      ,
                      const VFS::UInt32 memLoadSize ) const
{GUCEF_TRACE;

    TFileIndexMap::const_iterator i = m_index.find( file.Lowercase().ReplaceChar( '/', '\\' ) );
    if ( i != m_index.end() )
    {
        const TVPIndexEntry& entry = (*i).second;

        if ( memLoadSize >= entry.size )
        {
            FILE* fptr = fopen( m_archivePath.C_String(), "rb" );
            if ( GUCEF_NULL == fptr ) 
                return CORE::IOAccessPtr();

            if ( 0 == fseek( fptr, entry.offset, SEEK_CUR ) )
            {
                // prepare a memory buffer for the file
                CORE::CDynamicBuffer* fileBuffer = new CORE::CDynamicBuffer();
                fileBuffer->SetDataSize( entry.size );

                if ( 1 == fread( fileBuffer->GetBufferPtr(), entry.size, 1, fptr ) )
                {
                    // Successfully read file into memory
                    fclose( fptr );
                    return CORE::IOAccessPtr( GUCEF_NEW CORE::CDynamicBufferAccess( fileBuffer, true ) );
                }

                // unable to read entire file
                delete fileBuffer;
            }

            fclose( fptr );
        }
        else
        {
            CORE::CSubFileAccess* fileAccess = new CORE::CSubFileAccess();
            if ( fileAccess->Load( m_archivePath ,
                                   entry.offset  ,
                                   entry.size    ) )
            {
                return CORE::IOAccessPtr( fileAccess );
            }
            delete fileAccess;
        }
    }
    return CORE::IOAccessPtr();
}

/*-------------------------------------------------------------------------*/

VFS::CString
CVPArchive::GetFileHash( const VFS::CString& file ) const
{GUCEF_TRACE;

    CORE::IOAccessPtr fileAccess = LoadFile( file.Lowercase().ReplaceChar( '/', '\\' ), 102400 );
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

CORE::CDateTime
CVPArchive::GetFileModificationTime( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    TFileIndexMap::const_iterator i = m_index.find( filePath.Lowercase().ReplaceChar( '/', '\\' ) );
    if ( i != m_index.end() )
    {
        return CORE::CDateTime( (time_t) (*i).second.timestamp, true );
    }
    return CORE::CDateTime();
}

/*-------------------------------------------------------------------------*/

const VFS::CString&
CVPArchive::GetArchiveName( void ) const
{GUCEF_TRACE;

    return m_archiveName;
}

/*-------------------------------------------------------------------------*/

bool
CVPArchive::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVPArchive::LoadArchive( const VFS::CArchiveSettings& settings )
{GUCEF_TRACE;

    // We do not support writable VP archives
    if ( settings.GetWriteableRequested() ) 
        return false;

    FILE* fptr = fopen( settings.GetActualArchivePath().C_String(), "rb" );
    if ( GUCEF_NULL == fptr ) return false;

    if ( fread( &m_header, VP_HEADER_SIZE, 1, fptr ) == 1 )
    {
        if ( ( memcmp( m_header.sig, "VPVP", 4 ) == 0 ) &&
             ( m_header.version == 2 ) )
        {

            // Move to the index location at the end of the file
            if ( 0 != fseek( fptr, m_header.indexoffset, SEEK_SET ) )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFSPLUGIN VP: Error: unable to archive header" );
                fclose( fptr );
                return false;
            }

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFSPLUGIN VP: Successfully read the archive header" );

            // read the index
            VFS::CString path;
            TVPFileIndexEntry fileEntry;
            for ( VFS::UInt32 i=0; i<m_header.idxentries; ++i )
            {
                if ( fread( &fileEntry, VP_INDEX_ENTRY_SIZE, 1, fptr ) != 1 )
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFSPLUGIN VP: Error: unable to read index entry" );
                    m_header.idxentries = i;
                    break;
                }

                if ( fileEntry.offset == 0 || fileEntry.size == 0 )
                {
                    // directory entry
                    VFS::CString dirName;
                    dirName.Scan( fileEntry.filename, 32 );

                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFSPLUGIN VP: Found directory entry: " +  dirName);
                    if ( dirName == ".." )
                    {
                        path = CORE::StripLastSubDir( path );
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFSPLUGIN VP: Going up to directory: " + path );
                    }
                    else
                    {
                        CORE::AppendToPath( path, dirName );
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFSPLUGIN VP: Entering directory: " + dirName );
                    }

                    // Add the entry for the directory to our index
                    TVPIndexEntry entry;
                    entry.offset = 0;
                    entry.size = 0;
                    entry.timestamp = 0;
                    m_index[ path.Lowercase().ReplaceChar( '/', '\\' ) ] = entry;
                }
                else
                {
                    // file entry
                    TVPIndexEntry entry;
                    entry.offset = fileEntry.offset;
                    entry.size = fileEntry.size;
                    entry.timestamp = fileEntry.timestamp;

                    VFS::CString filenameBuffer;
                    filenameBuffer.Scan( fileEntry.filename, 32 );

                    VFS::CString filename = path;
                    CORE::AppendToPath( filename, filenameBuffer );

                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFSPLUGIN VP: Found file entry: " +  filenameBuffer );

                    m_index[ filename.Lowercase().ReplaceChar( '/', '\\' ) ] = entry;
                }
            }

            fclose( fptr );

            m_archiveName = settings.GetArchiveName();
            m_archivePath = settings.GetArchivePath();

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFSPLUGIN VP: Successfully finished reading the index" );
            return true;
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VFSPLUGIN VP: Error: Archive header not recognized" );
            fclose( fptr );
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CVPArchive::LoadArchive( const VFS::CString& archiveName       ,
                         VFS::TBasicVfsResourcePtr vfsResource ,
                         const bool writeableRequest           )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CVPArchive::UnloadArchive( void )
{GUCEF_TRACE;

    m_index.clear();
    m_archiveName = GUCEF_NULL;
    m_archivePath = GUCEF_NULL;
    return true;
}

/*-------------------------------------------------------------------------*/

const VFS::CString& 
CVPArchive::GetType( void ) const
{GUCEF_TRACE;

    return VPArchiveTypeName;
}

/*-------------------------------------------------------------------------*/

void
CVPArchive::DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const
{GUCEF_TRACE;

    delete objectToBeDestroyed;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VP */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
