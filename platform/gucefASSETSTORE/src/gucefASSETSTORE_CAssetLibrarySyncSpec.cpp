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
#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#include "gucefASSETSTORE_CAssetPipelineRequest.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#include "gucefASSETSTORE_CAssetLibrarySyncSpec.h"

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

const CORE::CString CAssetLibrarySyncSpec::ModeRun = "run";
const CORE::CString CAssetLibrarySyncSpec::ModePlan = "plan";

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncSpec::CAssetLibrarySyncSpec( void )
    : CORE::CIDataNodeSerializable()
    , storeId()
    , mode( ModeRun )
    , retryFailed( false )
    , maxItems( 0 )
    , pageSize( 0 )
    , itemIds()
    , scheduled( false )
    , manual( false )
    , useStoreProfile( false )
    , profile()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncSpec::~CAssetLibrarySyncSpec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncSpec::IsPlanOnly( void ) const
{GUCEF_TRACE;

    return ModePlan == mode;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncSpec::IsValid( CORE::CString& reason ) const
{GUCEF_TRACE;

    reason.Clear();
    if ( storeId.IsNULLOrEmpty() )
    {
        reason = "The sync names no store";
        return false;
    }
    if ( ModeRun != mode && ModePlan != mode )
    {
        reason = "The mode \"" + mode + "\" is not known: run or plan";
        return false;
    }
    return profile.IsValid( reason );
}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncSpec
CAssetLibrarySyncSpec::FromRequest( const CAssetPipelineRequest& request )
{GUCEF_TRACE;

    CAssetLibrarySyncSpec spec;
    CORE::CDataNodeSerializableSettings settings;
    spec.Deserialize( request.arguments, settings );
    if ( spec.storeId.IsNULLOrEmpty() )
        spec.storeId = request.storeId;
    spec.storeId = spec.storeId.Lowercase();
    return spec;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncSpec::ToRequest( CAssetPipelineRequest& request ) const
{GUCEF_TRACE;

    request.operation = CAssetPipelineOperations::SyncLibrary;
    request.storeId = storeId;
    request.arguments.Clear();
    request.arguments.SetName( "arguments" );
    request.arguments.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNodeSerializableSettings settings;
    Serialize( request.arguments, settings );
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncSpec::Serialize( CORE::CDataNode& domRootNode                        ,
                                  const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "storeId", storeId );
    success = domRootNode.SetAttribute( "mode", mode ) && success;
    success = domRootNode.SetAttribute( "retryFailed", retryFailed ) && success;
    success = domRootNode.SetAttribute( "maxItems", maxItems ) && success;
    success = domRootNode.SetAttribute( "pageSize", pageSize ) && success;
    success = domRootNode.SetValuesOfChildByName( "itemIds", itemIds ) && success;
    success = domRootNode.SetAttribute( "scheduled", scheduled ) && success;
    if ( manual )
        success = domRootNode.SetAttribute( "manual", manual ) && success;
    if ( useStoreProfile )
    {
        // the profile of the store is the service's: only the choice travels (an older service ignores it and takes the defaults)
        success = domRootNode.SetAttribute( "useStoreProfile", useStoreProfile ) && success;
    }
    else
    {
        CORE::CDataNode* profileNode = domRootNode.AddChild( "profile", GUCEF_DATATYPE_OBJECT );
        success = GUCEF_NULL != profileNode && profile.Serialize( *profileNode, settings ) && success;
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncSpec::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                    const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    storeId = domRootNode.GetAttributeValueOrChildValueByName( "storeId" ).AsString( storeId, true );
    mode = domRootNode.GetAttributeValueOrChildValueByName( "mode" ).AsString( mode, true );
    retryFailed = domRootNode.GetAttributeValueOrChildValueByName( "retryFailed" ).AsBool( retryFailed, true );
    maxItems = domRootNode.GetAttributeValueOrChildValueByName( "maxItems" ).AsUInt32( maxItems, true );
    pageSize = domRootNode.GetAttributeValueOrChildValueByName( "pageSize" ).AsUInt32( pageSize, true );
    if ( GUCEF_NULL != domRootNode.FindChild( "itemIds" ) )
    {
        itemIds.clear();
        domRootNode.GetValuesOfChildByName( "itemIds", itemIds );
    }
    scheduled = domRootNode.GetAttributeValueOrChildValueByName( "scheduled" ).AsBool( scheduled, true );
    useStoreProfile = domRootNode.GetAttributeValueOrChildValueByName( "useStoreProfile" ).AsBool( useStoreProfile, true );
    manual = domRootNode.GetAttributeValueOrChildValueByName( "manual" ).AsBool( manual, true );
    const CORE::CDataNode* profileNode = domRootNode.FindChild( "profile" );
    if ( GUCEF_NULL != profileNode )
        profile.Deserialize( *profileNode, settings );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetLibrarySyncSpec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetLibrarySyncSpec( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetLibrarySyncSpec::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetLibrarySyncSpec";
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
