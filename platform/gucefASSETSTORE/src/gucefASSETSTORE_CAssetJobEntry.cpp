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
#include "gucefASSETSTORE_CAssetJobEntry.h"

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

CAssetJobEntry::CAssetJobEntry( void )
    : CORE::CIDataNodeSerializable()
    , jobId()
    , kind()
    , name()
    , detail()
    , scope()
    , subKind()
    , resultText()
    , state( ASSETJOB_STATE_QUEUED )
    , stateText()
    , message()
    , status( ASSETSTORE_STATUS_OK )
    , folder()
    , totalBytes( 0 )
    , doneBytes( 0 )
    , fraction( -1.0f )
    , bytesPerSecond( 0 )
    , secondsLeft( 0 )
    , itemsDone( 0 )
    , itemsTotal( 0 )
    , providerId()
    , itemId()
    , packId()
    , forIngest( false )
    , ingestRequested( false )
    , files()
    , actions()
    , createdAtMs( 0 )
    , updatedAtMs( 0 )
    , ownerDevice()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetJobEntry::~CAssetJobEntry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetJobEntry::Clear( void )
{GUCEF_TRACE;

    *this = CAssetJobEntry();
}

/*-------------------------------------------------------------------------*/

bool
CAssetJobEntry::IsEnded( void ) const
{GUCEF_TRACE;

    return AssetJobStateIsFinal( state );
}

/*-------------------------------------------------------------------------*/

bool
CAssetJobEntry::IsActionAvailable( TAssetJobAction action ) const
{GUCEF_TRACE;

    return actions.find( AssetJobActionToString( action ) ) != actions.end();
}

/*-------------------------------------------------------------------------*/

void
CAssetJobEntry::SetActionAvailable( TAssetJobAction action, bool available )
{GUCEF_TRACE;

    if ( available )
        actions.insert( AssetJobActionToString( action ) );
    else
        actions.erase( AssetJobActionToString( action ) );
}

/*-------------------------------------------------------------------------*/

