/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONREQUEST_H
#define GUCEF_ASSETSTORE_CASSETCONVERSIONREQUEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#include "gucefASSETSTORE_CAssetPackRecord.h"
#define GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKRECORD_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H
#include "gucefASSETSTORE_CIAssetConversionObserver.h"
#define GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H
#endif /* GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a converter hook is asked to convert (CIAssetConverterHook::ConvertPack()): the pack with its licence, where
 *  its original files are now and where the converted files go.
 *
 *  The paths in the records of the assets are logical paths of the layout of the pack (CPackStorageLayout). 'sourceVfsPath'
 *  is the VFS folder that holds the original files, and 'sourcePrefix' the start of the original paths of the assets that
 *  this folder stands for: "original/" when the pack is placed in its storage location, empty while the pack is still
 *  staged. The hook writes the intermediate files, the report, the licence record and the manifest below 'convertedVfsFolder'
 *  and the cooked files below 'cookedVfsFolder' (below 'convertedVfsFolder' when that is empty), and sets the converted
 *  path of an asset as the prefix of the folder of its main file followed by the path below that folder.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetConversionRequest
{
    public:

    CAssetPackRecord pack;
    CORE::CString sourceVfsPath;                    /**< VFS folder with the original files */
    CORE::CString sourcePrefix;                     /**< the start of original paths of the assets that sourceVfsPath stands for */
    CORE::CString convertedVfsFolder;               /**< VFS folder for the results: the intermediate folder of the pack ("<pack root>/converted/<profile>" in the earlier layout), or its staged equivalent */
    CORE::CString convertedPathPrefix;              /**< the logical path of convertedVfsFolder: "converted/<profile>" (layout 1) or "intermediate" */
    CORE::CString cookedVfsFolder;                  /**< VFS folder for the cooked files when it is not convertedVfsFolder (layout 2: "<location>/gucef-ogre3d/<pack folder>"); empty: the cooked files go below convertedVfsFolder */
    CORE::CString cookedPathPrefix;                 /**< the logical path of cookedVfsFolder: "gucef-ogre3d" */
    CIAssetConversionObserver* observer;            /**< cancel and progress, not owned, may be GUCEF_NULL */
    bool reconvert;                                 /**< convert again, also what the cache of the converter says is unchanged */
    CORE::CString::StringVector assetGlobs;         /**< only the assets whose path below sourceVfsPath matches one of these (exact paths or globs); empty: all. The cache, references and sizes of the other assets stay as they are */
    bool cookOnly;                                  /**< cook what the intermediate in convertedVfsFolder holds into cookedVfsFolder; nothing is imported and nothing is written to convertedVfsFolder but what the cooker needs */
    bool noCook;                                    /**< convert to the intermediate and stop there: no cooker runs */
    CORE::CString::StringVector cookTargets;        /**< only these cook targets (names of the cookers); empty: those of the converter */
    bool keepPartialResult;                         /**< a run that ended with errors in its report still maps the assets that converted (the outcome stays FAILED), and nothing of what it wrote is taken away */
    CORE::CString reportVfsPath;                    /**< where the report of the run goes; empty: "<convertedVfsFolder>/report.json" */
    UInt32 maxThreads;                              /**< the converter threads of this run (the share of a reprocessing job); 0: the converter's own setting */
    CORE::CString threadPoolName;                   /**< the thread pool of this run, used by this run alone while it runs; empty: the converter's own pool */

    CAssetConversionRequest( void );
    ~CAssetConversionRequest();

    /** The path of the asset below sourceVfsPath; empty when the asset is not below it (a file of an archive, for example) */
    CORE::CString GetSourceRelativePath( const CAssetRecord& asset ) const;

    /** Whether the path below sourceVfsPath is one of assetGlobs (an exact path or a glob, not case sensitive); true when assetGlobs is empty */
    bool IsInScope( const CORE::CString& sourceRelativePath ) const;

    /** IsInScope() for the globs given */
    static bool IsInScope( const CORE::CString::StringVector& globs, const CORE::CString& sourceRelativePath );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONREQUEST_H ? */
