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

#ifndef GUCEF_WEB_CWEBVIEWGLOBALSETTINGS_H
#define GUCEF_WEB_CWEBVIEWGLOBALSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

#ifndef GUCEF_WEB_WEBVIEWTYPES_H
#include "gucefWEB_WebViewTypes.h"
#define GUCEF_WEB_WEBVIEWTYPES_H
#endif /* GUCEF_WEB_WEBVIEWTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings of the web views of gucefWEB for an application: the order of preference of engines, the watchdog and
 *  shutdown times of the engine threads, and whether last resort engines may be chosen. Nothing in it is secret.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewGlobalSettings : public CORE::CIConfigurable
{
    public:

    static const UInt32 DefaultEngineWatchdogMs = 10000;
    static const UInt32 DefaultShutdownTimeoutMs = 5000;
    static const UInt32 DefaultEngineIdleWakeMs = 1000;

    CString::StringVector engineOrder;      /**< engine names, preferred first; empty: the order the plugins registered them */
    UInt32 engineWatchdogMs;                /**< an engine thread that does not answer a command this long is stalled; 0 never */
    UInt32 shutdownTimeoutMs;               /**< how long Shutdown() waits for an engine thread */
    UInt32 engineIdleWakeMs;                /**< an idle engine thread wakes this often to notice a stop of the task manager; 0 only when woken */
    bool allowLastResortEngines;            /**< engines such as ChromeDevTools may be chosen */

    CWebViewGlobalSettings( void );

    virtual ~CWebViewGlobalSettings();

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWGLOBALSETTINGS_H ? */
