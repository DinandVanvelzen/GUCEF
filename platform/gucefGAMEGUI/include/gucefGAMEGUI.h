/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
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

/*
 *  This is the main include header for the gucefGAMEGUI library.
 *  It is part of a platform called GUCEF.
 *  All classes in this library are prebaked GUI forms for rapid game development
 *  on gucefGUI: the multiplayer screens (game browser, lobby, chat, ...) on gucefMULTIPLAY
 */

#ifndef GUCEF_GAMEGUI_H
#define GUCEF_GAMEGUI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_GAMEGUI_MACROS_H
#include "gucefGAMEGUI_macros.h"
#define GUCEF_GAMEGUI_MACROS_H
#endif /* GUCEF_GAMEGUI_MACROS_H ? */

/*---------------------------------------------------------------------------/

        C++ includes

/---------------------------------------------------------------------------*/
#ifdef __cplusplus

#ifndef GUCEF_GAMEGUI_CGAMEGUIGLOBAL_H
#include "gucefGAMEGUI_CGameGuiGlobal.h"
#define GUCEF_GAMEGUI_CGAMEGUIGLOBAL_H
#endif /* GUCEF_GAMEGUI_CGAMEGUIGLOBAL_H ? */

#ifndef GUCEF_GAMEGUI_UTILS_H
#include "gucefGAMEGUI_Utils.h"
#define GUCEF_GAMEGUI_UTILS_H
#endif /* GUCEF_GAMEGUI_UTILS_H ? */

#ifndef GUCEF_GAMEGUI_CSERVERBROWSERFORM_H
#include "gucefGAMEGUI_CServerBrowserForm.h"
#define GUCEF_GAMEGUI_CSERVERBROWSERFORM_H
#endif /* GUCEF_GAMEGUI_CSERVERBROWSERFORM_H ? */

#ifndef GUCEF_GAMEGUI_CCREATELOBBYFORM_H
#include "gucefGAMEGUI_CCreateLobbyForm.h"
#define GUCEF_GAMEGUI_CCREATELOBBYFORM_H
#endif /* GUCEF_GAMEGUI_CCREATELOBBYFORM_H ? */

#ifndef GUCEF_GAMEGUI_CLOBBYFORM_H
#include "gucefGAMEGUI_CLobbyForm.h"
#define GUCEF_GAMEGUI_CLOBBYFORM_H
#endif /* GUCEF_GAMEGUI_CLOBBYFORM_H ? */

#ifndef GUCEF_GAMEGUI_CLOBBYSETTINGSPANELFORM_H
#include "gucefGAMEGUI_CLobbySettingsPanelForm.h"
#define GUCEF_GAMEGUI_CLOBBYSETTINGSPANELFORM_H
#endif /* GUCEF_GAMEGUI_CLOBBYSETTINGSPANELFORM_H ? */

#ifndef GUCEF_GAMEGUI_CCHATFORM_H
#include "gucefGAMEGUI_CChatForm.h"
#define GUCEF_GAMEGUI_CCHATFORM_H
#endif /* GUCEF_GAMEGUI_CCHATFORM_H ? */

#ifndef GUCEF_GAMEGUI_CVOTEPROMPTFORM_H
#include "gucefGAMEGUI_CVotePromptForm.h"
#define GUCEF_GAMEGUI_CVOTEPROMPTFORM_H
#endif /* GUCEF_GAMEGUI_CVOTEPROMPTFORM_H ? */

#ifndef GUCEF_GAMEGUI_CTURNWAITOVERLAYFORM_H
#include "gucefGAMEGUI_CTurnWaitOverlayForm.h"
#define GUCEF_GAMEGUI_CTURNWAITOVERLAYFORM_H
#endif /* GUCEF_GAMEGUI_CTURNWAITOVERLAYFORM_H ? */

#ifndef GUCEF_GAMEGUI_CCONNECTINGFORM_H
#include "gucefGAMEGUI_CConnectingForm.h"
#define GUCEF_GAMEGUI_CCONNECTINGFORM_H
#endif /* GUCEF_GAMEGUI_CCONNECTINGFORM_H ? */

#ifndef GUCEF_GAMEGUI_CDIRECTCONNECTFORM_H
#include "gucefGAMEGUI_CDirectConnectForm.h"
#define GUCEF_GAMEGUI_CDIRECTCONNECTFORM_H
#endif /* GUCEF_GAMEGUI_CDIRECTCONNECTFORM_H ? */

#ifndef GUCEF_GAMEGUI_CPLAYERPROFILEFORM_H
#include "gucefGAMEGUI_CPlayerProfileForm.h"
#define GUCEF_GAMEGUI_CPLAYERPROFILEFORM_H
#endif /* GUCEF_GAMEGUI_CPLAYERPROFILEFORM_H ? */

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_H ? */
