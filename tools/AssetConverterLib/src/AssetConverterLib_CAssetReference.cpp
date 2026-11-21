/*
 *  AssetConverterLib: a normalized relation of an asset to another file of its pack
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

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CAssetReference.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetReference::KindUsesMaterial = "usesMaterial";
const CORE::CString CAssetReference::KindUsesTexture = "usesTexture";
const CORE::CString CAssetReference::KindUsesMesh = "usesMesh";
const CORE::CString CAssetReference::KindUsesSkeleton = "usesSkeleton";
const CORE::CString CAssetReference::KindUsesAnimation = "usesAnimation";
const CORE::CString CAssetReference::KindInstancesPrefab = "instancesPrefab";
const CORE::CString CAssetReference::KindInScene = "inScene";
const CORE::CString CAssetReference::KindFitsSkeleton = "fitsSkeleton";
const CORE::CString CAssetReference::KindDependsOn = "dependsOn";
const CORE::CString CAssetReference::KindUsesFile = "usesFile";

const UInt32 CAssetReference::MaxClosureDepth = 8;
const UInt32 CAssetReference::MaxClosureFiles = 4096;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetReference::CAssetReference( void )
    : kind()
    , target()
    , via()
    , optional( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetReference::CAssetReference( const CORE::CString& relationKind, const CORE::CString& relationTarget, const CORE::CString& original, bool isOptional )
    : kind( relationKind )
    , target( relationTarget )
    , via( original )
    , optional( isOptional )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetReference::IsResolved( void ) const
{GUCEF_TRACE;

    return !target.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CAssetReference::operator<( const CAssetReference& other ) const
{GUCEF_TRACE;

    if ( kind != other.kind )
        return kind < other.kind;
    if ( target != other.target )
        return target < other.target;
    if ( via != other.via )
        return via < other.via;
    return !optional && other.optional;
}

/*-------------------------------------------------------------------------*/

bool
CAssetReference::operator==( const CAssetReference& other ) const
{GUCEF_TRACE;

    return kind == other.kind && target == other.target && via == other.via && optional == other.optional;
}

/*-------------------------------------------------------------------------*/

bool
CAssetReference::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "kind", kind );
    if ( !target.IsNULLOrEmpty() )
        node.SetAttribute( "path", target );
    if ( !via.IsNULLOrEmpty() )
        node.SetAttribute( "via", via );
    if ( optional )
        node.SetAttribute( "optional", true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetReference::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    kind = node.GetAttributeValueOrChildValueByName( "kind" ).AsString();
    target = node.GetAttributeValueOrChildValueByName( "path" ).AsString();
    via = node.GetAttributeValueOrChildValueByName( "via" ).AsString();
    optional = node.GetAttributeValueOrChildValueByName( "optional" ).AsBool( false );
    return !kind.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CAssetReference::IsKnownKind( const CORE::CString& kind )
{GUCEF_TRACE;

    return KindUsesMaterial == kind || KindUsesTexture == kind || KindUsesMesh == kind || KindUsesSkeleton == kind || KindUsesAnimation == kind ||
           KindInstancesPrefab == kind || KindInScene == kind || KindFitsSkeleton == kind || KindDependsOn == kind || KindUsesFile == kind;
}

/*-------------------------------------------------------------------------*/

bool
CAssetReference::IsFollowedKind( const CORE::CString& kind )
{GUCEF_TRACE;

    return IsKnownKind( kind ) && KindInScene != kind && KindFitsSkeleton != kind;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetReference::GetKindOfDependency( const CORE::CString& path )
{GUCEF_TRACE;

    const CORE::CString extension = CVfsPaths::ExtensionOf( path ).Lowercase();
    if ( "meta" == extension || "import" == extension )
        return KindUsesFile;
    return KindDependsOn;
}

/*-------------------------------------------------------------------------*/

void
CAssetReference::Normalise( TReferenceVector& references )
{GUCEF_TRACE;

    std::sort( references.begin(), references.end() );
    references.erase( std::unique( references.begin(), references.end() ), references.end() );
}

/*-------------------------------------------------------------------------*/

void
CAssetReference::SerializeList( const TReferenceVector& references, CORE::CDataNode& parent, const char* name )
{GUCEF_TRACE;

    TReferenceVector sorted = references;
    Normalise( sorted );
    CORE::CDataNode* list = parent.AddChild( name, GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<sorted.size(); ++i )
        sorted[ i ].Serialize( *list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
}

/*-------------------------------------------------------------------------*/

bool
CAssetReference::DeserializeList( const CORE::CDataNode& parent, const char* name, TReferenceVector& references )
{GUCEF_TRACE;

    references.clear();
    const CORE::CDataNode* list = parent.FindChild( name );
    if ( GUCEF_NULL == list )
        return true;
    bool ok = true;
    for ( const CORE::CDataNode* entry = list->GetFirstChild(); GUCEF_NULL != entry; entry = entry->GetNext() )
    {
        CAssetReference reference;
        if ( reference.Deserialize( *entry ) )
            references.push_back( reference );
        else
            ok = false;
    }
    return ok;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
