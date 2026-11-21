/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CACTIVITYINFO_H
#define GUCEF_GUI_CACTIVITYINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A snapshot of one operation of the CActivityService, as the indicator form and the jobs list of an application read it:
 *  what the operation said when it announced itself, its progress and whether it is shown to the user yet.
 */
class GUCEF_GUI_PUBLIC_CPP CActivityInfo
{
    public:

    UInt32 id;                  /**< what CActivityService::Begin() answered */
    CORE::CString label;
    UInt32 expectedDurationInMs;/**< 0 when the operation did not say */
    bool determinate;           /**< progress is a fraction (progress), else the operation only says that it is busy */
    Float32 progress;           /**< between 0 and 1, for a determinate operation */
    bool cancellable;
    bool cancelRequested;       /**< the user asked to cancel: the operation is expected to stop and End() */
    bool shown;                 /**< the operation passed the threshold, or announced a long duration: the user sees it */
    UInt64 startedAtInMs;       /**< on the clock of the service */

    CActivityInfo( void );
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CACTIVITYINFO_H ? */
