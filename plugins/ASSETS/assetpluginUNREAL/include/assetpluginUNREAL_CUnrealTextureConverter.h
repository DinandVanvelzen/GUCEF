/*
 *  assetpluginUNREAL: Texture2D assets to image files and textures of the scene
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTURECONVERTER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTURECONVERTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTUREREADER_H
#include "assetpluginUNREAL_CUnrealTextureReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTUREREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTUREREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H
#include "assetpluginUNREAL_CIUnrealOodleProvider.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#include "assetpluginUNREAL_CUnrealObjectPathResolver.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H ? */

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

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
 *  Reads the source image of a Texture2D and writes it as an image file below a folder of the VFS (the intermediate of the pack),
 *  as the other importers do with their textures, and adds it to the scene. The source forms:
 *
 *   - TSCF_PNG and TSCF_JPEG: the file as it is stored (its sizes and channels are read from its header; the texture baker decodes it
 *     with the image codecs of gucefIMAGE).
 *   - TSCF_None with the formats TSF_G8, TSF_BGRA8, TSF_RGBA8, TSF_RGBA16 and TSF_G16: raw pixels, written as a PNG file; 16 bit
 *     channels are reduced to 8 bits and the report says so.
 *   - TSCF_UEDELTA, the form of the MetaHuman textures of UE 5.6 and later: raw pixels behind a row prediction, reversed by
 *     CUnrealDeltaTextureDecoder and then written as the raw formats are. Formats and sizes whose layout is not established are
 *     reported as unsupported with the reason, before the payload is read.
 *   - Four 8 bit channels whose colour channels are equal in every pixel and whose alpha is opaque (masks stored as BGRA8, such
 *     as the hidden face map of an outfit) are written as one grey channel; this is lossless and the report says so.
 *
 *   - Legacy bulk data compressed with zlib in blocks (the engines before UE 5.0, flag 0x2; the 4.14 project of the corpus): not supported, reported as such.
 *
 *  A payload that is compressed with Oodle is read through the provider. Without the tool the texture carries the reason
 *  "needs the oodle provider: tool not found at ..." in its report and nothing fails.
 *
 *  Not done: reducing very large sources while decoding (no source is reduced), and the mips of a platform (an editor package
 *  holds the source only).
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealTextureConverter
{
    public:

    struct GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP SResult
    {
        CORE::CString file;             /**< the VFS path of the image file */
        CORE::CString textureName;
        CORE::CString sourceFormat;     /**< for example "TSCF_PNG/TSF_BGRA8" */
        UInt32 width;
        UInt32 height;
        UInt32 channels;
        UInt32 bitsPerChannel;
        UInt32 fileChecksum;            /**< CRC-32 of the image file */
        UInt64 fileSize;
        bool reducedTo8Bit;
        bool greyAsOneChannel;          /**< four equal 8 bit channels with opaque alpha were written as one grey channel */
        bool unsupported;               /**< the source is a form this reader does not decode (see error) */
        bool providerMissing;           /**< the oodle tool or the zlib codec is not there (see error) */
        bool cancelled;
        Int32 toolCode;
        CORE::CString error;            /**< why no file was made */

        SResult( void );
    };

    /** Reads the source of an export and writes the image file into the folder (a VFS folder that exists or is made) */
    static bool Convert( CUnrealPackage& package                       ,
                         UInt32 exportIndex                            ,
                         CIUnrealOodleProvider* provider               ,
                         const ASSETCONVERTER::CConversionReport* cancellation ,
                         const CORE::CString& folder                   ,
                         SResult& result                               );

    /**
     *  As Convert(), and 'pixels' receives the pixels of a raw source (TSCF_UEDELTA, TSCF_None) as the image file holds them:
     *  result.channels 8 bit samples per pixel, rows from the top. 'pixels' stays empty for sources that are PNG or JPEG files.
     *  With an empty folder no file is written; the pixels are decoded all the same.
     */
    static bool ConvertWithPixels( CUnrealPackage& package                       ,
                                   UInt32 exportIndex                            ,
                                   CIUnrealOodleProvider* provider               ,
                                   const ASSETCONVERTER::CConversionReport* cancellation ,
                                   const CORE::CString& folder                   ,
                                   SResult& result                               ,
                                   GUCEF::vector< UInt8 >& pixels                );

    /**
     *  Converts the texture and adds it to the scene, with the usage the settings of the texture say (normal map, linear mask, colour).
     *  Failures go to the report of the asset; the texture index is -1 then. The first Texture2D of the package is used when exportIndex is -1.
     *  A source that cannot be read is a warning (a texture a material names), or an error with 'readFailureIsError' (the texture is
     *  the asset itself); a source form that is not supported and a missing provider are skips either way.
     */
    static bool AddToScene( CUnrealPackage& package                    ,
                            Int32 exportIndex                          ,
                            CIUnrealOodleProvider* provider            ,
                            const CORE::CString& folder                ,
                            ASSETCONVERTER::CAssetScene& scene         ,
                            ASSETCONVERTER::CConversionReport& report  ,
                            Int32& textureIndex                        ,
                            bool readFailureIsError = false            );

    /**
     *  Gives the materials of the scene the textures their parameters name: every texture parameter that the material mapper recorded
     *  (an object path and the slot it is meant for) is resolved in the pack, converted once and set in the slot: base colour, normal
     *  (the green flip of the DirectX convention is the normaliser's), the packed occlusion roughness metallic map as occlusion and
     *  metallic-roughness texture, and emissive. A texture that cannot be read leaves the slot empty and is reported.
     *  @return the number of slots that were filled
     */
    static UInt32 AttachToMaterials( const CUnrealObjectPathResolver& resolver ,
                                     CIUnrealOodleProvider* provider           ,
                                     const CORE::CString& folder               ,
                                     ASSETCONVERTER::CAssetScene& scene        ,
                                     ASSETCONVERTER::CConversionReport& report );

    private:

    static bool ConvertSource( CUnrealPackage& package                       ,
                               UInt32 exportIndex                            ,
                               CIUnrealOodleProvider* provider               ,
                               const ASSETCONVERTER::CConversionReport* cancellation ,
                               const CORE::CString& folder                   ,
                               SResult& result                               ,
                               GUCEF::vector< UInt8 >* pixelsOut             );

    CUnrealTextureConverter( void );    /**< only static members */
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTURECONVERTER_H ? */
