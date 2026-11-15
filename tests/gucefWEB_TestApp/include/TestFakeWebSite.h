/*
 *  gucefWEB_TestApp: tests of the gucefWEB module, its web views
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

#ifndef GUCEF_WEBVIEWTEST_TESTFAKEWEBSITE_H
#define GUCEF_WEBVIEWTEST_TESTFAKEWEBSITE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_WEBVIEW_H
#include "gucefWEB_WebView.h"
#define GUCEF_WEB_WEBVIEW_H
#endif /* GUCEF_WEB_WEBVIEW_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEBVIEWTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The site the fake engine (CFakeWebViewEngine) loads: pages in memory, by address, with a small model of what a page
 *  has (title, lines of text, fields and buttons) and does when it loads (redirects, a session cookie on sign in, a
 *  popup, a download, permission requests, script dialogs, a frame of another host, a console message, window.close(),
 *  a loop that holds the engine thread, a crash). Deterministic, no network. Hosts:
 *      app.test            the site
 *      challenge.test      a second host, for frames of a challenge provider
 *      bad-cert.test       a host whose certificate is refused
 *  Any other host is unknown: a navigation to it fails as a network error, as a request that may not leave the
 *  machine would.
 */
class CFakeWebSite
{
    public:

    enum EElementKind
    {
        ELEMENT_INPUT       = 0 ,
        ELEMENT_PASSWORD        ,
        ELEMENT_BUTTON          ,
        ELEMENT_LINK                /**< navigates to 'target' when clicked */
    };

    struct SElement
    {
        CORE::CString id;
        EElementKind kind;
        CORE::CString label;
        CORE::CString value;
        CORE::CString target;           /**< link: where it goes; button "submit": where the form goes */
        bool trustedOnly;               /**< reacts to trusted input (injected by the engine) only, not to scripts */
        bool disabled;

        SElement( void );
    };
    typedef GUCEF::vector< SElement > TElementVector;

    struct SPage
    {
        CORE::CString title;
        CORE::CString::StringVector lines;
        TElementVector elements;
        CORE::CString redirectTo;
        bool requiresSession;           /**< without the cookie "sid" it redirects to /login */
        bool signsIn;                   /**< its button "submit" sets the cookie "sid" when both fields are filled */
        CORE::CString popupUrl;
        CORE::CString downloadUrl;
        CORE::CString downloadName;
        GUCEF::vector< UInt32 > permissions;    /**< WEB::TWebViewPermissionKind values */
        GUCEF::vector< UInt32 > dialogs;        /**< WEB::TWebViewScriptDialogKind values, shown one after the other */
        CORE::CString subframeUrl;
        CORE::CString consoleText;
        bool colourBlocks;              /**< four solid quadrants, for golden frame probes */
        bool closesWindow;
        bool loops;                     /**< holds the engine thread (CFakeWebViewEngine::SetLoopMs()) */
        bool crashes;
        bool redefinesQuerySelector;    /**< its own scripts replace document.querySelector in its world */

        SPage( void );
    };
    typedef struct SPage TPage;

    /** The colours of the quadrants of the colour page: top left, top right, bottom left, bottom right */
    static const UInt8 QuadrantColours[ 4 ][ 3 ];

    CFakeWebSite( void );

    /** The page of an address; false for an unknown host ('knownHost' false) or path (a "Not found" page is given then) */
    bool FindPage( const CORE::CString& url, TPage& page, bool& knownHost, bool& certificateError ) const;

    /** Adds or replaces a page of app.test: path "/x" */
    void SetPage( const CORE::CString& path, const TPage& page );

    /** "https://app.test" + path */
    static CORE::CString Url( const CORE::CString& path );

    /** Splits an address of the site into host and path (query and fragment dropped) */
    static bool SplitUrl( const CORE::CString& url, CORE::CString& host, CORE::CString& path );

    private:

    void BuildPages( void );

    typedef GUCEF::map< CORE::CString, TPage > TPageMap;

    TPageMap m_pages;           /**< by host + path */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEBVIEWTEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEBVIEWTEST_TESTFAKEWEBSITE_H ? */
