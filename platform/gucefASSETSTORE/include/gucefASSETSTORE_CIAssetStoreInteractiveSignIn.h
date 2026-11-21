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

#ifndef GUCEF_ASSETSTORE_CIASSETSTOREINTERACTIVESIGNIN_H
#define GUCEF_ASSETSTORE_CIASSETSTOREINTERACTIVESIGNIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSIGNINPROMPT_H
#include "gucefASSETSTORE_CAssetSignInPrompt.h"
#define GUCEF_ASSETSTORE_CASSETSIGNINPROMPT_H
#endif /* GUCEF_ASSETSTORE_CASSETSIGNINPROMPT_H ? */

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

/**
 *  The sign in of a store that needs no browser on the machine of the store but steps of a person: the person opens an
 *  address in their own browser and pastes back a code (the redirect modes "code-entry" and "manual" of gucefAUTH), or
 *  reads a device code. A store plugin offers one with CIAssetStoreProvider::GetInteractiveSignIn(); the asset store
 *  service drives it and turns its steps into prompts (CAssetSignInPrompt), which a remote client answers with
 *  CIAssetStoreService::AnswerPrompt().
 *
 *  Threading: the service calls every method on the thread that pumps the pulse generator of the service, which is where
 *  the sign in flows of gucefAUTH run, one call at a time; an implementation needs no lock of its own against the
 *  service. Update() is called at a short interval while the sign in runs, so an implementation polls the flow it started.
 *
 *  Secrets: an answer is a secret. It is never logged, never part of a reason or a message, and wiped when the
 *  implementation is done with it. A reason and a message are plain text for a person.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CIAssetStoreInteractiveSignIn
{
    public:

    CIAssetStoreInteractiveSignIn( void );

    virtual ~CIAssetStoreInteractiveSignIn();

    /**
     *  Starts the sign in. 'prompts' gets the steps of the person in the order they are shown (the kind, text, url, code,
     *  expiry and whether the answer is sensitive; the service sets the ids). False, with the reason, when it cannot start
     *  (the client credentials of the store are not provisioned, no flow engine, no https client)
     */
    virtual bool Begin( TAssetSignInPromptVector& prompts, CORE::CString& reason ) = 0;

    /**
     *  The answer of the person to the step of the kind (a prompt kind, "enterCode"). False, with the reason, when the
     *  answer is not taken (not a code, no sign in waits for it); the sign in then goes on and the person tries again
     */
    virtual bool Answer( const CORE::CString& promptKind, const AUTH::CSecretString& answer, CORE::CString& reason ) = 0;

    /** Stops a sign in that runs; the sign in ends as signed out */
    virtual void Cancel( void ) = 0;

    /**
     *  Looks at the sign in. IN_PROGRESS while it waits for the person or the store, SIGNED_IN once there is a session,
     *  FAILED once it ended without one ('message' says why), SIGNED_OUT when none runs. The two final states are given
     *  once: the next call says SIGNED_OUT until a new sign in begins
     */
    virtual TAssetStoreSignInState Update( CORE::CString& message ) = 0;

    private:

    CIAssetStoreInteractiveSignIn( const CIAssetStoreInteractiveSignIn& src );              /**< not implemented */
    CIAssetStoreInteractiveSignIn& operator=( const CIAssetStoreInteractiveSignIn& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CIASSETSTOREINTERACTIVESIGNIN_H ? */
