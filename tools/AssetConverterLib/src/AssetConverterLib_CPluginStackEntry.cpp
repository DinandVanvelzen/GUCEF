/*
 *  AssetConverterLib: a post-processor or validator named by a manifest, with its settings
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

#include "AssetConverterLib_CPluginStackEntry.h"

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

CPluginStackEntry::CPluginStackEntry( void )
    : name()
    , settings( "settings", GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPluginStackEntry::CPluginStackEntry( const CORE::CString& pluginName )
    : name( pluginName )
    , settings( "settings", GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPluginStackEntry::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "name", name );
    CORE::CDataNode* child = node.AddChild( settings );
    child->SetName( "settings" );
    child->SetNodeType( GUCEF_DATATYPE_OBJECT );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPluginStackEntry::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    settings = CORE::CDataNode( "settings", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_DATATYPE_OBJECT != node.GetNodeType() && GUCEF_DATATYPE_ARRAY != node.GetNodeType() )
    {
        name = node.GetValue().AsString();
        return !name.IsNULLOrEmpty();
    }
    name = node.GetAttributeValueOrChildValueByName( "name" ).AsString();
    const CORE::CDataNode* child = node.FindChild( "settings" );
    if ( GUCEF_NULL != child )
    {
        settings = *child;
        settings.SetName( "settings" );
    }
    return !name.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CPluginStackEntry::DeserializeList( const CORE::CDataNode& arrayNode, TEntryVector& entries )
{GUCEF_TRACE;

    entries.clear();
    bool allRead = true;
    CORE::CDataNode::const_iterator i = arrayNode.ConstBegin();
    while ( i != arrayNode.ConstEnd() )
    {
        CPluginStackEntry entry;
        if ( entry.Deserialize( *(*i) ) )
            entries.push_back( entry );
        else
            allRead = false;
        ++i;
    }
    return allRead;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
