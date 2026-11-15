/*
 *  AssetConverterLib: what a cooker gets: target, profile, policies and the assets to cook
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

#ifndef ASSETCONVERTERLIB_CCOOKSETTINGS_H
#define ASSETCONVERTERLIB_CCOOKSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_CCOOKPROFILE_H
#include "AssetConverterLib_CCookProfile.h"
#define ASSETCONVERTERLIB_CCOOKPROFILE_H
#endif /* ASSETCONVERTERLIB_CCOOKPROFILE_H ? */


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

/**
 *  The input of one Cook() call: the target, the cook profile, the pack, the LOD policy ("source", "generate" or
 *  "none") with its settings (the reduction table of "generate"), the collider policy, whether validation errors are
 *  overridden, the pack-wide settings (the rules of the manifest) and the assets to cook with their intermediate files.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CCookSettings
{
    public:

    struct SAsset
    {
        CORE::CString assetPath;                /**< the path of the asset in the pack, for CConversionReport::SelectAsset() */
        CORE::CString sidecarFile;              /**< VFS path of its <asset>.conversion.json */
        CORE::CString::StringVector files;      /**< VFS paths of every file of its intermediate */
    };
    typedef GUCEF::vector< SAsset > TAssetVector;

    CORE::CString target;
    CCookProfile profile;
    CORE::CString packId;
    CORE::CString sourceRoot;               /**< the VFS folder that is the root of the source of the pack, to name files that come from it */
    CORE::CString lodPolicy;
    CORE::CDataNode lodSettings;
    CORE::CString colliderPolicy;
    bool force;
    CORE::CDataNode settings;
    TAssetVector assets;

    CCookSettings( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CCOOKSETTINGS_H ? */
