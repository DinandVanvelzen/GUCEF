/*
 *  assetpluginUNREAL: The importer unreal: recognises Unreal packages and reports what it can read
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CIASSETIMPORTER_H
#include "AssetConverterLib_CIAssetImporter.h"
#define ASSETCONVERTERLIB_CIASSETIMPORTER_H
#endif /* ASSETCONVERTERLIB_CIASSETIMPORTER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#include "assetpluginUNREAL_macros.h"
#define GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_MACROS_H ? */

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
 *  The importer "unreal". At this step it reads the package structure and the tagged properties (assetpluginUNREAL_CUnrealPackage
 *  and the property reader) and recognises .uasset, .umap and .mhpkg; it reports every asset kind beyond the properties as not
 *  yet supported, with the class names found. IndexPack() maps the package names of a pack to its files. It keeps no state
 *  between calls, so the tasks of the pipeline can use it at the same time.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealImporter : public ASSETCONVERTER::CIAssetImporter
{
    public:

    /** The name of the index of the context that maps package names to asset paths */
    static const char* const PackageIndexName;

    CUnrealImporter( void );

    virtual ~CUnrealImporter();

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetFileKinds( ASSETCONVERTER::TAssetFileKindVector& kinds ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RecognisePack( const ASSETCONVERTER::CAssetPackListing& listing, ASSETCONVERTER::CAssetPackProfile& profile ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CanImport( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& context ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Import( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& context, ASSETCONVERTER::CAssetScene& out, ASSETCONVERTER::CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IndexPack( ASSETCONVERTER::CAssetPackContext& context ) GUCEF_VIRTUAL_OVERRIDE;

    /** What the importer reads at this step and what not, as one line for the log and the capability lists */
    static CORE::CString GetCapabilities( void );
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALIMPORTER_H ? */
