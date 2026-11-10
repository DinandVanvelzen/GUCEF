/*
 *  gucefAUTHGUI: GUCEF module providing the user interface for logging in with external identities and the account page of settings
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

/*
 *  This is the main include header for the gucefAUTHGUI library.
 *  It is part of a platform called GUCEF.
 *  All classes in this library are GUI forms on gucefGUI for logging in with external identities (gucefAUTH): the window to
 *  log in with and the settings page "Account" for any tool.
 */

#ifndef GUCEF_AUTHGUI_H
#define GUCEF_AUTHGUI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_AUTHGUI_MACROS_H
#include "gucefAUTHGUI_macros.h"
#define GUCEF_AUTHGUI_MACROS_H
#endif /* GUCEF_AUTHGUI_MACROS_H ? */

/*---------------------------------------------------------------------------/

        C++ includes

/---------------------------------------------------------------------------*/
#ifdef __cplusplus

#ifndef GUCEF_AUTHGUI_CAUTHGUIGLOBAL_H
#include "gucefAUTHGUI_CAuthGuiGlobal.h"
#define GUCEF_AUTHGUI_CAUTHGUIGLOBAL_H
#endif /* GUCEF_AUTHGUI_CAUTHGUIGLOBAL_H ? */

#ifndef GUCEF_AUTHGUI_UTILS_H
#include "gucefAUTHGUI_Utils.h"
#define GUCEF_AUTHGUI_UTILS_H
#endif /* GUCEF_AUTHGUI_UTILS_H ? */

#ifndef GUCEF_AUTHGUI_CACCOUNTSPANEL_H
#include "gucefAUTHGUI_CAccountsPanel.h"
#define GUCEF_AUTHGUI_CACCOUNTSPANEL_H
#endif /* GUCEF_AUTHGUI_CACCOUNTSPANEL_H ? */

#ifndef GUCEF_AUTHGUI_CLOGINFORM_H
#include "gucefAUTHGUI_CLoginForm.h"
#define GUCEF_AUTHGUI_CLOGINFORM_H
#endif /* GUCEF_AUTHGUI_CLOGINFORM_H ? */

#ifndef GUCEF_AUTHGUI_CACCOUNTSETTINGSFORM_H
#include "gucefAUTHGUI_CAccountSettingsForm.h"
#define GUCEF_AUTHGUI_CACCOUNTSETTINGSFORM_H
#endif /* GUCEF_AUTHGUI_CACCOUNTSETTINGSFORM_H ? */

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHGUI_H ? */
