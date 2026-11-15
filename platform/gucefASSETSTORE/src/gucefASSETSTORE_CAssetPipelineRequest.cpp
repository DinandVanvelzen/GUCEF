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
#include "gucefASSETSTORE_CAssetPipelineRequest.h"

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

CAssetPipelineRequest::CAssetPipelineRequest( void )
    : CORE::CIDataNodeSerializable()
    , operation()
    , packId()
    , assetPath()
    , sourcePath()
    , storeId()
    , itemId()
    , storeAssetId()
    , assets()
    , cookTargets()
    , profiles()
    , section()
    , force( false )
    , reimport( false )
    , apply( false )
    , changeToken()
    , pageToken()
    , maxEntries( 0 )
    , arguments()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetPipelineRequest::~CAssetPipelineRequest()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetPipelineRequest::Clear( void )
{GUCEF_TRACE;

    *this = CAssetPipelineRequest();
}

/*-------------------------------------------------------------------------*/

bool
CAssetPipelineRequest::Serialize( CORE::CDataNode& domRootNode                        ,
                                 const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "operation", operation ) && success;
    success = domRootNode.SetAttribute( "packId", packId ) && success;
    success = domRootNode.SetAttribute( "assetPath", assetPath ) && success;
    success = domRootNode.SetAttribute( "sourcePath", sourcePath ) && success;
    success = domRootNode.SetAttribute( "storeId", storeId ) && success;
    success = domRootNode.SetAttribute( "itemId", itemId ) && success;
    success = domRootNode.SetAttribute( "storeAssetId", storeAssetId ) && success;
    success = domRootNode.SetValuesOfChildByName( "assets", assets ) && success;
    success = domRootNode.SetValuesOfChildByName( "cookTargets", cookTargets ) && success;
    success = domRootNode.SetValuesOfChildByName( "profiles", profiles ) && success;
    success = domRootNode.SetAttribute( "section", section ) && success;
    success = domRootNode.SetAttribute( "force", force ) && success;
    success = domRootNode.SetAttribute( "reimport", reimport ) && success;
    success = domRootNode.SetAttribute( "apply", apply ) && success;
    success = domRootNode.SetAttribute( "changeToken", changeToken ) && success;
    success = domRootNode.SetAttribute( "pageToken", pageToken ) && success;
    success = domRootNode.SetAttribute( "maxEntries", maxEntries ) && success;
    CORE::CDataNode* argumentsNode = domRootNode.AddChild( "arguments", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != argumentsNode )
    {
        argumentsNode->Copy( arguments );
        argumentsNode->SetName( "arguments" );
    }
    else
        success = false;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPipelineRequest::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                   const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    operation = domRootNode.GetAttributeValueOrChildValueByName( "operation" ).AsString( operation );
    packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString( packId );
    assetPath = domRootNode.GetAttributeValueOrChildValueByName( "assetPath" ).AsString( assetPath );
    sourcePath = domRootNode.GetAttributeValueOrChildValueByName( "sourcePath" ).AsString( sourcePath );
    storeId = domRootNode.GetAttributeValueOrChildValueByName( "storeId" ).AsString( storeId );
    itemId = domRootNode.GetAttributeValueOrChildValueByName( "itemId" ).AsString( itemId );
    storeAssetId = domRootNode.GetAttributeValueOrChildValueByName( "storeAssetId" ).AsString( storeAssetId );
    domRootNode.GetValuesOfChildByName( "assets", assets );
    domRootNode.GetValuesOfChildByName( "cookTargets", cookTargets );
    domRootNode.GetValuesOfChildByName( "profiles", profiles );
    section = domRootNode.GetAttributeValueOrChildValueByName( "section" ).AsString( section );
    force = domRootNode.GetAttributeValueOrChildValueByName( "force" ).AsBool( force, true );
    reimport = domRootNode.GetAttributeValueOrChildValueByName( "reimport" ).AsBool( reimport, true );
    apply = domRootNode.GetAttributeValueOrChildValueByName( "apply" ).AsBool( apply, true );
    changeToken = domRootNode.GetAttributeValueOrChildValueByName( "changeToken" ).AsString( changeToken );
    pageToken = domRootNode.GetAttributeValueOrChildValueByName( "pageToken" ).AsString( pageToken );
    maxEntries = domRootNode.GetAttributeValueOrChildValueByName( "maxEntries" ).AsUInt32( maxEntries, true );
    const CORE::CDataNode* argumentsNode = domRootNode.FindChild( "arguments" );
    if ( GUCEF_NULL != argumentsNode )
        arguments.Copy( *argumentsNode );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetPipelineRequest::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetPipelineRequest( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetPipelineRequest::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetPipelineRequest::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetPipelineRequest";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetPipelineRequest::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetPipelineRequest";
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
