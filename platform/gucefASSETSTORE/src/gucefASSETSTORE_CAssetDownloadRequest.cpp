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
#include "gucefASSETSTORE_CAssetDownloadRequest.h"

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

CAssetDownloadRequest::CAssetDownloadRequest( void )
    : CORE::CIDataNodeSerializable()
    , providerId()
    , itemId()
    , itemTitle()
    , formatId()
    , fileIds()
    , destination()
    , forIngest( false )
    , keep()
    , locationId()
    , expectedRevision()
    , syncJobId()
    , expectedBytes( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetDownloadRequest::~CAssetDownloadRequest()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetDownloadRequest::Clear( void )
{GUCEF_TRACE;

    *this = CAssetDownloadRequest();
}

/*-------------------------------------------------------------------------*/

bool
CAssetDownloadRequest::Serialize( CORE::CDataNode& domRootNode                        ,
                                 const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "providerId", providerId ) && success;
    success = domRootNode.SetAttribute( "itemId", itemId ) && success;
    success = domRootNode.SetAttribute( "itemTitle", itemTitle ) && success;
    success = domRootNode.SetAttribute( "formatId", formatId ) && success;
    success = domRootNode.SetValuesOfChildByName( "fileIds", fileIds ) && success;
    success = domRootNode.SetAttribute( "destination", destination ) && success;
    success = domRootNode.SetAttribute( "forIngest", forIngest ) && success;
    success = domRootNode.SetAttribute( "keep", keep ) && success;
    success = domRootNode.SetAttribute( "locationId", locationId ) && success;
    // the members of the library sync, only when set: a service of an earlier version sees the request it knows
    if ( !expectedRevision.IsNULLOrEmpty() )
        success = domRootNode.SetAttribute( "expectedRevision", expectedRevision ) && success;
    if ( !syncJobId.IsNULLOrEmpty() )
        success = domRootNode.SetAttribute( "syncJobId", syncJobId ) && success;
    if ( 0 != expectedBytes )
        success = domRootNode.SetAttribute( "expectedBytes", expectedBytes ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetDownloadRequest::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                   const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    providerId = domRootNode.GetAttributeValueOrChildValueByName( "providerId" ).AsString( providerId );
    itemId = domRootNode.GetAttributeValueOrChildValueByName( "itemId" ).AsString( itemId );
    itemTitle = domRootNode.GetAttributeValueOrChildValueByName( "itemTitle" ).AsString( itemTitle );
    formatId = domRootNode.GetAttributeValueOrChildValueByName( "formatId" ).AsString( formatId );
    domRootNode.GetValuesOfChildByName( "fileIds", fileIds );
    destination = domRootNode.GetAttributeValueOrChildValueByName( "destination" ).AsString( destination );
    forIngest = domRootNode.GetAttributeValueOrChildValueByName( "forIngest" ).AsBool( forIngest, true );
    keep = domRootNode.GetAttributeValueOrChildValueByName( "keep" ).AsString( keep );
    locationId = domRootNode.GetAttributeValueOrChildValueByName( "locationId" ).AsString( locationId );
    expectedRevision = domRootNode.GetAttributeValueOrChildValueByName( "expectedRevision" ).AsString( expectedRevision );
    syncJobId = domRootNode.GetAttributeValueOrChildValueByName( "syncJobId" ).AsString( syncJobId );
    expectedBytes = domRootNode.GetAttributeValueOrChildValueByName( "expectedBytes" ).AsUInt64( 0, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetDownloadRequest::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetDownloadRequest( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetDownloadRequest::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetDownloadRequest::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetDownloadRequest";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetDownloadRequest::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetDownloadRequest";
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
