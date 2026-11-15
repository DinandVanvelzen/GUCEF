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

#ifndef ASSETSTORECONVERTERHOOK_CCONVERTERPACK_H
#define ASSETSTORECONVERTERHOOK_CCONVERTERPACK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETTOOLROOT_H
#include "gucefASSETSTORE_CAssetToolRoot.h"
#define GUCEF_ASSETSTORE_CASSETTOOLROOT_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLROOT_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#include "AssetConverterLib_CAssetPackManifest.h"
#define ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#endif /* ASSETCONVERTERLIB_CASSETPACKMANIFEST_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef ASSETCONVERTERLIB_CIMPORTACTIVITYLOG_H
#include "AssetConverterLib_CImportActivityLog.h"
#define ASSETCONVERTERLIB_CIMPORTACTIVITYLOG_H
#endif /* ASSETCONVERTERLIB_CIMPORTACTIVITYLOG_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CCONVERTERLICENCEDECISION_H
#include "AssetStoreConverterHook_CConverterLicenceDecision.h"
#define ASSETSTORECONVERTERHOOK_CCONVERTERLICENCEDECISION_H
#endif /* ASSETSTORECONVERTERHOOK_CCONVERTERLICENCEDECISION_H ? */

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
 *  A converter pack as the pipeline operations see it: a manifest (<pack>.assetpack.json) below a root of the kind
 *  "manifests", the licence decision of its record, and whether the tools may run and edit it. A pack is runnable
 *  when every folder its manifest names lies in a root of the matching kind (section 11.1 of the asset tools design):
 *  the source in a source or staging root, the intermediate in an intermediate root, the output in an output root, the
 *  overrides in an overrides root and the licence record in a licences root, none of them with a ".." segment and none
 *  leading out of its root through a symbolic link. A manifest that the owner wrote with folders elsewhere stays usable
 *  for the converter's own command line; the tools only read it.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CConverterPack
{
    public:

    typedef GUCEF::vector< CConverterPack > TPackVector;

    CORE::CString packId;
    ASSETCONVERTER::CAssetPackManifest manifest;
    CORE::CString manifestVfsPath;                      /**< never shown to a client */
    CORE::CString manifestRooted;                       /**< the rooted path of the manifest */
    bool ambiguous;                                     /**< another manifest has the same id */
    CORE::CString::StringVector otherManifests;         /**< the rooted paths of the manifests with the same id */
    bool runnable;
    CORE::CString runnableReason;                       /**< why it is not runnable; empty when it is */
    CConverterLicenceDecision licence;

    CConverterPack( void );

    /** Every manifest below the roots of the kind "manifests", with the checks done; ids that occur twice are marked ambiguous */
    static void Discover( const ASSETSTORE::TAssetToolRootVector& roots, TPackVector& packs );

    /**
     *  Whether every folder of a manifest lies in a root of the right kind; 'problem' names the first that does not.
     *  Also used for a manifest that is still to be written (manifest_create, manifest_edit)
     */
    static bool CheckManifestPaths( const ASSETSTORE::TAssetToolRootVector& roots, const ASSETCONVERTER::CAssetPackManifest& manifest, CORE::CString& problem );

    /** The rooted path of a VFS path; false when no root holds it */
    static bool ToRooted( const ASSETSTORE::TAssetToolRootVector& roots, const CORE::CString& vfsPath, CORE::CString& rooted );

    /** The report of the last run, from the intermediate folder; false when there is none */
    bool LoadReport( ASSETCONVERTER::CConversionReport& report ) const;

    /** The import activity log of the intermediate folder; false when there is none */
    bool LoadActivity( ASSETCONVERTER::CImportActivityLog& log ) const;

    /** The VFS path of the report of the last run */
    CORE::CString GetReportPath( void ) const;

    /** The VFS folder of the cooked files of a profile: the output folder, below the cook target when the manifest has more than one */
    CORE::CString FindProfileFolder( const CORE::CString& profile ) const;

    /** Whether the intermediate folder exists */
    bool HasIntermediate( void ) const;
};

/*-------------------------------------------------------------------------*/

typedef CConverterPack::TPackVector     TConverterPackVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CCONVERTERPACK_H ? */
