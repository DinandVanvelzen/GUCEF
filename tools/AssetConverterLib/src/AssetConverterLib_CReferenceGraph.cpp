/*
 *  AssetConverterLib: the reference graph of the cooked files of a profile
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
#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CReferenceGraph.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString
RelativeTo( const CORE::CString& folder, const CORE::CString& path )
{
    CORE::CString relative;
    if ( CVfsPaths::MakeRelative( folder, path, relative ) )
        return relative;
    return CVfsPaths::Normalize( path );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CReferenceGraph::CReferenceGraph( void )
    : m_profile()
    , m_files()
    , m_assetFiles()
    , m_relations()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CReferenceGraph::Build( const CConversionReport& report, const CORE::CString& profile, const CORE::CString& profileDir )
{GUCEF_TRACE;

    m_profile = profile;
    m_files.clear();
    m_assetFiles.clear();
    m_relations.clear();
    const CConversionReport::TEntryVector& entries = report.GetEntries();
    for ( size_t e=0; e<entries.size(); ++e )
    {
        for ( size_t f=0; f<entries[ e ].files.size(); ++f )
        {
            const CConversionReportFile& file = entries[ e ].files[ f ];
            if ( file.profile != profile )
                continue;
            const CORE::CString relativePath = RelativeTo( profileDir, file.path );
            m_assetFiles[ entries[ e ].assetPath ].insert( relativePath );
            SFile& node = m_files[ relativePath ];
            if ( node.asset.IsNULLOrEmpty() )
                node.asset = entries[ e ].assetPath;
            for ( size_t r=0; r<file.references.size(); ++r )
            {
                const CORE::CString reference = RelativeTo( profileDir, file.references[ r ] );
                bool known = false;
                for ( size_t k=0; k<node.references.size() && !known; ++k )
                    known = node.references[ k ] == reference;
                if ( !known )
                    node.references.push_back( reference );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString& CReferenceGraph::GetProfile( void ) const { return m_profile; }
const CReferenceGraph::TFileMap& CReferenceGraph::GetFiles( void ) const { return m_files; }
const CReferenceGraph::TAssetRelationMap& CReferenceGraph::GetAssetRelations( void ) const { return m_relations; }

/*-------------------------------------------------------------------------*/

void
CReferenceGraph::GetFilesOfAsset( const CORE::CString& assetPath, CORE::CString::StringVector& files ) const
{GUCEF_TRACE;

    files.clear();
    TAssetFileMap::const_iterator i = m_assetFiles.find( assetPath );
    if ( i == m_assetFiles.end() )
        return;
    files.assign( (*i).second.begin(), (*i).second.end() );
}

/*-------------------------------------------------------------------------*/

void
CReferenceGraph::SetRelations( const CORE::CString& assetPath, const CAssetReference::TReferenceVector& relations )
{GUCEF_TRACE;

    if ( relations.empty() )
    {
        m_relations.erase( assetPath );
        return;
    }
    CAssetReference::TReferenceVector sorted = relations;
    CAssetReference::Normalise( sorted );
    TRelationVector& list = m_relations[ assetPath ];
    list.clear();
    for ( size_t r=0; r<sorted.size(); ++r )
    {
        SRelation relation;
        relation.relation = sorted[ r ];
        if ( sorted[ r ].IsResolved() )
            GetFilesOfAsset( sorted[ r ].target, relation.files );
        list.push_back( relation );
    }
}

/*-------------------------------------------------------------------------*/

void
CReferenceGraph::GetClosure( const CORE::CString& file, CORE::CString::StringSet& closure ) const
{GUCEF_TRACE;

    CORE::CString::StringVector pending;
    pending.push_back( file );
    while ( !pending.empty() )
    {
        const CORE::CString current = pending.back();
        pending.pop_back();
        if ( !closure.insert( current ).second )
            continue;
        TFileMap::const_iterator i = m_files.find( current );
        if ( i == m_files.end() )
            continue;
        for ( size_t r=0; r<(*i).second.references.size(); ++r )
            pending.push_back( (*i).second.references[ r ] );
    }
}

