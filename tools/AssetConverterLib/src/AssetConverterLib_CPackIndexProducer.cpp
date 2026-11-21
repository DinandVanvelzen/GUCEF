/*
 *  AssetConverterLib: which tools and versions produced a pack index
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#include "AssetConverterLib_CPackIndexSerial.h"
#define ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXSERIAL_H ? */

#include "AssetConverterLib_CPackIndexProducer.h"

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

CPackIndexProducer::CPackIndexProducer( void )
    : tool()
    , toolVersion()
    , schemaWriter( 0 )
    , indexKey()
    , settingsHash()
    , plugins()
    , kindRegistryVersion( 0 )
    , kindRegistryHash()
    , groupRulesVersion( 0 )
    , groupRulesHash()
    , outputRoot()
    , derivedFromCatalogue( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPackIndexProducer::operator==( const CPackIndexProducer& other ) const
{GUCEF_TRACE;

    return tool == other.tool && toolVersion == other.toolVersion && schemaWriter == other.schemaWriter && indexKey == other.indexKey &&
           settingsHash == other.settingsHash && plugins == other.plugins && kindRegistryVersion == other.kindRegistryVersion &&
           kindRegistryHash == other.kindRegistryHash && groupRulesVersion == other.groupRulesVersion && groupRulesHash == other.groupRulesHash &&
           outputRoot == other.outputRoot && derivedFromCatalogue == other.derivedFromCatalogue;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexProducer::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CPackIndexSerial::SetTextAlways( node, "tool", tool );
    CPackIndexSerial::SetText( node, "toolVersion", toolVersion );
    node.SetAttribute( "schemaWriter", schemaWriter );
    CPackIndexSerial::SetTextAlways( node, "indexKey", indexKey );
    CPackIndexSerial::SetTextAlways( node, "settingsHash", settingsHash );
    CORE::CDataNode* pluginNode = node.AddChild( "plugins", GUCEF_DATATYPE_OBJECT );
    TPluginVersionMap::const_iterator p = plugins.begin();
    while ( p != plugins.end() )
    {
        pluginNode->SetAttribute( (*p).first, (*p).second );
        ++p;
    }
    node.SetAttribute( "kindRegistryVersion", kindRegistryVersion );
    CPackIndexSerial::SetText( node, "kindRegistryHash", kindRegistryHash );
    node.SetAttribute( "groupRulesVersion", groupRulesVersion );
    CPackIndexSerial::SetText( node, "groupRulesHash", groupRulesHash );
    CPackIndexSerial::SetText( node, "outputRoot", outputRoot );
    CPackIndexSerial::SetFlag( node, "derivedFromCatalogue", derivedFromCatalogue );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexProducer::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    *this = CPackIndexProducer();
    tool = CPackIndexSerial::GetText( node, "tool" );
    toolVersion = CPackIndexSerial::GetText( node, "toolVersion" );
    schemaWriter = (UInt32) CPackIndexSerial::GetCount( node, "schemaWriter" );
    indexKey = CPackIndexSerial::GetText( node, "indexKey" );
    settingsHash = CPackIndexSerial::GetText( node, "settingsHash" );
    const CORE::CDataNode* pluginNode = node.FindChild( "plugins" );
    if ( GUCEF_NULL != pluginNode )
    {
        CORE::CDataNode::TAttributeMap::const_iterator p = pluginNode->AttributeBegin();
        while ( p != pluginNode->AttributeEnd() )
        {
            plugins[ (*p).first ] = (*p).second.AsUInt32( 0, true );
            ++p;
        }
    }
    kindRegistryVersion = (UInt32) CPackIndexSerial::GetCount( node, "kindRegistryVersion" );
    kindRegistryHash = CPackIndexSerial::GetText( node, "kindRegistryHash" );
    groupRulesVersion = (UInt32) CPackIndexSerial::GetCount( node, "groupRulesVersion" );
    groupRulesHash = CPackIndexSerial::GetText( node, "groupRulesHash" );
    outputRoot = CPackIndexSerial::GetText( node, "outputRoot" );
    derivedFromCatalogue = CPackIndexSerial::GetFlag( node, "derivedFromCatalogue" );
    return !indexKey.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
