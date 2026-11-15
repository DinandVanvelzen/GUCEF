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

#ifndef GUCEF_ASSETSTORE_CASSETIMAGEHEADERPROBE_H
#define GUCEF_ASSETSTORE_CASSETIMAGEHEADERPROBE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
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
 *  Reads the size, channels, bit depth and block compression of an image from the first bytes of its file, without
 *  decoding it: PNG, JPEG, TGA, BMP, GIF and DDS (with the DX10 header). For the texture analyser; other formats are
 *  decoded through the codecs of gucefIMAGE.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetImageHeaderProbe
{
    public:

    /** The bytes of the start of a file Probe() wants at most */
    static const UInt32 HeaderBytes = 65536;

    struct SImageInfo
    {
        UInt32 width;
        UInt32 height;
        UInt32 channels;
        UInt32 bitDepth;            /**< per channel */
        CORE::CString compression;  /**< "none", "bc1" .. "bc7", "jpeg", "png", "rle" */
        CORE::CString format;       /**< "png", "jpeg", "tga", "bmp", "gif", "dds" */
    };

    /** True when the bytes are the start of an image of a known format with a readable header */
    static bool Probe( const UInt8* data, UInt32 size, SImageInfo& info );

    private:

    CAssetImageHeaderProbe( void );   /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETIMAGEHEADERPROBE_H ? */
