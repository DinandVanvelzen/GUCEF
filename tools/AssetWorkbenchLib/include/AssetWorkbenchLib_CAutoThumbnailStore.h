/*
 *  AssetWorkbenchLib: automatic thumbnails: worthwhile renders of presentations, kept in the thumbnail cache under a name of their own
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

#ifndef ASSETWORKBENCHLIB_CAUTOTHUMBNAILSTORE_H
#define ASSETWORKBENCHLIB_CAUTOTHUMBNAILSTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The automatic thumbnails: when a pack or a model has no picture of its own and the workbench rendered something worthwhile
 *  (the generated showcase, the portrait of a character, the frame of an effect, the default view of a model), the render is kept so
 *  that the content browser can show it as the thumbnail the next time. The files are in the folder of the thumbnails under the name
 *  "&lt;content hash&gt;_v&lt;version&gt;_&lt;size&gt;_auto.png": a name of their own, so the files of the thumbnails of the content,
 *  which always win, are not touched and the cache of an older version of the workbench stays valid. The hash is the hash of the content
 *  of the asset, or of the names and sizes of the files of a pack, so the picture is replaced when the content changes.
 *
 *  Only the rendered picture is kept, scaled to the size of the thumbnails: nothing else of the pack, and nothing leaves the computer.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CAutoThumbnailStore
{
    public:

    enum EDefaults
    {
        DefaultSize = 256       /**< the size of a thumbnail of the content browser */
    };

    /** What the check of a picture found, for the log and the tests */
    struct ASSETWORKBENCHLIB_PUBLIC_CPP SQuality
    {
        Float32 deviation;      /**< the standard deviation of the brightness, 0 to 255 */
        Float32 coverage;       /**< the part of the picture that differs from the colour of the background, 0 to 1 */
        Float32 maximum;        /**< the brightest value of any channel */
        bool worthwhile;
        SQuality( void );
    };

    CAutoThumbnailStore( void );

    /** The folder of the thumbnails (a path of the disk) */
    void SetFolder( const CORE::CString& folder );
    const CORE::CString& GetFolder( void ) const;
    /** The switch of the settings; nothing is stored while it is off (what is stored stays and is shown) */
    void SetEnabled( bool enabled );
    bool IsEnabled( void ) const;

    /** The name of the file of the automatic thumbnail of a content hash */
    static CORE::CString MakeFileName( const CORE::CString& contentHash, UInt32 size );
    /** Whether a name is the name of an automatic thumbnail */
    static bool IsAutoFileName( const CORE::CString& name );

    /** Whether a picture is worth keeping: it is not (nearly) one colour, and something stands out from its background; the numbers go to 'quality' */
    static bool IsWorthwhile( const IMAGE::TPixelMapPtr& pixels, SQuality* quality = GUCEF_NULL );
    /** The picture in a square of this size (fitted, the shape kept, the background of the thumbnails around it) */
    static bool FitToSquare( const IMAGE::TPixelMapPtr& pixels, UInt32 size, IMAGE::TPixelMapPtr& fitted );

    /**
     *  Keeps a render as the automatic thumbnail of the content with this hash. Nothing is stored (false, 'reason' says why) when it is
     *  switched off, the folder is not set, the load had errors, the picture is not worthwhile, the content has a thumbnail of its own
     *  in the folder, or an automatic one is there already (at most one per content hash and size)
     */
    bool Store( const CORE::CString& contentHash, const IMAGE::TPixelMapPtr& pixels, bool loadHadErrors, CORE::CString& reason, UInt32 size = DefaultSize );
    /** Whether a thumbnail of the content browser of the content itself (the real one) is in the folder */
    bool HasRealThumbnail( const CORE::CString& contentHash, UInt32 size = DefaultSize ) const;
    bool Has( const CORE::CString& contentHash, UInt32 size = DefaultSize ) const;
    bool Read( const CORE::CString& contentHash, UInt32 size, IMAGE::TPixelMapPtr& pixels ) const;
    /** Deletes the automatic thumbnails of the folder (and no other file); the number deleted */
    UInt32 Clear( void );
    /** The number of automatic thumbnails in the folder */
    UInt32 GetCount( void ) const;
    UInt32 GetStoredCount( void ) const;

    /** The hash of the content of a pack: of the paths and sizes of its files below the VFS folder, in the order of the paths; empty when the folder holds no files */
    static CORE::CString HashPack( const CORE::CString& vfsFolder );

    private:

    CORE::CString m_folder;
    bool m_enabled;
    UInt32 m_stored;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CAUTOTHUMBNAILSTORE_H ? */
