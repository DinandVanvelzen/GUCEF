/*
 *  AssetConverterLib: a shard entry of the manifest of a sharded pack index
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

#include "AssetConverterLib_CPackIndexShardInfo.h"

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

CPackIndexShardInfo::CPackIndexShardInfo( void )
    : file()
    , count( 0 )
    , firstPath()
    , lastPath()
    , group()
    , sha256()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPackIndexShardInfo::operator<( const CPackIndexShardInfo& other ) const
{GUCEF_TRACE;

    return file < other.file;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexShardInfo::operator==( const CPackIndexShardInfo& other ) const
{GUCEF_TRACE;

    return file == other.file && count == other.count && firstPath == other.firstPath && lastPath == other.lastPath && group == other.group && sha256 == other.sha256;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexShardInfo::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CPackIndexSerial::SetTextAlways( node, "file", file );
    node.SetAttribute( "count", count );
    CPackIndexSerial::SetTextAlways( node, "firstPath", firstPath );
    CPackIndexSerial::SetTextAlways( node, "lastPath", lastPath );
    CPackIndexSerial::SetText( node, "group", group );
    CPackIndexSerial::SetTextAlways( node, "sha256", sha256 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexShardInfo::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    file = CPackIndexSerial::GetText( node, "file" );
    count = (UInt32) CPackIndexSerial::GetCount( node, "count" );
    firstPath = CPackIndexSerial::GetText( node, "firstPath" );
    lastPath = CPackIndexSerial::GetText( node, "lastPath" );
    group = CPackIndexSerial::GetText( node, "group" );
    sha256 = CPackIndexSerial::GetText( node, "sha256" );
    return !file.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
