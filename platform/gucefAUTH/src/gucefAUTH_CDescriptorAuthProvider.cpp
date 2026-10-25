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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_AUTH_COAUTH2HTTP_H
#include "gucefAUTH_COAuth2Http.h"
#define GUCEF_AUTH_COAUTH2HTTP_H
#endif /* GUCEF_AUTH_COAUTH2HTTP_H ? */

#include "gucefAUTH_CDescriptorAuthProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDescriptorAuthProvider::CDescriptorAuthProvider( void )
    : CIAuthProvider()
    , IAuthFlowSink()
    , m_descriptor()
    , m_settings()
    , m_engine( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDescriptorAuthProvider::CDescriptorAuthProvider( const CAuthProviderDescriptor& descriptor )
    : CIAuthProvider()
    , IAuthFlowSink()
    , m_descriptor( descriptor )
    , m_settings()
    , m_engine( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDescriptorAuthProvider::~CDescriptorAuthProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDescriptorAuthProvider::SetDescriptor( const CAuthProviderDescriptor& descriptor )
{GUCEF_TRACE;

    m_descriptor = descriptor;
}

/*-------------------------------------------------------------------------*/

const CAuthProviderDescriptor&
CDescriptorAuthProvider::GetDescriptor( void ) const
{GUCEF_TRACE;

    return m_descriptor;
}

/*-------------------------------------------------------------------------*/

const CAuthProviderSettings&
CDescriptorAuthProvider::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CDescriptorAuthProvider::SetFlowEngine( IAuthFlowEngine* engine )
{GUCEF_TRACE;

    m_engine = engine;
}

/*-------------------------------------------------------------------------*/

IAuthFlowEngine*
CDescriptorAuthProvider::GetEngine( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_engine )
        return m_engine;
    CAuthGlobal* global = CAuthGlobal::Instance();
    return GUCEF_NULL != global ? global->GetFlowEngine() : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDescriptorAuthProvider::GetProviderName( void ) const
{GUCEF_TRACE;

    return m_descriptor.name;
}

/*-------------------------------------------------------------------------*/

UInt32
CDescriptorAuthProvider::GetCapabilities( void ) const
{GUCEF_TRACE;

    return m_descriptor.capabilities;
}

/*-------------------------------------------------------------------------*/

bool
CDescriptorAuthProvider::IsAvailable( void ) const
{GUCEF_TRACE;

    if ( !m_descriptor.IsWellFormed() )
        return false;
    if ( 0 != ( m_descriptor.capabilities & AUTHCAP_REQUIRES_CLIENT_ID ) && !m_settings.HasClientId() )
        return false;
    IAuthFlowEngine* engine = GetEngine();
    return GUCEF_NULL != engine && engine->IsSupported( m_descriptor, m_settings );
}

/*-------------------------------------------------------------------------*/

bool
CDescriptorAuthProvider::TrySilentLogin( CORE::CFutureResult& future )
{GUCEF_TRACE;

    // A descriptor describes a login that involves the user, platform sessions are plugins
    future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDescriptorAuthProvider::BeginLogin( const CAuthOptions& options, CORE::CFutureResult& future )
{GUCEF_TRACE;

    future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    IAuthFlowEngine* engine = GetEngine();
    if ( !IsAvailable() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DescriptorAuthProvider: " << m_descriptor.name << " is not available, there is no flow engine for it or it is not configured" );
        return false;
    }

    CORE::CFutureResultSource result;
    if ( !engine->StartInteractive( m_descriptor, m_settings, options, *this, result ) )
        return false;
    future = result.GetFuture();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDescriptorAuthProvider::Refresh( const CAccountRef& account, const CSecretString& refreshToken, CORE::CFutureResult& future )
{GUCEF_TRACE;

    future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    IAuthFlowEngine* engine = GetEngine();
    if ( !IsAvailable() || 0 == ( m_descriptor.capabilities & AUTHCAP_REFRESHABLE ) || refreshToken.IsEmpty() )
        return false;

    CORE::CFutureResultSource result;
    if ( !engine->StartRefresh( m_descriptor, m_settings, account, refreshToken, *this, result ) )
        return false;
    future = result.GetFuture();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDescriptorAuthProvider::Logout( const CAccountRef& account, const CSecretString& refreshToken )
{GUCEF_TRACE;

    IAuthFlowEngine* engine = GetEngine();
    // A provider that has no revocation endpoint of its own may still have one in its discovery document, which the engine knows
    bool mayRevoke = !m_descriptor.revocationEndpoint.IsNULLOrEmpty() || !m_settings.revocationEndpoint.IsNULLOrEmpty() ||
                     !m_settings.discoveryUrl.IsNULLOrEmpty() || !m_descriptor.discoveryUrl.IsNULLOrEmpty();
    if ( GUCEF_NULL == engine || refreshToken.IsEmpty() || !mayRevoke )
        return true;
    return engine->Revoke( m_descriptor, m_settings, account, refreshToken );
}

/*-------------------------------------------------------------------------*/

bool
CDescriptorAuthProvider::Verify( const CAuthProof& proof, CAuthResult& result )
{GUCEF_TRACE;

    IAuthFlowEngine* engine = GetEngine();
    if ( GUCEF_NULL == engine || 0 == ( m_descriptor.capabilities & AUTHCAP_SERVER_VERIFIABLE ) || proof.IsEmpty() )
        return false;
    if ( !engine->Verify( m_descriptor, m_settings, proof, result ) )
        return false;
    result.providerName = m_descriptor.name;
    return result.IsValid();
}

/*-------------------------------------------------------------------------*/

bool
CDescriptorAuthProvider::SubmitRedirectUrl( const CORE::CString& redirectUrl )
{GUCEF_TRACE;

    IAuthFlowEngine* engine = GetEngine();
    return GUCEF_NULL != engine && engine->SubmitRedirectUrl( redirectUrl );
}

/*-------------------------------------------------------------------------*/

bool
CDescriptorAuthProvider::GetUnavailability( CAuthUnavailableProvider& unavailable ) const
{GUCEF_TRACE;

    unavailable.Clear();
    if ( IsAvailable() )
        return false;

    const CORE::CString keyBase = "Auth/Providers/" + m_descriptor.name;
    unavailable.providerName = m_descriptor.name;

    if ( !m_descriptor.IsWellFormed() )
    {
        unavailable.reason = AUTHUNAVAILABLE_NOT_AVAILABLE;
        unavailable.message = "Not available: the description of this login is incomplete";
        return true;
    }

    if ( 0 != ( m_descriptor.capabilities & AUTHCAP_REQUIRES_CLIENT_ID ) && !m_settings.HasClientId() )
    {
        unavailable.reason = AUTHUNAVAILABLE_NO_CLIENT_ID;
        unavailable.message = "Not set up in this build: needs a client id";
        unavailable.configKey = keyBase + "/clientId";
        return true;
    }

    // what the descriptor asks of the application besides the client id: a value of the settings, or a default of the descriptor
    CORE::CString missing;
    CORE::CString firstMissing;
    for ( size_t i=0; i<m_descriptor.requiredParameters.size(); ++i )
    {
        const CORE::CString& name = m_descriptor.requiredParameters[ i ];
        if ( !m_settings.GetParameter( name ).IsNULLOrEmpty() )
            continue;
        CORE::CString::StringMap::const_iterator d = m_descriptor.defaultParameters.find( name );
        if ( d != m_descriptor.defaultParameters.end() && !(*d).second.IsNULLOrEmpty() )
            continue;
        if ( missing.IsNULLOrEmpty() )
            firstMissing = name;
        missing += ( missing.IsNULLOrEmpty() ? "" : ", " ) + name;
    }
    if ( !missing.IsNULLOrEmpty() )
    {
        unavailable.reason = AUTHUNAVAILABLE_MISSING_SETTING;
        unavailable.message = "Not set up in this build: needs " + missing;
        unavailable.configKey = keyBase + "/Parameters/" + firstMissing;
        return true;
    }

    IAuthFlowEngine* engine = GetEngine();
    if ( GUCEF_NULL == engine )
    {
        unavailable.reason = AUTHUNAVAILABLE_PLATFORM_NOT_SUPPORTED;
        unavailable.message = "Not available in this build: there is no login engine";
        return true;
    }

    // What the login asks of the provider goes over https and the HTTP client of this build has none: the plugin is not loaded
    if ( engine->IsSecureTransportMissing( m_descriptor, m_settings ) )
    {
        unavailable.reason = AUTHUNAVAILABLE_MISSING_PLUGIN;
        unavailable.message = COAuth2Http::GetHttpsUnavailableMessage();
        return true;
    }

    unavailable.reason = AUTHUNAVAILABLE_PLATFORM_NOT_SUPPORTED;
    unavailable.message = "Not available on this platform";
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDescriptorAuthProvider::ForgetDetectedAccountHint( void )
{GUCEF_TRACE;

    if ( AUTHPROTOCOL_OPENID2 != m_descriptor.protocol )
        return;
    IOpenId2ProviderProfile* profile = CAuthGlobal::Instance()->GetOpenId2Profiles().Find( m_descriptor.name );
    if ( GUCEF_NULL != profile )
        profile->ForgetDetectedAccountHint();
}

/*-------------------------------------------------------------------------*/

bool
CDescriptorAuthProvider::GetDetectedAccountHint( CORE::CString& displayName, bool& isRunning ) const
{GUCEF_TRACE;

    displayName.Clear();
    isRunning = false;
    if ( AUTHPROTOCOL_OPENID2 != m_descriptor.protocol )
        return false;
    const IOpenId2ProviderProfile* profile = CAuthGlobal::Instance()->GetOpenId2Profiles().Find( m_descriptor.name );
    return GUCEF_NULL != profile && profile->GetDetectedAccountHint( displayName, isRunning );
}

/*-------------------------------------------------------------------------*/

bool
CDescriptorAuthProvider::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return m_settings.SaveConfig( config );
}

/*-------------------------------------------------------------------------*/

bool
CDescriptorAuthProvider::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    return m_settings.LoadConfig( config );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDescriptorAuthProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CDescriptorAuthProvider";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CDescriptorAuthProvider::OnLoginCompleted( CORE::CFutureResultSource& result, const CAuthResult& authResult )
{GUCEF_TRACE;

    return CompleteLogin( result, authResult );
}

/*-------------------------------------------------------------------------*/

bool
CDescriptorAuthProvider::OnLoginFailed( CORE::CFutureResultSource& result, TAuthFailureReason reason, const CORE::CString& message )
{GUCEF_TRACE;

    return FailLogin( result, reason, message );
}

/*-------------------------------------------------------------------------*/

void
CDescriptorAuthProvider::OnActionRequired( const CAuthActionRequired& action )
{GUCEF_TRACE;

    RaiseActionRequired( action );
}

/*-------------------------------------------------------------------------*/

CDescriptorAuthProviderFactory::CDescriptorAuthProviderFactory( const CORE::CString& descriptorName )
    : TBase()
    , m_descriptorName( descriptorName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDescriptorAuthProviderFactory::~CDescriptorAuthProviderFactory()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDescriptorAuthProviderFactory::TProductPtr
CDescriptorAuthProviderFactory::Create( void )
{GUCEF_TRACE;

    TProductPtr product = TBase::Create();
    CDescriptorAuthProvider* provider = static_cast< CDescriptorAuthProvider* >( product.GetPointerAlways() );
    CAuthProviderDescriptor descriptor;
    if ( GUCEF_NULL != provider && CAuthGlobal::Instance()->GetDescriptorRegistry().GetDescriptor( m_descriptorName, descriptor ) )
        provider->SetDescriptor( descriptor );
    return product;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
