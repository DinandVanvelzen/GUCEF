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

#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#include "gucefASSETSTORE_CAssetThumbnailFiles.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString
Clean( const CORE::CString& text, const CORE::CString& fallback, UInt32 maxLength = 24 )
{
    CORE::CString clean;
    for ( UInt32 i=0; i<text.Length() && clean.Length() < maxLength; ++i )
    {
        const char c = text[ i ];
        if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '.' == c || '-' == c )
            clean += c;
    }
    return clean.IsNULLOrEmpty() ? fallback : clean;
}

CORE::CString
FolderOfDigest( const CORE::CString& digest )
{
    return digest.Length() >= 2 ? digest.SubstrFromRange( 0, 2 ) : CORE::CString( "00" );
}

} // anonymous namespace

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CAssetThumbnailFiles::MakeFolder( const CORE::CString& locationRoot )
{GUCEF_TRACE;

    CORE::CString root = locationRoot.ReplaceChar( '\\', '/' );
    while ( root.Length() > 0 && '/' == root[ root.Length()-1 ] )
        root = root.CutChars( 1, false );
    return root.IsNULLOrEmpty() ? CORE::CString::Empty : root + "/thumbnails";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetThumbnailFiles::MakePackDigest( const CORE::CString& packId )
{GUCEF_TRACE;

    return "pack-" + Clean( packId, "unknown", 64 );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetThumbnailFiles::MakeFileName( const CAssetThumbnailKey& key )
{GUCEF_TRACE;

    return Clean( key.digest, "none", 80 ) + "_" + Clean( key.rendererId, "workbench" ) + "_" + Clean( key.rendererVersion, "1" ) + "_" + CORE::ToString( key.size ) + ".png";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetThumbnailFiles::MakePath( const CORE::CString& folder, const CAssetThumbnailKey& key )
{GUCEF_TRACE;

    return folder + "/" + FolderOfDigest( Clean( key.digest, "none", 80 ) ) + "/" + MakeFileName( key );
}

/*-------------------------------------------------------------------------*/

bool
CAssetThumbnailFiles::Find( const CORE::CString& folder, const CORE::CString& digestText, UInt32 size, CAssetThumbnailKey& key, CORE::CString& vfsPath )
{GUCEF_TRACE;

    const CORE::CString digest = Clean( digestText, "none", 80 );
    const CORE::CString directory = folder + "/" + FolderOfDigest( digest );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::CVFS::TStringVector names;
    vfs.GetFileList( names, directory, false, false, CORE::CString::EmptyStringSet );
    const CORE::CString prefix = digest + "_";
    bool found = false;
    UInt32 bestSize = 0;
    CORE::CString bestName;
    CORE::CString bestRenderer;
    CORE::CString bestVersion;
    for ( size_t i=0; i<names.size(); ++i )
    {
        const CORE::CString& name = names[ i ];
        if ( 0 != name.HasSubstr( prefix ) || name.Length() < prefix.Length() + 8 || name.SubstrFromRange( name.Length() - 4, name.Length() ) != ".png" )
            continue;
        const CORE::CString body = name.SubstrFromRange( prefix.Length(), name.Length() - 4 );     // renderer_version_size
        const Int32 last = body.HasChar( '_', false );
        const Int32 first = body.HasChar( '_', true );
        if ( last <= 0 || first <= 0 || first == last )
            continue;
        const UInt32 candidate = (UInt32) atoi( body.CutChars( (UInt32) last + 1, true ).C_String() );
        if ( 0 == candidate )
            continue;
        bool better = !found;
        if ( found )
        {
            const bool candidateEnough = candidate >= size;
            const bool bestEnough = bestSize >= size;
            if ( candidateEnough && bestEnough )
                better = candidate < bestSize;
            else
            if ( candidateEnough != bestEnough )
                better = candidateEnough;
            else
                better = candidate > bestSize;
        }
        if ( better )
        {
            found = true;
            bestSize = candidate;
            bestName = name;
            bestRenderer = body.SubstrFromRange( 0, (UInt32) first );
            bestVersion = body.SubstrFromRange( (UInt32) first + 1, (UInt32) last );
        }
    }
    if ( !found )
        return false;
    key.digest = digest;
    key.rendererId = bestRenderer;
    key.rendererVersion = bestVersion;
    key.size = bestSize;
    vfsPath = directory + "/" + bestName;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetThumbnailFiles::IsPng( const CORE::CDynamicBuffer& bytes, UInt32* width, UInt32* height )
{GUCEF_TRACE;

    static const UInt8 signature[ 8 ] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    if ( bytes.GetDataSize() < 33 )
        return false;
    const UInt8* data = static_cast< const UInt8* >( bytes.GetConstBufferPtr() );
    if ( 0 != memcmp( data, signature, 8 ) || 0 != memcmp( data + 12, "IHDR", 4 ) )
        return false;
    const UInt32 w = ( (UInt32) data[ 16 ] << 24 ) | ( (UInt32) data[ 17 ] << 16 ) | ( (UInt32) data[ 18 ] << 8 ) | (UInt32) data[ 19 ];
    const UInt32 h = ( (UInt32) data[ 20 ] << 24 ) | ( (UInt32) data[ 21 ] << 16 ) | ( (UInt32) data[ 22 ] << 8 ) | (UInt32) data[ 23 ];
    if ( GUCEF_NULL != width )
        *width = w;
    if ( GUCEF_NULL != height )
        *height = h;
    return w > 0 && h > 0;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetThumbnailFiles::Put( const CORE::CString& folder, const CAssetThumbnailKey& key, const CORE::CDynamicBuffer& png, CORE::CString& vfsPath, CORE::CString& message )
{GUCEF_TRACE;

    if ( folder.IsNULLOrEmpty() )
    {
        message = "The service has no folder for thumbnails";
        return ASSETSTORE_STATUS_UNSUPPORTED;
    }
    if ( key.digest.IsNULLOrEmpty() || 0 == key.size || key.size > MaxSize )
    {
        message = "A thumbnail has a digest of the content and a size of 1 to " + CORE::ToString( (UInt32) MaxSize ) + " pixels";
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    }
    UInt32 width = 0;
    UInt32 height = 0;
    if ( png.GetDataSize() > MaxPngBytes || !IsPng( png, &width, &height ) || width > MaxSize || height > MaxSize )
    {
        message = "The picture of a thumbnail is a PNG of at most " + CORE::ToString( (UInt32) MaxPngBytes ) + " bytes and " + CORE::ToString( (UInt32) MaxSize ) + " pixels on a side";
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    }
    vfsPath = MakePath( folder, key );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.StoreAsFile( vfsPath, png, 0, true ) )
    {
        message = "The thumbnail cannot be stored: the storage location is not writeable";
        return ASSETSTORE_STATUS_IO_ERROR;
    }
    // a put replaces: the pictures of the same content and size by another renderer or version go, so Find() has one to choose
    const CORE::CString digest = Clean( key.digest, "none", 80 );
    const CORE::CString directory = folder + "/" + FolderOfDigest( digest );
    const CORE::CString prefix = digest + "_";
    const CORE::CString suffix = "_" + CORE::ToString( key.size ) + ".png";
    const CORE::CString kept = MakeFileName( key );
    VFS::CVFS::TStringVector names;
    vfs.GetFileList( names, directory, false, false, CORE::CString::EmptyStringSet );
    for ( size_t i=0; i<names.size(); ++i )
    {
        const CORE::CString& name = names[ i ];
        if ( name == kept || 0 != name.HasSubstr( prefix ) || name.Length() <= suffix.Length() || name.SubstrFromRange( name.Length() - suffix.Length(), name.Length() ) != suffix )
            continue;
        vfs.DeleteFile( directory + "/" + name, true );
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
