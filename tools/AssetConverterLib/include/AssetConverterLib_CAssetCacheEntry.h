/*
 *  AssetConverterLib: the cache entry of an asset: its key, the parts of the key and the files written
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

#ifndef ASSETCONVERTERLIB_CASSETCACHEENTRY_H
#define ASSETCONVERTERLIB_CASSETCACHEENTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORTFILE_H
#include "AssetConverterLib_CConversionReportFile.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORTFILE_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORTFILE_H ? */


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
 *  The cache entry of an asset, <intermediate>/.cache/<asset path>.cache.json. The key is a SHA-256 over its parts:
 *
 *  - the SHA-256 of the source bytes;
 *  - the SHA-256 of the effective settings and of the processing settings of the run (the post-processor stacks, the
 *    validators, the exporter, the cook targets and profiles, the bone limit of the cookers), as canonical JSON;
 *  - the name and version of every plugin of the stack of the asset ("importer:<name>", "postProcessor:<name>",
 *    "validator:<name>", "exporter:<name>", "cooker:<target>", and "converter:AssetConverterLib");
 *  - the SHA-256 of every dependency (the files the import read besides the source) by VFS path.
 *
 *  It also lists the files the run wrote (intermediate and cooked, with their references). When the key of a new run
 *  differs, CompareWith() names the first part that differs, which goes to the activity log.
 *
 *  The pipeline writes the entry of an asset as soon as its export finished, with cookPending set when cookers are to
 *  run, and again without it once the cooked files are known: a run that was cancelled or interrupted between the two
 *  leaves entries that let the next run keep the intermediate and only cook. cookPending is no part of the key.
 *  Save() writes atomically (a temporary file, then a rename), so a reader never sees half an entry.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetCacheEntry
{
    public:

    typedef GUCEF::map< CORE::CString, UInt32 > TPluginVersionMap;
    typedef GUCEF::map< CORE::CString, CORE::CString > THashMap;
    typedef GUCEF::vector< CConversionReportFile > TFileVector;

    CORE::CString assetPath;
    CORE::CString key;
    CORE::CString sourceHash;
    CORE::CString settingsHash;
    TPluginVersionMap plugins;
    THashMap dependencies;
    TFileVector files;
    bool cookPending;

    CAssetCacheEntry( void );

    /** Sets the key from the parts */
    void ComputeKey( void );

    /** The key the parts give, without setting it */
    CORE::CString GetKeyOfParts( void ) const;

    /**
     *  What differs in 'current' (a later state of the same asset) from this entry: the first of source, settings,
     *  plugins (a version, a plugin added or removed), dependencies; ACTIVITYREASON_UNCHANGED when the keys are equal.
     *  'detail' names the plugin or dependency.
     */
    TActivityReason CompareWith( const CAssetCacheEntry& current, CORE::CString& detail ) const;

    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );

    bool Load( const CORE::CString& vfsPath );
    bool Save( const CORE::CString& vfsPath ) const;

    /** <intermediate>/.cache/<asset path>.cache.json */
    static CORE::CString GetEntryPath( const CORE::CString& intermediateDir, const CORE::CString& assetPath );

    private:

    void SerializeParts( CORE::CDataNode& node ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETCACHEENTRY_H ? */
