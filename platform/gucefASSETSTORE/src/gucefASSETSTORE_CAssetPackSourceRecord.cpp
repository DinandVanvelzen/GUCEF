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

#include "gucefASSETSTORE_CAssetPackSourceRecord.h"

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

CAssetPackSourceRecord::CAssetPackSourceRecord( void )
    : CORE::CIDataNodeSerializable()
    , packId()
    , sourceId()
    , sourceItemId()
    , sourceFormatId()
    , revision()
    , changedMs( 0 )
    , contentDigest()
    , supersededBy()
    , supersededMs( 0 )
    , syncedMs( 0 )
    , syncJobId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetPackSourceRecord::~CAssetPackSourceRecord()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetPackSourceRecord::Clear( void )
{GUCEF_TRACE;

    *this = CAssetPackSourceRecord();
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackSourceRecord::IsCurrent( void ) const
{GUCEF_TRACE;

    return supersededBy.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackSourceRecord::MatchesFormat( const CORE::CString& formatId ) const
{GUCEF_TRACE;

    return sourceFormatId.IsNULLOrEmpty() || formatId.IsNULLOrEmpty() || sourceFormatId == formatId;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackSourceRecord::Serialize( CORE::CDataNode& domRootNode                        ,
                                   const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "packId", packId );
    success = domRootNode.SetAttribute( "sourceId", sourceId ) && success;
    success = domRootNode.SetAttribute( "sourceItemId", sourceItemId ) && success;
    success = domRootNode.SetAttribute( "sourceFormatId", sourceFormatId ) && success;
    success = domRootNode.SetAttribute( "revision", revision ) && success;
    success = domRootNode.SetAttribute( "changedMs", changedMs ) && success;
    success = domRootNode.SetAttribute( "contentDigest", contentDigest ) && success;
    success = domRootNode.SetAttribute( "supersededBy", supersededBy ) && success;
    success = domRootNode.SetAttribute( "supersededMs", supersededMs ) && success;
    success = domRootNode.SetAttribute( "syncedMs", syncedMs ) && success;
    success = domRootNode.SetAttribute( "syncJobId", syncJobId ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackSourceRecord::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                     const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString();
    sourceId = domRootNode.GetAttributeValueOrChildValueByName( "sourceId" ).AsString();
    sourceItemId = domRootNode.GetAttributeValueOrChildValueByName( "sourceItemId" ).AsString();
    sourceFormatId = domRootNode.GetAttributeValueOrChildValueByName( "sourceFormatId" ).AsString();
    revision = domRootNode.GetAttributeValueOrChildValueByName( "revision" ).AsString();
    changedMs = domRootNode.GetAttributeValueOrChildValueByName( "changedMs" ).AsInt64( 0, true );
    contentDigest = domRootNode.GetAttributeValueOrChildValueByName( "contentDigest" ).AsString();
    supersededBy = domRootNode.GetAttributeValueOrChildValueByName( "supersededBy" ).AsString();
    supersededMs = domRootNode.GetAttributeValueOrChildValueByName( "supersededMs" ).AsInt64( 0, true );
    syncedMs = domRootNode.GetAttributeValueOrChildValueByName( "syncedMs" ).AsInt64( 0, true );
    syncJobId = domRootNode.GetAttributeValueOrChildValueByName( "syncJobId" ).AsString();
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetPackSourceRecord::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetPackSourceRecord( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetPackSourceRecord::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetPackSourceRecord";
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
