/*
 *  AssetConverterLib: default settings for the assets a glob matches
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

#include "AssetConverterLib_CAssetPreset.h"

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

CAssetPreset::CAssetPreset( void )
    : match( "**" )
    , importer()
    , settings( "settings", GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetPreset::Matches( const CORE::CString& assetPath, const CORE::CString& importerName ) const
{GUCEF_TRACE;

    if ( !importer.IsNULLOrEmpty() && importer.Lowercase() != importerName.Lowercase() )
        return false;
    return CORE::CGlobPattern::IsMatch( match, assetPath, false );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPreset::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "match", match );
    if ( !importer.IsNULLOrEmpty() )
        node.SetAttribute( "importer", importer );
    CORE::CDataNode* child = node.AddChild( settings );
    child->SetName( "settings" );
    child->SetNodeType( GUCEF_DATATYPE_OBJECT );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPreset::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    match = node.GetAttributeValueOrChildValueByName( "match" ).AsString( "**" );
    importer = node.GetAttributeValueOrChildValueByName( "importer" ).AsString();
    const CORE::CDataNode* child = node.FindChild( "settings" );
    settings = GUCEF_NULL != child ? *child : CORE::CDataNode( "settings", GUCEF_DATATYPE_OBJECT );
    settings.SetName( "settings" );
    return !match.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
