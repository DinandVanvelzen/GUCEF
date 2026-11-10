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

#ifndef GUCEF_CORE_CSUBIOACCESS_H
#include "gucefCORE_CSubIOAccess.h"
#define GUCEF_CORE_CSUBIOACCESS_H
#endif /* GUCEF_CORE_CSUBIOACCESS_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#include "vfspluginITV_CResArchive.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace ITV {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const VFS::CString CResArchive::ResArchiveTypeName = "res";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The index and data files are little endian on disk, regardless of the byte order of the platform we run on
 */
static bool
ReadLittleEndianInt32( CORE::CIOAccess& access, CORE::Int32& value )
{GUCEF_TRACE;

    CORE::Int32 raw = 0;
    if ( !access.ReadValue( raw ) )
        return false;

    CORE::TVariantData data;
    memset( &data, 0, sizeof( data ) );
    data.containedType = GUCEF_DATATYPE_LE_INT32;
    data.union_data.int32_data = raw;

    CORE::CVariant variant( data, false );
    value = variant.ToNativeEndian().AsInt32();
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
ReadLittleEndianInt16( CORE::CIOAccess& access, CORE::Int16& value )
{GUCEF_TRACE;

    CORE::Int16 raw = 0;
    if ( !access.ReadValue( raw ) )
        return false;

    CORE::TVariantData data;
    memset( &data, 0, sizeof( data ) );
    data.containedType = GUCEF_DATATYPE_LE_INT16;
    data.union_data.int16_data = raw;

    CORE::CVariant variant( data, false );
    value = variant.ToNativeEndian().AsInt16();
    return true;
}

/*-------------------------------------------------------------------------*/

CResArchive::CResArchive( void )
    : CArchive()        ,
      m_archiveName()   ,
      m_archivePath()   ,
      m_resAccess()     ,
      m_idxResource()   ,
      m_resResource()   ,
      m_index()         ,
      m_indexLookup()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CResArchive::~CResArchive()
{GUCEF_TRACE;

    UnloadArchive();
}

/*-------------------------------------------------------------------------*/

VFS::TBasicVfsResourcePtr
CResArchive::GetFile( const VFS::CString& file      ,
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
        return VFS::TVfsResourcePtr( new VFS::CVFSHandle( fileAccess, file, filePath ), this );
    }
    return VFS::TBasicVfsResourcePtr();
}

/*-------------------------------------------------------------------------*/

