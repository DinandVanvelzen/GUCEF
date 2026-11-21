/*
 *  AssetConverterLib: a hidden face set of the intermediate model: the faces of a body that an outfit hides while it is worn
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

#include "AssetConverterLib_CAssetHiddenFaceSet.h"

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

const CORE::CString CAssetHiddenFaceSet::SceneExtrasKey = "gucefHiddenFaceSets";
const CORE::CString CAssetHiddenFaceSet::SubMeshExtrasKey = "gucefHiddenBy";

/*-------------------------------------------------------------------------*/

CAssetHiddenFaceSet::CAssetHiddenFaceSet( void )
    : name()
    , source()
    , garments()
    , active( false )
    , hiddenTriangles( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CAssetHiddenFaceSet::BitOf( UInt32 index )
{GUCEF_TRACE;

    return index < MaxSets ? ( (UInt32) 1 << index ) : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetHiddenFaceSet::GetActiveMask( const THiddenFaceSetVector& sets )
{GUCEF_TRACE;

    UInt32 mask = 0;
    for ( size_t i=0; i<sets.size() && i<MaxSets; ++i )
        if ( sets[ i ].active )
            mask |= BitOf( (UInt32) i );
    return mask;
}

/*-------------------------------------------------------------------------*/

Int32
CAssetHiddenFaceSet::Find( const THiddenFaceSetVector& sets, const CORE::CString& setName )
{GUCEF_TRACE;

    for ( size_t i=0; i<sets.size(); ++i )
        if ( sets[ i ].name == setName )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetHiddenFaceSet::NamesOf( const THiddenFaceSetVector& sets, UInt32 mask, const CORE::CString& separator )
{GUCEF_TRACE;

    CORE::CString names;
    for ( size_t i=0; i<sets.size() && i<MaxSets; ++i )
    {
        if ( 0 == ( mask & BitOf( (UInt32) i ) ) )
            continue;
        if ( !names.IsNULLOrEmpty() )
            names += separator;
        names += sets[ i ].name;
    }
    return names;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetHiddenFaceSet::Activate( THiddenFaceSetVector& sets, const CORE::CString::StringVector& names, CORE::CString::StringVector& unknown )
{GUCEF_TRACE;

    for ( size_t i=0; i<sets.size(); ++i )
        sets[ i ].active = false;
    UInt32 activeCount = 0;
    for ( size_t n=0; n<names.size(); ++n )
    {
        const CORE::CString& setName = names[ n ];
        if ( setName.IsNULLOrEmpty() || setName.Equals( "none", false ) )
            continue;
        const Int32 index = Find( sets, setName );
        if ( index < 0 )
        {
            unknown.push_back( setName );
            continue;
        }
        if ( !sets[ (size_t) index ].active )
            ++activeCount;
        sets[ (size_t) index ].active = true;
    }
    return activeCount;
}

/*-------------------------------------------------------------------------*/

bool
CAssetHiddenFaceSet::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "name", name );
    if ( !source.IsNULLOrEmpty() )
        node.SetAttribute( "source", source );
    node.SetAttribute( "active", active );
    node.SetAttribute( "hiddenTriangles", hiddenTriangles );
    node.SetValuesOfChildByName( "garments", garments );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetHiddenFaceSet::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    *this = CAssetHiddenFaceSet();
    name = node.GetAttributeValueOrChildValueByName( "name" ).AsString();
    source = node.GetAttributeValueOrChildValueByName( "source" ).AsString();
    active = node.GetAttributeValueOrChildValueByName( "active" ).AsBool( false );
    hiddenTriangles = node.GetAttributeValueOrChildValueByName( "hiddenTriangles" ).AsUInt32( 0 );
    node.GetValuesOfChildByName( "garments", garments );
    return !name.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CAssetHiddenFaceSet::SerializeList( const THiddenFaceSetVector& sets, CORE::CDataNode& array )
{GUCEF_TRACE;

    bool ok = true;
    for ( size_t i=0; i<sets.size(); ++i )
        ok = sets[ i ].Serialize( *array.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) ) && ok;
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CAssetHiddenFaceSet::DeserializeList( const CORE::CDataNode& array, THiddenFaceSetVector& sets )
{GUCEF_TRACE;

    sets.clear();
    bool ok = true;
    for ( const CORE::CDataNode* entry = array.GetFirstChild(); GUCEF_NULL != entry; entry = entry->GetNext() )
    {
        CAssetHiddenFaceSet set;
        if ( set.Deserialize( *entry ) && sets.size() < MaxSets )
            sets.push_back( set );
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
