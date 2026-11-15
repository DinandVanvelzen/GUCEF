/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_WEBVIEWTYPES_H
#define GUCEF_WEB_WEBVIEWTYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_ETYPES_H
#include "gucefWEB_ETypes.h"            /* simple types of the module, CString */
#define GUCEF_WEB_ETYPES_H
#endif /* GUCEF_WEB_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace WEB {
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The values of all enumerations below are stable: new ones are only ever appended. Their names in settings files
 *  are given by the functions of gucefWEB_WebViewTypeNames.h.
 */

/** How a view is shown: its frames drawn by the application (offscreen), or in a window of the engine's own */
enum EWebViewMode
{
    WEBVIEWMODE_OFFSCREEN       = 0 ,
    WEBVIEWMODE_NATIVEWINDOW
};
typedef enum EWebViewMode TWebViewMode;

/** What the connection of the committed page is */
enum EWebViewSecurityState
{
    WEBVIEWSECURITY_NONE                = 0 ,   /**< nothing loaded, about:blank, a data: page */
    WEBVIEWSECURITY_SECURE                  ,   /**< https with a valid certificate */
    WEBVIEWSECURITY_INSECURE                ,   /**< http */
    WEBVIEWSECURITY_CERTIFICATE_ERROR           /**< https with a certificate that was refused */
};
typedef enum EWebViewSecurityState TWebViewSecurityState;

/** Why a load or a command failed; WEBVIEWERROR_NONE for success */
enum EWebViewError
{
    WEBVIEWERROR_NONE                   = 0 ,
    WEBVIEWERROR_NETWORK                    ,   /**< host not found, connection refused, timed out */
    WEBVIEWERROR_REFUSED_BY_POLICY          ,   /**< the policy of the view refused it (host not allowed, scheme, default world) */
    WEBVIEWERROR_CERTIFICATE                ,   /**< the certificate of the host was refused */
    WEBVIEWERROR_CANCELLED                  ,   /**< stopped, replaced by another navigation */
    WEBVIEWERROR_PROCESS_ENDED              ,   /**< the web process of the engine ended */
    WEBVIEWERROR_SHUTTING_DOWN              ,   /**< the engine is shutting down */
    WEBVIEWERROR_ENGINE_STALLED             ,   /**< the engine thread did not answer within the watchdog time */
    WEBVIEWERROR_NOT_SUPPORTED              ,   /**< the engine cannot do this (see its capabilities) */
    WEBVIEWERROR_VIEW_CLOSED                ,   /**< the view was closed */
    WEBVIEWERROR_SCRIPT                     ,   /**< the script threw: the text is the message of the exception */
    WEBVIEWERROR_RESULT_TOO_LARGE           ,   /**< the result is larger than the view allows (maxScriptResultBytes) */
    WEBVIEWERROR_INVALID_ARGUMENT           ,   /**< a malformed address, cookie or size */
    WEBVIEWERROR_OTHER
};
typedef enum EWebViewError TWebViewError;

/** What the policy does with something a page asks for */
enum EWebViewDecision
{
    WEBVIEWDECISION_ALLOW               = 0 ,
    WEBVIEWDECISION_REFUSE                  ,
    WEBVIEWDECISION_ASK                         /**< deferred: the owner of the view answers with ResolveDecision() */
};
typedef enum EWebViewDecision TWebViewDecision;

/** What kind of deferred decision a decision id belongs to */
enum EWebViewDecisionKind
{
    WEBVIEWDECISIONKIND_NAVIGATION      = 0 ,
    WEBVIEWDECISIONKIND_POPUP               ,
    WEBVIEWDECISIONKIND_DOWNLOAD            ,
    WEBVIEWDECISIONKIND_PERMISSION          ,
    WEBVIEWDECISIONKIND_CERTIFICATE_ERROR   ,
    WEBVIEWDECISIONKIND_SCRIPT_DIALOG
};
typedef enum EWebViewDecisionKind TWebViewDecisionKind;

enum EWebViewNavigationPolicy
{
    WEBVIEWNAVIGATION_ANY               = 0 ,   /**< any http and https host */
    WEBVIEWNAVIGATION_ALLOWLIST                 /**< only the hosts of the allow-list */
};
typedef enum EWebViewNavigationPolicy TWebViewNavigationPolicy;

enum EWebViewPopupPolicy
{
    WEBVIEWPOPUPS_REFUSE                = 0 ,
    WEBVIEWPOPUPS_SAMEVIEW                  ,   /**< opened in the same view when the navigation policy allows the address */
    WEBVIEWPOPUPS_ASK
};
typedef enum EWebViewPopupPolicy TWebViewPopupPolicy;

enum EWebViewDownloadPolicy
{
    WEBVIEWDOWNLOADS_REFUSE             = 0 ,
    WEBVIEWDOWNLOADS_ASK
};
typedef enum EWebViewDownloadPolicy TWebViewDownloadPolicy;

enum EWebViewPermissionPolicy
{
    WEBVIEWPERMISSIONS_DENYALL          = 0 ,
    WEBVIEWPERMISSIONS_ASK
};
typedef enum EWebViewPermissionPolicy TWebViewPermissionPolicy;

/** Certificate errors are never accepted by themselves: refused, or asked when the profile is not one of a sign-in */
enum EWebViewCertificateErrorPolicy
{
    WEBVIEWCERTERRORS_REFUSE            = 0 ,
    WEBVIEWCERTERRORS_ASK
};
typedef enum EWebViewCertificateErrorPolicy TWebViewCertificateErrorPolicy;

