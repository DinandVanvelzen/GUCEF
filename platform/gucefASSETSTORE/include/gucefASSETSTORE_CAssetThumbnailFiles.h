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

#ifndef GUCEF_ASSETSTORE_CASSETTHUMBNAILFILES_H
#define GUCEF_ASSETSTORE_CASSETTHUMBNAILFILES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTHUMBNAIL_H
#include "gucefASSETSTORE_CAssetThumbnail.h"
#define GUCEF_ASSETSTORE_CASSETTHUMBNAIL_H
#endif /* GUCEF_ASSETSTORE_CASSETTHUMBNAIL_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The stored thumbnails of the catalogue (remote service design 5.2) as PNG files in a folder of the VFS: the folder "thumbnails" of
 *  a storage location, in a subfolder of the first two characters of the digest, named "digest_renderer_version_size.png". The key of
 *  a thumbnail is the SHA-256 of the main file of the asset, the renderer and its version and the size, so the same content renders to
 *  the same name for every renderer of the same version and a picture never needs to be invalidated. Because the files are in a
 *  location of the VFS, a client reads a picture through the files of the service, like any other file.
 *
 *  The pictures of packs (the automatic thumbnail of a pack) use "pack-" and the id of the pack in place of the digest.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetThumbnailFiles
{
    public:

    enum EDefaults
    {
        MaxPngBytes = 2097152,          /**< a larger picture is refused */
        MaxSize = 1024                  /**< the longest side of a stored picture; a larger one is refused */
    };

    /** The folder of the thumbnails of a location: its root and "thumbnails" */
    static CORE::CString MakeFolder( const CORE::CString& locationRoot );

    /** The key part of a thumbnail of a pack */
    static CORE::CString MakePackDigest( const CORE::CString& packId );

    /** The name of the file of a key: the characters of the renderer and its version that are not letters, digits, '.' and '-' are left out */
    static CORE::CString MakeFileName( const CAssetThumbnailKey& key );

    /** The path of the file of a key below the folder */
    static CORE::CString MakePath( const CORE::CString& folder, const CAssetThumbnailKey& key );

    /**
     *  The best stored thumbnail of a digest for a size: the one of that size, else the smallest that is larger, else the largest
     *  that is smaller (the client scales). False when the digest has none
     */
    static bool Find( const CORE::CString& folder, const CORE::CString& digest, UInt32 size, CAssetThumbnailKey& key, CORE::CString& vfsPath );

    /** Whether the bytes are a PNG picture (the signature and a header chunk) */
    static bool IsPng( const CORE::CDynamicBuffer& bytes, UInt32* width = GUCEF_NULL, UInt32* height = GUCEF_NULL );

    /** Stores the picture of a key; the path of the file in 'vfsPath'. INVALID_ARGUMENT with a reason for what is no PNG, too large or too small a size */
    static TAssetStoreStatus Put( const CORE::CString& folder, const CAssetThumbnailKey& key, const CORE::CDynamicBuffer& png, CORE::CString& vfsPath, CORE::CString& message );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTHUMBNAILFILES_H ? */
