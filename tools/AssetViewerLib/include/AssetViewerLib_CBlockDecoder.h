/*
 *  AssetViewerLib: decoder of the block compressed formats BC1 to BC5 (DXT1 to DXT5, ATI1, ATI2)
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

#ifndef ASSETVIEWERLIB_CBLOCKDECODER_H
#define ASSETVIEWERLIB_CBLOCKDECODER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <OgrePixelFormat.h>

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef ASSETVIEWERLIB_MACROS_H
#include "AssetViewerLib_macros.h"
#define ASSETVIEWERLIB_MACROS_H
#endif /* ASSETVIEWERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Makes RGBA pictures of the block compressed formats of the desktop: BC1 (DXT1), BC2 (DXT2 and DXT3), BC3 (DXT4 and DXT5),
 *  BC4 (as grey) and BC5 (as red and green, blue 0). Every format is 4 x 4 blocks of 8 or 16 bytes. The other compressed formats
 *  (BC6H, BC7, ETC, ASTC, PVRTC) are not decoded: the texture view shows their format and size only.
 */
class ASSETVIEWERLIB_PUBLIC_CPP CBlockDecoder
{
    public:

    /** Whether the format is one this class decodes */
    static bool CanDecode( Ogre::PixelFormat format );

    /** The bytes the blocks of a picture of this size take in this format */
    static UInt32 GetBlocksSize( Ogre::PixelFormat format, UInt32 width, UInt32 height );

    /**
     *  Decodes the blocks of a picture of width x height pixels into 'rgba' (width * height * 4 bytes, the top row first);
     *  false for a format that is not decoded
     */
    static bool Decode( Ogre::PixelFormat format, const void* blocks, UInt32 width, UInt32 height, UInt8* rgba );

    private:

    CBlockDecoder( void );
    CBlockDecoder( const CBlockDecoder& src );
    CBlockDecoder& operator=( const CBlockDecoder& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CBLOCKDECODER_H ? */