bool 
CResArchive::DeleteFile( const VFS::CString& filePath )
{GUCEF_TRACE;

    // Not implemented / supported at this time
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CResArchive::StoreAsFile( const CORE::CString& filepath    ,
                          const CORE::CDynamicBuffer& data ,
                          const CORE::UInt64 offset        ,
                          const bool overwrite             )
{GUCEF_TRACE;

    // Not implemented / supported at this time
    return false;
}

/*-------------------------------------------------------------------------*/

void
CResArchive::GetFileList( TStringVector& outputList                   ,
                          const CORE::CString& mountLocation          , 
                          const CORE::CString& archiveLocation        ,
                          bool recursive                              ,
                          bool includePathInFilename                  ,
                          const CORE::CString::StringSet& nameFilters ,
                          UInt32 maxListEntries                       ) const
{GUCEF_TRACE;

    // This format does not support (sub)dirs
    if ( !archiveLocation.IsNULLOrEmpty() ) 
        return;

    // Generate list of all resource Id's
    CORE::CString resourceType = CORE::Int16ToString( m_index.recordType );
    IndexVector::const_iterator i = m_index.index.begin();
    while ( i != m_index.index.end() )
    {
        CORE::CString resourceId = CORE::Int32ToString( (*i).resourceNr ) + ".itv" + resourceType;
        if ( includePathInFilename )
        {
            resourceId = CORE::CombinePath( mountLocation, resourceId );
        }
        outputList.push_back( resourceId );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CResArchive::GetDirList( TStringVector& outputList                   ,
                         const CORE::CString& mountLocation          , 
                         const CORE::CString& archiveLocation        ,
                         bool recursive                              ,
                         bool includeParentDirInName                 ,
                         const CORE::CString::StringSet& nameFilters ,
                         UInt32 maxListEntries                       ) const
{GUCEF_TRACE;

    // this format does not support sub-dirs
}

/*-------------------------------------------------------------------------*/

bool 
CResArchive::GetFileMetaData( const VFS::CString& filePath      ,
                              CORE::CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    metaData.Clear();
    
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ResArchive: request for file size for file: " << filePath );
    
    // First perform a sanity check on the resource type
    VFS::CString typeIdStr = filePath.SubstrToChar( '.', false );
    typeIdStr = typeIdStr.CutChars( 3, true, 0 );
    CORE::UInt32 typeId = CORE::StringToUInt32( typeIdStr );
    if ( typeId != m_index.recordType )
        return false;

    CORE::Int32 offset = 0;
    CORE::Int32 size = 0;
    CORE::Int32 resourceId = CORE::StringToInt32( filePath.SubstrToChar( '.', true ) );
    if ( GetResourceInfo( resourceId ,
                          offset     ,
                          size       ) )
    {
        metaData.resourceExists = true;
        metaData.resourceSizeInBytes = size;
        metaData.hasResourceSizeInBytes = true;
        metaData.name = CORE::ToString( resourceId );
        metaData.hasName = true;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CResArchive::FileExists( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    // First perform a sanity check on the resource type
    VFS::CString typeIdStr = filePath.SubstrToChar( '.', false );
    typeIdStr = typeIdStr.CutChars( 3, true, 0 );
    CORE::UInt32 typeId = CORE::StringToUInt32( typeIdStr );
    if ( typeId != m_index.recordType )
        return false;

    VFS::CString resourceIdStr = filePath.SubstrToChar( '.', true );
    CORE::Int32 resourceId = CORE::StringToInt32( resourceIdStr );

    return m_indexLookup.find( resourceId ) != m_indexLookup.end();
}

/*-------------------------------------------------------------------------*/

VFS::UInt64
CResArchive::GetFileSize( const VFS::CString& filePath ) const
{GUCEF_TRACE;

    // First perform a sanity check on the resource type
    VFS::CString typeIdStr = filePath.SubstrToChar( '.', false );
    typeIdStr = typeIdStr.CutChars( 3, true, 0 );
    CORE::UInt32 typeId = CORE::StringToUInt32( typeIdStr );
    if ( typeId != m_index.recordType )
        return 0;

    CORE::Int32 offset = 0;
    CORE::Int32 size = 0;
    CORE::Int32 resourceId = CORE::StringToInt32( filePath.SubstrToChar( '.', true ) );
    if ( GetResourceInfo( resourceId ,
                          offset     ,
                          size       ) )
    {
        return size;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CResArchive::GetFileModificationTime( const VFS::CString& filePath ) const
{
    // not supported by this format, maybe get archive mod time?
    return CORE::CDateTime();
}

/*-------------------------------------------------------------------------*/

bool 
CResArchive::MoveFile( const VFS::CString& oldFilePath ,
                       const VFS::CString& newFilePath ,
                       const bool overwrite            )
{GUCEF_TRACE;

    // Not implemented / supported at this time
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CResArchive::DirExists( const VFS::CString& dirPath ) const
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CResArchive: request to check if dir exists: " +  dirPath );

    // this container does not support sub-dirs
    return dirPath.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CResArchive::GetResourceInfo( CORE::Int32 resourceId ,
                              CORE::Int32& offset    ,
                              CORE::Int32& size      ) const
{
    IndexLookupMap::const_iterator i = m_indexLookup.find( resourceId );
    if ( i == m_indexLookup.end() )
        return false;

    // Resources are stored back to back, the size of a resource is the distance to the next one.
    // For the last resource that is the distance to the end of the res file.
    size_t position = (*i).second;
    offset = m_index.index[ position ].offset;
    if ( position + 1 < m_index.index.size() )
    {
        size = m_index.index[ position + 1 ].offset - offset;
    }
    else
    {
        size = m_resAccess.IsNULL() ? 0 : (CORE::Int32) ( m_resAccess->GetSize() - offset );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::IOAccessPtr
CResArchive::GetResAccessForReading( void ) const
{GUCEF_TRACE;

    // If the data file can be cloned (for example a file on disk) every reader gets its own access
    // which is the most robust. Otherwise all readers share the one access and its position gets moved around.
    if ( !m_resAccess.IsNULL() )
    {
        CORE::CICloneable* clone = m_resAccess->Clone();
        if ( GUCEF_NULL != clone )
        {
            CORE::CIOAccess* cloneAccess = static_cast< CORE::CIOAccess* >( clone );
            return CORE::IOAccessPtr( cloneAccess );
        }
    }
    return m_resAccess;
}

/*-------------------------------------------------------------------------*/

CORE::IOAccessPtr
CResArchive::LoadFile( const VFS::CString& file      ,
                       const VFS::UInt32 memLoadSize ) const
{GUCEF_TRACE;

    if ( m_resAccess.IsNULL() )
        return CORE::IOAccessPtr();

    // First perform a sanity check on the resource type
    VFS::CString typeIdStr = file.SubstrToChar( '.', false );
    typeIdStr = typeIdStr.CutChars( 3, true, 0 );
    CORE::UInt32 typeId = CORE::StringToUInt32( typeIdStr );
    if ( typeId != m_index.recordType )
        return CORE::IOAccessPtr();
    
    CORE::Int32 offset = 0;
    CORE::Int32 size = 0;
    CORE::Int32 resourceId = CORE::StringToInt32( file.SubstrToChar( '.', true ) );
    if ( !GetResourceInfo( resourceId ,
                           offset     ,
                           size       ) || size < 0 || offset < 0 )
    {
        return CORE::IOAccessPtr();
    }

    CORE::IOAccessPtr resAccess = GetResAccessForReading();
    if ( resAccess.IsNULL() )
        return CORE::IOAccessPtr();
        
    if ( (CORE::UInt32) size <= memLoadSize )
    {
        CORE::CDynamicBuffer* buffer = GUCEF_NEW CORE::CDynamicBuffer( (CORE::UInt32) size, true );
        resAccess->Setpos( (CORE::UInt64) offset );
        bool readOk = resAccess->Tell() == (CORE::UInt64) offset;
        readOk = readOk && ( 0 == size || 1 == resAccess->Read( buffer->GetBufferPtr(), (CORE::UInt32) size, 1 ) );
        if ( readOk )
        {
            buffer->SetDataSize( (CORE::UInt32) size );
            return CORE::DynamicBufferAccessPtr( GUCEF_NEW CORE::CDynamicBufferAccess( buffer, true ) );
        } 
        GUCEF_DELETE buffer;
    }
    else
    {
        CORE::SubIOAccessPtr window( GUCEF_NEW CORE::CSubIOAccess() );
        if ( window->Load( resAccess, (CORE::UInt64) offset, (CORE::UInt64) size ) )
        {
            return window;
        }
    }

    return CORE::IOAccessPtr();
}

/*-------------------------------------------------------------------------*/

VFS::CString
CResArchive::GetFileHash( const VFS::CString& file ) const
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
CResArchive::GetArchiveName( void ) const
{GUCEF_TRACE;

    return m_archiveName;
}

/*-------------------------------------------------------------------------*/

bool
CResArchive::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CResArchive::LoadIndex( CORE::CIOAccess& file )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CResArchive:LoadIndex: Starting index load" );

    m_index.index.clear();
    m_indexLookup.clear();

    const CORE::UInt64 fileSize = file.GetSize();
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CResArchive:LoadIndex: Index is " << fileSize << " bytes" );

    file.Setpos( 0 );
    if ( fileSize < 2 || 0 != file.Tell() || !ReadLittleEndianInt16( file, m_index.recordType ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CResArchive:LoadIndex: Failed to read index header at position " << file.Tell() );
        return false;
    }

    // Get the nr of index entries plus do a sanity check
    CORE::UInt32 nrOfEntries = (CORE::UInt32) fileSize - 2;
    if ( 0 != nrOfEntries % 8 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CResArchive:LoadIndex: The number of bytes in the index is not what we expected, the data will probably be corrupt" );
    }
    nrOfEntries /= 8;

    // read all the index entries
    for ( CORE::UInt32 i=0; i<nrOfEntries; ++i )
    {
        IdxRecord idxRecord;
        if ( !ReadLittleEndianInt32( file, idxRecord.resourceNr ) || 
             !ReadLittleEndianInt32( file, idxRecord.offset )      )
        { 
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CResArchive:LoadIndex: Failed to read index entry at pos in index " << file.Tell() << 
                " Entry " << (i+1) << "/" << nrOfEntries );
            return false; 
        }

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Read index entry: id " << idxRecord.resourceNr << 
            ", offset " << idxRecord.offset << ". Pos in index " << file.Tell() << 
            " Entry " << (i+1) << "/" << nrOfEntries );
        
        m_indexLookup[ idxRecord.resourceNr ] = m_index.index.size();
        m_index.index.push_back( idxRecord );
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CResArchive:LoadIndex: Successfully loaded " << m_index.index.size() << " entries from index" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CResArchive::LoadArchive( const VFS::CString& archiveName ,
                          const VFS::CString& archivePath ,
                          CORE::IOAccessPtr idxAccess     ,
                          CORE::IOAccessPtr resAccess     )
{GUCEF_TRACE;

    if ( idxAccess.IsNULL() || resAccess.IsNULL() )
        return false;

    UnloadArchive();

    if ( !LoadIndex( *idxAccess ) )
    {
        UnloadArchive();
        return false;
    }

    m_resAccess = resAccess;
    m_archiveName = archiveName;
    m_archivePath = archivePath;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CResArchive::LoadArchive( const VFS::CArchiveSettings& settings )
{GUCEF_TRACE;

    // We do not support writable archives
    if ( settings.GetWriteableRequested() ) 
        return false;

    // Since we always need both the index file and the resource file
    // already construct the paths for both
    CORE::CString idxPath;
    CORE::CString resPath;

    CORE::CString fileExt = CORE::ExtractFileExtention( settings.GetActualArchivePath() ).Uppercase();
    if ( fileExt.IsNULLOrEmpty() )
    {
        idxPath = resPath = settings.GetActualArchivePath();
        idxPath += ".IDX";
        resPath += ".RES";
    }
    else
    if ( fileExt.Equals( "RES", false ) )
    {
        resPath = settings.GetActualArchivePath();
        idxPath = settings.GetActualArchivePath().CutChars( 4, false, 0 ) + ".IDX";
    }
    else
    if ( fileExt.Equals( "IDX", false ) )
    {
        idxPath = settings.GetActualArchivePath();
        resPath = settings.GetActualArchivePath().CutChars( 4, false, 0 ) + ".RES";
    }
    else
    {
        return false;
    }

    CORE::IOAccessPtr idxAccess( GUCEF_NEW CORE::CFileAccess( idxPath, "rb" ) );
    if ( !idxAccess->Opened() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CResArchive:LoadArchive: Failed to open index file: " << idxPath );
        return false;
    }
    CORE::IOAccessPtr resAccess( GUCEF_NEW CORE::CFileAccess( resPath, "rb" ) );
    if ( !resAccess->Opened() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CResArchive:LoadArchive: Failed to open resource file: " << resPath );
        return false;
    }

    return LoadArchive( settings.GetArchiveName(), settings.GetArchivePath(), idxAccess, resAccess );
}

/*-------------------------------------------------------------------------*/

bool 
CResArchive::LoadArchive( const VFS::CString& archiveName       ,
                          VFS::TBasicVfsResourcePtr vfsResource ,
                          const bool writeableRequest           )
{GUCEF_TRACE;

    // We do not support writable archives
    if ( writeableRequest || vfsResource.IsNULL() || vfsResource->GetAccess().IsNULL() )
        return false;

    // The archive consists of an index and a data file. We were handed one of the two,
    // the other is expected to sit next to it with the other extension, using the same case.
    // Preferably we find it through the VFS path the resource was obtained from. Failing that the
    // file path of the resource is the best we have, which for plain files on disk is a usable path.
    const bool hasVfsPath = !vfsResource->GetVfsPath().IsNULLOrEmpty();
    const VFS::CString givenPath = hasVfsPath ? vfsResource->GetVfsPath() : vfsResource->GetFilePath();
    const CORE::CString givenExt = CORE::ExtractFileExtention( givenPath );
    const bool givenIsRes = givenExt.Equals( "RES", false );
    const bool givenIsIdx = givenExt.Equals( "IDX", false );
    if ( !givenIsRes && !givenIsIdx )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CResArchive:LoadArchive: Resource \"" << givenPath << "\" is not an IDX or RES file" );
        return false;
    }

    const bool lowerCase = givenExt == givenExt.Lowercase();
    CORE::CString otherExt = givenIsRes ? "IDX" : "RES";
    if ( lowerCase )
        otherExt = otherExt.Lowercase();
    const VFS::CString otherPath = givenPath.CutChars( givenExt.Length(), false, 0 ) + otherExt;

    VFS::TBasicVfsResourcePtr otherResource = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( otherPath, "rb", false );
    if ( otherResource.IsNULL() && !hasVfsPath )
    {
        CORE::IOAccessPtr otherFile( GUCEF_NEW CORE::CFileAccess( otherPath, "rb" ) );
        if ( otherFile->Opened() )
            otherResource = VFS::TBasicVfsResourcePtr( VFS::TVfsResourcePtr( GUCEF_NEW VFS::CVFSHandle( otherFile, CORE::ExtractFilename( otherPath ), otherPath ) ) );
    }
    if ( otherResource.IsNULL() || otherResource->GetAccess().IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CResArchive:LoadArchive: Failed to open companion file: " << otherPath );
        return false;
    }

    VFS::TBasicVfsResourcePtr idxResource = givenIsIdx ? vfsResource : otherResource;
    VFS::TBasicVfsResourcePtr resResource = givenIsRes ? vfsResource : otherResource;
    if ( !LoadArchive( archiveName, givenPath, idxResource->GetAccess(), resResource->GetAccess() ) )
        return false;

    // Keep the resources themselves alive, they own the access and may own more
    m_idxResource = idxResource;
    m_resResource = resResource;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CResArchive::UnloadArchive( void )
{GUCEF_TRACE;

    // wipe the index
    m_index.index.clear();
    m_indexLookup.clear();
    m_resAccess.Unlink();
    m_idxResource.Unlink();
    m_resResource.Unlink();
    return true;
}

/*-------------------------------------------------------------------------*/

const VFS::CString& 
CResArchive::GetType( void ) const
{GUCEF_TRACE;

    return ResArchiveTypeName;
}

/*-------------------------------------------------------------------------*/

void
CResArchive::DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != objectToBeDestroyed )
    {
        objectToBeDestroyed->GetAccess().Unlink();
        GUCEF_DELETE objectToBeDestroyed;
    }    
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ITV */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
