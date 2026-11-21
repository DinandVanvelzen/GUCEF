/*
 *  AssetConverterLib: the identity of a pack in the pack index: its key, its fingerprint and where it came from
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

#include "AssetConverterLib_CPackIndexIdentity.h"

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

CPackIndexIdentity::CPackIndexIdentity( void )
    : packKey()
    , fileCount( 0 )
    , totalBytes( 0 )
    , treeHash()
    , name()
    , title()
    , storePackId()
    , sourceStoreId()
    , sourceItemId()
    , sourceVersion()
    , sourceUrl()
    , root()
    , contentHash()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPackIndexIdentity::operator==( const CPackIndexIdentity& other ) const
{GUCEF_TRACE;

    return packKey == other.packKey && fileCount == other.fileCount && totalBytes == other.totalBytes && treeHash == other.treeHash &&
           name == other.name && title == other.title && storePackId == other.storePackId && sourceStoreId == other.sourceStoreId &&
           sourceItemId == other.sourceItemId && sourceVersion == other.sourceVersion && sourceUrl == other.sourceUrl &&
           root == other.root && contentHash == other.contentHash;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexIdentity::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CPackIndexSerial::SetTextAlways( node, "packKey", packKey );
    CORE::CDataNode* fingerprint = node.AddChild( "fingerprint", GUCEF_DATATYPE_OBJECT );
    fingerprint->SetAttribute( "fileCount", fileCount );
    fingerprint->SetAttribute( "totalBytes", totalBytes );
    CPackIndexSerial::SetTextAlways( *fingerprint, "treeHash", treeHash );
    CPackIndexSerial::SetTextAlways( node, "name", name );
    CPackIndexSerial::SetText( node, "title", title );
    CPackIndexSerial::SetText( node, "storePackId", storePackId );
    if ( !sourceStoreId.IsNULLOrEmpty() || !sourceItemId.IsNULLOrEmpty() || !sourceVersion.IsNULLOrEmpty() || !sourceUrl.IsNULLOrEmpty() )
    {
        CORE::CDataNode* source = node.AddChild( "source", GUCEF_DATATYPE_OBJECT );
        CPackIndexSerial::SetText( *source, "storeId", sourceStoreId );
        CPackIndexSerial::SetText( *source, "itemId", sourceItemId );
        CPackIndexSerial::SetText( *source, "version", sourceVersion );
        CPackIndexSerial::SetText( *source, "url", sourceUrl );
    }
    CPackIndexSerial::SetText( node, "root", root );
    CPackIndexSerial::SetText( node, "contentHash", contentHash );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexIdentity::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    *this = CPackIndexIdentity();
    packKey = CPackIndexSerial::GetText( node, "packKey" );
    const CORE::CDataNode* fingerprint = node.FindChild( "fingerprint" );
    if ( GUCEF_NULL != fingerprint )
    {
        fileCount = CPackIndexSerial::GetCount( *fingerprint, "fileCount" );
        totalBytes = CPackIndexSerial::GetCount( *fingerprint, "totalBytes" );
        treeHash = CPackIndexSerial::GetText( *fingerprint, "treeHash" );
    }
    name = CPackIndexSerial::GetText( node, "name" );
    title = CPackIndexSerial::GetText( node, "title" );
    storePackId = CPackIndexSerial::GetText( node, "storePackId" );
    const CORE::CDataNode* source = node.FindChild( "source" );
    if ( GUCEF_NULL != source )
    {
        sourceStoreId = CPackIndexSerial::GetText( *source, "storeId" );
        sourceItemId = CPackIndexSerial::GetText( *source, "itemId" );
        sourceVersion = CPackIndexSerial::GetText( *source, "version" );
        sourceUrl = CPackIndexSerial::GetText( *source, "url" );
    }
    root = CPackIndexSerial::GetText( node, "root" );
    contentHash = CPackIndexSerial::GetText( node, "contentHash" );
    return !packKey.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
