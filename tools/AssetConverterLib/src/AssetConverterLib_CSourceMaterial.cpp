/*
 *  AssetConverterLib: the material of a source as it was: shader and every property
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

#include "AssetConverterLib_CSourceMaterial.h"

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

CSourceMaterial::CSourceMaterial( void )
    : shaderName()
    , properties( "properties", GUCEF_DATATYPE_OBJECT )
    , textures( "textures", GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CSourceMaterial::IsEmpty( void ) const
{GUCEF_TRACE;

    return shaderName.IsNULLOrEmpty() && 0 == properties.GetAttCount() && !properties.HasChildren() &&
           0 == textures.GetAttCount() && !textures.HasChildren();
}

/*-------------------------------------------------------------------------*/

bool
CSourceMaterial::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "shader", shaderName );
    CORE::CDataNode* props = node.AddChild( properties );
    props->SetName( "properties" );
    props->SetNodeType( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* texs = node.AddChild( textures );
    texs->SetName( "textures" );
    texs->SetNodeType( GUCEF_DATATYPE_OBJECT );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSourceMaterial::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    shaderName = node.GetAttributeValueOrChildValueByName( "shader" ).AsString();
    const CORE::CDataNode* props = node.FindChild( "properties" );
    properties = GUCEF_NULL != props ? *props : CORE::CDataNode( "properties", GUCEF_DATATYPE_OBJECT );
    const CORE::CDataNode* texs = node.FindChild( "textures" );
    textures = GUCEF_NULL != texs ? *texs : CORE::CDataNode( "textures", GUCEF_DATATYPE_OBJECT );
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
