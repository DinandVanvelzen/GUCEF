/*
 *  AssetConverterLib: a sub-group summary of a function group of the pack index
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

#include "AssetConverterLib_CPackIndexSubGroup.h"

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

CPackIndexSubGroup::CPackIndexSubGroup( void )
    : id()
    , name()
    , count( 0 )
    , bytes( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPackIndexSubGroup::operator==( const CPackIndexSubGroup& other ) const
{GUCEF_TRACE;

    return id == other.id && name == other.name && count == other.count && bytes == other.bytes;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexSubGroup::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CPackIndexSerial::SetTextAlways( node, "id", id );
    CPackIndexSerial::SetTextAlways( node, "name", name );
    node.SetAttribute( "count", count );
    node.SetAttribute( "bytes", bytes );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexSubGroup::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    id = CPackIndexSerial::GetText( node, "id" );
    name = CPackIndexSerial::GetText( node, "name" );
    count = (UInt32) CPackIndexSerial::GetCount( node, "count" );
    bytes = CPackIndexSerial::GetCount( node, "bytes" );
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
