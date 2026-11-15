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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "AssetConverterLib_CAssetCacheEntry.h"

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

CAssetCacheEntry::CAssetCacheEntry( void )
    : assetPath()
    , key()
    , sourceHash()
    , settingsHash()
    , plugins()
    , dependencies()
    , files()
    , cookPending( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetCacheEntry::SerializeParts( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "source", sourceHash );
    node.SetAttribute( "settings", settingsHash );
    CORE::CDataNode* pluginNode = node.AddChild( "plugins", GUCEF_DATATYPE_OBJECT );
    TPluginVersionMap::const_iterator p = plugins.begin();
    while ( p != plugins.end() )
    {
        pluginNode->SetAttribute( (*p).first, (*p).second );
        ++p;
    }
    CORE::CDataNode* dependencyNode = node.AddChild( "dependencies", GUCEF_DATATYPE_OBJECT );
    THashMap::const_iterator d = dependencies.begin();
    while ( d != dependencies.end() )
    {
        dependencyNode->SetAttribute( (*d).first, (*d).second );
        ++d;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetCacheEntry::GetKeyOfParts( void ) const
{GUCEF_TRACE;

    CORE::CDataNode parts( "parts", GUCEF_DATATYPE_OBJECT );
    SerializeParts( parts );
    return CCanonicalJson::Hash( parts );
}

/*-------------------------------------------------------------------------*/

void
CAssetCacheEntry::ComputeKey( void )
{GUCEF_TRACE;

    key = GetKeyOfParts();
}

/*-------------------------------------------------------------------------*/

TActivityReason
CAssetCacheEntry::CompareWith( const CAssetCacheEntry& current, CORE::CString& detail ) const
{GUCEF_TRACE;

    detail.Clear();
    if ( GetKeyOfParts() == current.GetKeyOfParts() )
        return ACTIVITYREASON_UNCHANGED;
    if ( sourceHash != current.sourceHash )
        return ACTIVITYREASON_SOURCE_CHANGED;
    if ( settingsHash != current.settingsHash )
        return ACTIVITYREASON_SETTINGS_CHANGED;

    TPluginVersionMap::const_iterator p = current.plugins.begin();
    while ( p != current.plugins.end() )
    {
        TPluginVersionMap::const_iterator stored = plugins.find( (*p).first );
        if ( stored == plugins.end() )
        {
            detail = (*p).first + " added";
            return ACTIVITYREASON_VERSION_CHANGED;
        }
        if ( (*stored).second != (*p).second )
        {
            detail = (*p).first + " " + CORE::UInt32ToString( (*stored).second ) + " -> " + CORE::UInt32ToString( (*p).second );
            return ACTIVITYREASON_VERSION_CHANGED;
        }
        ++p;
    }
    p = plugins.begin();
    while ( p != plugins.end() )
    {
        if ( current.plugins.find( (*p).first ) == current.plugins.end() )
        {
            detail = (*p).first + " removed";
            return ACTIVITYREASON_VERSION_CHANGED;
        }
        ++p;
    }

    THashMap::const_iterator d = current.dependencies.begin();
    while ( d != current.dependencies.end() )
    {
        THashMap::const_iterator stored = dependencies.find( (*d).first );
        if ( stored == dependencies.end() || (*stored).second != (*d).second )
        {
            detail = (*d).first;
            return ACTIVITYREASON_DEPENDENCY_CHANGED;
        }
        ++d;
    }
    d = dependencies.begin();
    while ( d != dependencies.end() )
    {
        if ( current.dependencies.find( (*d).first ) == current.dependencies.end() )
        {
            detail = (*d).first;
            return ACTIVITYREASON_DEPENDENCY_CHANGED;
        }
        ++d;
    }
    return ACTIVITYREASON_UNCHANGED;
}

/*-------------------------------------------------------------------------*/

bool
CAssetCacheEntry::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "asset", assetPath );
    node.SetAttribute( "key", key );
    CORE::CDataNode* parts = node.AddChild( "parts", GUCEF_DATATYPE_OBJECT );
    SerializeParts( *parts );
    CORE::CDataNode* fileList = node.AddChild( "files", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<files.size(); ++i )
        files[ i ].Serialize( *fileList->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    if ( cookPending )
        node.SetAttribute( "cookPending", true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetCacheEntry::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CAssetCacheEntry entry;
    entry.assetPath = node.GetAttributeValueOrChildValueByName( "asset" ).AsString();
    entry.key = node.GetAttributeValueOrChildValueByName( "key" ).AsString();
    const CORE::CDataNode* parts = node.FindChild( "parts" );
    if ( GUCEF_NULL == parts || entry.key.IsNULLOrEmpty() )
        return false;
    entry.sourceHash = parts->GetAttributeValueOrChildValueByName( "source" ).AsString();
    entry.settingsHash = parts->GetAttributeValueOrChildValueByName( "settings" ).AsString();
    const CORE::CDataNode* pluginNode = parts->FindChild( "plugins" );
    if ( GUCEF_NULL != pluginNode )
    {
        CORE::CDataNode::TAttributeMap::const_iterator a = pluginNode->AttributeBegin();
        while ( a != pluginNode->AttributeEnd() )
        {
            entry.plugins[ (*a).first ] = (*a).second.AsUInt32( 0 );
            ++a;
        }
    }
    const CORE::CDataNode* dependencyNode = parts->FindChild( "dependencies" );
    if ( GUCEF_NULL != dependencyNode )
    {
        CORE::CDataNode::TAttributeMap::const_iterator a = dependencyNode->AttributeBegin();
        while ( a != dependencyNode->AttributeEnd() )
        {
            entry.dependencies[ (*a).first ] = (*a).second.AsString();
            ++a;
        }
    }
    const CORE::CDataNode* fileList = node.FindChild( "files" );
    if ( GUCEF_NULL != fileList )
    {
        CORE::CDataNode::const_iterator i = fileList->ConstBegin();
        while ( i != fileList->ConstEnd() )
        {
            CConversionReportFile file;
            if ( file.Deserialize( *(*i) ) )
                entry.files.push_back( file );
            ++i;
        }
    }
    entry.cookPending = node.GetAttributeValueOrChildValueByName( "cookPending" ).AsBool( false, true );
    *this = entry;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetCacheEntry::Load( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CDataNode node;
    if ( !CCanonicalJson::Load( vfsPath, node ) )
        return false;
    return Deserialize( node );
}

/*-------------------------------------------------------------------------*/

bool
CAssetCacheEntry::Save( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    CORE::CDataNode node( "cache", GUCEF_DATATYPE_OBJECT );
    Serialize( node );

    // atomically where the archive can rename (a folder of the file system): a reader never sees half an entry
    const CORE::CString temporaryPath = vfsPath + ".tmp";
    if ( CCanonicalJson::Store( temporaryPath, node ) )
    {
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        if ( vfs.MoveFileAtomically( temporaryPath, vfsPath, true ) )
            return true;
        vfs.DeleteFile( temporaryPath, true );
    }
    return CCanonicalJson::Store( vfsPath, node );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetCacheEntry::GetEntryPath( const CORE::CString& intermediateDir, const CORE::CString& assetPath )
{GUCEF_TRACE;

    return CVfsPaths::Join( CVfsPaths::Join( intermediateDir, ".cache" ), assetPath + ".cache.json" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
