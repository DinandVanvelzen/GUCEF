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

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#define GUCEF_AUTH_CAUTHGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_AUTH_CIAUTHPROVIDER_H
#include "gucefAUTH_CIAuthProvider.h"
#define GUCEF_AUTH_CIAUTHPROVIDER_H
#endif /* GUCEF_AUTH_CIAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTH_CAUTHPROVIDERREGISTRY_H
#include "gucefAUTH_CAuthProviderRegistry.h"
#define GUCEF_AUTH_CAUTHPROVIDERREGISTRY_H
#endif /* GUCEF_AUTH_CAUTHPROVIDERREGISTRY_H ? */

#ifndef GUCEF_AUTH_IAUTHFLOWENGINE_H
#include "gucefAUTH_IAuthFlowEngine.h"
#define GUCEF_AUTH_IAUTHFLOWENGINE_H
#endif /* GUCEF_AUTH_IAUTHFLOWENGINE_H ? */

#ifndef GUCEF_AUTH_COPENID2PROFILEREGISTRY_H
#include "gucefAUTH_COpenId2ProfileRegistry.h"
#define GUCEF_AUTH_COPENID2PROFILEREGISTRY_H
#endif /* GUCEF_AUTH_COPENID2PROFILEREGISTRY_H ? */

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

class CAuthManager;
class CDescriptorAuthProviderFactory;
class COAuth2Engine;

/**
 *  The global systems of gucefAUTH, set up when the module loads: the factory of the login providers, the registry of
 *  the provider descriptors with the built in ones, the registry of the OpenID 2.0 profiles, the flow engine for the
 *  descriptor driven providers and the manager of the process.
 *
 *  The extension point for backends. The module knows the generic protocols (OpenID Connect, OAuth 2.0, OpenID 2.0) and
 *  a number of identity providers that follow them as data; a backend that is more than that (a platform, a store, an
 *  SDK: Steam, Epic Online Services, Google Play Games) is a plugin that registers what it brings when it loads and
 *  takes it away when it unloads. A build without the plugin has none of it: no provider, no descriptor, no code path.
 *    - the login provider, in the factory under its provider name: GetProviderFactory().RegisterConcreteFactory()
 *    - the descriptor of the provider when it is driven by one (OpenID Connect, OAuth 2.0, OpenID 2.0, or the
 *      description of a platform session), in GetDescriptorRegistry(). RegisterDescriptorProvider() does the factory part for a
 *      provider that is a plain CDescriptorAuthProvider, which is all a plugin needs for a provider that is only a descriptor
 *    - for an OpenID 2.0 descriptor the profile, see IOpenId2ProviderProfile, in GetOpenId2Profiles()
 *  The manager makes the providers of the factory when RefreshProviders() is called, and the plugin removes the provider
 *  from the manager again before the plugin is unloaded. The plugin of Steam is the example (authpluginSTEAM):
 *  @code
 *  GUCEFPlugin_Load()
 *  {
 *      AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();
 *      global->GetDescriptorRegistry().RegisterDescriptor( descriptor );
 *      global->GetOpenId2Profiles().Register( &profile );
 *      global->GetProviderFactory().RegisterConcreteFactory( "steam", &providerFactory );
 *      global->GetManager().RefreshProviders();
 *  }
 *  GUCEFPlugin_Unload()
 *  {
 *      AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();
 *      global->GetManager().RemoveProvider( "steam" );
 *      global->GetProviderFactory().UnregisterConcreteFactory( "steam" );
 *      global->GetOpenId2Profiles().Unregister( "steam" );
 *      global->GetDescriptorRegistry().UnregisterDescriptor( "steam" );
 *  }
 *  @endcode
 */
class GUCEF_AUTH_EXPORT_CPP CAuthGlobal
{
    public:

    static CAuthGlobal* Instance( void );

    /** The login providers by provider name: the descriptor driven ones of the built in descriptors, the ones plugins add */
    CAuthProviderFactory& GetProviderFactory( void );

    /** The descriptors of the OAuth 2.0 / OIDC / OpenID 2.0 providers */
    CAuthProviderRegistry& GetDescriptorRegistry( void );

    /** The profiles of the OpenID 2.0 providers, by provider name: empty until a plugin adds one */
    COpenId2ProfileRegistry& GetOpenId2Profiles( void );

    /** The manager of the process; call RefreshProviders() (LoadConfig() does) after the providers are registered */
    CAuthManager& GetManager( void );

    /**
     *  The engine that the descriptor driven providers use, not owned; GUCEF_NULL when there is none (they are then not available).
     *  When the module is set up this is the built in engine, GetOAuth2Engine().
     */
    IAuthFlowEngine* GetFlowEngine( void ) const;

    /** Replaces the engine of the descriptor driven providers, for example by an engine of a platform; GUCEF_NULL leaves them without one */
    void SetFlowEngine( IAuthFlowEngine* engine );

    /** The built in engine (OpenID Connect, OAuth 2.0, OpenID 2.0), made when the module is set up and registered as the flow engine. Its settings are those of the application */
    COAuth2Engine* GetOAuth2Engine( void ) const;

    /** Makes the built in engine the flow engine again */
    void RegisterBuiltInEngine( void );

    /**
     *  Registers a provider for a descriptor of the registry under the name of the descriptor, as the built in ones are.
     *  False when the registry has no such descriptor or the name has a provider already.
     */
    bool RegisterDescriptorProvider( const CORE::CString& descriptorName );

    /**
     *  Takes back what RegisterDescriptorProvider() did for the descriptor: the factory of the name goes. The provider that
     *  the manager made is destroyed through that factory, so it has to be removed from the manager first
     *  (CAuthManager::RemoveProvider()). False when RegisterDescriptorProvider() did not register that name.
     */
    bool UnregisterDescriptorProvider( const CORE::CString& descriptorName );

    private:
    friend class CModule;

    static void Deinstance( void );

    void Initialize( void );

    CAuthGlobal( void );
    ~CAuthGlobal();
    CAuthGlobal( const CAuthGlobal& src );              /**< not implemented */
    CAuthGlobal& operator=( const CAuthGlobal& src );   /**< not implemented */

    private:

    typedef GUCEF::vector< CDescriptorAuthProviderFactory* > TDescriptorFactoryVector;

    CAuthProviderFactory* m_providerFactory;
    CAuthProviderRegistry* m_descriptorRegistry;
    COpenId2ProfileRegistry* m_openId2Profiles;
    CAuthManager* m_manager;
    IAuthFlowEngine* m_flowEngine;
    COAuth2Engine* m_oauth2Engine;
    TDescriptorFactoryVector m_descriptorFactories;
    CORE::CString::StringVector m_descriptorFactoryNames;

    static MT::CMutex g_dataLock;
    static CAuthGlobal* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */
