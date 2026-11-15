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

#ifndef GUCEF_ASSETSTORE_CCHROMESIGNINBROWSER_H
#define GUCEF_ASSETSTORE_CCHROMESIGNINBROWSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */
#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */
#ifndef GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#include "gucefWEB_CChromeDevToolsClient.h"
#define GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#endif /* GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H ? */
#ifndef GUCEF_ASSETSTORE_CIWEBSIGNINBROWSER_H
#include "gucefASSETSTORE_CIWebSignInBrowser.h"
#define GUCEF_ASSETSTORE_CIWEBSIGNINBROWSER_H
#endif /* GUCEF_ASSETSTORE_CIWEBSIGNINBROWSER_H ? */

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
 *  The browser of a web session sign in: Chrome, Chromium, Edge or Brave (CChromeBrowserLocator), started with its own
 *  profile folder and a debugging port on the loopback interface, and driven through the Chrome DevTools protocol
 *  (WEB::CChromeDevToolsClient) over the connection to the browser with the page attached as a session.
 *
 *  The port is the one in the settings, or with 0 the one the browser picked and wrote to DevToolsActivePort in its
 *  profile folder. The page is only asked the fixed questions of CWebPageScriptHelpers; values are typed with
 *  Input.insertText. Cookies are read and cleared with Storage.getCookies and Storage.clearCookies.
 *
 *  The DevTools client runs on a pulse generator of this object, driven by the thread that called Start(): every member
 *  is to be called on that thread, which waits for the answers by pumping that pulse generator only.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CChromeSignInBrowser : public CIWebSignInBrowser
{
    public:

    static const CORE::CString ActivePortFileName;      /**< "DevToolsActivePort" */

    CChromeSignInBrowser( void );
    virtual ~CChromeSignInBrowser() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Start( const CWebSignInBrowserSettings& settings, CORE::CString& problem ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Stop( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Release( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsAlive( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Navigate( const CORE::CString& url ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetLocation( CORE::CString& url ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TWebElementState GetElementState( const CORE::CString& selector ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool ContainsText( const CORE::CString& text, bool& contains ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool FillField( const CORE::CString& selector, const AUTH::CSecretString& value ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Click( const CORE::CString& selector ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Submit( const CORE::CString& selector ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetCookies( TWebSessionCookieVector& cookies ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool ClearCookies( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** The arguments the browser is started with for the settings and profile folder (without the executable) */
    static void BuildArguments( const CWebSignInBrowserSettings& settings, const CORE::CString& profileDirectory, CORE::CProcess::TStringVector& arguments );

    /** Reads the port and the browser path of DevToolsActivePort; false when the file is not (yet) complete */
    static bool ReadActivePortFile( const CORE::CString& path, UInt16& port, CORE::CString& browserPath );

    private:

    CChromeSignInBrowser( const CChromeSignInBrowser& src );                /**< not implemented */
    CChromeSignInBrowser& operator=( const CChromeSignInBrowser& src );     /**< not implemented */

    bool Evaluate( const CORE::CString& expression, CORE::CDataNode& result );
    bool ConnectToBrowser( UInt16 port, const CORE::CString& browserPath, UInt32 timeoutInMs, CORE::CString& problem );
    void Disconnect( bool closeBrowser );

    private:

    CORE::CProcess m_process;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    WEB::CChromeDevToolsClient* m_client;
    CORE::CString m_pageSessionId;
    CWebSignInBrowserSettings m_settings;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CCHROMESIGNINBROWSER_H ? */
