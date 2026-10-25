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

#ifndef GUCEF_AUTH_CDESCRIPTORAUTHPROVIDER_H
#define GUCEF_AUTH_CDESCRIPTORAUTHPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_CIAUTHPROVIDER_H
#include "gucefAUTH_CIAuthProvider.h"
#define GUCEF_AUTH_CIAUTHPROVIDER_H
#endif /* GUCEF_AUTH_CIAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTH_IAUTHFLOWENGINE_H
#include "gucefAUTH_IAuthFlowEngine.h"
#define GUCEF_AUTH_IAUTHFLOWENGINE_H
#endif /* GUCEF_AUTH_IAUTHFLOWENGINE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A login provider that is a descriptor and some settings: all OAuth 2.0 / OpenID Connect / OpenID 2.0 providers of
 *  the module are this class, registered under the name of their descriptor. A plugin that brings a backend with more
 *  than the protocol (a platform session next to the browser login) derives from it and overrides what differs. The work is done by the flow engine
 *  (IAuthFlowEngine), the engine of CAuthGlobal unless one is set on the provider. Without an engine, or with an
 *  engine that cannot run the provider as configured, the provider is not available and refuses every operation.
 *
 *  Config: the settings of the provider, see CAuthProviderSettings
 */
class GUCEF_AUTH_EXPORT_CPP CDescriptorAuthProvider : public CIAuthProvider ,
                                                      public IAuthFlowSink
{
    public:

    CDescriptorAuthProvider( void );

    explicit CDescriptorAuthProvider( const CAuthProviderDescriptor& descriptor );

    virtual ~CDescriptorAuthProvider();

    void SetDescriptor( const CAuthProviderDescriptor& descriptor );

    const CAuthProviderDescriptor& GetDescriptor( void ) const;

    const CAuthProviderSettings& GetSettings( void ) const;

    /** The engine to use instead of the one of CAuthGlobal, not owned; GUCEF_NULL to use the global one */
    void SetFlowEngine( IAuthFlowEngine* engine );

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool TrySilentLogin( CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool BeginLogin( const CAuthOptions& options, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Refresh( const CAccountRef& account, const CSecretString& refreshToken, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Logout( const CAccountRef& account, const CSecretString& refreshToken ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Verify( const CAuthProof& proof, CAuthResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Says why the provider is not available: no client id (the key is Auth/Providers/<name>/clientId), another setting the
     *  descriptor requires, no flow engine, or an engine that cannot run it here. False when it is available
     */
    virtual bool GetUnavailability( CAuthUnavailableProvider& unavailable ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  What the machine says about the account of the person at the provider, for a descriptor of the protocol OpenID 2.0: the
     *  profile of the provider (IOpenId2ProviderProfile::GetDetectedAccountHint()) decides what is looked at. Other
     *  providers detect nothing
     */
    virtual bool GetDetectedAccountHint( CORE::CString& displayName, bool& isRunning ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Forgets what the detection found, the next GetDetectedAccountHint() looks again (IOpenId2ProviderProfile::ForgetDetectedAccountHint()) */
    void ForgetDetectedAccountHint( void );

    /**
     *  For the redirect modes "custom-scheme" and "manual": hands the engine the address the login was redirected to
     *  (see IAuthFlowEngine::SubmitRedirectUrl()). False when there is no engine, or no login of this provider waits for it
     */
    bool SubmitRedirectUrl( const CORE::CString& redirectUrl );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnLoginCompleted( CORE::CFutureResultSource& result, const CAuthResult& authResult ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnLoginFailed( CORE::CFutureResultSource& result, TAuthFailureReason reason, const CORE::CString& message ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnActionRequired( const CAuthActionRequired& action ) GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** The engine of this provider: the one that was set, else the one of CAuthGlobal; GUCEF_NULL when there is none */
    IAuthFlowEngine* GetEngine( void ) const;

    private:

    CDescriptorAuthProvider( const CDescriptorAuthProvider& src );
    CDescriptorAuthProvider& operator=( const CDescriptorAuthProvider& src );

    private:

    CAuthProviderDescriptor m_descriptor;
    CAuthProviderSettings m_settings;
    IAuthFlowEngine* m_engine;
};

/*-------------------------------------------------------------------------*/

/** Makes the provider of one descriptor: this is what CAuthGlobal registers under the name of each descriptor */
class GUCEF_AUTH_EXPORT_CPP CDescriptorAuthProviderFactory : public CORE::CTFactory< CIAuthProvider, CDescriptorAuthProvider, MT::CMutex >
{
    public:

    typedef CORE::CTFactory< CIAuthProvider, CDescriptorAuthProvider, MT::CMutex > TBase;

    explicit CDescriptorAuthProviderFactory( const CORE::CString& descriptorName );

    virtual ~CDescriptorAuthProviderFactory();

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDescriptorAuthProviderFactory( const CDescriptorAuthProviderFactory& src );            /**< not implemented */
    CDescriptorAuthProviderFactory& operator=( const CDescriptorAuthProviderFactory& src );

    private:

    CORE::CString m_descriptorName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CDESCRIPTORAUTHPROVIDER_H ? */
