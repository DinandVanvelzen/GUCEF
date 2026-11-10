/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_BROWSERLAUNCHER_H
#define GUCEF_CORE_BROWSERLAUNCHER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Replaces the platform code of OpenUrlInSystemBrowser(). A plugin that knows how to open a browser on a platform
 *  gucefCORE does not (an Android Intent) installs one, tests install one that records the URL instead of opening a browser.
 */
class GUCEF_CORE_PUBLIC_CPP CIBrowserLauncherHandler
{
    public:

    CIBrowserLauncherHandler( void );

    virtual ~CIBrowserLauncherHandler();

    /**
     *  Opens the URL in the browser of the system. Only called with a URL that passed IsOpenableBrowserUrl().
     *  May be called from any thread.
     *  @return true when the URL was handed to a browser
     */
    virtual bool OpenUrl( const CString& url ) = 0;
};

/*-------------------------------------------------------------------------*/

/** The kind of credential a CIActivationTokenProvider hands over */
enum EActivationTokenKind
{
    ACTIVATIONTOKEN_XDG_ACTIVATION = 0,     /**< a token of xdg-activation-v1 (Wayland, GNOME and KDE): the launched program gets XDG_ACTIVATION_TOKEN and DESKTOP_STARTUP_ID with it, as GLib does */
    ACTIVATIONTOKEN_STARTUP_ID = 1          /**< a startup notification id of the X11 desktop specification (carries the time of the last user input): the launched program gets DESKTOP_STARTUP_ID */
};

typedef enum EActivationTokenKind TActivationTokenKind;

/**
 *  Gives the launcher the proof of a recent user action that a desktop wants before it lets a program that was started
 *  by another one come to the front: without it the window of a browser that was opened from a game appears behind the
 *  game. The provider of a window system (the Wayland window driver, the X11 window driver) installs itself with
 *  SetActivationTokenProvider(); only the launcher of Linux asks for the token (Windows and macOS let the launched
 *  default browser take the foreground from the program that got the last input).
 *
 *  The token is a credential that is good for one activation: it is only handed to the child process through its
 *  environment and is never logged and never kept.
 */
class GUCEF_CORE_PUBLIC_CPP CIActivationTokenProvider
{
    public:

    CIActivationTokenProvider( void );

    virtual ~CIActivationTokenProvider();

    /**
     *  Makes a token for the program that is about to be started. Called on the thread that opens the URL, which has to be
     *  the thread the window system is used from: a provider that is called from another thread says false. The provider
     *  never waits longer than maxWaitInMs.
     *  @param token        receives the token or the startup id; empty and false when there is none
     *  @param kind         receives what kind of token it is
     *  @param maxWaitInMs  the longest time the provider may spend, 0 for no waiting at all
     *  @return true when a token was made
     */
    virtual bool GetActivationToken( CString& token, TActivationTokenKind& kind, UInt32 maxWaitInMs ) = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  Starts the helper program that hands a URL to the browser (Linux xdg-open, macOS open) with the environment the
 *  launcher made for it. The default starts a process; a test installs one that records what it is given and starts nothing.
 */
class GUCEF_CORE_PUBLIC_CPP CIBrowserHelperLauncher
{
    public:

    CIBrowserHelperLauncher( void );

    virtual ~CIBrowserHelperLauncher();

