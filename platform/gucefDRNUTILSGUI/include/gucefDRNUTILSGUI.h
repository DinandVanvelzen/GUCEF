/*
 *  gucefDRNUTILSGUI: GUCEF module providing the forms that view and control job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILSGUI_H
#define GUCEF_DRNUTILSGUI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRNUTILSGUI_MACROS_H
#include "gucefDRNUTILSGUI_macros.h"
#define GUCEF_DRNUTILSGUI_MACROS_H
#endif /* GUCEF_DRNUTILSGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  This is the main include header for the gucefDRNUTILSGUI library, part of the GUCEF platform: prebaked gucefGUI forms
 *  that view and control the job schedulers and the task manager of a node through the gucefDRNUTILS client facades
 *  and table sets.
 */
#ifdef __cplusplus

#include "gucefDRNUTILSGUI_CDrnUtilsGuiGlobal.h"
#include "gucefDRNUTILSGUI_CTimeDisplaySettings.h"
#include "gucefDRNUTILSGUI_CTimeDisplayFormatter.h"
#include "gucefDRNUTILSGUI_CTimeDisplayTableModel.h"
#include "gucefDRNUTILSGUI_CTableGridBinding.h"
#include "gucefDRNUTILSGUI_CSchedulerForm.h"
#include "gucefDRNUTILSGUI_CJobRunHistoryForm.h"
#include "gucefDRNUTILSGUI_CTaskManagerForm.h"
#include "gucefDRNUTILSGUI_CControlConfirmForm.h"

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILSGUI_H ? */
