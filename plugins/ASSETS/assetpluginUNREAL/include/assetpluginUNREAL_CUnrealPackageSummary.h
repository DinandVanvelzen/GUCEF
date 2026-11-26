/*
 *  assetpluginUNREAL: The summary at the start of a package: versions, flags and the places of the tables
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESUMMARY_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESUMMARY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#include "assetpluginUNREAL_CUnrealArchiveReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H
#include "assetpluginUNREAL_CUnrealReaderLimits.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H
#include "assetpluginUNREAL_CUnrealVersionContext.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALGUID_H
#include "assetpluginUNREAL_CUnrealGuid.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALGUID_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALGUID_H ? */

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
 *  The summary (header start) of a package, read in the order of the file with every version condition named in
 *  assetpluginUNREAL_UnrealVersions.h. Reading refuses a package that the version context does not support, with a
 *  message that names the value found and the supported limit.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealPackageSummary
{
    public:

    struct GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP SEngineVersion
    {
        UInt16 major;
        UInt16 minor;
        UInt16 patch;
        UInt32 changelist;
        CORE::CString branch;

        SEngineVersion( void );

        /** For example 5.8.2-56702186+++UE5+Release-5.8 */
        CORE::CString ToString( void ) const;
    };

    struct SGeneration
    {
        Int32 exportCount;
        Int32 nameCount;
    };

    typedef GUCEF::vector< SGeneration > TGenerationVector;
    typedef GUCEF::vector< Int32 > TInt32Vector;

    CUnrealVersionContext versions;
    Int32 totalHeaderSize;
    UInt8 savedHash[ 20 ];
    bool hasSavedHash;
    CORE::CString packageName;
    UInt32 packageFlags;
    Int32 nameCount;
    Int32 nameOffset;
    Int32 softObjectPathsCount;
    Int32 softObjectPathsOffset;
    CORE::CString localizationId;
    Int32 gatherableTextDataCount;
    Int32 gatherableTextDataOffset;
    Int32 exportCount;
    Int32 exportOffset;
    Int32 importCount;
    Int32 importOffset;
    Int32 cellExportCount;
    Int32 cellExportOffset;
    Int32 cellImportCount;
    Int32 cellImportOffset;
    Int32 metaDataOffset;
    Int32 dependsOffset;
    Int32 softPackageReferencesCount;
    Int32 softPackageReferencesOffset;
    Int32 searchableNamesOffset;
    Int32 thumbnailTableOffset;
    Int32 importTypeHierarchiesCount;
    Int32 importTypeHierarchiesOffset;
    CUnrealGuid guid;
    CUnrealGuid persistentGuid;
    TGenerationVector generations;
    SEngineVersion savedByEngineVersion;
    SEngineVersion compatibleWithEngineVersion;
    UInt32 compressionFlags;
    UInt32 packageSource;
    Int32 assetRegistryDataOffset;
    Int64 bulkDataStartOffset;
    Int32 worldTileInfoDataOffset;
    TInt32Vector chunkIds;
    Int32 preloadDependencyCount;
    Int32 preloadDependencyOffset;
    Int32 namesReferencedFromExportDataCount;
    Int64 payloadTocOffset;
    Int32 dataResourceOffset;
    UInt64 summarySize;                     /**< bytes the summary itself takes */

    CUnrealPackageSummary( void );

    void Clear( void );

    /** Reads from the start of the range of the reader; on false the reader holds the reason, or the message does for a refusal */
    bool Read( CUnrealArchiveReader& reader, const CUnrealReaderLimits& limits, CORE::CString& problem );

    bool IsEditorPackage( void ) const;

    private:

    bool ReadCustomVersions( CUnrealArchiveReader& reader );

    void ReadEngineVersion( CUnrealArchiveReader& reader, SEngineVersion& version );
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESUMMARY_H ? */