bool
CAssetJobEntry::Serialize( CORE::CDataNode& domRootNode                        ,
                          const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "jobId", jobId ) && success;
    success = domRootNode.SetAttribute( "kind", kind ) && success;
    success = domRootNode.SetAttribute( "name", name ) && success;
    success = domRootNode.SetAttribute( "detail", detail ) && success;
    success = domRootNode.SetAttribute( "scope", scope ) && success;
    success = domRootNode.SetAttribute( "subKind", subKind ) && success;
    success = domRootNode.SetAttribute( "resultText", resultText ) && success;
    success = domRootNode.SetAttribute( "state", CORE::CString( AssetJobStateToString( state ) ) ) && success;
    success = domRootNode.SetAttribute( "stateText", stateText ) && success;
    success = domRootNode.SetAttribute( "message", message ) && success;
    success = domRootNode.SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) ) && success;
    success = domRootNode.SetAttribute( "folder", folder ) && success;
    success = domRootNode.SetAttribute( "totalBytes", totalBytes ) && success;
    success = domRootNode.SetAttribute( "doneBytes", doneBytes ) && success;
    success = domRootNode.SetAttribute( "fraction", fraction ) && success;
    success = domRootNode.SetAttribute( "bytesPerSecond", bytesPerSecond ) && success;
    success = domRootNode.SetAttribute( "secondsLeft", secondsLeft ) && success;
    success = domRootNode.SetAttribute( "itemsDone", itemsDone ) && success;
    success = domRootNode.SetAttribute( "itemsTotal", itemsTotal ) && success;
    success = domRootNode.SetAttribute( "providerId", providerId ) && success;
    success = domRootNode.SetAttribute( "itemId", itemId ) && success;
    success = domRootNode.SetAttribute( "packId", packId ) && success;
    success = domRootNode.SetAttribute( "forIngest", forIngest ) && success;
    success = domRootNode.SetAttribute( "ingestRequested", ingestRequested ) && success;
    success = domRootNode.SetValuesOfChildByName( "files", files ) && success;
    success = domRootNode.SetValuesOfChildByName( "actions", actions ) && success;
    success = domRootNode.SetAttribute( "createdAtMs", createdAtMs ) && success;
    success = domRootNode.SetAttribute( "updatedAtMs", updatedAtMs ) && success;
    success = domRootNode.SetAttribute( "ownerDevice", ownerDevice ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetJobEntry::Deserialize( const CORE::CDataNode& domRootNode                  ,
                            const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    CORE::CString text;
    jobId = domRootNode.GetAttributeValueOrChildValueByName( "jobId" ).AsString( jobId );
    kind = domRootNode.GetAttributeValueOrChildValueByName( "kind" ).AsString( kind );
    name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString( name );
    detail = domRootNode.GetAttributeValueOrChildValueByName( "detail" ).AsString( detail );
    scope = domRootNode.GetAttributeValueOrChildValueByName( "scope" ).AsString( scope );
    subKind = domRootNode.GetAttributeValueOrChildValueByName( "subKind" ).AsString( subKind );
    resultText = domRootNode.GetAttributeValueOrChildValueByName( "resultText" ).AsString( resultText );
    text = domRootNode.GetAttributeValueOrChildValueByName( "state" ).AsString();
    if ( !text.IsNULLOrEmpty() )
        state = AssetJobStateFromString( text );
    stateText = domRootNode.GetAttributeValueOrChildValueByName( "stateText" ).AsString( stateText );
    message = domRootNode.GetAttributeValueOrChildValueByName( "message" ).AsString( message );
    text = domRootNode.GetAttributeValueOrChildValueByName( "status" ).AsString();
    if ( !text.IsNULLOrEmpty() )
        status = AssetStoreStatusFromString( text );
    folder = domRootNode.GetAttributeValueOrChildValueByName( "folder" ).AsString( folder );
    totalBytes = domRootNode.GetAttributeValueOrChildValueByName( "totalBytes" ).AsUInt64( totalBytes, true );
    doneBytes = domRootNode.GetAttributeValueOrChildValueByName( "doneBytes" ).AsUInt64( doneBytes, true );
    fraction = domRootNode.GetAttributeValueOrChildValueByName( "fraction" ).AsFloat32( fraction, true );
    bytesPerSecond = domRootNode.GetAttributeValueOrChildValueByName( "bytesPerSecond" ).AsUInt64( bytesPerSecond, true );
    secondsLeft = domRootNode.GetAttributeValueOrChildValueByName( "secondsLeft" ).AsUInt64( secondsLeft, true );
    itemsDone = domRootNode.GetAttributeValueOrChildValueByName( "itemsDone" ).AsUInt32( itemsDone, true );
    itemsTotal = domRootNode.GetAttributeValueOrChildValueByName( "itemsTotal" ).AsUInt32( itemsTotal, true );
    providerId = domRootNode.GetAttributeValueOrChildValueByName( "providerId" ).AsString( providerId );
    itemId = domRootNode.GetAttributeValueOrChildValueByName( "itemId" ).AsString( itemId );
    packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString( packId );
    forIngest = domRootNode.GetAttributeValueOrChildValueByName( "forIngest" ).AsBool( forIngest, true );
    ingestRequested = domRootNode.GetAttributeValueOrChildValueByName( "ingestRequested" ).AsBool( ingestRequested, true );
    domRootNode.GetValuesOfChildByName( "files", files );
    domRootNode.GetValuesOfChildByName( "actions", actions );
    createdAtMs = domRootNode.GetAttributeValueOrChildValueByName( "createdAtMs" ).AsInt64( createdAtMs, true );
    updatedAtMs = domRootNode.GetAttributeValueOrChildValueByName( "updatedAtMs" ).AsInt64( updatedAtMs, true );
    ownerDevice = domRootNode.GetAttributeValueOrChildValueByName( "ownerDevice" ).AsString( ownerDevice );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetJobEntry::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetJobEntry( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetJobEntry::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetJobEntry::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetJobEntry";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetJobEntry::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetJobEntry";
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
