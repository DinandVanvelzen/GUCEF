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
#include "gucefASSETSTORE_CAssetIngestHistoryEntry.h"

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

CAssetIngestHistoryEntry::CAssetIngestHistoryEntry( void )
    : CORE::CIDataNodeSerializable()
    , entryId()
    , ingestId()
    , packId()
    , sourceId()
    , origin()
    , policy()
    , startedAtMs( 0 )
    , durationMs( 0 )
    , outcome()
    , step()
    , problems()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetIngestHistoryEntry::~CAssetIngestHistoryEntry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetIngestHistoryEntry::Clear( void )
{GUCEF_TRACE;

    *this = CAssetIngestHistoryEntry();
}

/*-------------------------------------------------------------------------*/

bool
CAssetIngestHistoryEntry::Serialize( CORE::CDataNode& domRootNode                        ,
                                    const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "entryId", entryId ) && success;
    success = domRootNode.SetAttribute( "ingestId", ingestId ) && success;
    success = domRootNode.SetAttribute( "packId", packId ) && success;
    success = domRootNode.SetAttribute( "sourceId", sourceId ) && success;
    success = domRootNode.SetAttribute( "origin", origin ) && success;
    success = domRootNode.SetAttribute( "policy", policy ) && success;
    success = domRootNode.SetAttribute( "startedAtMs", startedAtMs ) && success;
    success = domRootNode.SetAttribute( "durationMs", durationMs ) && success;
    success = domRootNode.SetAttribute( "outcome", outcome ) && success;
    success = domRootNode.SetAttribute( "step", step ) && success;
    success = domRootNode.SetValuesOfChildByName( "problems", problems ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetIngestHistoryEntry::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                      const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetIngestHistoryEntry defaults;
    entryId = domRootNode.GetAttributeValueOrChildValueByName( "entryId" ).AsString( defaults.entryId );
    ingestId = domRootNode.GetAttributeValueOrChildValueByName( "ingestId" ).AsString( defaults.ingestId );
    packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString( defaults.packId );
    sourceId = domRootNode.GetAttributeValueOrChildValueByName( "sourceId" ).AsString( defaults.sourceId );
    origin = domRootNode.GetAttributeValueOrChildValueByName( "origin" ).AsString( defaults.origin );
    policy = domRootNode.GetAttributeValueOrChildValueByName( "policy" ).AsString( defaults.policy );
    startedAtMs = domRootNode.GetAttributeValueOrChildValueByName( "startedAtMs" ).AsInt64( defaults.startedAtMs, true );
    durationMs = domRootNode.GetAttributeValueOrChildValueByName( "durationMs" ).AsInt64( defaults.durationMs, true );
    outcome = domRootNode.GetAttributeValueOrChildValueByName( "outcome" ).AsString( defaults.outcome );
    step = domRootNode.GetAttributeValueOrChildValueByName( "step" ).AsString( defaults.step );
    problems.clear();
    domRootNode.GetValuesOfChildByName( "problems", problems );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetIngestHistoryEntry::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetIngestHistoryEntry( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetIngestHistoryEntry::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetIngestHistoryEntry";
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
