/*
 *  AssetWorkbenchLib: what the inspector shows of a folder or a recognised pack: engine, version, counts per kind, size and licence
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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#include "AssetConverterLib_CAssetConverterGlobal.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#include "AssetConverterLib_CAssetPluginRegistry.h"
#define ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETSTOREINSPECTORDATA_H
#include "AssetWorkbenchLib_CAssetStoreInspectorData.h"
#define ASSETWORKBENCHLIB_CASSETSTOREINSPECTORDATA_H
#endif /* ASSETWORKBENCHLIB_CASSETSTOREINSPECTORDATA_H ? */

#include "AssetWorkbenchLib_CAssetPackSummary.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetPackSummary::CAssetPackSummary( void )
    : m_folder()
    , m_profile()
    , m_fileCount( 0 )
    , m_metadataCount( 0 )
    , m_totalSize( 0 )
    , m_truncated( false )
    , m_hasLicence( false )
    , m_licence()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CAssetPackSummary::Clear( void )
{GUCEF_TRACE;

    m_folder.Clear();
    m_profile.Clear();
    m_listing.Clear();
    for ( UInt32 i=0; i<(UInt32) ASSETCONVERTER::ASSETKIND_COUNT; ++i )
        m_kindCounts[ i ] = 0;
    m_fileCount = 0;
    m_metadataCount = 0;
    m_totalSize = 0;
    m_truncated = false;
    m_hasLicence = false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackSummary::Read( const CORE::CString& vfsFolder, UInt32 maxFiles )
{GUCEF_TRACE;

    Clear();
    ASSETCONVERTER::CAssetConverterGlobal* global = ASSETCONVERTER::CAssetConverterGlobal::Instance();
    if ( GUCEF_NULL == global )
        return false;
    m_folder = vfsFolder;
    ASSETCONVERTER::CAssetPackListing& listing = m_listing;
    if ( !listing.LoadFromVfs( vfsFolder, maxFiles ) )
        return false;
    m_truncated = listing.GetFileCount() >= maxFiles;
    ASSETCONVERTER::CAssetPackProfile profile;
    if ( global->GetPluginRegistry().RecognisePack( listing, profile ) )
        m_profile = profile;

    const ASSETCONVERTER::CAssetKindRegistry& kinds = global->GetPluginRegistry().GetKinds();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString::StringVector& files = listing.GetFiles();
    m_fileCount = (UInt32) files.size();
    for ( size_t i=0; i<files.size(); ++i )
    {
        const ASSETCONVERTER::EAssetKind kind = kinds.GetKindOfPath( files[ i ] );
        ++m_kindCounts[ kind ];
        if ( ASSETCONVERTER::ASSETKIND_METADATA == kind )
            ++m_metadataCount;
        m_totalSize += vfs.GetFileSize( ASSETCONVERTER::CVfsPaths::Join( listing.GetVfsRoot(), files[ i ] ) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetPackSummary::SetLicence( const ASSETSTORE::CAssetLicence& licence )
{GUCEF_TRACE;

    m_licence = licence;
    m_hasLicence = true;
}

/*-------------------------------------------------------------------------*/

bool CAssetPackSummary::IsRecognisedPack( void ) const { return m_profile.IsRecognised(); }
const ASSETCONVERTER::CAssetPackProfile& CAssetPackSummary::GetProfile( void ) const { return m_profile; }
const ASSETCONVERTER::CAssetPackListing& CAssetPackSummary::GetListing( void ) const { return m_listing; }
UInt32 CAssetPackSummary::GetKindCount( ASSETCONVERTER::EAssetKind kind ) const { return (UInt32) kind < (UInt32) ASSETCONVERTER::ASSETKIND_COUNT ? m_kindCounts[ kind ] : 0; }
UInt32 CAssetPackSummary::GetFileCount( void ) const { return m_fileCount; }
UInt32 CAssetPackSummary::GetMetadataCount( void ) const { return m_metadataCount; }
UInt64 CAssetPackSummary::GetTotalSize( void ) const { return m_totalSize; }
bool CAssetPackSummary::IsTruncated( void ) const { return m_truncated; }
const CORE::CString& CAssetPackSummary::GetFolder( void ) const { return m_folder; }

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetPackSummary::FormatSize( UInt64 bytes )
{GUCEF_TRACE;

    // bytes up to a kilobyte, then one decimal: "512 B", "12.5 KB", "3.2 MB"
    if ( bytes < 1024 )
        return CORE::UInt64ToString( bytes ) + " B";
    static const char* const units[] = { "KB", "MB", "GB", "TB" };
    Float64 value = (Float64) bytes / 1024.0;
    UInt32 unit = 0;
    while ( value >= 1024.0 && unit < 3 )
    {
        value /= 1024.0;
        ++unit;
    }
    const UInt32 whole = (UInt32) value;
    const UInt32 tenth = (UInt32) ( ( value - (Float64) whole ) * 10.0 );
    return CORE::UInt32ToString( whole ) + "." + CORE::UInt32ToString( tenth ) + " " + units[ unit ];
}

/*-------------------------------------------------------------------------*/

void
CAssetPackSummary::WriteInspectorData( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    const bool pack = m_profile.IsRecognised();
    node.SetName( pack ? "Pack" : "Folder" );
    node.SetAttribute( "Name", m_folder );
    if ( pack )
    {
        node.SetAttribute( "Type", m_profile.packTypeName );
        if ( !m_profile.engine.IsNULLOrEmpty() )
            node.SetAttribute( "Engine", m_profile.engine );
        node.SetAttribute( "Version", m_profile.engineVersion.IsNULLOrEmpty() ? CORE::CString( "not stated" ) : m_profile.engineVersion );
    }
    node.SetAttribute( "Files", CORE::UInt32ToString( m_fileCount - m_metadataCount ) + ( m_truncated ? CORE::CString( " (the first ones: the folder holds more)" ) : CORE::CString::Empty ) );
    node.SetAttribute( "Size", FormatSize( m_totalSize ) );
    if ( m_metadataCount > 0 )
        node.SetAttribute( "Metadata files", CORE::UInt32ToString( m_metadataCount ) + " (they belong to the assets; the content browser shows them on request)" );

    CORE::CDataNode* kinds = node.AddChild( "Kinds" );
    for ( UInt32 i=0; i<(UInt32) ASSETCONVERTER::ASSETKIND_COUNT; ++i )
    {
        const ASSETCONVERTER::EAssetKind kind = static_cast< ASSETCONVERTER::EAssetKind >( i );
        if ( ASSETCONVERTER::ASSETKIND_METADATA == kind || ASSETCONVERTER::ASSETKIND_FOLDER == kind || 0 == m_kindCounts[ i ] )
            continue;
        kinds->SetAttribute( ASSETCONVERTER::CAssetKindRegistry::GetKindName( kind ), CORE::UInt32ToString( m_kindCounts[ i ] ) );
    }

    if ( pack && !m_profile.showcaseCandidates.empty() )
    {
        CORE::CDataNode* entries = node.AddChild( "Entry points" );
        for ( size_t i=0; i<m_profile.showcaseCandidates.size() && i<5; ++i )
            entries->SetAttribute( "Candidate " + CORE::UInt32ToString( (UInt32) i + 1 ), m_profile.showcaseCandidates[ i ] );
    }
    if ( m_hasLicence )
        CAssetStoreInspectorData::FillLicence( m_licence, node );
    else
        if ( pack )
            node.SetAttribute( "Licence", "no record in the store" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
