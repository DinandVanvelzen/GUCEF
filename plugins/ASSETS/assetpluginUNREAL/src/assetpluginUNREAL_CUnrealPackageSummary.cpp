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

#include <stdio.h>
#include <string.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#include "assetpluginUNREAL_UnrealVersions.h"
#define GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESUMMARY_H
#include "assetpluginUNREAL_CUnrealPackageSummary.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESUMMARY_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESUMMARY_H ? */

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnrealPackageSummary::SEngineVersion::SEngineVersion( void )
    : major( 0 )
    , minor( 0 )
    , patch( 0 )
    , changelist( 0 )
    , branch()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealPackageSummary::SEngineVersion::ToString( void ) const
{GUCEF_TRACE;

    return CORE::UInt32ToString( major ) + "." + CORE::UInt32ToString( minor ) + "." + CORE::UInt32ToString( patch ) + "-" + CORE::UInt32ToString( changelist ) + "+" + branch;
}

/*-------------------------------------------------------------------------*/

CUnrealPackageSummary::CUnrealPackageSummary( void )
    : versions()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CUnrealPackageSummary::Clear( void )
{GUCEF_TRACE;

    versions.Clear();
    totalHeaderSize = 0;
    memset( savedHash, 0, sizeof( savedHash ) );
    hasSavedHash = false;
    packageName.Clear();
    packageFlags = 0;
    nameCount = nameOffset = 0;
    softObjectPathsCount = softObjectPathsOffset = 0;
    localizationId.Clear();
    gatherableTextDataCount = gatherableTextDataOffset = 0;
    exportCount = exportOffset = importCount = importOffset = 0;
    cellExportCount = cellExportOffset = cellImportCount = cellImportOffset = 0;
    metaDataOffset = dependsOffset = 0;
    softPackageReferencesCount = softPackageReferencesOffset = 0;
    searchableNamesOffset = thumbnailTableOffset = 0;
    importTypeHierarchiesCount = importTypeHierarchiesOffset = 0;
    guid = CUnrealGuid();
    persistentGuid = CUnrealGuid();
    generations.clear();
    savedByEngineVersion = SEngineVersion();
    compatibleWithEngineVersion = SEngineVersion();
    compressionFlags = packageSource = 0;
    assetRegistryDataOffset = 0;
    bulkDataStartOffset = 0;
    worldTileInfoDataOffset = 0;
    chunkIds.clear();
    preloadDependencyCount = -1;
    preloadDependencyOffset = 0;
    namesReferencedFromExportDataCount = 0;
    payloadTocOffset = -1;
    dataResourceOffset = -1;
    summarySize = 0;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackageSummary::IsEditorPackage( void ) const
{GUCEF_TRACE;

    return 0 == ( packageFlags & ( UE_PKG_COOKED | UE_PKG_FILTER_EDITOR_ONLY | UE_PKG_UNVERSIONED_PROPERTIES ) );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackageSummary::ReadCustomVersions( CUnrealArchiveReader& reader )
{GUCEF_TRACE;

    const Int32 count = reader.ReadInt32();
    if ( reader.HasError() )
        return false;
    if ( count < 0 || !reader.CheckCount( (UInt64) count, 20, "custom versions" ) )
        return reader.SetError( "Custom version count " + CORE::Int32ToString( count ) + " is not valid" );
    for ( Int32 i=0; i<count; ++i )
    {
        CUnrealGuid key;
        reader.ReadGuid( key );
        const Int32 value = reader.ReadInt32();
        if ( reader.HasError() )
            return false;
        versions.SetCustomVersion( key, value );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CUnrealPackageSummary::ReadEngineVersion( CUnrealArchiveReader& reader, SEngineVersion& version )
{GUCEF_TRACE;

    version.major = reader.ReadUInt16();
    version.minor = reader.ReadUInt16();
    version.patch = reader.ReadUInt16();
    version.changelist = reader.ReadUInt32();
    reader.ReadString( version.branch );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackageSummary::Read( CUnrealArchiveReader& reader, const CUnrealReaderLimits& /* limits */, CORE::CString& problem )
{GUCEF_TRACE;

    Clear();
    problem.Clear();

    const UInt32 tag = reader.ReadUInt32();
    if ( reader.HasError() )
    {
        problem = "The package is too small to hold a summary: " + reader.GetError();
        return false;
    }
    if ( UE_PACKAGE_FILE_TAG != tag )
    {
        if ( 0xC1832A9E == tag )
            problem = "The package is byte swapped (big endian), which is not supported";
        else
        {
            char hex[ 16 ];
            sprintf( hex, "%08X", (unsigned int) tag );
            problem = CORE::CString( "Not an Unreal package: the tag is 0x" ) + hex + " instead of 0x9E2A83C1";
        }
        return false;
    }

    const Int32 legacyFileVersion = reader.ReadInt32();
    if ( legacyFileVersion >= 0 )
    {
        problem = "Legacy file version " + CORE::Int32ToString( legacyFileVersion ) + ": packages of the engine generation before UE4 are not supported";
        return false;
    }
    Int32 legacyUe3Version = 0;
    if ( -4 != legacyFileVersion )
        legacyUe3Version = reader.ReadInt32();
    const Int32 ue4Version = reader.ReadInt32();
    const Int32 ue5Version = legacyFileVersion <= -8 ? reader.ReadInt32() : 0;
    const Int32 licenseeVersion = reader.ReadInt32();
    if ( reader.HasError() )
    {
        problem = "The summary is truncated: " + reader.GetError();
        return false;
    }
    versions.SetFileVersions( legacyFileVersion, legacyUe3Version, ue4Version, ue5Version, licenseeVersion );
    if ( !versions.CheckSupported( problem ) )
        return false;

    if ( versions.IsAtLeast( UE5_PACKAGE_SAVED_HASH ) )
    {
        reader.Read( savedHash, sizeof( savedHash ) );
        hasSavedHash = true;
        totalHeaderSize = reader.ReadInt32();
    }
    if ( !ReadCustomVersions( reader ) )
    {
        problem = "The custom versions cannot be read: " + reader.GetError();
        return false;
    }
    if ( !versions.IsAtLeast( UE5_PACKAGE_SAVED_HASH ) )
        totalHeaderSize = reader.ReadInt32();
    reader.ReadString( packageName );
    packageFlags = reader.ReadUInt32();
    nameCount = reader.ReadInt32();
    nameOffset = reader.ReadInt32();
    if ( versions.IsAtLeast( UE5_ADD_SOFTOBJECTPATH_LIST ) )
    {
        softObjectPathsCount = reader.ReadInt32();
        softObjectPathsOffset = reader.ReadInt32();
    }
    if ( 0 == ( packageFlags & UE_PKG_FILTER_EDITOR_ONLY ) && versions.IsAtLeast( UE4_ADDED_PACKAGE_SUMMARY_LOCALIZATION_ID ) )
        reader.ReadString( localizationId );
    if ( versions.IsAtLeast( UE4_SERIALIZE_TEXT_IN_PACKAGES ) )
    {
        gatherableTextDataCount = reader.ReadInt32();
        gatherableTextDataOffset = reader.ReadInt32();
    }
    exportCount = reader.ReadInt32();
    exportOffset = reader.ReadInt32();
    importCount = reader.ReadInt32();
    importOffset = reader.ReadInt32();
    if ( versions.IsAtLeast( UE5_VERSE_CELLS ) )
    {
        cellExportCount = reader.ReadInt32();
        cellExportOffset = reader.ReadInt32();
        cellImportCount = reader.ReadInt32();
        cellImportOffset = reader.ReadInt32();
    }
    if ( versions.IsAtLeast( UE5_METADATA_SERIALIZATION_OFFSET ) )
        metaDataOffset = reader.ReadInt32();
    dependsOffset = reader.ReadInt32();
    if ( versions.IsAtLeast( UE4_ADD_STRING_ASSET_REFERENCES_MAP ) )
    {
        softPackageReferencesCount = reader.ReadInt32();
        softPackageReferencesOffset = reader.ReadInt32();
    }
    if ( versions.IsAtLeast( UE4_ADDED_SEARCHABLE_NAMES ) )
        searchableNamesOffset = reader.ReadInt32();
    thumbnailTableOffset = reader.ReadInt32();
    if ( versions.IsAtLeast( UE5_IMPORT_TYPE_HIERARCHIES ) )
    {
        importTypeHierarchiesCount = reader.ReadInt32();
        importTypeHierarchiesOffset = reader.ReadInt32();
    }
    if ( !versions.IsAtLeast( UE5_PACKAGE_SAVED_HASH ) )
        reader.ReadGuid( guid );
    if ( 0 == ( packageFlags & UE_PKG_FILTER_EDITOR_ONLY ) )
    {
        if ( versions.IsAtLeast( UE4_ADDED_PACKAGE_OWNER ) )
            reader.ReadGuid( persistentGuid );
        else
            persistentGuid = guid;
        if ( versions.IsAtLeast( UE4_ADDED_PACKAGE_OWNER ) && !versions.IsAtLeast( UE4_NON_OUTER_PACKAGE_IMPORT ) )
        {
            CUnrealGuid ownerGuid;
            reader.ReadGuid( ownerGuid );
        }
    }
    const Int32 generationCount = reader.ReadInt32();
    if ( reader.HasError() )
    {
        problem = "The summary is truncated: " + reader.GetError();
        return false;
    }
    if ( generationCount < 0 || !reader.CheckCount( (UInt64) generationCount, 8, "generations" ) )
    {
        problem = "Generation count " + CORE::Int32ToString( generationCount ) + " is not valid";
        return false;
    }
    for ( Int32 i=0; i<generationCount; ++i )
    {
        SGeneration generation;
        generation.exportCount = reader.ReadInt32();
        generation.nameCount = reader.ReadInt32();
        generations.push_back( generation );
    }
    ReadEngineVersion( reader, savedByEngineVersion );
    if ( versions.IsAtLeast( UE4_PACKAGE_SUMMARY_HAS_COMPATIBLE_ENGINE_VERSION ) )
        ReadEngineVersion( reader, compatibleWithEngineVersion );
    else
        compatibleWithEngineVersion = savedByEngineVersion;
    compressionFlags = reader.ReadUInt32();
    const Int32 compressedChunks = reader.ReadInt32();
    if ( !reader.HasError() && 0 != compressedChunks )
    {
        problem = "Package compression (" + CORE::Int32ToString( compressedChunks ) + " compressed chunks) is not supported";
        return false;
    }
    packageSource = reader.ReadUInt32();
    const Int32 additionalPackages = reader.ReadInt32();
    if ( reader.HasError() )
    {
        problem = "The summary is truncated: " + reader.GetError();
        return false;
    }
    if ( additionalPackages < 0 || !reader.CheckCount( (UInt64) additionalPackages, 4, "additional packages" ) )
    {
        problem = "Additional package count " + CORE::Int32ToString( additionalPackages ) + " is not valid";
        return false;
    }
    for ( Int32 i=0; i<additionalPackages; ++i )
    {
        CORE::CString ignored;
        reader.ReadString( ignored );
    }
    if ( versions.IsAtLeast( UE4_ASSET_REGISTRY_TAGS ) )
        assetRegistryDataOffset = reader.ReadInt32();
    if ( versions.IsAtLeast( UE4_SUMMARY_HAS_BULKDATA_OFFSET ) )
        bulkDataStartOffset = reader.ReadInt64();
    worldTileInfoDataOffset = reader.ReadInt32();
    if ( versions.IsAtLeast( UE4_CHANGED_CHUNKID_TO_BE_AN_ARRAY_OF_CHUNKIDS ) )
    {
        const Int32 chunkCount = reader.ReadInt32();
        if ( reader.HasError() || chunkCount < 0 || !reader.CheckCount( (UInt64) chunkCount, 4, "chunk ids" ) )
        {
            problem = "Chunk id count is not valid";
            return false;
        }
        for ( Int32 i=0; i<chunkCount; ++i )
            chunkIds.push_back( reader.ReadInt32() );
    }
    if ( versions.IsAtLeast( UE4_PRELOAD_DEPENDENCIES_IN_COOKED_EXPORTS ) )
    {
        preloadDependencyCount = reader.ReadInt32();
        preloadDependencyOffset = reader.ReadInt32();
    }
    if ( versions.IsAtLeast( UE5_NAMES_REFERENCED_FROM_EXPORT_DATA ) )
        namesReferencedFromExportDataCount = reader.ReadInt32();
    else
        namesReferencedFromExportDataCount = nameCount;
    if ( versions.IsAtLeast( UE5_PAYLOAD_TOC ) )
        payloadTocOffset = reader.ReadInt64();
    if ( versions.IsAtLeast( UE5_DATA_RESOURCES ) )
        dataResourceOffset = reader.ReadInt32();
    summarySize = reader.GetPosition() - reader.GetStart();
    if ( reader.HasError() )
    {
        problem = "The summary is truncated: " + reader.GetError();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
