/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_MULTIPLAY_CIIDENTITYPROVIDER_H
#define GUCEF_MULTIPLAY_CIIDENTITYPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_AUTH_IACCOUNTPROFILE_H
#include "gucefAUTH_IAccountProfile.h"
#define GUCEF_AUTH_IACCOUNTPROFILE_H
#endif /* GUCEF_AUTH_IACCOUNTPROFILE_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#include "gucefMULTIPLAY_CPlayerIdentity.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Where the identity of the local player comes from: a profile file, a platform account
 *
 *  A provider that can log the player in with an external account (CAuthIdentityProvider) overrides Login() and
 *  AutoLogin(), which start the login and give the outcome through a future; the identity as it is after a login
 *  comes through IdentityChangedEvent. The others do not log in, the defaults say so.
 *
 *  Every identity provider is also the profile of an account page (AUTH::IAccountProfile): the name and the id come from
 *  GetIdentity(), SetDisplayName() is the one of the provider, and the profile changes with IdentityChangedEvent.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CIIdentityProvider : public CORE::CObservingNotifier ,
                                                      public CORE::CIConfigurable    ,
                                                      public AUTH::IAccountProfile
{
    public:

    static const CORE::CEvent IdentityChangedEvent;     /**< CPlayerIdentity: the identity as it is now */

    static void RegisterEvents( void );

    CIIdentityProvider( void );

    virtual ~CIIdentityProvider();

    /** Gets the identity, made on first use where the provider makes it */
    virtual bool GetIdentity( CPlayerIdentity& identity ) = 0;

    virtual bool SetDisplayName( const CORE::CString& displayName ) GUCEF_VIRTUAL_OVERRIDE = 0;

    /** The name the provider is registered with in the factory, for example "local" */
    virtual const CORE::CString& GetProviderName( void ) const = 0;

    /**
     *  Starts a login with the external identity provider of that name, for example "google". The future holds the
     *  outcome. False when this provider does not log in or the login cannot start. Default: false
     */
    virtual bool Login( const CORE::CString& externalProviderName, CORE::CFutureResult& future );

    /**
     *  Starts the login that needs no help from the player: a session of the platform, a login of an earlier run.
     *  It never blocks. False when there is none to try. Default: false
     */
    virtual bool AutoLogin( CORE::CFutureResult& future );

    /** @name AUTH::IAccountProfile, from GetIdentity() */
    ///@{
    virtual CORE::CString GetDisplayName( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetProfileId( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CNotifier* GetProfileNotifier( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CEvent& GetProfileChangedEvent( void ) const GUCEF_VIRTUAL_OVERRIDE;
    ///@}

    private:

    CIIdentityProvider( const CIIdentityProvider& src );
    CIIdentityProvider& operator=( const CIIdentityProvider& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTAbstractFactory< CORE::CString, CIIdentityProvider, MT::CMutex > CIdentityProviderFactory;
typedef CIdentityProviderFactory::TProductPtr                                     CIIdentityProviderPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CIIDENTITYPROVIDER_H ? */
