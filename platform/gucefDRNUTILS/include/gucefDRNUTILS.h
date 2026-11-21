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

#ifndef GUCEF_DRNUTILS_H
#define GUCEF_DRNUTILS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  This is the main include header for the gucefDRNUTILS library, part of the GUCEF platform: ready made components,
 *  served to remote operators through DRN, that view and control the job schedulers and the task manager of a node
 *  (RPC contracts gucef.drnutils.scheduler and gucef.drnutils.tasks, DRN published tables, access rules, audit,
 *  client facades).
 */
#ifdef __cplusplus

#include "gucefDRNUTILS_CDrnUtilsGlobal.h"
#include "gucefDRNUTILS_DrnUtilsTypes.h"
#include "gucefDRNUTILS_CISchedulerSource.h"
#include "gucefDRNUTILS_CJobSchedulerSource.h"
#include "gucefDRNUTILS_CITaskManagerSource.h"
#include "gucefDRNUTILS_CTaskManagerSource.h"
#include "gucefDRNUTILS_CSchedulerHost.h"
#include "gucefDRNUTILS_CTaskManagerHost.h"
#include "gucefDRNUTILS_CSchedulerContracts.h"
#include "gucefDRNUTILS_CTaskManagerContracts.h"
#include "gucefDRNUTILS_CDrnUtilsAccessRules.h"
#include "gucefDRNUTILS_CDrnUtilsGridAccessRules.h"
#include "gucefDRNUTILS_CDrnUtilsDigest.h"
#include "gucefDRNUTILS_CJobsTableSchema.h"
#include "gucefDRNUTILS_CLanesTableSchema.h"
#include "gucefDRNUTILS_CRunsTableSchema.h"
#include "gucefDRNUTILS_CPoolsTableSchema.h"
#include "gucefDRNUTILS_CThreadsTableSchema.h"
#include "gucefDRNUTILS_CTasksTableSchema.h"
#include "gucefDRNUTILS_CScheduledJobsTableSchema.h"
#include "gucefDRNUTILS_CSchedulerClient.h"
#include "gucefDRNUTILS_CTaskManagerClient.h"
#include "gucefDRNUTILS_CDrnUtilsTableBinder.h"
#include "gucefDRNUTILS_RemoteServiceTypes.h"
#include "gucefDRNUTILS_CRemoteServiceDetailCodes.h"
#include "gucefDRNUTILS_CDeviceRecord.h"
#include "gucefDRNUTILS_CDeviceRegistry.h"
#include "gucefDRNUTILS_CDeviceAccessRules.h"
#include "gucefDRNUTILS_CIRemoteShareDecision.h"
#include "gucefDRNUTILS_CDeviceDrnAccessPolicy.h"
#include "gucefDRNUTILS_CDevicePairingCrypto.h"
#include "gucefDRNUTILS_CPairingWindowRequest.h"
#include "gucefDRNUTILS_CDeviceCredentials.h"
#include "gucefDRNUTILS_CRemoteSessionEventRing.h"
#include "gucefDRNUTILS_CRemoteSession.h"
#include "gucefDRNUTILS_CRemoteSessionCache.h"
#include "gucefDRNUTILS_CIRemoteSessionExtension.h"
#include "gucefDRNUTILS_CRemoteSessionContracts.h"
#include "gucefDRNUTILS_CDeviceContracts.h"
#include "gucefDRNUTILS_CRemoteServicePrincipal.h"
#include "gucefDRNUTILS_CRemoteServiceHostSettings.h"
#include "gucefDRNUTILS_CRemoteServiceHost.h"
#include "gucefDRNUTILS_CDevicePairingHost.h"
#include "gucefDRNUTILS_CDevicePairingClient.h"
#include "gucefDRNUTILS_CReconnectBackoff.h"
#include "gucefDRNUTILS_CRemoteServiceConnectionSettings.h"
#include "gucefDRNUTILS_CRemoteServiceChannel.h"
#include "gucefDRNUTILS_CRemoteServiceConnection.h"
#include "gucefDRNUTILS_CRemoteShareMountSettings.h"
#include "gucefDRNUTILS_CRemoteShareMount.h"
#include "gucefDRNUTILS_CRemoteShareMounts.h"
#include "gucefDRNUTILS_CCommandOutcomeCache.h"
#include "gucefDRNUTILS_CCommandJournal.h"
#include "gucefDRNUTILS_CUploadRangeSet.h"
#include "gucefDRNUTILS_CUploadStaging.h"

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_H ? */
