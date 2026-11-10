/*
 *  gucefINPUT: GUCEF module providing input device interaction
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#ifndef GUCEFINPUT_H
#define GUCEFINPUT_H

#ifdef __cplusplus
namespace GUCEF {

/**
 *      This is the main include header for the gucefINPUT library.
 *      It is part of a framework called GUCEF.
 *      All classes in this library have to do with user input management and
 *      the interfacing with the hardware.
 */
namespace INPUT {} }
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_INPUT_ETYPES_H
#include "gucefINPUT_ETypes.h"
#define GUCEF_INPUT_ETYPES_H
#endif /* GUCEF_INPUT_ETYPES_H ? */

#ifndef GUCEF_INPUT_CONFIG_H
#include "gucefINPUT_config.h"
#define GUCEF_INPUT_CONFIG_H
#endif /* GUCEF_INPUT_CONFIG_H ? */

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_INPUT_MACROS_H
#include "gucefINPUT_macros.h"
#define GUCEF_INPUT_MACROS_H
#endif /* GUCEF_INPUT_MACROS_H ? */

/*---------------------------------------------------------------------------/

        C++ includes

/---------------------------------------------------------------------------*/
#ifdef __cplusplus

#ifndef GUCEF_INPUT_CINPUTCONTROLLER_H
#include "CInputController.h"
#define GUCEF_INPUT_CINPUTCONTROLLER_H
#endif /* GUCEF_INPUT_CINPUTCONTROLLER_H ? */

#ifndef GUCEF_INPUT_CINPUTCONTEXT_H
#include "CInputContext.h"
#define GUCEF_INPUT_CINPUTCONTEXT_H
#endif /* GUCEF_INPUT_CINPUTCONTEXT_H ? */

#ifndef GUCEF_INPUT_CINPUTACTIONMAP_H
#include "CInputActionMap.h"
#define GUCEF_INPUT_CINPUTACTIONMAP_H
#endif /* GUCEF_INPUT_CINPUTACTIONMAP_H ? */

#ifndef GUCEF_INPUT_ACTIONEVENTDATA_H
#include "gucefINPUT_ActionEventData.h"
#define GUCEF_INPUT_ACTIONEVENTDATA_H
#endif /* GUCEF_INPUT_ACTIONEVENTDATA_H ? */

#ifndef GUCEF_INPUT_CTCONCRETEACTIONEVENTDATA_H
#include "gucefINPUT_CTConcreteActionEventData.h"
#define GUCEF_INPUT_CTCONCRETEACTIONEVENTDATA_H
#endif /* GUCEF_INPUT_CTCONCRETEACTIONEVENTDATA_H ? */

#ifndef GUCEF_INPUT_CACTIONEVENTDATA_H
#include "gucefINPUT_CActionEventData.h"
#define GUCEF_INPUT_CACTIONEVENTDATA_H
#endif /* GUCEF_INPUT_CACTIONEVENTDATA_H ? */

#ifndef GUCEF_INPUT_CINPUTDEVICE_H
#include "gucefINPUT_CInputDevice.h"
#define GUCEF_INPUT_CINPUTDEVICE_H
#endif /* GUCEF_INPUT_CINPUTDEVICE_H ? */

#ifndef GUCEF_INPUT_CINPUTDRIVER_H
#include "gucefINPUT_CInputDriver.h"
#define GUCEF_INPUT_CINPUTDRIVER_H
#endif /* GUCEF_INPUT_CINPUTDRIVER_H ? */

#ifndef GUCEF_INPUT_CINPUTOBSERVERSWITCH_H
#include "gucefINPUT_CInputObserverSwitch.h"
#define GUCEF_INPUT_CINPUTOBSERVERSWITCH_H
#endif /* GUCEF_INPUT_CINPUTOBSERVERSWITCH_H ? */

#ifndef GUCEF_INPUT_CKEYBOARD_H
#include "gucefINPUT_CKeyboard.h"
#define GUCEF_INPUT_CKEYBOARD_H
#endif /* GUCEF_INPUT_CKEYBOARD_H ? */

#ifndef GUCEF_INPUT_CKEYMODSTATECHANGEDEVENTDATA_H
#include "gucefINPUT_CKeyModStateChangedEventData.h"
#define GUCEF_INPUT_CKEYMODSTATECHANGEDEVENTDATA_H
#endif /* GUCEF_INPUT_CKEYMODSTATECHANGEDEVENTDATA_H ? */

#ifndef GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H
#include "gucefINPUT_CKeyStateChangedEventData.h"
#define GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H
#endif /* GUCEF_INPUT_CKEYSTATECHANGEDEVENTDATA_H ? */

#ifndef GUCEF_INPUT_CKEYBOARDTEXTEVENTDATA_H
#include "gucefINPUT_CKeyboardTextEventData.h"
#define GUCEF_INPUT_CKEYBOARDTEXTEVENTDATA_H
#endif /* GUCEF_INPUT_CKEYBOARDTEXTEVENTDATA_H ? */

#ifndef GUCEF_INPUT_CMOUSE_H
#include "gucefINPUT_CMouse.h"
#define GUCEF_INPUT_CMOUSE_H
#endif /* GUCEF_INPUT_CMOUSE_H ? */

#ifndef GUCEF_INPUT_CMOUSEBUTTONEVENTDATA_H
#include "gucefINPUT_CMouseButtonEventData.h"
#define GUCEF_INPUT_CMOUSEBUTTONEVENTDATA_H
#endif /* GUCEF_INPUT_CMOUSEBUTTONEVENTDATA_H ? */

#ifndef GUCEF_INPUT_CMOUSEMOVEDEVENTDATA_H
#include "gucefINPUT_CMouseMovedEventData.h"
#define GUCEF_INPUT_CMOUSEMOVEDEVENTDATA_H
#endif /* GUCEF_INPUT_CMOUSEMOVEDEVENTDATA_H ? */

#ifndef GUCEF_INPUT_CMOUSEVARIABLECHANGEDEVENTDATA_H
#include "gucefINPUT_CMouseVariableChangedEventData.h"
#define GUCEF_INPUT_CMOUSEVARIABLECHANGEDEVENTDATA_H
#endif /* GUCEF_INPUT_CMOUSEVARIABLECHANGEDEVENTDATA_H ? */

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

#ifndef GUCEF_INPUT_INPUTDRIVERPLUGINSTRUCTS_H
#include "inputdriverpluginstructs.h"
#define GUCEF_INPUT_INPUTDRIVERPLUGINSTRUCTS_H
#endif /* GUCEF_INPUT_INPUTDRIVERPLUGINSTRUCTS_H ? */

#endif /* __cplusplus ? */
/*--------------------------------------------------------------------------*/

#endif /* GUCEFINPUT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 13-09-2005 :
       - Initial version.

-----------------------------------------------------------------------------*/
