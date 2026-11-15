/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include <stdlib.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERCLIENT_H
#include "gucefDRNUTILS_CSchedulerClient.h"
#define GUCEF_DRNUTILS_CSCHEDULERCLIENT_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERCLIENT_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERCLIENT_H
#include "gucefDRNUTILS_CTaskManagerClient.h"
#define GUCEF_DRNUTILS_CTASKMANAGERCLIENT_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERCLIENT_H ? */

#ifndef GUCEF_DRNUTILS_CISCHEDULERSOURCE_H
#include "gucefDRNUTILS_CISchedulerSource.h"
#define GUCEF_DRNUTILS_CISCHEDULERSOURCE_H
#endif /* GUCEF_DRNUTILS_CISCHEDULERSOURCE_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#include "gucefCOMCORE_CRpcContractRegistry.h"
#define GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERCONTRACTS_H
#include "gucefDRNUTILS_CSchedulerContracts.h"
#define GUCEF_DRNUTILS_CSCHEDULERCONTRACTS_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERCONTRACTS_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERCONTRACTS_H
#include "gucefDRNUTILS_CTaskManagerContracts.h"
#define GUCEF_DRNUTILS_CTASKMANAGERCONTRACTS_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERCONTRACTS_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERSAMPLERTASK_H
#include "gucefDRNUTILS_CTaskManagerSamplerTask.h"
#define GUCEF_DRNUTILS_CTASKMANAGERSAMPLERTASK_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERSAMPLERTASK_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H
#include "gucefDRNUTILS_CRemoteSessionContracts.h"
#define GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H
#endif /* GUCEF_DRNUTILS_CREMOTESESSIONCONTRACTS_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICECONTRACTS_H
#include "gucefDRNUTILS_CDeviceContracts.h"
#define GUCEF_DRNUTILS_CDEVICECONTRACTS_H
#endif /* GUCEF_DRNUTILS_CDEVICECONTRACTS_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H
#include "gucefDRNUTILS_CRemoteServiceConnection.h"
#define GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICECONNECTION_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEPAIRINGCLIENT_H
#include "gucefDRNUTILS_CDevicePairingClient.h"
#define GUCEF_DRNUTILS_CDEVICEPAIRINGCLIENT_H
#endif /* GUCEF_DRNUTILS_CDEVICEPAIRINGCLIENT_H ? */

#include "gucefDRNUTILS_CDrnUtilsGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex CDrnUtilsGlobal::g_dataLock;
CDrnUtilsGlobal* CDrnUtilsGlobal::g_instance = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDrnUtilsGlobal::CDrnUtilsGlobal( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDrnUtilsGlobal::Initialize( void )
{GUCEF_TRACE;

    // The modules we depend on
    CORE::CCoreGlobal::Instance();
    COMCORE::CComCoreGlobal::Instance();
    DRN::CDRNGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefDRNUTILS Global systems initializing" );

    CISchedulerSource::RegisterEvents();
    CSchedulerClient::RegisterEvents();
    CTaskManagerClient::RegisterEvents();
    CTaskManagerSamplerTask::RegisterEvents();
    CRemoteServiceConnection::RegisterEvents();
    CDevicePairingClient::RegisterEvents();

    // the built-in contracts, so that endpoints can bind them without the JSON codec
    COMCORE::CRpcContractRegistry& registry = COMCORE::CComCoreGlobal::Instance()->GetRpcContractRegistry();
    COMCORE::CRpcContractPtr scheduler = CSchedulerContracts::GetContract();
    if ( scheduler.IsNULL() || !registry.Register( scheduler, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_BUILTIN ) )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefDRNUTILS: the contract " << CSchedulerContracts::ContractName << " could not be registered" );
    COMCORE::CRpcContractPtr tasks = CTaskManagerContracts::GetContract();
    if ( tasks.IsNULL() || !registry.Register( tasks, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_BUILTIN ) )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefDRNUTILS: the contract " << CTaskManagerContracts::ContractName << " could not be registered" );
    // the contracts of the remote service kit register themselves when they are first built
    if ( CRemoteSessionContracts::GetContract().IsNULL() || CDeviceContracts::GetContract().IsNULL() )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefDRNUTILS: the contracts of the remote service kit could not be built" );

    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefDRNUTILS" );
}

/*-------------------------------------------------------------------------*/

CDrnUtilsGlobal::~CDrnUtilsGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefDRNUTILS Global systems shutting down" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefDRNUTILS" );
}

/*-------------------------------------------------------------------------*/

CDrnUtilsGlobal*
CDrnUtilsGlobal::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CDrnUtilsGlobal();
            if ( GUCEF_NULL != g_instance )
            {
                g_instance->Initialize();
                atexit( CDrnUtilsGlobal::Deinstance );
            }
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CDrnUtilsGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_dataLock );
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
