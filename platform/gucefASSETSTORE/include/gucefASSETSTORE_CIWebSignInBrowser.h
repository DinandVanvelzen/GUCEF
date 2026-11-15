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

#ifndef GUCEF_ASSETSTORE_CIWEBSIGNINBROWSER_H
#define GUCEF_ASSETSTORE_CIWEBSIGNINBROWSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */
#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_CWEBSESSIONCOOKIE_H
#include "gucefASSETSTORE_CWebSessionCookie.h"
#define GUCEF_ASSETSTORE_CWEBSESSIONCOOKIE_H
#endif /* GUCEF_ASSETSTORE_CWEBSESSIONCOOKIE_H ? */
#ifndef GUCEF_ASSETSTORE_CWEBSIGNINBROWSERSETTINGS_H
#include "gucefASSETSTORE_CWebSignInBrowserSettings.h"
#define GUCEF_ASSETSTORE_CWEBSIGNINBROWSERSETTINGS_H
#endif /* GUCEF_ASSETSTORE_CWEBSIGNINBROWSERSETTINGS_H ? */

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
 *  The browser a web session sign in drives (CWebSessionSignIn): what it needs of a page, no more. CChromeSignInBrowser
 *  is the real one (a Chromium based browser through the Chrome DevTools protocol); tests put a double in its place.
 *
 *  An implementation is used by one thread at a time: the thread that runs the sign in, from Start() to Stop() or
 *  Release(). Members other than Start() fail (or answer WEBELEMENT_STATE_NO_ANSWER) when the browser is not there.
 *  Values of credentials are only passed to FillField() and must never be logged by an implementation.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CIWebSignInBrowser
{
    public:

    CIWebSignInBrowser( void );
    virtual ~CIWebSignInBrowser();

    /**
     *  Starts the browser with the settings (CWebSessionSignIn sets the profile folder) on an empty page
     *  @param problem  why it could not, for the owner and a log line
     */
    virtual bool Start( const CWebSignInBrowserSettings& settings, CORE::CString& problem ) = 0;

    /** Closes the browser */
    virtual void Stop( void ) = 0;

    /** Lets go of the browser and leaves it open (CWebSignInBrowserSettings::keepOpen) */
    virtual void Release( void ) = 0;

    /** Whether the browser is still there: false once the owner closed it */
    virtual bool IsAlive( void ) = 0;

    /** Shows the address */
    virtual bool Navigate( const CORE::CString& url ) = 0;

    /** The address of the page */
    virtual bool GetLocation( CORE::CString& url ) = 0;

    virtual TWebElementState GetElementState( const CORE::CString& selector ) = 0;

    /** Whether the text of the page holds the text; false when the page could not be asked */
    virtual bool ContainsText( const CORE::CString& text, bool& contains ) = 0;

    /** Types the value into the field, replacing what was in it, as a person would */
    virtual bool FillField( const CORE::CString& selector, const AUTH::CSecretString& value ) = 0;

    virtual bool Click( const CORE::CString& selector ) = 0;

    /** Submits the form (the element or the form it is in) */
    virtual bool Submit( const CORE::CString& selector ) = 0;

    /** Every cookie of the browser profile, the caller picks those of the store */
    virtual bool GetCookies( TWebSessionCookieVector& cookies ) = 0;

    /** Removes every cookie of the browser profile, so a session that ended is not taken again */
    virtual bool ClearCookies( void ) = 0;

    private:

    CIWebSignInBrowser( const CIWebSignInBrowser& src );                /**< not implemented */
    CIWebSignInBrowser& operator=( const CIWebSignInBrowser& src );     /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CIWEBSIGNINBROWSER_H ? */
