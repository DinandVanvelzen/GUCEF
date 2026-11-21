/*
 *  assetpluginUNREAL: The Texture2D export: the properties and the place of the source image
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTUREREADER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTUREREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What an editor package says of a Texture2D: the size, slices, mips and format of the source (the struct "Source" of the
 *  properties), how the source is stored (the compression format of the source: none, PNG, JPEG, or the delta form of the newer
 *  engines), the settings that tell its use (sRGB, compression settings, group, flipped green channel) and where the bytes of the
 *  source are. The engine stores the source image, not the platform data with the block compressed mips: a cooked package holds
 *  that, an editor package does not, so no mip chain of a platform exists here.
 *
 *  The native data after the properties is, as far as read: the flag and value of the object guid, the strip flags, and the bulk
 *  data of the source: an editor bulk data record (UE5 file version 1002 and later; the payload is a compressed buffer in the
 *  package trailer) or the legacy bulk data header (UE 4; the bytes are stored as they are or compressed with zlib in chunks).
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealTextureReader
{
    public:

    struct STextureSource
    {
        CORE::CString name;                     /**< the object name of the export */
        UInt32 width;
        UInt32 height;
        UInt32 slices;
        UInt32 mips;
        CORE::CString format;                   /**< ETextureSourceFormat, for example TSF_BGRA8 */
        CORE::CString compressionFormat;        /**< ETextureSourceCompressionFormat: TSCF_None, TSCF_PNG, TSCF_JPEG, TSCF_UEDELTA */
        bool srgb;
        CORE::CString compressionSettings;      /**< for example TC_Default, TC_Normalmap, TC_Masks */
        CORE::CString lodGroup;
        bool flipGreenChannel;
        bool hasPayload;
        bool isLegacyPayload;                   /**< the payload is not a compressed buffer: the bytes are the source as they are */
        CUnrealPayloadReference payload;

        STextureSource( void );

        /** Bytes per pixel of the format; 0 for the formats this reader does not know */
        UInt32 GetBytesPerPixel( void ) const;

        /** Normal map by the compression settings */
        bool IsNormalMap( void ) const;
    };

    CUnrealTextureReader( void );

    bool Read( CUnrealPackage& package, UInt32 exportIndex, STextureSource& out );

    const CORE::CString& GetError( void ) const;

    private:
    CORE::CString m_error;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTUREREADER_H ? */