    /**
     *  @param exePath      the helper, found through the search path
     *  @param args         its arguments, the URL is the only one
     *  @param envOverrides what is set in its environment on top of the environment of the caller
     *  @param isolated     the helper, and the browser it starts, must have no terminal and nothing of the caller: /dev/null as
     *                      standard input, output and error, a session of its own and no other open file descriptor of the caller
     *  @return true when the helper was started
     */
    virtual bool LaunchHelper( const CString& exePath                      ,
                               const CProcess::TStringVector& args         ,
                               const CProcess::TStringMap& envOverrides    ,
                               bool isolated                               ) = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The longest time the launcher gives a CIActivationTokenProvider to make a token, in milliseconds: the browser is opened without one when it takes longer */
static const UInt32 ActivationTokenMaxWaitInMs = 200;

/** The names of the environment variables the launcher sets for the program it starts */
static const char* const ActivationTokenEnvironmentVariable = "XDG_ACTIVATION_TOKEN";
static const char* const StartupIdEnvironmentVariable = "DESKTOP_STARTUP_ID";

/** The longest URL OpenUrlInSystemBrowser() takes, in bytes */
static const UInt32 BrowserLauncherMaxUrlLength = 2048;

/**
 *  Whether a URL may be handed to a browser: an "http://" or "https://" URL (the scheme in any case) with a host,
 *  at most BrowserLauncherMaxUrlLength bytes, and only the characters  A-Z a-z 0-9 - . _ ~ : / ? # [ ] @ ! & ( ) * + , ; = %
 *  Whitespace, control characters, non ASCII bytes and the characters that break out of a command line or a markup
 *  (space " ' \ ` $ < > ^ { } |) are refused: the caller percent encodes what it appends to a URL.
 *  A URL with user information ("https://user@host/") is refused as well, as it is a way to disguise the real host.
 */
GUCEF_CORE_PUBLIC_CPP bool
IsOpenableBrowserUrl( const CString& url );

/**
 *  Opens the URL in the browser of the system, without a shell and without waiting for the browser:
 *  Windows ShellExecuteW "open", macOS "open", Linux "xdg-open", Emscripten window.open.
 *  So that the window of the browser comes to the front and not up behind the program, Linux asks the installed
 *  CIActivationTokenProvider for a token (at most ActivationTokenMaxWaitInMs) and puts it in the environment of
 *  xdg-open as XDG_ACTIVATION_TOKEN (and DESKTOP_STARTUP_ID) or, for an X11 startup id, DESKTOP_STARTUP_ID; without a
 *  provider or token nothing is set. Windows lets any process take the foreground (AllowSetForegroundWindow) first.
 *  Anything but a URL that passes IsOpenableBrowserUrl() is refused (false) and never reaches the handler or the system.
 *  A handler installed with SetBrowserLauncherHandler() takes the place of the platform code on every platform.
 *  Android has no platform code in gucefCORE: without a handler it returns false.
 *  The URL is not written to the log, it can carry a state or a nonce.
 *  @return true when the URL was handed to a browser
 */
GUCEF_CORE_PUBLIC_CPP bool
OpenUrlInSystemBrowser( const CString& url );

/**
 *  Installs the handler that OpenUrlInSystemBrowser() uses, NULL goes back to the platform code.
 *  The handler is not owned: it must stay valid until it is replaced or removed.
 *  @return the handler that was installed before, NULL for none
 */
GUCEF_CORE_PUBLIC_CPP CIBrowserLauncherHandler*
SetBrowserLauncherHandler( CIBrowserLauncherHandler* handler );

/** The installed handler, NULL for none */
GUCEF_CORE_PUBLIC_CPP CIBrowserLauncherHandler*
GetBrowserLauncherHandler( void );

/**
 *  Installs the provider the Linux launcher gets its activation token from, NULL for none. The provider is not owned: it
 *  has to be removed before it is destroyed. Replacing or removing it waits for a call that is going on in another thread.
 *  @return the provider that was installed before, NULL for none
 */
GUCEF_CORE_PUBLIC_CPP CIActivationTokenProvider*
SetActivationTokenProvider( CIActivationTokenProvider* provider );

/** The installed provider, NULL for none */
GUCEF_CORE_PUBLIC_CPP CIActivationTokenProvider*
GetActivationTokenProvider( void );

/**
 *  Installs the launcher of the helper program that Linux and macOS use for OpenUrlInSystemBrowser() (a handler of
 *  SetBrowserLauncherHandler() still comes first), NULL goes back to starting a process. Not owned.
 *  @return the launcher that was installed before, NULL for none
 */
GUCEF_CORE_PUBLIC_CPP CIBrowserHelperLauncher*
SetBrowserHelperLauncher( CIBrowserHelperLauncher* launcher );

/**
 *  Makes the environment of the browser helper from a token: XDG_ACTIVATION_TOKEN and DESKTOP_STARTUP_ID for a token of
 *  xdg-activation, DESKTOP_STARTUP_ID for a startup id. Nothing is added for an empty token. Returns whether something was added.
 */
GUCEF_CORE_PUBLIC_CPP bool
AddActivationTokenToEnvironment( const CString& token, TActivationTokenKind kind, CProcess::TStringMap& envOverrides );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_BROWSERLAUNCHER_H ? */
