/*
 *  AssetConverterLib: the options of a pack index build
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXBUILDOPTIONS_H
#define ASSETCONVERTERLIB_CPACKINDEXBUILDOPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CPACKINDEXLICENCE_H
#include "AssetConverterLib_CPackIndexLicence.h"
#define ASSETCONVERTERLIB_CPACKINDEXLICENCE_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXLICENCE_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXPRODUCER_H
#include "AssetConverterLib_CPackIndexProducer.h"
#define ASSETCONVERTERLIB_CPACKINDEXPRODUCER_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXPRODUCER_H ? */

#ifndef ASSETCONVERTERLIB_CPACKGROUPRULE_H
#include "AssetConverterLib_CPackGroupRule.h"
#define ASSETCONVERTERLIB_CPACKGROUPRULE_H
#endif /* ASSETCONVERTERLIB_CPACKGROUPRULE_H ? */

#ifndef ASSETCONVERTERLIB_CIPACKINDEXOBSERVER_H
#include "AssetConverterLib_CIPackIndexObserver.h"
#define ASSETCONVERTERLIB_CIPACKINDEXOBSERVER_H
#endif /* ASSETCONVERTERLIB_CIPACKINDEXOBSERVER_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

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

/** What a build of the structure level of a pack index needs besides the listing and the profile */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexBuildOptions
{
    public:

    CORE::CString packName;                             /**< identity.name; the name of the listing when empty */
    CORE::CString packTitle;
    CORE::CString storePackId;
    CPackIndexLicence licence;
    CPackGroupRule::TRuleVector profileRules;           /**< explicit rules added to those the recogniser gave (shipped profile data) */
    CORE::CString presentationDefault;
    CPackIndexProducer::TPluginVersionMap plugins;      /**< "recogniser:<name>" and so on: versions the key depends on */
    CORE::CString tool;                                 /**< "AssetConverterLib" when empty */
    CORE::CString toolVersion;
    CORE::CString outputRoot;                           /**< "pack" or "intermediate" */
    bool useFileSystemSizes;                            /**< ask the VFS for the size of each file of the listing (true), or take the sizes given to the builder only (false) */
    CIPackIndexObserver* observer;

    CPackIndexBuildOptions( void );

    /** The hash of the settings that change the structure level (the depth of sub-groups): part of the index key */
    CORE::CString GetSettingsHash( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKINDEXBUILDOPTIONS_H ? */

