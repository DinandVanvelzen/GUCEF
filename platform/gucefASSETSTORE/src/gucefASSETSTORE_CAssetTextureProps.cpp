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
#include "gucefASSETSTORE_CAssetTextureProps.h"

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

CAssetTextureProps::CAssetTextureProps( void )
    : CORE::CIDataNodeSerializable()
    , isSet( false )
    , width( 0 )
    , height( 0 )
    , channels( 0 )
    , bitDepth( 0 )
    , colourSpace()
    , compression()
    , role()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetTextureProps::~CAssetTextureProps()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetTextureProps::Clear( void )
{GUCEF_TRACE;

    *this = CAssetTextureProps();
}

/*-------------------------------------------------------------------------*/

bool
CAssetTextureProps::Serialize( CORE::CDataNode& domRootNode                        ,
                              const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "isSet", isSet ) && success;
    success = domRootNode.SetAttribute( "width", width ) && success;
    success = domRootNode.SetAttribute( "height", height ) && success;
    success = domRootNode.SetAttribute( "channels", channels ) && success;
    success = domRootNode.SetAttribute( "bitDepth", bitDepth ) && success;
    success = domRootNode.SetAttribute( "colourSpace", colourSpace ) && success;
    success = domRootNode.SetAttribute( "compression", compression ) && success;
    success = domRootNode.SetAttribute( "role", role ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetTextureProps::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetTextureProps defaults;
    isSet = domRootNode.GetAttributeValueOrChildValueByName( "isSet" ).AsBool( defaults.isSet, true );
    width = domRootNode.GetAttributeValueOrChildValueByName( "width" ).AsInt64( defaults.width, true );
    height = domRootNode.GetAttributeValueOrChildValueByName( "height" ).AsInt64( defaults.height, true );
    channels = domRootNode.GetAttributeValueOrChildValueByName( "channels" ).AsInt64( defaults.channels, true );
    bitDepth = domRootNode.GetAttributeValueOrChildValueByName( "bitDepth" ).AsInt64( defaults.bitDepth, true );
    colourSpace = domRootNode.GetAttributeValueOrChildValueByName( "colourSpace" ).AsString( defaults.colourSpace );
    compression = domRootNode.GetAttributeValueOrChildValueByName( "compression" ).AsString( defaults.compression );
    role = domRootNode.GetAttributeValueOrChildValueByName( "role" ).AsString( defaults.role );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetTextureProps::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetTextureProps( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetTextureProps::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetTextureProps";
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
