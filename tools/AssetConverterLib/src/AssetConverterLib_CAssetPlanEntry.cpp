/*
 *  AssetConverterLib: what a run would do with one asset of a pack
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

#include "AssetConverterLib_CAssetPlanEntry.h"

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

CAssetPlanEntry::CAssetPlanEntry( void )
    : assetPath()
    , importer()
    , action( PLANACTION_IMPORT )
    , reason( ACTIVITYREASON_NEW )
    , detail()
    , settings()
    , layers()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TActivityAction
CAssetPlanEntry::GetExpectedActivityAction( void ) const
{GUCEF_TRACE;

    switch ( action )
    {
        case PLANACTION_CACHED:
        case PLANACTION_COOK:       { return ACTIVITYACTION_CACHED; }
        case PLANACTION_REFUSED:    { return ACTIVITYACTION_REFUSED; }
        case PLANACTION_FAILED:     { return ACTIVITYACTION_FAILED; }
        case PLANACTION_IMPORT:
        case PLANACTION_VALIDATE:
        case PLANACTION_NO_INTERMEDIATE:
        default:                    { return ACTIVITYACTION_IMPORTED; }
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetPlanEntry::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "asset", assetPath );
    node.SetAttribute( "importer", importer );
    node.SetAttribute( "action", ToString( action ) );
    node.SetAttribute( "reason", ToString( reason ) );
    if ( !detail.IsNULLOrEmpty() )
        node.SetAttribute( "detail", detail );
    CORE::CDataNode* settingsNode = node.AddChild( settings );
    if ( GUCEF_NULL != settingsNode )
    {
        settingsNode->SetName( "settings" );
        if ( GUCEF_DATATYPE_OBJECT != settingsNode->GetNodeType() && !settingsNode->HasChildren() )
            settingsNode->SetNodeType( GUCEF_DATATYPE_OBJECT );
    }
    node.SetValuesOfChildByName( "layers", layers );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPlanEntry::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CAssetPlanEntry entry;
    entry.assetPath = node.GetAttributeValueOrChildValueByName( "asset" ).AsString();
    if ( entry.assetPath.IsNULLOrEmpty() )
        return false;
    entry.importer = node.GetAttributeValueOrChildValueByName( "importer" ).AsString();
    if ( !FromString( node.GetAttributeValueOrChildValueByName( "action" ).AsString(), entry.action ) )
        return false;
    FromString( node.GetAttributeValueOrChildValueByName( "reason" ).AsString(), entry.reason );
    entry.detail = node.GetAttributeValueOrChildValueByName( "detail" ).AsString();
    const CORE::CDataNode* settingsNode = node.FindChild( "settings" );
    if ( GUCEF_NULL != settingsNode )
        entry.settings = *settingsNode;
    node.GetValuesOfChildByName( "layers", entry.layers );
    *this = entry;
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
