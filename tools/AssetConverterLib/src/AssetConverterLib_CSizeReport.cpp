/*
 *  AssetConverterLib: the size report of the cooked files of a profile
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

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CSizeReport.h"

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

CSizeReport::CSizeReport( void )
    : m_profile()
    , m_fileSizes()
    , m_assets()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CSizeReport::Build( const CReferenceGraph& graph, const CORE::CString& profileDir )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    TFileSizeMap sizes;
    CReferenceGraph::TFileMap::const_iterator i = graph.GetFiles().begin();
    while ( i != graph.GetFiles().end() )
    {
        CORE::CString::StringSet closure;
        graph.GetClosure( (*i).first, closure );
        CORE::CString::StringSet::const_iterator f = closure.begin();
        while ( f != closure.end() )
        {
            if ( sizes.find( *f ) == sizes.end() )
            {
                const CORE::CString path = CVfsPaths::Join( profileDir, *f );
                sizes[ *f ] = vfs.FileExists( path ) ? vfs.GetFileSize( path ) : 0;
            }
            ++f;
        }
        ++i;
    }
    Build( graph, sizes );
}

/*-------------------------------------------------------------------------*/

void
CSizeReport::Build( const CReferenceGraph& graph, const TFileSizeMap& fileSizes )
{GUCEF_TRACE;

    m_profile = graph.GetProfile();
    m_fileSizes.clear();
    m_assets.clear();

    // the files of every asset: those cooked for it and all they reference
    typedef GUCEF::map< CORE::CString, CORE::CString::StringSet > TAssetFiles;
    TAssetFiles assetFiles;
    CReferenceGraph::TFileMap::const_iterator i = graph.GetFiles().begin();
    while ( i != graph.GetFiles().end() )
    {
        graph.GetClosure( (*i).first, assetFiles[ (*i).second.asset ] );
        ++i;
    }

    // how many assets have each file
    GUCEF::map< CORE::CString, UInt32 > owners;
    TAssetFiles::const_iterator a = assetFiles.begin();
    while ( a != assetFiles.end() )
    {
        CORE::CString::StringSet::const_iterator f = (*a).second.begin();
        while ( f != (*a).second.end() )
        {
            ++owners[ *f ];
            TFileSizeMap::const_iterator size = fileSizes.find( *f );
            m_fileSizes[ *f ] = size == fileSizes.end() ? 0 : (*size).second;
            ++f;
        }
        ++a;
    }

    a = assetFiles.begin();
    while ( a != assetFiles.end() )
    {
        SAsset asset;
        asset.exclusiveSize = 0;
        asset.sharedSize = 0;
        asset.fileCount = (UInt32) (*a).second.size();
        CORE::CString::StringSet::const_iterator f = (*a).second.begin();
        while ( f != (*a).second.end() )
        {
            if ( owners[ *f ] > 1 )
                asset.sharedSize += m_fileSizes[ *f ];
            else
                asset.exclusiveSize += m_fileSizes[ *f ];
            ++f;
        }
        m_assets[ (*a).first ] = asset;
        ++a;
    }
}

/*-------------------------------------------------------------------------*/

const CSizeReport::TFileSizeMap& CSizeReport::GetFileSizes( void ) const { return m_fileSizes; }
const CSizeReport::TAssetMap& CSizeReport::GetAssets( void ) const { return m_assets; }

/*-------------------------------------------------------------------------*/

bool
CSizeReport::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "profile", m_profile );
    CORE::CDataNode* files = node.AddChild( "files", GUCEF_DATATYPE_ARRAY );
    TFileSizeMap::const_iterator f = m_fileSizes.begin();
    while ( f != m_fileSizes.end() )
    {
        CORE::CDataNode* file = files->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        file->SetAttribute( "file", (*f).first );
        file->SetAttribute( "size", (*f).second );
        ++f;
    }
    CORE::CDataNode* assets = node.AddChild( "assets", GUCEF_DATATYPE_ARRAY );
    TAssetMap::const_iterator a = m_assets.begin();
    while ( a != m_assets.end() )
    {
        CORE::CDataNode* asset = assets->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        asset->SetAttribute( "asset", (*a).first );
        asset->SetAttribute( "exclusiveSize", (*a).second.exclusiveSize );
        asset->SetAttribute( "sharedSize", (*a).second.sharedSize );
        asset->SetAttribute( "inclusiveSize", (*a).second.exclusiveSize + (*a).second.sharedSize );
        asset->SetAttribute( "files", (*a).second.fileCount );
        ++a;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSizeReport::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    m_profile = node.GetAttributeValueOrChildValueByName( "profile" ).AsString();
    m_fileSizes.clear();
    m_assets.clear();
    const CORE::CDataNode* files = node.FindChild( "files" );
    if ( GUCEF_NULL != files )
    {
        CORE::CDataNode::const_iterator i = files->ConstBegin();
        while ( i != files->ConstEnd() )
        {
            m_fileSizes[ (*i)->GetAttributeValueOrChildValueByName( "file" ).AsString() ] = (*i)->GetAttributeValueOrChildValueByName( "size" ).AsUInt64( 0 );
            ++i;
        }
    }
    const CORE::CDataNode* assets = node.FindChild( "assets" );
    if ( GUCEF_NULL != assets )
    {
        CORE::CDataNode::const_iterator i = assets->ConstBegin();
        while ( i != assets->ConstEnd() )
        {
            SAsset asset;
            asset.exclusiveSize = (*i)->GetAttributeValueOrChildValueByName( "exclusiveSize" ).AsUInt64( 0 );
            asset.sharedSize = (*i)->GetAttributeValueOrChildValueByName( "sharedSize" ).AsUInt64( 0 );
            asset.fileCount = (*i)->GetAttributeValueOrChildValueByName( "files" ).AsUInt32( 0 );
            m_assets[ (*i)->GetAttributeValueOrChildValueByName( "asset" ).AsString() ] = asset;
            ++i;
        }
    }
    return GUCEF_NULL != files;
}

/*-------------------------------------------------------------------------*/

bool
CSizeReport::Load( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CDataNode node;
    if ( !CCanonicalJson::Load( vfsPath, node ) )
        return false;
    return Deserialize( node );
}

/*-------------------------------------------------------------------------*/

bool
CSizeReport::Save( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    CORE::CDataNode node( "sizes", GUCEF_DATATYPE_OBJECT );
    Serialize( node );
    return CCanonicalJson::Store( vfsPath, node );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
