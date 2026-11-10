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

#ifndef GUCEF_AUTH_IOPENID2PROVIDERPROFILE_H
#define GUCEF_AUTH_IOPENID2PROVIDERPROFILE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

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

class COpenId2Assertion;

/**
 *  What the generic OpenID 2.0 support of the module (COpenId2, the flow engine) cannot know about one provider: the
 *  provider specific parts of a login. The protocol is the same for every OpenID 2.0 provider (the checkid_setup request,
 *  the positive assertion, the nonce, check_authentication, the return address with an optional relay); what differs is
 *  what an identity looks like and what the machine of the person can say about the account.
 *
 *  A provider that is an OpenID 2.0 provider is two things that a plugin registers when it loads, and takes away again
 *  when it unloads:
 *    - a descriptor (CAuthProviderDescriptor, protocol AUTHPROTOCOL_OPENID2) in CAuthGlobal::GetDescriptorRegistry(): the
 *      name, the display name, the account namespace and the endpoint of the provider
 *    - a profile, an implementation of this interface, in CAuthGlobal::GetOpenId2Profiles() under the same name
 *  A descriptor with the protocol OpenID 2.0 and no profile is not supported by the engine: it is not available and
 *  assertions that are presented for it are refused. The module itself knows no OpenID 2.0 provider.
 *
 *  A profile is used from the thread that pumps the pulse generator (the login) and from the threads that verify proofs
 *  (a server): the implementation must be usable from several threads at once, and it has to stay valid until it is
 *  unregistered, which a plugin does only when no login or verification runs any more.
 */
class GUCEF_AUTH_EXPORT_CPP IOpenId2ProviderProfile
{
    public:

    IOpenId2ProviderProfile( void );

    virtual ~IOpenId2ProviderProfile();

    /** The name of the provider (the name of its descriptor) that this profile belongs to: the key in the registry of profiles */
    virtual const CORE::CString& GetProviderName( void ) const = 0;

    /**
     *  The subject of the account reference (the part after the account namespace) from the identity of a positive
     *  assertion. This is where a provider says what its identities look like and what it requires of claimed_id and
     *  identity (that they are the same, in the common case): every pattern a claimed id has to follow is checked here, and
     *  anything else is refused with false. The subject is made of characters CAccountRef accepts.
     *
     *  @param endpoint the endpoint of the provider (the op_endpoint the assertion has to come from)
     *  @param claimedId the value of openid.claimed_id
     *  @param identity the value of openid.identity
     *  @param subject receives the subject; empty when false is returned
     */
    virtual bool ExtractSubject( const CORE::CString& endpoint ,
                                 const CORE::CString& claimedId ,
                                 const CORE::CString& identity  ,
                                 CORE::CString& subject         ) const = 0;

    /**
     *  Realm policy: whether the provider takes the page of a relay (the parameter relayUrl of the settings of the provider) as
     *  the realm and the return address of the login. A provider that shows the realm to the person is helped by it, as the
     *  loopback address of the application is a poor thing to show. Default: true
     */
    virtual bool IsRelayAllowed( void ) const;

    /**
     *  The name of the person at the provider as far as a verified assertion says it (an extension of the provider in the
     *  response). Default: false, the assertion has no name
     */
    virtual bool GetDisplayName( const COpenId2Assertion& assertion, CORE::CString& displayName ) const;

    /**
     *  A hint of who this machine says the person is at the provider, for the offer to log in (CIAuthProvider::GetDetectedAccountHint()
     *  forwards to it). True when something was detected; see CIAuthProvider::GetDetectedAccountHint() for what the arguments mean.
     *  Default: nothing is detected
     */
    virtual bool GetDetectedAccountHint( CORE::CString& displayName, bool& isRunning ) const;

    /** Forgets what the detection found, so that the next GetDetectedAccountHint() looks again. Default: nothing is kept */
    virtual void ForgetDetectedAccountHint( void );

    private:

    IOpenId2ProviderProfile( const IOpenId2ProviderProfile& src );              /**< not implemented */
    IOpenId2ProviderProfile& operator=( const IOpenId2ProviderProfile& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_IOPENID2PROVIDERPROFILE_H ? */
