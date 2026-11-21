/*
 *  AssetConverterLib: a thumbnail of a resource of the pack index
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

#include "AssetConverterLib_CPackIndexThumbnail.h"

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

CPackIndexThumbnail::CPackIndexThumbnail( void )
    : size( 0 )
    , digest()
    , rendererId()
    , rendererVersion( 0 )
    , path()
    , automatic( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPackIndexThumbnail::operator<( const CPackIndexThumbnail& other ) const
{GUCEF_TRACE;

    if ( size != other.size )
        return size < other.size;
    if ( rendererId != other.rendererId )
        return rendererId < other.rendererId;
    return rendererVersion < other.rendererVersion;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexThumbnail::operator==( const CPackIndexThumbnail& other ) const
{GUCEF_TRACE;

    return size == other.size && digest == other.digest && rendererId == other.rendererId && rendererVersion == other.rendererVersion &&
           path == other.path && automatic == other.automatic;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexThumbnail::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "size", size );
    CORE::CDataNode* key = node.AddChild( "key", GUCEF_DATATYPE_OBJECT );
    CPackIndexSerial::SetTextAlways( *key, "digest", digest );
    CPackIndexSerial::SetTextAlways( *key, "rendererId", rendererId );
    key->SetAttribute( "rendererVersion", rendererVersion );
    CPackIndexSerial::SetText( node, "path", path );
    CPackIndexSerial::SetFlag( node, "automatic", automatic );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexThumbnail::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    size = (UInt32) CPackIndexSerial::GetCount( node, "size" );
    const CORE::CDataNode* key = node.FindChild( "key" );
    if ( GUCEF_NULL != key )
    {
        digest = CPackIndexSerial::GetText( *key, "digest" );
        rendererId = CPackIndexSerial::GetText( *key, "rendererId" );
        rendererVersion = (UInt32) CPackIndexSerial::GetCount( *key, "rendererVersion" );
    }
    path = CPackIndexSerial::GetText( node, "path" );
    automatic = CPackIndexSerial::GetFlag( node, "automatic" );
    return size > 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