/*-------------------------------------------------------------------------*/

bool
CReferenceGraph::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "profile", m_profile );
    CORE::CDataNode* files = node.AddChild( "files", GUCEF_DATATYPE_ARRAY );
    TFileMap::const_iterator i = m_files.begin();
    while ( i != m_files.end() )
    {
        CORE::CDataNode* file = files->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        file->SetAttribute( "file", (*i).first );
        file->SetAttribute( "asset", (*i).second.asset );
        CORE::CString::StringVector references = (*i).second.references;
        std::sort( references.begin(), references.end() );
        file->SetValuesOfChildByName( "references", references );
        ++i;
    }
    if ( !m_relations.empty() )
    {
        CORE::CDataNode* assets = node.AddChild( "assets", GUCEF_DATATYPE_ARRAY );
        TAssetRelationMap::const_iterator a = m_relations.begin();
        while ( a != m_relations.end() )
        {
            CORE::CDataNode* asset = assets->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            asset->SetAttribute( "asset", (*a).first );
            CORE::CDataNode* relations = asset->AddChild( "relations", GUCEF_DATATYPE_ARRAY );
            for ( size_t r=0; r<(*a).second.size(); ++r )
            {
                CORE::CDataNode* relation = relations->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                (*a).second[ r ].relation.Serialize( *relation );
                if ( !(*a).second[ r ].files.empty() )
                    relation->SetValuesOfChildByName( "files", (*a).second[ r ].files );
            }
            ++a;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CReferenceGraph::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    m_profile = node.GetAttributeValueOrChildValueByName( "profile" ).AsString();
    m_files.clear();
    m_assetFiles.clear();
    m_relations.clear();
    const CORE::CDataNode* files = node.FindChild( "files" );
    if ( GUCEF_NULL == files )
        return false;
    CORE::CDataNode::const_iterator i = files->ConstBegin();
    while ( i != files->ConstEnd() )
    {
        const CORE::CString path = (*i)->GetAttributeValueOrChildValueByName( "file" ).AsString();
        if ( !path.IsNULLOrEmpty() )
        {
            SFile& file = m_files[ path ];
            file.asset = (*i)->GetAttributeValueOrChildValueByName( "asset" ).AsString();
            (*i)->GetValuesOfChildByName( "references", file.references );
            if ( !file.asset.IsNULLOrEmpty() )
                m_assetFiles[ file.asset ].insert( path );
        }
        ++i;
    }
    const CORE::CDataNode* assets = node.FindChild( "assets" );
    if ( GUCEF_NULL != assets )
    {
        for ( const CORE::CDataNode* asset = assets->GetFirstChild(); GUCEF_NULL != asset; asset = asset->GetNext() )
        {
            const CORE::CString assetPath = asset->GetAttributeValueOrChildValueByName( "asset" ).AsString();
            const CORE::CDataNode* relations = asset->FindChild( "relations" );
            if ( assetPath.IsNULLOrEmpty() || GUCEF_NULL == relations )
                continue;
            TRelationVector& list = m_relations[ assetPath ];
            for ( const CORE::CDataNode* entry = relations->GetFirstChild(); GUCEF_NULL != entry; entry = entry->GetNext() )
            {
                SRelation relation;
                if ( !relation.relation.Deserialize( *entry ) )
                    continue;
                entry->GetValuesOfChildByName( "files", relation.files );
                list.push_back( relation );
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CReferenceGraph::Load( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CDataNode node;
    if ( !CCanonicalJson::Load( vfsPath, node ) )
        return false;
    return Deserialize( node );
}

/*-------------------------------------------------------------------------*/

bool
CReferenceGraph::Save( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    CORE::CDataNode node( "references", GUCEF_DATATYPE_OBJECT );
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
