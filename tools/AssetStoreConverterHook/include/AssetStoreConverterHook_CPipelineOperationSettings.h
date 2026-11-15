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

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONSETTINGS_H
#define ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLROOT_H
#include "gucefASSETSTORE_CAssetToolRoot.h"
#define GUCEF_ASSETSTORE_CASSETTOOLROOT_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLROOT_H ? */

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
 *  The settings of the operations of the converter pipeline that the hook carries out for the tools of the asset
 *  store service (CPipelineConverterHook::RunPipelineOperation()), a section of the settings of the application that
 *  serves the tools:
 *
 *  - "roots": the folders the operations may work in (ASSETSTORE::CAssetToolRoot: id, VFS folder, kind, writable).
 *    Every path an operation takes or gives is a rooted path against them; packs are the manifests below the roots of
 *    the kind "manifests".
 *  - "historyVfsPath": the VFS folder that keeps the earlier content of the files the authoring operations replace or
 *    remove; empty: no history (an apply says so in its warnings).
 *  - "changeTokenTtlMs": how long the change token of a preview stays valid; "maxChangeTokensPerPrincipal": open
 *    tokens per principal, the oldest expires first.
 *  - "defaultPageEntries", "maxPageEntries": the paging of lists; "planSampleEntries": the entries of a plan that a
 *    convert_plan answer shows by default; "maxResourceBytes": the largest resource read_resource gives;
 *    "maxDiffLines": the lines of the diff of a preview.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CPipelineOperationSettings : public CORE::CIDataNodeSerializable
{
    public:

    ASSETSTORE::TAssetToolRootVector roots;
    CORE::CString historyVfsPath;
    UInt32 changeTokenTtlMs;
    UInt32 maxChangeTokensPerPrincipal;
    UInt32 defaultPageEntries;
    UInt32 maxPageEntries;
    UInt32 planSampleEntries;
    UInt32 maxResourceBytes;
    UInt32 maxDiffLines;

    CPipelineOperationSettings( void );

    virtual ~CPipelineOperationSettings() GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the settings can be used: valid roots with unique ids, limits that are not 0; 'problem' says why not */
    bool IsValid( CORE::CString& problem ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Members that are absent keep their defaults; roots that cannot be read make it fail */
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

#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONSETTINGS_H ? */
