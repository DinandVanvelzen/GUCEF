/*
 *  AssetConverterLib: a reference between two resources of the pack index
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

#include "AssetConverterLib_CPackIndexReference.h"

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

CPackIndexReference::CPackIndexReference( void )
    : kind()
    , to()
    , via()
    , path()
    , optional( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPackIndexReference::operator<( const CPackIndexReference& other ) const
{GUCEF_TRACE;

    if ( kind != other.kind )
        return kind < other.kind;
    if ( to != other.to )
        return to < other.to;
    if ( via != other.via )
        return via < other.via;
    return path < other.path;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexReference::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CPackIndexSerial::SetTextAlways( node, "kind", kind );
    CPackIndexSerial::SetTextAlways( node, "to", to );
    CPackIndexSerial::SetText( node, "via", via );
    CPackIndexSerial::SetText( node, "path", path );
    CPackIndexSerial::SetFlag( node, "optional", optional );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexReference::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    kind = CPackIndexSerial::GetText( node, "kind" );
    to = CPackIndexSerial::GetText( node, "to" );
    via = CPackIndexSerial::GetText( node, "via" );
    path = CPackIndexSerial::GetText( node, "path" );
    optional = CPackIndexSerial::GetFlag( node, "optional" );
    return !kind.IsNULLOrEmpty() && !to.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
