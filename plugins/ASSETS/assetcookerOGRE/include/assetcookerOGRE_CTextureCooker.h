/*
 *  assetcookerOGRE: GUCEF asset converter plugin: the cooker of the intermediate into Ogre 1.x runtime files
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

#ifndef GUCEF_ASSETCOOKER_OGRE_CTEXTURECOOKER_H
#define GUCEF_ASSETCOOKER_OGRE_CTEXTURECOOKER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETTEXTURE_H
#include "AssetConverterLib_CAssetTexture.h"
#define ASSETCONVERTERLIB_CASSETTEXTURE_H
#endif /* ASSETCONVERTERLIB_CASSETTEXTURE_H ? */

#ifndef ASSETCONVERTERLIB_CCOOKPROFILE_H
#include "AssetConverterLib_CCookProfile.h"
#define ASSETCONVERTERLIB_CCOOKPROFILE_H
#endif /* ASSETCONVERTERLIB_CCOOKPROFILE_H ? */

#ifndef ASSETCONVERTERLIB_CTEXTUREBAKER_H
#include "AssetConverterLib_CTextureBaker.h"
#define ASSETCONVERTERLIB_CTEXTUREBAKER_H
#endif /* ASSETCONVERTERLIB_CTEXTUREBAKER_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef GUCEF_ASSETCOOKER_OGRE_MACROS_H
#include "assetcookerOGRE_macros.h"
#define GUCEF_ASSETCOOKER_OGRE_MACROS_H
#endif /* GUCEF_ASSETCOOKER_OGRE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Writes the textures of one cook profile, each once per Cook() call, however many assets use it.
 *
 *  Names (relative to the profile folder, also the Ogre texture names):
 *    - a file of the intermediate (baked textures, extracted images): <pack id>/<its path in the intermediate>;
 *    - a file of the source of the pack: <pack id>/<its path in the pack>;
 *    - another file: <pack id>/textures/<its file name>.
 *  PNG and JPEG files are copied as they are. A file in another format, and a file larger than the largest texture
 *  size of the profile, is written as PNG (resampled to fit, in linear light for colour), with ".png" in place of its
 *  extension. Formats other than PNG are made by the compressor plugins once they exist; until then a profile that
 *  asks for one gets PNG, which is reported.
 *
 *  Derived textures, which the material mapping makes (a metal-roughness texture with the occlusion packed into red
 *  and the factors folded in, an emissive texture with its factor folded in), are baked with CTextureBaker into
 *  <pack id>/textures/derived/, named after their first source and the hash of the sources and the operations, so the
 *  same derivation is written once, within a run and across runs.
 */
class GUCEF_ASSETCOOKER_OGRE_EXPORT_CPP CTextureCooker
{
    public:

    CTextureCooker( const CORE::CString& profileDir      ,
                    const CORE::CString& packId          ,
                    const CORE::CString& intermediateDir ,
                    const CORE::CString& sourceRoot      ,
                    const ASSETCONVERTER::CCookProfile& profile );

    ~CTextureCooker();

    /** The texture as the profile needs it; its resource name in 'name'. False, reported, when it cannot be written */
    bool Cook( const ASSETCONVERTER::CAssetTexture& texture, CORE::CString& name, ASSETCONVERTER::CConversionReport& report );

    /** A texture made with the operations of the texture from its sources; its resource name in 'name' */
    bool CookDerived( const ASSETCONVERTER::CAssetTexture& derived, CORE::CString& name, ASSETCONVERTER::CConversionReport& report );

    /** The VFS path of a resource name */
    CORE::CString ToVfsPath( const CORE::CString& name ) const;

    /** The number of files written in this Cook() call */
    UInt32 GetWrittenCount( void ) const;

    private:

    CORE::CString GetOutputName( const CORE::CString& sourcePath ) const;

    bool FitToProfile( const CORE::CString& vfsPath, const ASSETCONVERTER::CAssetTexture& texture, ASSETCONVERTER::CConversionReport& report );

    CTextureCooker( const CTextureCooker& src );              /**< not implemented */
    CTextureCooker& operator=( const CTextureCooker& src );   /**< not implemented */

    private:

    typedef GUCEF::map< CORE::CString, CORE::CString > TStringMap;

    CORE::CString m_profileDir;
    CORE::CString m_packId;
    CORE::CString m_intermediateDir;
    CORE::CString m_sourceRoot;
    ASSETCONVERTER::CCookProfile m_profile;
    ASSETCONVERTER::CTextureBaker m_baker;
    TStringMap m_cooked;            /**< source VFS path to the resource name written for it */
    TStringMap m_owners;            /**< lower case resource name to the source VFS path it was written for */
    UInt32 m_written;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETCOOKER_OGRE_CTEXTURECOOKER_H ? */
