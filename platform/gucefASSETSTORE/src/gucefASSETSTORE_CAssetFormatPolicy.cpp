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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefASSETSTORE_CAssetFormatPolicy.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetFormatPolicy::CAssetFormatPolicy( void )
    : CORE::CIDataNodeSerializable()
    , preference( GetDefaultPreference() )
    , onlyPreferred( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetFormatPolicy::~CAssetFormatPolicy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CAssetFormatPolicy::GetDefaultPreference( void )
{GUCEF_TRACE;

    CORE::CString::StringVector list;
    list.push_back( "gltf" );
    list.push_back( "glb" );
    list.push_back( "fbx" );
    list.push_back( "obj" );
    list.push_back( "usd" );
    return list;
}

/*-------------------------------------------------------------------------*/

bool
CAssetFormatPolicy::IsEngineExport( const CAssetStoreFormat& format )
{GUCEF_TRACE;

    const CORE::CString text = ( format.formatId + " " + format.name + " " + format.group ).Lowercase();
    return text.HasSubstr( "unreal" ) >= 0 || text.HasSubstr( "uasset" ) >= 0 || text.HasSubstr( "engine" ) >= 0 || 0 == text.HasSubstr( "ue:" );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetFormatPolicy::RankOf( const CAssetStoreFormat& format ) const
{GUCEF_TRACE;

    const CORE::CString id = format.formatId.Lowercase();
    const CORE::CString name = format.name.Lowercase();
    for ( size_t p=0; p<preference.size(); ++p )
    {
        const CORE::CString wanted = preference[ p ].Lowercase();
        if ( id == wanted || name == wanted )
            return (UInt32) p;
    }
    return IsEngineExport( format ) ? (UInt32) preference.size() + 1 : (UInt32) preference.size();
}

/*-------------------------------------------------------------------------*/

bool
CAssetFormatPolicy::IsPreferred( const CAssetStoreFormat& format ) const
{GUCEF_TRACE;

    return RankOf( format ) < (UInt32) preference.size();
}

/*-------------------------------------------------------------------------*/

void
CAssetFormatPolicy::RankFormats( const CAssetStoreItem& item, CORE::CString::StringVector& formatIds ) const
{GUCEF_TRACE;

    formatIds.clear();
    GUCEF::vector< UInt32 > ranks;
    GUCEF::vector< size_t > order;
    for ( size_t f=0; f<item.formats.size(); ++f )
    {
        if ( item.formats[ f ].formatId.IsNULLOrEmpty() )
            continue;
        bool seen = false;
        for ( size_t o=0; o<order.size() && !seen; ++o )
            seen = item.formats[ order[ o ] ].formatId == item.formats[ f ].formatId;
        if ( seen )
            continue;
        const UInt32 rank = RankOf( item.formats[ f ] );
        if ( onlyPreferred && rank >= (UInt32) preference.size() )
            continue;
        // stable insertion by rank
        size_t at = order.size();
        while ( at > 0 && ranks[ at - 1 ] > rank )
            --at;
        order.insert( order.begin() + at, f );
        ranks.insert( ranks.begin() + at, rank );
    }
    for ( size_t o=0; o<order.size(); ++o )
        formatIds.push_back( item.formats[ order[ o ] ].formatId );
}

/*-------------------------------------------------------------------------*/

bool
CAssetFormatPolicy::Serialize( CORE::CDataNode& domRootNode                        ,
                               const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetValuesOfChildByName( "preference", preference );
    success = domRootNode.SetAttribute( "onlyPreferred", onlyPreferred ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetFormatPolicy::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                 const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    if ( GUCEF_NULL != domRootNode.FindChild( "preference" ) )
    {
        preference.clear();
        domRootNode.GetValuesOfChildByName( "preference", preference );
    }
    onlyPreferred = domRootNode.GetAttributeValueOrChildValueByName( "onlyPreferred" ).AsBool( onlyPreferred, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetFormatPolicy::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetFormatPolicy( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetFormatPolicy::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetFormatPolicy";
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
