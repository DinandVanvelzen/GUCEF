/*
 *  AssetConverterLib: a function group summary of the pack index, with its sub-groups
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

#include "AssetConverterLib_CPackIndexGroup.h"

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

CPackIndexGroup::CPackIndexGroup( void )
    : id()
    , name()
    , count( 0 )
    , bytes( 0 )
    , subGroups()
    , subGroupsTruncated( false )
    , presentation()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CPackIndexSubGroup*
CPackIndexGroup::FindSubGroup( const CORE::CString& subGroupId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<subGroups.size(); ++i )
        if ( subGroups[ i ].id == subGroupId )
            return &subGroups[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexGroup::operator==( const CPackIndexGroup& other ) const
{GUCEF_TRACE;

    return id == other.id && name == other.name && count == other.count && bytes == other.bytes && subGroups == other.subGroups &&
           subGroupsTruncated == other.subGroupsTruncated && presentation == other.presentation;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexGroup::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CPackIndexSerial::SetTextAlways( node, "id", id );
    CPackIndexSerial::SetTextAlways( node, "name", name );
    node.SetAttribute( "count", count );
    node.SetAttribute( "bytes", bytes );
    CPackIndexSerial::SetText( node, "presentation", presentation );
    CPackIndexSerial::SetFlag( node, "subGroupsTruncated", subGroupsTruncated );
    CORE::CDataNode* list = node.AddChild( "subGroups", GUCEF_DATATYPE_ARRAY );
    const size_t limit = subGroups.size() > MaxSubGroupsInManifest ? (size_t) MaxSubGroupsInManifest : subGroups.size();
    for ( size_t i=0; i<limit; ++i )
        subGroups[ i ].Serialize( *list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexGroup::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    *this = CPackIndexGroup();
    id = CPackIndexSerial::GetText( node, "id" );
    name = CPackIndexSerial::GetText( node, "name" );
    count = (UInt32) CPackIndexSerial::GetCount( node, "count" );
    bytes = CPackIndexSerial::GetCount( node, "bytes" );
    presentation = CPackIndexSerial::GetText( node, "presentation" );
    subGroupsTruncated = CPackIndexSerial::GetFlag( node, "subGroupsTruncated" );
    const CORE::CDataNode* list = node.FindChild( "subGroups" );
    if ( GUCEF_NULL != list )
    {
        CORE::CDataNode::const_iterator i = list->ConstBegin();
        while ( i != list->ConstEnd() )
        {
            CPackIndexSubGroup sub;
            if ( sub.Deserialize( *(*i) ) )
                subGroups.push_back( sub );
            ++i;
        }
    }
    return !id.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
