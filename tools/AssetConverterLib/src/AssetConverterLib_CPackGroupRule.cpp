/*
 *  AssetConverterLib: an explicit group rule of a pack profile (stage P of the group assignment)
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

#include "AssetConverterLib_CPackGroupRule.h"

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

CPackGroupRule::CPackGroupRule( void )
    : glob()
    , folder()
    , token()
    , kind()
    , group()
    , subGroup()
    , priority( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPackGroupRule::operator==( const CPackGroupRule& other ) const
{GUCEF_TRACE;

    return glob == other.glob && folder == other.folder && token == other.token && kind == other.kind &&
           group == other.group && subGroup == other.subGroup && priority == other.priority;
}

/*-------------------------------------------------------------------------*/

bool
CPackGroupRule::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* match = node.AddChild( "match", GUCEF_DATATYPE_OBJECT );
    CPackIndexSerial::SetText( *match, "glob", glob );
    CPackIndexSerial::SetText( *match, "folder", folder );
    CPackIndexSerial::SetText( *match, "token", token );
    CPackIndexSerial::SetText( *match, "kind", kind );
    CPackIndexSerial::SetText( node, "group", group );
    CPackIndexSerial::SetText( node, "subGroup", subGroup );
    node.SetAttribute( "priority", priority );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackGroupRule::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    *this = CPackGroupRule();
    const CORE::CDataNode* match = node.FindChild( "match" );
    if ( GUCEF_NULL != match )
    {
        glob = CPackIndexSerial::GetText( *match, "glob" );
        folder = CPackIndexSerial::GetText( *match, "folder" );
        token = CPackIndexSerial::GetText( *match, "token" );
        kind = CPackIndexSerial::GetText( *match, "kind" );
    }
    group = CPackIndexSerial::GetText( node, "group" );
    subGroup = CPackIndexSerial::GetText( node, "subGroup" );
    priority = (Int32) CPackIndexSerial::GetInt( node, "priority" );
    return ( !group.IsNULLOrEmpty() || !subGroup.IsNULLOrEmpty() ) && ( !glob.IsNULLOrEmpty() || !folder.IsNULLOrEmpty() || !token.IsNULLOrEmpty() || !kind.IsNULLOrEmpty() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