enum EWebViewScriptDialogPolicy
{
    WEBVIEWSCRIPTDIALOGS_SHOW           = 0 ,   /**< deferred to the owner, who shows them */
    WEBVIEWSCRIPTDIALOGS_REFUSE                 /**< alert dismissed, confirm and prompt answered with cancel */
};
typedef enum EWebViewScriptDialogPolicy TWebViewScriptDialogPolicy;

enum EWebViewThirdPartyCookies
{
    WEBVIEWTHIRDPARTYCOOKIES_BLOCK      = 0 ,
    WEBVIEWTHIRDPARTYCOOKIES_ALLOW
};
typedef enum EWebViewThirdPartyCookies TWebViewThirdPartyCookies;

enum EWebViewProxyMode
{
    WEBVIEWPROXY_NONE                   = 0 ,
    WEBVIEWPROXY_SYSTEM                     ,
    WEBVIEWPROXY_EXPLICIT
};
typedef enum EWebViewProxyMode TWebViewProxyMode;

enum EWebViewHardwareAcceleration
{
    WEBVIEWHWACCEL_NEVER                = 0 ,   /**< rendering on the CPU: no GL, no DMA-BUF */
    WEBVIEWHWACCEL_AUTO
};
typedef enum EWebViewHardwareAcceleration TWebViewHardwareAcceleration;

enum EWebViewPermissionKind
{
    WEBVIEWPERMISSION_CAMERA            = 0 ,
    WEBVIEWPERMISSION_MICROPHONE            ,
    WEBVIEWPERMISSION_GEOLOCATION           ,
    WEBVIEWPERMISSION_NOTIFICATIONS         ,
    WEBVIEWPERMISSION_CLIPBOARD             ,
    WEBVIEWPERMISSION_OTHER
};
typedef enum EWebViewPermissionKind TWebViewPermissionKind;

enum EWebViewScriptDialogKind
{
    WEBVIEWDIALOG_ALERT                 = 0 ,
    WEBVIEWDIALOG_CONFIRM                   ,
    WEBVIEWDIALOG_PROMPT                    ,
    WEBVIEWDIALOG_BEFOREUNLOAD
};
typedef enum EWebViewScriptDialogKind TWebViewScriptDialogKind;

enum EWebViewConsoleLevel
{
    WEBVIEWCONSOLE_DEBUG                = 0 ,
    WEBVIEWCONSOLE_INFO                     ,
    WEBVIEWCONSOLE_WARNING                  ,
    WEBVIEWCONSOLE_ERROR
};
typedef enum EWebViewConsoleLevel TWebViewConsoleLevel;

enum EWebViewTermination
{
    WEBVIEWTERMINATION_CRASH            = 0 ,
    WEBVIEWTERMINATION_OUT_OF_MEMORY        ,
    WEBVIEWTERMINATION_KILLED
};
typedef enum EWebViewTermination TWebViewTermination;

enum EWebViewEditCommand
{
    WEBVIEWEDIT_CUT                     = 0 ,
    WEBVIEWEDIT_COPY                        ,
    WEBVIEWEDIT_PASTE                       ,
    WEBVIEWEDIT_SELECTALL                   ,
    WEBVIEWEDIT_UNDO                        ,
    WEBVIEWEDIT_REDO
};
typedef enum EWebViewEditCommand TWebViewEditCommand;

/** Pointer shapes of a page; the values are those of gucefGUI's TCursorShape, so a GUI casts them */
enum EWebViewCursorShape
{
    WEBVIEWCURSOR_ARROW                 = 0  ,
    WEBVIEWCURSOR_TEXT                  = 1  ,
    WEBVIEWCURSOR_RESIZE_ALL            = 2  ,
    WEBVIEWCURSOR_RESIZE_NS             = 3  ,
    WEBVIEWCURSOR_RESIZE_EW             = 4  ,
    WEBVIEWCURSOR_RESIZE_NESW           = 5  ,
    WEBVIEWCURSOR_RESIZE_NWSE           = 6  ,
    WEBVIEWCURSOR_HAND                  = 7  ,
    WEBVIEWCURSOR_WAIT                  = 8  ,
    WEBVIEWCURSOR_PROGRESS              = 9  ,
    WEBVIEWCURSOR_NOT_ALLOWED           = 10
};
typedef enum EWebViewCursorShape TWebViewCursorShape;

enum EWebViewInputKind
{
    WEBVIEWINPUT_NONE                   = 0 ,
    WEBVIEWINPUT_POINTER_MOVE               ,
    WEBVIEWINPUT_POINTER_DOWN               ,
    WEBVIEWINPUT_POINTER_UP                 ,
    WEBVIEWINPUT_POINTER_LEAVE              ,
    WEBVIEWINPUT_WHEEL                      ,
    WEBVIEWINPUT_KEY_DOWN                   ,
    WEBVIEWINPUT_KEY_UP                     ,
    WEBVIEWINPUT_TEXT                       ,   /**< text after keyboard layout and input method: an input method commit where the engine has one */
    WEBVIEWINPUT_FOCUS_IN                   ,
    WEBVIEWINPUT_FOCUS_OUT
};
typedef enum EWebViewInputKind TWebViewInputKind;

enum EWebViewSameSite
{
    WEBVIEWSAMESITE_UNSET               = 0 ,
    WEBVIEWSAMESITE_NONE                    ,
    WEBVIEWSAMESITE_LAX                     ,
    WEBVIEWSAMESITE_STRICT
};
typedef enum EWebViewSameSite TWebViewSameSite;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace WEB */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_WEBVIEWTYPES_H ? */
