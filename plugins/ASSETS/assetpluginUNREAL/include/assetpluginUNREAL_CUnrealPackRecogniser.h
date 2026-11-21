/*
 *  assetpluginUNREAL: the file kinds of Unreal Engine and the recogniser of its projects and MetaHuman packages
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKRECOGNISER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKRECOGNISER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETPACKLISTING_H
#include "AssetConverterLib_CAssetPackListing.h"
#define ASSETCONVERTERLIB_CASSETPACKLISTING_H
#endif /* ASSETCONVERTERLIB_CASSETPACKLISTING_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKPROFILE_H
#include "AssetConverterLib_CAssetPackProfile.h"
#define ASSETCONVERTERLIB_CASSETPACKPROFILE_H
#endif /* ASSETCONVERTERLIB_CASSETPACKPROFILE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETFILEKIND_H
#include "AssetConverterLib_CAssetFileKind.h"
#define ASSETCONVERTERLIB_CASSETFILEKIND_H
#endif /* ASSETCONVERTERLIB_CASSETFILEKIND_H ? */

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
 *  What the importer says about the files of Unreal Engine without reading a package: the extensions with their kinds (.uasset,
 *  .umap, the .uexp, .ubulk and .uptnl files that belong to a package, .uproject, .mhpkg) and the recogniser of a project (a
 *  .uproject, with the engine association and the default map of Config/DefaultEngine.ini), of a MetaHuman package (the
 *  Manifest.json and FileList.json of a mounted .mhpkg) and of loose packages.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealPackRecogniser
{
    public:

    static void GetFileKinds( ASSETCONVERTER::TAssetFileKindVector& kinds );

    static bool Recognise( const ASSETCONVERTER::CAssetPackListing& listing, ASSETCONVERTER::CAssetPackProfile& profile );

    private:

    CUnrealPackRecogniser( void );
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKRECOGNISER_H ? */
