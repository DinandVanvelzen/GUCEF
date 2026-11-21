/*
 *  AssetConverterLib: builds the structure level of a pack index from a pack listing
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CPACKGROUPRULES_H
#include "AssetConverterLib_CPackGroupRules.h"
#define ASSETCONVERTERLIB_CPACKGROUPRULES_H
#endif /* ASSETCONVERTERLIB_CPACKGROUPRULES_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#include "AssetConverterLib_CPackIndexSerial.h"
#define ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXSERIAL_H ? */

#ifndef ASSETCONVERTERLIB_CCONTENTHASH_H
#include "AssetConverterLib_CContentHash.h"
#define ASSETCONVERTERLIB_CCONTENTHASH_H
#endif /* ASSETCONVERTERLIB_CCONTENTHASH_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#include "AssetConverterLib_CAssetConverterGlobal.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */

#include "AssetConverterLib_CPackIndexBuilder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPackIndexBuilder::CPackIndexBuilder( const CAssetKindRegistry& kinds )
    : m_kinds( kinds )
    , m_givenSizes()
    , m_sizes()
    , m_inputs()
    , m_preparedFor()
    , m_prepared( false )
    , m_groupedCount( 0 )
    , m_keptCount( 0 )
    , m_error()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPackIndexBuilder::SetFileSize( const CORE::CString& path, UInt64 size )
{GUCEF_TRACE;

    m_givenSizes[ path ] = size;
    m_prepared = false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& CPackIndexBuilder::GetError( void ) const { return m_error; }
UInt32 CPackIndexBuilder::GetGroupedCount( void ) const { return m_groupedCount; }
UInt32 CPackIndexBuilder::GetKeptCount( void ) const { return m_keptCount; }

/*-------------------------------------------------------------------------*/

CORE::CString
CPackIndexBuilder::GetKindRegistryHash( const CAssetKindRegistry& kinds )
{GUCEF_TRACE;

    CORE::CString text;
    for ( UInt32 k=0; k<(UInt32) ASSETKIND_COUNT; ++k )
    {
        const EAssetKind kind = static_cast< EAssetKind >( k );
        CORE::CString::StringVector extensions;
        kinds.GetExtensions( kind, extensions );
        std::sort( extensions.begin(), extensions.end() );
        text += CAssetKindRegistry::GetKindId( kind ) + ":";
        for ( size_t e=0; e<extensions.size(); ++e )
            text += extensions[ e ] + ",";
        text += "\n";
    }
    return CContentHash::HashText( text );
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexBuilder::PrepareInputs( const CAssetPackListing& listing, const CAssetPackProfile& profile, const CPackIndexBuildOptions& options, CPackIndexDocument& inputs )
{GUCEF_TRACE;

    m_error.Clear();
    m_inputs.Clear();
    m_sizes.clear();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    CORE::CString::StringVector files = listing.GetFiles();
    std::sort( files.begin(), files.end() );
    CORE::CString treeText;
    CORE::CString pathsText;
    UInt64 totalBytes = 0;
    for ( size_t i=0; i<files.size(); ++i )
    {
        UInt64 size = 0;
        TSizeMap::const_iterator given = m_givenSizes.find( files[ i ] );
        if ( given != m_givenSizes.end() )
            size = (*given).second;
        else
        if ( options.useFileSystemSizes && !listing.GetVfsRoot().IsNULLOrEmpty() )
            size = vfs.GetFileSize( CVfsPaths::Join( listing.GetVfsRoot(), files[ i ] ) );
        m_sizes[ files[ i ] ] = size;
        totalBytes += size;
        treeText += files[ i ] + "\t" + CORE::UInt64ToString( size ) + "\n";
        pathsText += files[ i ] + "\n";
        if ( 0 == ( i & 1023 ) && GUCEF_NULL != options.observer )
        {
            if ( options.observer->IsCancelRequested() )
            {
                m_error = "cancelled";
                return false;
            }
            options.observer->OnProgress( "structure", (UInt32) i, (UInt32) files.size() * 2 );
        }
    }

    CPackIndexDocument& doc = m_inputs;
    doc.identity.treeHash = CContentHash::HashText( treeText );
    doc.identity.packKey = doc.identity.treeHash;
    doc.identity.fileCount = files.size();
    doc.identity.totalBytes = totalBytes;
    doc.identity.name = options.packName.IsNULLOrEmpty() ? listing.GetPackName() : options.packName;
    doc.identity.title = options.packTitle;
    doc.identity.storePackId = options.storePackId;
    doc.identity.root = profile.root;
    doc.licence = options.licence;

    doc.profile.SetFromProfile( profile );
    doc.profile.groupRules = options.profileRules;
    doc.profile.presentationDefault = options.presentationDefault;
    CORE::CString::StringVector recognitionParts;
    recognitionParts.push_back( CContentHash::HashText( pathsText ) );
    CPackIndexProducer::TPluginVersionMap::const_iterator p = options.plugins.begin();
    while ( p != options.plugins.end() )
    {
        if ( (*p).first.WildcardEquals( "recogniser:*", '*', true ) )
            recognitionParts.push_back( (*p).first + "=" + CORE::UInt32ToString( (*p).second ) );
        ++p;
    }
    doc.profile.recognitionKey = CPackIndexSerial::HashParts( recognitionParts );

    CPackGroupRules rules( m_kinds );
    rules.SetProfile( doc.profile );
    CPackIndexProducer& producer = doc.producer;
    producer.tool = options.tool;
    producer.toolVersion = options.toolVersion;
    producer.schemaWriter = CPackIndexDocument::IndexerVersion;
    producer.settingsHash = options.GetSettingsHash();
    producer.plugins = options.plugins;
    producer.kindRegistryVersion = m_kinds.GetDeclarationCount();
    producer.kindRegistryHash = GetKindRegistryHash( m_kinds );
    producer.groupRulesVersion = CPackGroupRules::Version;
    producer.groupRulesHash = rules.GetRulesHash();
    producer.outputRoot = options.outputRoot;

    CORE::CString::StringVector keyParts;
    keyParts.push_back( doc.identity.packKey );
    keyParts.push_back( producer.settingsHash );
    p = producer.plugins.begin();
    while ( p != producer.plugins.end() )
    {
        keyParts.push_back( (*p).first + "=" + CORE::UInt32ToString( (*p).second ) );
        ++p;
    }
    keyParts.push_back( producer.kindRegistryHash );
    keyParts.push_back( "rules=" + CORE::UInt32ToString( producer.groupRulesVersion ) + ":" + producer.groupRulesHash );
    keyParts.push_back( "schema=" + CORE::UInt32ToString( CPackIndexDocument::SchemaVersion ) );
    keyParts.push_back( "indexer=" + CORE::UInt32ToString( producer.schemaWriter ) );
    producer.indexKey = CPackIndexSerial::HashParts( keyParts );

    m_preparedFor = doc.identity.packKey;
    m_prepared = true;
    inputs = doc;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexBuilder::BuildStructure( const CAssetPackListing& listing, const CAssetPackProfile& profile, const CPackIndexBuildOptions& options, CPackIndexDocument& document, const CPackIndexDocument* previous )
{GUCEF_TRACE;

    m_groupedCount = 0;
    m_keptCount = 0;
    CPackIndexDocument inputs;
    if ( !m_prepared || m_sizes.size() != listing.GetFileCount() )
    {
        if ( !PrepareInputs( listing, profile, options, inputs ) )
            return false;
    }
    if ( listing.GetFileCount() > CPackIndexDocument::MaxResources )
    {
        m_error = "the pack has " + CORE::UInt32ToString( listing.GetFileCount() ) + " files: more than the limit of " + CORE::UInt32ToString( CPackIndexDocument::MaxResources );
        return false;
    }

    document = m_inputs;
    document.state = CPackIndexDocument::StateBuilding;
    CPackGroupRules rules( m_kinds );
    rules.SetProfile( document.profile );

    // N: the files that are resources
    CORE::CString::StringVector files = listing.GetFiles();
    std::sort( files.begin(), files.end() );
    CORE::CString::StringVector resourcePaths;
    CORE::CString::StringVector metadataPaths;
    UInt32 noise = 0;
    for ( size_t i=0; i<files.size(); ++i )
    {
        if ( rules.IsNoise( files[ i ] ) )
        {
            ++noise;
            continue;
        }
        const EAssetKind kind = m_kinds.GetKindOfPath( files[ i ] );
        if ( ASSETKIND_METADATA == kind )
            metadataPaths.push_back( files[ i ] );
        else
        if ( ASSETKIND_FOLDER != kind )
            resourcePaths.push_back( files[ i ] );
    }
    rules.Prepare( resourcePaths );

    // the records of the previous document, by path, for the files that did not change
    typedef GUCEF::map< CORE::CString, const CPackIndexResource* > TPreviousMap;
    TPreviousMap previousByPath;
    if ( GUCEF_NULL != previous )
        for ( size_t i=0; i<previous->resources.size(); ++i )
            previousByPath[ previous->resources[ i ].path ] = &previous->resources[ i ];

    // the id base is the store pack id or the name of the pack, not the pack key: a changed file changes the key but no id
    const CORE::CString idBase = !document.identity.storePackId.IsNULLOrEmpty() ? document.identity.storePackId : document.identity.name;
    document.resources.clear();
    document.resources.reserve( resourcePaths.size() );
    for ( size_t i=0; i<resourcePaths.size(); ++i )
    {
        CPackIndexResource resource;
        resource.path = resourcePaths[ i ];
        resource.size = m_sizes[ resourcePaths[ i ] ];
        resource.id = CPackIndexResource::MakeId( idBase, resource.path );
        TPreviousMap::const_iterator old = previousByPath.find( resource.path );
        if ( old != previousByPath.end() && (*old).second->size == resource.size )
        {
            const CPackIndexResource& kept = *(*old).second;
            resource.digest = kept.digest;
            resource.stats = kept.stats;
            resource.hasBounds = kept.hasBounds;
            for ( UInt32 b=0; b<3; ++b )
            {
                resource.boundsMin[ b ] = kept.boundsMin[ b ];
                resource.boundsMax[ b ] = kept.boundsMax[ b ];
            }
            resource.refs = kept.refs;
            resource.unresolvedRefs = kept.unresolvedRefs;
            resource.presentedUnder = kept.presentedUnder;
            resource.outputs = kept.outputs;
            resource.thumbnails = kept.thumbnails;
            resource.presentation = kept.presentation;
            resource.tags = kept.tags;
            resource.held = kept.held;
            resource.licenceRef = kept.licenceRef;
            resource.status = kept.status;
            ++m_keptCount;
        }
        rules.Assign( resource );
        rules.Refine( resource );
        ++m_groupedCount;
        document.resources.push_back( resource );
        if ( 0 == ( i & 511 ) && GUCEF_NULL != options.observer )
        {
            if ( options.observer->IsCancelRequested() )
            {
                m_error = "cancelled";
                return false;
            }
            options.observer->OnProgress( "structure", (UInt32) ( resourcePaths.size() + i ), (UInt32) resourcePaths.size() * 2 );
        }
    }

    // the metadata files belong to their owners and are no resources of their own
    typedef GUCEF::map< CORE::CString, size_t > TIndexMap;
    TIndexMap indexByPath;
    for ( size_t i=0; i<document.resources.size(); ++i )
        indexByPath[ document.resources[ i ].path ] = i;
    for ( size_t m=0; m<metadataPaths.size(); ++m )
    {
        CORE::CString::StringVector owners;
        if ( !m_kinds.GetOwnerPaths( metadataPaths[ m ], owners ) )
            continue;
        for ( size_t o=0; o<owners.size(); ++o )
        {
            TIndexMap::const_iterator found = indexByPath.find( owners[ o ] );
            if ( found != indexByPath.end() )
            {
                document.resources[ (*found).second ].metadataFiles.push_back( metadataPaths[ m ] );
                break;
            }
        }
    }
    for ( size_t i=0; i<document.resources.size(); ++i )
        std::sort( document.resources[ i ].metadataFiles.begin(), document.resources[ i ].metadataFiles.end() );

    document.counts.noise = noise;
    document.counts.metadata = (UInt32) metadataPaths.size();
    document.levels.clear();
    document.AddLevel( CPackIndexDocument::LevelStructure );
    if ( GUCEF_NULL != previous && m_keptCount == document.resources.size() && previous->resources.size() == document.resources.size() )
        for ( size_t l=0; l<previous->levels.size(); ++l )
            document.AddLevel( previous->levels[ l ] );
    document.UpdateSummaries();
    document.Normalise();
    document.state = CPackIndexDocument::StateComplete;
    document.producedUtc = CORE::CDateTime::NowUTCDateTime().ToIso8601DateTimeString( true, true );
    document.producedHost = CORE::GetHostname();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
