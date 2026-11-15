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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#include "gucefASSETSTORE_CAssetRootedPath.h"
#define GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#endif /* GUCEF_ASSETSTORE_CASSETROOTEDPATH_H ? */

#include "gucefASSETSTORE_CAssetToolRoot.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetToolRoot::KindSource = "source";
const CORE::CString CAssetToolRoot::KindStaging = "staging";
const CORE::CString CAssetToolRoot::KindLicences = "licences";
const CORE::CString CAssetToolRoot::KindManifests = "manifests";
const CORE::CString CAssetToolRoot::KindOverrides = "overrides";
const CORE::CString CAssetToolRoot::KindIntermediate = "intermediate";
const CORE::CString CAssetToolRoot::KindOutput = "output";
const CORE::CString CAssetToolRoot::KindWork = "work";
const CORE::CString CAssetToolRoot::KindCaptures = "captures";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetToolRoot::CAssetToolRoot( void )
    : CORE::CIDataNodeSerializable()
    , id()
    , vfsPath()
    , diskPath()
    , kind()
    , writable( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolRoot::~CAssetToolRoot()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetToolRoot::IsKnownKind( const CORE::CString& kindName )
{GUCEF_TRACE;

    return KindSource == kindName || KindStaging == kindName || KindLicences == kindName || KindManifests == kindName ||
           KindOverrides == kindName || KindIntermediate == kindName || KindOutput == kindName || KindWork == kindName ||
           KindCaptures == kindName;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolRoot::AllowsWrites( void ) const
{GUCEF_TRACE;

    // sources and licence records are never written through the tools, whatever the settings say
    if ( KindSource == kind || KindLicences == kind || KindStaging == kind )
        return false;
    return writable;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolRoot::IsValid( CORE::CString& problem ) const
{GUCEF_TRACE;

    if ( !CAssetRootedPath::IsValidRootId( id ) )
    {
        problem = "the root id \"" + id + "\" does not match [a-z][a-z0-9-]{0,31}";
        return false;
    }
    if ( vfsPath.IsNULLOrEmpty() )
    {
        problem = "the root " + id + " names no folder";
        return false;
    }
    if ( !IsKnownKind( kind ) )
    {
        problem = "the root " + id + " has the unknown kind \"" + kind + "\"";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CAssetToolRoot*
CAssetToolRoot::Find( const TRootVector& roots, const CORE::CString& rootId )
{GUCEF_TRACE;

    for ( size_t i=0; i<roots.size(); ++i )
        if ( roots[ i ].id == rootId )
            return &roots[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CAssetToolRoot*
CAssetToolRoot::FindFirstOfKind( const TRootVector& roots, const CORE::CString& kindName )
{GUCEF_TRACE;

    for ( size_t i=0; i<roots.size(); ++i )
        if ( roots[ i ].kind == kindName )
            return &roots[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CAssetToolRoot*
CAssetToolRoot::FindHolder( const TRootVector& roots, const CORE::CString& path, const CORE::CString& kinds, CORE::CString& relativePath )
{GUCEF_TRACE;

    const CORE::CString::StringVector kindList = kinds.ParseElements( '|', false );
    const CAssetToolRoot* best = GUCEF_NULL;
    UInt32 bestLength = 0;
    for ( size_t i=0; i<roots.size(); ++i )
    {
        if ( !kindList.empty() )
        {
            bool match = false;
            for ( size_t k=0; k<kindList.size() && !match; ++k )
                match = kindList[ k ] == roots[ i ].kind;
            if ( !match )
                continue;
        }
        CAssetRootedPath rooted;
        if ( CAssetRootedPath::FromVfsPath( roots[ i ], path, rooted ) )
        {
            const UInt32 length = CAssetRootedPath::NormalizeVfsPath( roots[ i ].vfsPath ).Length();
            if ( GUCEF_NULL == best || length > bestLength )
            {
                best = &roots[ i ];
                bestLength = length;
                relativePath = rooted.relativePath;
            }
        }
    }
    return best;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolRoot::LoadList( const CORE::CDataNode& arrayNode, TRootVector& roots, CORE::CString& problem )
{GUCEF_TRACE;

    roots.clear();
    CORE::CDataNode::const_iterator i = arrayNode.ConstBegin();
    while ( i != arrayNode.ConstEnd() )
    {
        CAssetToolRoot root;
        CORE::CDataNodeSerializableSettings settings;
        if ( !root.Deserialize( *(*i), settings ) || !root.IsValid( problem ) )
        {
            if ( problem.IsNULLOrEmpty() )
                problem = "a root cannot be read";
            roots.clear();
            return false;
        }
        if ( GUCEF_NULL != Find( roots, root.id ) )
        {
            problem = "the root id " + root.id + " is used twice";
            roots.clear();
            return false;
        }
        roots.push_back( root );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetToolRoot::SaveList( const TRootVector& roots, CORE::CDataNode& arrayNode )
{GUCEF_TRACE;

    // Clear() also takes the name: a list written into a named child keeps its name
    const CORE::CString name = arrayNode.GetName();
    arrayNode.Clear();
    arrayNode.SetName( name );
    arrayNode.SetNodeType( GUCEF_DATATYPE_ARRAY );
    CORE::CDataNodeSerializableSettings settings;
    for ( size_t i=0; i<roots.size(); ++i )
        roots[ i ].Serialize( *arrayNode.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ), settings );
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolRoot::Serialize( CORE::CDataNode& domRootNode                        ,
                           const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "id", id ) && success;
    success = domRootNode.SetAttribute( "vfsPath", vfsPath ) && success;
    if ( !diskPath.IsNULLOrEmpty() )
        success = domRootNode.SetAttribute( "diskPath", diskPath ) && success;
    success = domRootNode.SetAttribute( "kind", kind ) && success;
    success = domRootNode.SetAttribute( "writable", writable ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolRoot::Deserialize( const CORE::CDataNode& domRootNode                  ,
                             const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    *this = CAssetToolRoot();
    id = domRootNode.GetAttributeValueOrChildValueByName( "id" ).AsString( id, true );
    vfsPath = domRootNode.GetAttributeValueOrChildValueByName( "vfsPath" ).AsString( vfsPath, true );
    diskPath = domRootNode.GetAttributeValueOrChildValueByName( "diskPath" ).AsString( diskPath, true );
    kind = domRootNode.GetAttributeValueOrChildValueByName( "kind" ).AsString( kind, true );
    writable = domRootNode.GetAttributeValueOrChildValueByName( "writable" ).AsBool( writable, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetToolRoot::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetToolRoot( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetToolRoot::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetToolRoot";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
