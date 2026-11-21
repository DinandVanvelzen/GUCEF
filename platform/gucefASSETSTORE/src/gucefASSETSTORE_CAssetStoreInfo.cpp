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
#include "gucefASSETSTORE_CAssetStoreInfo.h"

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

CAssetStoreInfo::CAssetStoreInfo( void )
    : CORE::CIDataNodeSerializable()
    , storeId()
    , displayName()
    , iconName()
    , homePageUrl()
    , signInKind( ASSETSTORE_SIGNIN_NONE )
    , signInState( ASSETSTORE_SIGNINSTATE_SIGNED_OUT )
    , signedIn( false )
    , sessionEnded( false )
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreInfo::~CAssetStoreInfo()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetStoreInfo::Clear( void )
{GUCEF_TRACE;

    *this = CAssetStoreInfo();
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreInfo::Serialize( CORE::CDataNode& domRootNode                        ,
                           const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "storeId", storeId ) && success;
    success = domRootNode.SetAttribute( "displayName", displayName ) && success;
    success = domRootNode.SetAttribute( "iconName", iconName ) && success;
    success = domRootNode.SetAttribute( "homePageUrl", homePageUrl ) && success;
    success = domRootNode.SetAttribute( "signInKind", CORE::CString( AssetStoreSignInKindToString( signInKind ) ) ) && success;
    success = domRootNode.SetAttribute( "signInState", CORE::CString( AssetStoreSignInStateToString( signInState ) ) ) && success;
    success = domRootNode.SetAttribute( "signedIn", signedIn ) && success;
    success = domRootNode.SetAttribute( "sessionEnded", sessionEnded ) && success;
    success = domRootNode.SetAttribute( "message", message ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreInfo::Deserialize( const CORE::CDataNode& domRootNode                  ,
                             const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    CORE::CString text;
    storeId = domRootNode.GetAttributeValueOrChildValueByName( "storeId" ).AsString( storeId );
    displayName = domRootNode.GetAttributeValueOrChildValueByName( "displayName" ).AsString( displayName );
    iconName = domRootNode.GetAttributeValueOrChildValueByName( "iconName" ).AsString( iconName );
    homePageUrl = domRootNode.GetAttributeValueOrChildValueByName( "homePageUrl" ).AsString( homePageUrl );
    text = domRootNode.GetAttributeValueOrChildValueByName( "signInKind" ).AsString();
    if ( !text.IsNULLOrEmpty() )
        signInKind = AssetStoreSignInKindFromString( text );
    text = domRootNode.GetAttributeValueOrChildValueByName( "signInState" ).AsString();
    const bool hasState = !text.IsNULLOrEmpty();
    if ( hasState )
        signInState = AssetStoreSignInStateFromString( text );
    signedIn = domRootNode.GetAttributeValueOrChildValueByName( "signedIn" ).AsBool( signedIn, true );
    sessionEnded = domRootNode.GetAttributeValueOrChildValueByName( "sessionEnded" ).AsBool( sessionEnded, true );
    // a record of an older sender has no state: it follows from the flags
    if ( !hasState )
        signInState = signedIn ? ASSETSTORE_SIGNINSTATE_SIGNED_IN : ( sessionEnded ? ASSETSTORE_SIGNINSTATE_EXPIRED : ASSETSTORE_SIGNINSTATE_SIGNED_OUT );
    message = domRootNode.GetAttributeValueOrChildValueByName( "message" ).AsString( message );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetStoreInfo::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetStoreInfo( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStoreInfo::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStoreInfo::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetStoreInfo";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreInfo::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreInfo";
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
