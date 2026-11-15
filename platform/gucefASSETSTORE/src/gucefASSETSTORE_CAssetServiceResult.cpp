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
#include "gucefASSETSTORE_CAssetServiceResult.h"

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

CAssetServiceResult::CAssetServiceResult( void )
    : CORE::CIDataNodeSerializable()
    , status( ASSETSTORE_STATUS_OK )
    , message()
    , id()
    , relatedId()
    , revision( 0 )
    , replayed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetServiceResult::~CAssetServiceResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetServiceResult::Clear( void )
{GUCEF_TRACE;

    *this = CAssetServiceResult();
}

/*-------------------------------------------------------------------------*/

bool
CAssetServiceResult::Serialize( CORE::CDataNode& domRootNode                        ,
                               const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) ) && success;
    success = domRootNode.SetAttribute( "message", message ) && success;
    success = domRootNode.SetAttribute( "id", id ) && success;
    success = domRootNode.SetAttribute( "relatedId", relatedId ) && success;
    success = domRootNode.SetAttribute( "revision", revision ) && success;
    success = domRootNode.SetAttribute( "replayed", replayed ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetServiceResult::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                 const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    CORE::CString text;
    text = domRootNode.GetAttributeValueOrChildValueByName( "status" ).AsString();
    if ( !text.IsNULLOrEmpty() )
        status = AssetStoreStatusFromString( text );
    message = domRootNode.GetAttributeValueOrChildValueByName( "message" ).AsString( message );
    id = domRootNode.GetAttributeValueOrChildValueByName( "id" ).AsString( id );
    relatedId = domRootNode.GetAttributeValueOrChildValueByName( "relatedId" ).AsString( relatedId );
    revision = domRootNode.GetAttributeValueOrChildValueByName( "revision" ).AsUInt64( revision, true );
    replayed = domRootNode.GetAttributeValueOrChildValueByName( "replayed" ).AsBool( replayed, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetServiceResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetServiceResult( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetServiceResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetServiceResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetServiceResult";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetServiceResult::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetServiceResult";
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
