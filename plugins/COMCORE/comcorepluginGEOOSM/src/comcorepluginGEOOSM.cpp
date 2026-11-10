/*
 *  comcorepluginGEOOSM: Generic GUCEF COMCORE plugin 
 *  providing Geo Data services based on OpenStreetMap data.
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CGEOLOCATIONLOOKUPSERVICE_H
#include "gucefCOMCORE_CGeoLocationLookupService.h"
#define GUCEF_COMCORE_CGEOLOCATIONLOOKUPSERVICE_H
#endif /* GUCEF_COMCORE_CGEOLOCATIONLOOKUPSERVICE_H ? */

#ifndef COMCOREPLUGIN_GEOOSM_COSMGEOLOCATIONLOOKUPSERVICE_H
#include "comcorepluginGEOOSM_COsmGeoLocationLookupService.h"
#define COMCOREPLUGIN_GEOOSM_COSMGEOLOCATIONLOOKUPSERVICE_H
#endif /* COMCOREPLUGIN_GEOOSM_COSMGEOLOCATIONLOOKUPSERVICE_H ? */

#include "comcorepluginGEOOSM.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCOREPLUGIN {
namespace GEOOSM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

COsmGeoLocationLookupServicePtr g_osmGeoLookupService;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "GUCEFPlugin_Load: Load called on COMCORE plugin GEOOSM" );

    COMCORE::CComCoreGlobal* comms = COMCORE::CComCoreGlobal::Instance();
    COMCORE::CGeoLocationLookupService& geoServices = comms->GetGeoLocationLookupService();
    
    g_osmGeoLookupService = COsmGeoLocationLookupService::CreateSharedObj();
    geoServices.GetGeoLocationLookupServiceRegistry().Register( COsmGeoLocationLookupService::TypeName, g_osmGeoLookupService );


    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "GUCEFPlugin_Load: Load finished for COMCORE plugin GEOOSM" );
    return 1;
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GUCEFPlugin_Unload: Unload called on COMCORE plugin GEOOSM" );

    COMCORE::CComCoreGlobal* comms = COMCORE::CComCoreGlobal::Instance();
    COMCORE::CGeoLocationLookupService& geoServices = comms->GetGeoLocationLookupService();

    geoServices.GetGeoLocationLookupServiceRegistry().Unregister( COsmGeoLocationLookupService::TypeName );
    
    if ( !g_osmGeoLookupService.IsNULL() )
    {
        g_osmGeoLookupService->Shutdown();
        g_osmGeoLookupService.Unlink();
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GUCEFPlugin_Unload: Unload finished for COMCORE plugin GEOOSM" );
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL != versionInfo )
    {
        versionInfo->major = 1;
        versionInfo->minor = 0;
        versionInfo->patch = 0;
        versionInfo->release = 0;
    }
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License v2";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Generic GUCEF plugin for COMCORE plugin for Geo data access based on Open Street Maps";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetLinkBackModuleDependencies( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefCORE,gucefCOMCORE";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GEOOSM */
}; /* namespace COMCOREPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
