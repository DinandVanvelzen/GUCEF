/*
 *  AssetConverterLib: the counts of a pack index: resources, bytes, noise, metadata and the breakdowns
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

#include "AssetConverterLib_CPackIndexCounts.h"

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

CPackIndexCounts::CPackIndexCounts( void )
    : resources( 0 )
    , bytes( 0 )
    , noise( 0 )
    , metadata( 0 )
    , byGroup()
    , byKind()
    , byStatus()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPackIndexCounts::operator==( const CPackIndexCounts& other ) const
{GUCEF_TRACE;

    return resources == other.resources && bytes == other.bytes && noise == other.noise && metadata == other.metadata &&
           byGroup == other.byGroup && byKind == other.byKind && byStatus == other.byStatus;
}

/*-------------------------------------------------------------------------*/

namespace {

void
WriteCounts( CORE::CDataNode& node, const char* name, const CPackIndexCounts::TCountMap& counts )
{GUCEF_TRACE;

    CORE::CDataNode* child = node.AddChild( name, GUCEF_DATATYPE_OBJECT );
    CPackIndexCounts::TCountMap::const_iterator i = counts.begin();
    while ( i != counts.end() )
    {
        child->SetAttribute( (*i).first, (*i).second );
        ++i;
    }
}

void
ReadCounts( const CORE::CDataNode& node, const char* name, CPackIndexCounts::TCountMap& counts )
{GUCEF_TRACE;

    counts.clear();
    const CORE::CDataNode* child = node.FindChild( name );
    if ( GUCEF_NULL == child )
        return;
    CORE::CDataNode::TAttributeMap::const_iterator i = child->AttributeBegin();
    while ( i != child->AttributeEnd() )
    {
        counts[ (*i).first ] = (*i).second.AsUInt32( 0, true );
        ++i;
    }
}

}

/*-------------------------------------------------------------------------*/

bool
CPackIndexCounts::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "resources", resources );
    node.SetAttribute( "bytes", bytes );
    node.SetAttribute( "noise", noise );
    node.SetAttribute( "metadata", metadata );
    WriteCounts( node, "byGroup", byGroup );
    WriteCounts( node, "byKind", byKind );
    WriteCounts( node, "byStatus", byStatus );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexCounts::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    resources = (UInt32) CPackIndexSerial::GetCount( node, "resources" );
    bytes = CPackIndexSerial::GetCount( node, "bytes" );
    noise = (UInt32) CPackIndexSerial::GetCount( node, "noise" );
    metadata = (UInt32) CPackIndexSerial::GetCount( node, "metadata" );
    ReadCounts( node, "byGroup", byGroup );
    ReadCounts( node, "byKind", byKind );
    ReadCounts( node, "byStatus", byStatus );
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
