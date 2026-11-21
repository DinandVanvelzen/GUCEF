/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefASSETSTORE_CAssetLibrarySyncItem.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetLibrarySyncItem::Ignored = "ignored";
const CORE::CString CAssetLibrarySyncItem::NoId = "no-id";
const CORE::CString CAssetLibrarySyncItem::Filtered = "filtered";
const CORE::CString CAssetLibrarySyncItem::Unavailable = "unavailable";
const CORE::CString CAssetLibrarySyncItem::InFlight = "in-flight";
const CORE::CString CAssetLibrarySyncItem::Pinned = "pinned";
const CORE::CString CAssetLibrarySyncItem::Current = "current";
const CORE::CString CAssetLibrarySyncItem::UnknownFreshness = "unknown-freshness";
const CORE::CString CAssetLibrarySyncItem::Stale = "stale";
const CORE::CString CAssetLibrarySyncItem::TooLarge = "too-large";
const CORE::CString CAssetLibrarySyncItem::FailedBefore = "failed-before";
const CORE::CString CAssetLibrarySyncItem::Missing = "missing";

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncItem::CAssetLibrarySyncItem( void )
    : CORE::CIDataNodeSerializable()
    , itemId()
    , title()
    , state()
    , reason()
    , formats()
    , queuedFormats()
    , estimatedBytes( 0 )
    , sizeKnown( false )
    , listedRevision()
    , storedRevision()
    , licenceClass()
    , packIds()
    , changedAtMs( 0 )
    , acquiredAtMs( 0 )
    , listIndex( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncItem::~CAssetLibrarySyncItem()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncItem::IsQueued( void ) const
{GUCEF_TRACE;

    return ( Missing == state || Stale == state ) && !queuedFormats.empty();
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncItem::Serialize( CORE::CDataNode& domRootNode                        ,
                                  const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "itemId", itemId );
    success = domRootNode.SetAttribute( "title", title ) && success;
    success = domRootNode.SetAttribute( "state", state ) && success;
    success = domRootNode.SetAttribute( "reason", reason ) && success;
    success = domRootNode.SetValuesOfChildByName( "formats", formats ) && success;
    success = domRootNode.SetValuesOfChildByName( "queuedFormats", queuedFormats ) && success;
    success = domRootNode.SetAttribute( "estimatedBytes", estimatedBytes ) && success;
    success = domRootNode.SetAttribute( "sizeKnown", sizeKnown ) && success;
    success = domRootNode.SetAttribute( "listedRevision", listedRevision ) && success;
    success = domRootNode.SetAttribute( "storedRevision", storedRevision ) && success;
    success = domRootNode.SetAttribute( "licenceClass", licenceClass ) && success;
    success = domRootNode.SetValuesOfChildByName( "packIds", packIds ) && success;
    success = domRootNode.SetAttribute( "changedAtMs", changedAtMs ) && success;
    success = domRootNode.SetAttribute( "acquiredAtMs", acquiredAtMs ) && success;
    success = domRootNode.SetAttribute( "listIndex", listIndex ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncItem::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                    const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    itemId = domRootNode.GetAttributeValueOrChildValueByName( "itemId" ).AsString();
    title = domRootNode.GetAttributeValueOrChildValueByName( "title" ).AsString();
    state = domRootNode.GetAttributeValueOrChildValueByName( "state" ).AsString();
    reason = domRootNode.GetAttributeValueOrChildValueByName( "reason" ).AsString();
    formats.clear();
    domRootNode.GetValuesOfChildByName( "formats", formats );
    queuedFormats.clear();
    domRootNode.GetValuesOfChildByName( "queuedFormats", queuedFormats );
    estimatedBytes = domRootNode.GetAttributeValueOrChildValueByName( "estimatedBytes" ).AsUInt64( 0, true );
    sizeKnown = domRootNode.GetAttributeValueOrChildValueByName( "sizeKnown" ).AsBool( false, true );
    listedRevision = domRootNode.GetAttributeValueOrChildValueByName( "listedRevision" ).AsString();
    storedRevision = domRootNode.GetAttributeValueOrChildValueByName( "storedRevision" ).AsString();
    licenceClass = domRootNode.GetAttributeValueOrChildValueByName( "licenceClass" ).AsString();
    packIds.clear();
    domRootNode.GetValuesOfChildByName( "packIds", packIds );
    changedAtMs = domRootNode.GetAttributeValueOrChildValueByName( "changedAtMs" ).AsInt64( 0, true );
    acquiredAtMs = domRootNode.GetAttributeValueOrChildValueByName( "acquiredAtMs" ).AsInt64( 0, true );
    listIndex = domRootNode.GetAttributeValueOrChildValueByName( "listIndex" ).AsUInt32( 0, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetLibrarySyncItem::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetLibrarySyncItem( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetLibrarySyncItem::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetLibrarySyncItem";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
