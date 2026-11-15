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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefWEB_CWebViewGlobalSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CWebViewGlobalSettings::DefaultEngineWatchdogMs;
const UInt32 CWebViewGlobalSettings::DefaultShutdownTimeoutMs;
const UInt32 CWebViewGlobalSettings::DefaultEngineIdleWakeMs;

/*-------------------------------------------------------------------------*/

CWebViewGlobalSettings::CWebViewGlobalSettings( void )
    : CORE::CIConfigurable()
    , engineOrder()
    , engineWatchdogMs( DefaultEngineWatchdogMs )
    , shutdownTimeoutMs( DefaultShutdownTimeoutMs )
    , engineIdleWakeMs( DefaultEngineIdleWakeMs )
    , allowLastResortEngines( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewGlobalSettings::~CWebViewGlobalSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWebViewGlobalSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    bool ok = config.SetAttribute( "engineOrder", CString::Empty.Combine( engineOrder, ',' ) );
    ok = config.SetAttribute( "engineWatchdogMs", engineWatchdogMs ) && ok;
    ok = config.SetAttribute( "shutdownTimeoutMs", shutdownTimeoutMs ) && ok;
    ok = config.SetAttribute( "engineIdleWakeMs", engineIdleWakeMs ) && ok;
    ok = config.SetAttribute( "allowLastResortEngines", allowLastResortEngines ) && ok;
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewGlobalSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    const CString order = config.GetAttributeValueOrChildValueByName( "engineOrder" ).AsString();
    if ( !order.IsNULLOrEmpty() )
        engineOrder = order.ParseElements( ',', false );
    engineWatchdogMs = config.GetAttributeValueOrChildValueByName( "engineWatchdogMs" ).AsUInt32( engineWatchdogMs, true );
    shutdownTimeoutMs = config.GetAttributeValueOrChildValueByName( "shutdownTimeoutMs" ).AsUInt32( shutdownTimeoutMs, true );
    engineIdleWakeMs = config.GetAttributeValueOrChildValueByName( "engineIdleWakeMs" ).AsUInt32( engineIdleWakeMs, true );
    allowLastResortEngines = config.GetAttributeValueOrChildValueByName( "allowLastResortEngines" ).AsBool( allowLastResortEngines, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewGlobalSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::WEB::CWebViewGlobalSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
