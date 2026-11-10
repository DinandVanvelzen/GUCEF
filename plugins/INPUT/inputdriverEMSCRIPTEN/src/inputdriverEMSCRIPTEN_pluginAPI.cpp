/*
 *  inputdriverEMSCRIPTEN: GUCEF input driver for web browsers: mouse, wheel, keys and text of a page
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_INPUT_CINPUTGLOBAL_H
#include "gucefINPUT_CInputGlobal.h"
#define GUCEF_INPUT_CINPUTGLOBAL_H
#endif /* GUCEF_INPUT_CINPUTGLOBAL_H ? */

#ifndef GUCEF_INPUT_CINPUTCONTROLLER_H
#include "CInputController.h"
#define GUCEF_INPUT_CINPUTCONTROLLER_H
#endif /* GUCEF_INPUT_CINPUTCONTROLLER_H ? */

#ifndef GUCEF_INPUTDRIVEREMSCRIPTEN_CEMSCRIPTENINPUTDRIVER_H
#include "inputdriverEMSCRIPTEN_CEmscriptenInputDriver.h"
#define GUCEF_INPUTDRIVEREMSCRIPTEN_CEMSCRIPTENINPUTDRIVER_H
#endif /* GUCEF_INPUTDRIVEREMSCRIPTEN_CEMSCRIPTENINPUTDRIVER_H ? */

#include "inputdriverEMSCRIPTEN_pluginAPI.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUTDRIVEREMSCRIPTEN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVEREMSCRIPTEN_ENTRY( GUCEFPlugin_Load )( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    INPUT::CInputController& controller = INPUT::CInputGlobal::Instance()->GetInputController();
    if ( controller.GetDriverByName( CEmscriptenInputDriver::DriverName ).IsNULL() )
        controller.RegisterDriver( INPUT::TInputDriverPtr( GUCEF_NEW CEmscriptenInputDriver() ) );
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVEREMSCRIPTEN_ENTRY( GUCEFPlugin_Unload )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    INPUT::CInputGlobal::Instance()->GetInputController().UnregisterDriver( CEmscriptenInputDriver::DriverName );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVEREMSCRIPTEN_ENTRY( GUCEFPlugin_GetVersion )( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    versionInfo->major = 1;
    versionInfo->minor = 0;
    versionInfo->patch = 0;
    versionInfo->release = 0;
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVEREMSCRIPTEN_ENTRY( GUCEFPlugin_GetCopyright )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License 2.0";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
INPUTDRIVEREMSCRIPTEN_ENTRY( GUCEFPlugin_GetDescription )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Input driver for web browsers: mouse, wheel, keys and text of a page";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUTDRIVEREMSCRIPTEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
