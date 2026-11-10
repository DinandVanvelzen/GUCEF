/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#include "gucefAUTH_CAuthOptions.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CAuthOptions::DefaultTimeoutInMs;
const UInt32 CAuthOptions::MaxDeviceCodeTimeoutInMs;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
AuthFailureReasonToString( TAuthFailureReason reason )
{GUCEF_TRACE;

    switch ( reason )
    {
        case AUTHFAIL_CANCELLED : return "cancelled";
        case AUTHFAIL_TIMEOUT : return "timeout";
        case AUTHFAIL_DENIED : return "denied";
        case AUTHFAIL_NOT_AVAILABLE : return "not-available";
        case AUTHFAIL_NOT_SUPPORTED : return "not-supported";
        case AUTHFAIL_NETWORK : return "network";
        case AUTHFAIL_INVALID_GRANT : return "invalid-grant";
        case AUTHFAIL_INVALID_RESPONSE : return "invalid-response";
        case AUTHFAIL_IN_PROGRESS : return "in-progress";
        case AUTHFAIL_ACCOUNT_LINKED_ELSEWHERE : return "account-linked-elsewhere";
        case AUTHFAIL_INTERNAL : return "internal";
        default : return "unknown";
    }
}

/*-------------------------------------------------------------------------*/

CAuthOptions::CAuthOptions( void )
    : CORE::CIDataNodeSerializable()
    , scopes()
    , loginHint()
    , forceInteractive( false )
    , timeoutInMs( DefaultTimeoutInMs )
    , preferBrowser( true )
    , preferDeviceCode( false )
    , linkToProfile( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthOptions::CAuthOptions( const CAuthOptions& src )
    : CORE::CIDataNodeSerializable( src )
    , scopes( src.scopes )
    , loginHint( src.loginHint )
    , forceInteractive( src.forceInteractive )
    , timeoutInMs( src.timeoutInMs )
    , preferBrowser( src.preferBrowser )
    , preferDeviceCode( src.preferDeviceCode )
    , linkToProfile( src.linkToProfile )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthOptions::~CAuthOptions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthOptions&
CAuthOptions::operator=( const CAuthOptions& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        scopes = src.scopes;
        loginHint = src.loginHint;
        forceInteractive = src.forceInteractive;
        timeoutInMs = src.timeoutInMs;
        preferBrowser = src.preferBrowser;
        preferDeviceCode = src.preferDeviceCode;
        linkToProfile = src.linkToProfile;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CAuthOptions::Serialize( CORE::CDataNode& domRootNode                        ,
                         const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetValuesOfChildByName( "scopes", scopes );
    domRootNode.SetAttribute( "loginHint", loginHint );
    domRootNode.SetAttribute( "forceInteractive", forceInteractive );
    domRootNode.SetAttribute( "timeoutInMs", timeoutInMs );
    domRootNode.SetAttribute( "preferBrowser", preferBrowser );
    domRootNode.SetAttribute( "preferDeviceCode", preferDeviceCode );
    domRootNode.SetAttribute( "linkToProfile", linkToProfile );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthOptions::Deserialize( const CORE::CDataNode& domRootNode                  ,
                           const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    domRootNode.GetValuesOfChildByName( "scopes", scopes );
    loginHint = domRootNode.GetAttributeValueOrChildValueByName( "loginHint" ).AsString( loginHint, true );
    forceInteractive = domRootNode.GetAttributeValueOrChildValueByName( "forceInteractive" ).AsBool( forceInteractive, true );
    timeoutInMs = domRootNode.GetAttributeValueOrChildValueByName( "timeoutInMs" ).AsUInt32( timeoutInMs, true );
    preferBrowser = domRootNode.GetAttributeValueOrChildValueByName( "preferBrowser" ).AsBool( preferBrowser, true );
    preferDeviceCode = domRootNode.GetAttributeValueOrChildValueByName( "preferDeviceCode" ).AsBool( preferDeviceCode, true );
    linkToProfile = domRootNode.GetAttributeValueOrChildValueByName( "linkToProfile" ).AsBool( linkToProfile, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAuthOptions::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAuthOptions( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAuthOptions::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CAuthOptions";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
