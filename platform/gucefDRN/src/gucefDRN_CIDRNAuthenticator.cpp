/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#include "gucefDRN_CIDRNAuthenticator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNAuthSettings::ClassTypeName = "GUCEF::DRN::CDRNAuthSettings";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNAuthContext::CDRNAuthContext( void )
    : serviceName()
    , initiatorNodeId()
    , acceptorNodeId()
{GUCEF_TRACE;

    memset( nonce, 0, sizeof( nonce ) );
    memset( transcriptHash, 0, sizeof( transcriptHash ) );
    memset( policyHash, 0, sizeof( policyHash ) );
    requirePolicyHash = false;
}

/*-------------------------------------------------------------------------*/

CDRNAuthResult::CDRNAuthResult( void )
    : userId()
    , role()
    , sessionName()
    , publicKey()
    , accounts()
    , refusalText()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIDRNAuthenticator::CIDRNAuthenticator( void )
    : CORE::CIConfigurable()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIDRNAuthenticator::~CIDRNAuthenticator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNAuthSettings::CDRNAuthSettings( void )
    : CORE::CIConfigurable()
    , authenticatorType( "anonymous" )
    , authenticatorConfig( "AuthenticatorConfig" )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNAuthSettings::CDRNAuthSettings( const CDRNAuthSettings& src )
    : CORE::CIConfigurable( src )
    , authenticatorType( src.authenticatorType )
    , authenticatorConfig( src.authenticatorConfig )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNAuthSettings::~CDRNAuthSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNAuthSettings&
CDRNAuthSettings::operator=( const CDRNAuthSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        authenticatorType = src.authenticatorType;
        authenticatorConfig = src.authenticatorConfig;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CDRNAuthSettings::SetSetting( const CORE::CString& name, const CORE::CVariant& value )
{GUCEF_TRACE;

    authenticatorConfig.SetAttribute( name, value );
}

/*-------------------------------------------------------------------------*/

CIDRNAuthenticatorPtr
CDRNAuthSettings::CreateAuthenticator( void ) const
{GUCEF_TRACE;

    CIDRNAuthenticatorPtr authenticator( CDRNGlobal::Instance()->GetAuthenticatorFactory().Create( authenticatorType ) );
    if ( authenticator.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNAuthSettings: Unknown authenticator type \"" << authenticatorType << "\"" );
        return CIDRNAuthenticatorPtr();
    }
    if ( !authenticator->LoadConfig( authenticatorConfig ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNAuthSettings: The settings of authenticator \"" << authenticatorType << "\" are not valid" );
        return CIDRNAuthenticatorPtr();
    }
    return authenticator;
}

/*-------------------------------------------------------------------------*/

bool
CDRNAuthSettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = cfg.SetAttribute( "authenticatorType", authenticatorType );
    CORE::CDataNode* child = cfg.FindOrAddChild( "AuthenticatorConfig" );
    if ( GUCEF_NULL != child )
    {
        child->Copy( authenticatorConfig );
        child->SetName( "AuthenticatorConfig" );
    }
    else
        totalSuccess = false;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDRNAuthSettings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    authenticatorType = cfg.GetAttributeValueOrChildValueByName( "authenticatorType" ).AsString( authenticatorType, true );
    const CORE::CDataNode* child = cfg.FindChild( "AuthenticatorConfig" );
    if ( GUCEF_NULL != child )
        authenticatorConfig = *child;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNAuthSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
