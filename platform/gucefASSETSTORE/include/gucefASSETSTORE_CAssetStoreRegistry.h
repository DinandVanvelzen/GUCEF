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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#define GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#include "gucefASSETSTORE_CIAssetStoreProvider.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H ? */
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAssetStoreGlobal;

/**
 *  The stores the application works with, by provider id (section 4.1): the providers made from the concrete
 *  factories that store plugins registered in the provider factory (RefreshProviders()), and providers the
 *  application adds itself, such as the primary store (AddProvider()). Reach it through CAssetStoreGlobal.
 *
 *  A store plugin registers its factory and calls RefreshProviders() when it loads; before it unloads it calls
 *  RemoveProvider() and then unregisters its factory, since the provider is destroyed through the factory:
 *  @code
 *  ASSETSTORE::CAssetStoreGlobal* global = ASSETSTORE::CAssetStoreGlobal::Instance();
 *  global->GetProviderFactory().RegisterConcreteFactory( "mystore", &factory );
 *  global->GetRegistry().RefreshProviders();
 *  ...
 *  global->GetRegistry().RemoveProvider( "mystore" );
 *  global->GetProviderFactory().UnregisterConcreteFactory( "mystore" );
 *  @endcode
 *  Thread safe. ProviderAddedEvent and ProviderRemovedEvent (TCloneableString: the provider id) are raised on the thread
 *  of the global pulse generator.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreRegistry : public CORE::CTSGNotifier
{
    public:

    static const CORE::CEvent ProviderAddedEvent;
    static const CORE::CEvent ProviderRemovedEvent;

    static void RegisterEvents( void );

    /** Makes a provider for every factory key that has none yet; the number made */
    UInt32 RefreshProviders( void );

    /** Adds a provider under its id; false when the id is taken or the provider is null */
    bool AddProvider( const TAssetStoreProviderPtr& provider );

    /** Removes the provider after its async operations ended; false when there is none */
    bool RemoveProvider( const CORE::CString& providerId );

    void RemoveAllProviders( void );

    bool TryGetProvider( const CORE::CString& providerId, TAssetStoreProviderPtr& provider ) const;

    bool HasProvider( const CORE::CString& providerId ) const;

    /** The provider ids in alphabetical order */
    void GetProviderIds( CORE::CString::StringVector& providerIds ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CAssetStoreGlobal;

    CAssetStoreRegistry( CAssetStoreProviderFactory& factory );
    virtual ~CAssetStoreRegistry() GUCEF_VIRTUAL_OVERRIDE;

    CAssetStoreRegistry( const CAssetStoreRegistry& src );              /**< not implemented */
    CAssetStoreRegistry& operator=( const CAssetStoreRegistry& src );   /**< not implemented */

    private:

    typedef GUCEF::map< CORE::CString, TAssetStoreProviderPtr > TProviderMap;

    CAssetStoreProviderFactory& m_factory;
    TProviderMap m_providers;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H ? */
