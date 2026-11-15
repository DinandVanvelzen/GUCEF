/*
 *  AssetConverterLib: bakes the operation chains of textures, once per unique chain
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

#ifndef ASSETCONVERTERLIB_CTEXTUREBAKER_H
#define ASSETCONVERTERLIB_CTEXTUREBAKER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef ASSETCONVERTERLIB_CASSETTEXTURE_H
#include "AssetConverterLib_CAssetTexture.h"
#define ASSETCONVERTERLIB_CASSETTEXTURE_H
#endif /* ASSETCONVERTERLIB_CASSETTEXTURE_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CConversionReport;

/*-------------------------------------------------------------------------*/

/**
 *  Bakes textures: reads the sources of a texture through the VFS and the image codecs of gucefIMAGE, runs its
 *  operation chain with the operations of gucefIMAGE (CChannelOperations; CPixelMapResampler brings the sources of a
 *  PACK step to the size of the first) and writes the result as PNG into its folder of the intermediate.
 *
 *  - The file is named after the first source and the SHA-256 of the contents of the sources and the steps of the
 *    chain that change texels: the same chain on the same sources is written once, in a run (several assets that
 *    share a texture) and across runs (the file is there already). A source without texel operations that is a PNG
 *    or JPEG is copied as it is.
 *  - Sources are read as RGB(A), red first, whatever order the codec gives, so channel numbers and masks of the
 *    operations always count R, G, B, A.
 *  - The baked path and size go into the texture; a texture whose sources cannot be read or whose chain fails is
 *    marked missing, with an error in the report.
 *
 *  Bake() is thread-safe: the tasks of the pipeline share one baker per run, and a chain that another task is baking
 *  is waited for, not baked twice.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CTextureBaker
{
    public:

    typedef GUCEF::vector< IMAGE::TPixelMapPtr > TPixelMapVector;

    /** 'outputFolder' is the VFS folder of the baked textures */
    explicit CTextureBaker( const CORE::CString& outputFolder );

    ~CTextureBaker();

    const CORE::CString& GetOutputFolder( void ) const;

    bool Bake( CAssetTexture& texture, CConversionReport& report );

    /** The number of files written, and of bakes answered by a file that was there or by another bake */
    UInt32 GetWrittenCount( void ) const;
    UInt32 GetReusedCount( void ) const;

    /**
     *  Whether baked textures are written (default true). Without, a bake reads the sources and runs the chain, so the
     *  texture gets its size and its errors as with writing, but no file is written and none is listed in the report:
     *  the validate-only mode of the pipeline
     */
    void SetWritesFiles( bool writesFiles );
    bool GetWritesFiles( void ) const;

    /** Runs the operation chain of the texture on the pictures of its sources */
    static bool ApplyOperations( const CAssetTexture& texture, const TPixelMapVector& sources, IMAGE::TPixelMapPtr& result, CORE::CString& problem );

    /** Reads a picture of the VFS as RGB, RGBA or a single channel, red first */
    static bool LoadPicture( const CORE::CString& vfsPath, IMAGE::TPixelMapPtr& picture );

    /** Writes the picture as PNG (8 bits per channel) to the VFS */
    static bool SavePng( const CORE::CString& vfsPath, const IMAGE::CPixelMap& picture );

    private:

    struct SBake
    {
        MT::CMutex lock;
        bool ok;
        CORE::CString path;
        UInt32 width;
        UInt32 height;
        CORE::CString problem;
    };
    typedef GUCEF::map< CORE::CString, SBake* > TBakeMap;

    bool DoBake( const CAssetTexture& texture, const CORE::CString& path, bool copySource, SBake& bake );

    CTextureBaker( const CTextureBaker& src );              /**< not implemented */
    CTextureBaker& operator=( const CTextureBaker& src );   /**< not implemented */

    private:

    CORE::CString m_folder;
    TBakeMap m_bakes;
    MT::CMutex m_lock;
    UInt32 m_written;
    UInt32 m_reused;
    bool m_writesFiles;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CTEXTUREBAKER_H ? */
