/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef ASSETSTORECONVERTERHOOK_CCONVERTERHOOKSETTINGS_H
#define ASSETSTORECONVERTERHOOK_CCONVERTERHOOKSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef ASSETCONVERTERLIB_CCOOKPROFILE_H
#include "AssetConverterLib_CCookProfile.h"
#define ASSETCONVERTERLIB_CCOOKPROFILE_H
#endif /* ASSETCONVERTERLIB_CCOOKPROFILE_H ? */

#ifndef ASSETSTORECONVERTERHOOK_MACROS_H
#include "AssetStoreConverterHook_macros.h"
#define ASSETSTORECONVERTERHOOK_MACROS_H
#endif /* ASSETSTORECONVERTERHOOK_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings of the converter hook of the primary store (CPipelineConverterHook), a section of the settings of the
 *  application ("conversion" below "AssetSources" in the viewer):
 *
 *  - "cookProfile": the cook profile of the store (name, texture formats, largest texture, mips, LOD bias, as in the
 *    manifests of the converter); its name is the folder of its cooked files in the cooked area of a pack. Default "desktop".
 *  - "cookTarget": the cooker whose output the store keeps ("ogre"), empty for the engine-neutral intermediate only. The name of
 *    the cooked area that its output goes to ("gucef-ogre3d") is accepted for it: the cooker registers it as an alias.
 *  - "importer" ("auto": the first importer, by name, that can import a file) and "exporter" ("gltf") of the intermediate.
 *  - "threadPoolName" and "threads" for the converter's tasks (0 threads: one per logical CPU). A reprocessing job of the
 *    asset store service gives its own share of the threads and a pool of its own with the conversion request
 *    (ASSETSTORE::CAssetConversionRequest::maxThreads and threadPoolName, from the "reprocess" settings of the service): those
 *    replace these two for that run. The cook step of a run (the Ogre cooker is not thread safe) runs one run at a time in
 *    the process (CCookPhaseLock), whatever the threads.
 *  - "convertibleKinds": the kinds of assets (of the vocabulary of the store) the converter is offered.
 *  - "mainExtensions": which file of what the converter made for an asset is "its converted file", the first extension
 *    of this list that the asset has among its cooked files, else among its intermediate files.
 *  - "nonStandardLicenceClasses": licence classes of the catalogue that the converter treats as non-standard licences,
 *    which it converts only when the licence has the restriction flag "reviewed".
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CConverterHookSettings : public CORE::CIDataNodeSerializable
{
    public:

    ASSETCONVERTER::CCookProfile cookProfile;
    CORE::CString cookTarget;
    CORE::CString importer;
    CORE::CString exporter;
    CORE::CString threadPoolName;
    UInt32 threads;
    CORE::CString::StringSet convertibleKinds;
    CORE::CString::StringVector mainExtensions;
    CORE::CString::StringSet nonStandardLicenceClasses;

    CConverterHookSettings( void );
    virtual ~CConverterHookSettings() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CCONVERTERHOOKSETTINGS_H ? */
