/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

/*
 *      This is the main include header for the gucefCORE library.
 *      It is part of a framework called GUCEF.
 *      All classes in this library have to do core framework functionality
 *      and many of the other modules will have a dependancy on this module.
 *      It houses the generic toolkits.
 */

#ifndef GUCEF_VFS_H
#define GUCEF_VFS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library build configuration      
 */
#ifndef GUCEF_VFS_CONFIG_H
#include "gucefVFS_config.h"
#define GUCEF_VFS_CONFIG_H
#endif /* GUCEF_VFS_CONFIG_H ? */

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_VFS_MACROS_H
#include "gucefVFS_macros.h"
#define GUCEF_VFS_MACROS_H
#endif /* GUCEF_VFS_MACROS_H ? */

/*
 *      Simple types
 */
#ifndef GUCEF_VFS_ETYPES_H
#include "gucefVFS_ETypes.h"
#define GUCEF_VFS_ETYPES_H
#endif /* GUCEF_VFS_ETYPES_H ? */

/*---------------------------------------------------------------------------/

        C++ includes

/---------------------------------------------------------------------------*/
#ifdef __cplusplus

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CARCHIVE_H
#include "gucefVFS_CArchive.h"
#define GUCEF_VFS_CARCHIVE_H
#endif /* GUCEF_VFS_IARCHIVE_H ? */

#ifndef GUCEF_VFS_CVFSHANDLE_H
#include "gucefVFS_CVFSHandle.h"
#define GUCEF_VFS_CVFSHANDLE_H
#endif /* GUCEF_VFS_CVFSHANDLE_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFSDIGEST_H
#include "gucefVFS_CVfsDigest.h"
#define GUCEF_VFS_CVFSDIGEST_H
#endif /* GUCEF_VFS_CVFSDIGEST_H ? */

#ifndef GUCEF_VFS_CVFSDIGESTHASHER_H
#include "gucefVFS_CVfsDigestHasher.h"
#define GUCEF_VFS_CVFSDIGESTHASHER_H
#endif /* GUCEF_VFS_CVFSDIGESTHASHER_H ? */

#ifndef GUCEF_VFS_CVFSCALLRESULT_H
#include "gucefVFS_CVfsCallResult.h"
#define GUCEF_VFS_CVFSCALLRESULT_H
#endif /* GUCEF_VFS_CVFSCALLRESULT_H ? */

#ifndef GUCEF_VFS_CVFSBLOCKCACHE_H
#include "gucefVFS_CVfsBlockCache.h"
#define GUCEF_VFS_CVFSBLOCKCACHE_H
#endif /* GUCEF_VFS_CVFSBLOCKCACHE_H ? */

#ifndef GUCEF_VFS_CVFSTHREADPOLICY_H
#include "gucefVFS_CVfsThreadPolicy.h"
#define GUCEF_VFS_CVFSTHREADPOLICY_H
#endif /* GUCEF_VFS_CVFSTHREADPOLICY_H ? */

#ifndef GUCEF_VFS_CVFSTHREADPOLICYREGISTRY_H
#include "gucefVFS_CVfsThreadPolicyRegistry.h"
#define GUCEF_VFS_CVFSTHREADPOLICYREGISTRY_H
#endif /* GUCEF_VFS_CVFSTHREADPOLICYREGISTRY_H ? */

#ifndef GUCEF_VFS_CVFSTHREADPOLICYSCOPE_H
#include "gucefVFS_CVfsThreadPolicyScope.h"
#define GUCEF_VFS_CVFSTHREADPOLICYSCOPE_H
#endif /* GUCEF_VFS_CVFSTHREADPOLICYSCOPE_H ? */

#ifndef GUCEF_VFS_CREMOTEARCHIVEEVENTS_H
#include "gucefVFS_CRemoteArchiveEvents.h"
#define GUCEF_VFS_CREMOTEARCHIVEEVENTS_H
#endif /* GUCEF_VFS_CREMOTEARCHIVEEVENTS_H ? */

#ifndef GUCEF_VFS_CVFSARRIVALINFO_H
#include "gucefVFS_CVfsArrivalInfo.h"
#define GUCEF_VFS_CVFSARRIVALINFO_H
#endif /* GUCEF_VFS_CVFSARRIVALINFO_H ? */

#ifndef GUCEF_VFS_CIVFSFETCHJOB_H
#include "gucefVFS_CIVfsFetchJob.h"
#define GUCEF_VFS_CIVFSFETCHJOB_H
#endif /* GUCEF_VFS_CIVFSFETCHJOB_H ? */

#ifndef GUCEF_VFS_CVFSFETCHSCHEDULER_H
#include "gucefVFS_CVfsFetchScheduler.h"
#define GUCEF_VFS_CVFSFETCHSCHEDULER_H
#endif /* GUCEF_VFS_CVFSFETCHSCHEDULER_H ? */

#ifndef GUCEF_VFS_CVFSLASTRESULTS_H
#include "gucefVFS_CVfsLastResults.h"
#define GUCEF_VFS_CVFSLASTRESULTS_H
#endif /* GUCEF_VFS_CVFSLASTRESULTS_H ? */

#ifndef GUCEF_VFS_CVFSFETCHREQUEST_H
#include "gucefVFS_CVfsFetchRequest.h"
#define GUCEF_VFS_CVFSFETCHREQUEST_H
#endif /* GUCEF_VFS_CVFSFETCHREQUEST_H ? */

#ifndef GUCEF_VFS_CVFSFETCHPROGRESS_H
#include "gucefVFS_CVfsFetchProgress.h"
#define GUCEF_VFS_CVFSFETCHPROGRESS_H
#endif /* GUCEF_VFS_CVFSFETCHPROGRESS_H ? */

#ifndef GUCEF_VFS_CVFSCACHESTATE_H
#include "gucefVFS_CVfsCacheState.h"
#define GUCEF_VFS_CVFSCACHESTATE_H
#endif /* GUCEF_VFS_CVFSCACHESTATE_H ? */

#ifndef GUCEF_VFS_CVFSREMOTESTATISTICS_H
#include "gucefVFS_CVfsRemoteStatistics.h"
#define GUCEF_VFS_CVFSREMOTESTATISTICS_H
#endif /* GUCEF_VFS_CVFSREMOTESTATISTICS_H ? */

#ifndef GUCEF_VFS_CIREMOTEARCHIVE_H
#include "gucefVFS_CIRemoteArchive.h"
#define GUCEF_VFS_CIREMOTEARCHIVE_H
#endif /* GUCEF_VFS_CIREMOTEARCHIVE_H ? */

#ifndef GUCEF_VFS_CIVFSBLOCKSOURCE_H
#include "gucefVFS_CIVfsBlockSource.h"
#define GUCEF_VFS_CIVFSBLOCKSOURCE_H
#endif /* GUCEF_VFS_CIVFSBLOCKSOURCE_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEENTRY_H
#include "gucefVFS_CVfsRemoteEntry.h"
#define GUCEF_VFS_CVFSREMOTEENTRY_H
#endif /* GUCEF_VFS_CVFSREMOTEENTRY_H ? */

#ifndef GUCEF_VFS_CIVFSREMOTEBACKEND_H
#include "gucefVFS_CIVfsRemoteBackend.h"
#define GUCEF_VFS_CIVFSREMOTEBACKEND_H
#endif /* GUCEF_VFS_CIVFSREMOTEBACKEND_H ? */

#ifndef GUCEF_VFS_CVFSREMOTESETTINGS_H
#include "gucefVFS_CVfsRemoteSettings.h"
#define GUCEF_VFS_CVFSREMOTESETTINGS_H
#endif /* GUCEF_VFS_CVFSREMOTESETTINGS_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEMETADATACACHE_H
#include "gucefVFS_CVfsRemoteMetadataCache.h"
#define GUCEF_VFS_CVFSREMOTEMETADATACACHE_H
#endif /* GUCEF_VFS_CVFSREMOTEMETADATACACHE_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEPATH_H
#include "gucefVFS_CVfsRemotePath.h"
#define GUCEF_VFS_CVFSREMOTEPATH_H
#endif /* GUCEF_VFS_CVFSREMOTEPATH_H ? */

#ifndef GUCEF_VFS_CVFSCACHEDIOACCESS_H
#include "gucefVFS_CVfsCachedIOAccess.h"
#define GUCEF_VFS_CVFSCACHEDIOACCESS_H
#endif /* GUCEF_VFS_CVFSCACHEDIOACCESS_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEARCHIVECORE_H
#include "gucefVFS_CVfsRemoteArchiveCore.h"
#define GUCEF_VFS_CVFSREMOTEARCHIVECORE_H
#endif /* GUCEF_VFS_CVFSREMOTEARCHIVECORE_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEARCHIVE_H
#include "gucefVFS_CVfsRemoteArchive.h"
#define GUCEF_VFS_CVFSREMOTEARCHIVE_H
#endif /* GUCEF_VFS_CVFSREMOTEARCHIVE_H ? */

#ifndef GUCEF_VFS_CVFSBLOCKRUNFETCHER_H
#include "gucefVFS_CVfsBlockRunFetcher.h"
#define GUCEF_VFS_CVFSBLOCKRUNFETCHER_H
#endif /* GUCEF_VFS_CVFSBLOCKRUNFETCHER_H ? */

#ifndef GUCEF_VFS_CVFSREMOTECOUNTERS_H
#include "gucefVFS_CVfsRemoteCounters.h"
#define GUCEF_VFS_CVFSREMOTECOUNTERS_H
#endif /* GUCEF_VFS_CVFSREMOTECOUNTERS_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEFETCHJOB_H
#include "gucefVFS_CVfsRemoteFetchJob.h"
#define GUCEF_VFS_CVFSREMOTEFETCHJOB_H
#endif /* GUCEF_VFS_CVFSREMOTEFETCHJOB_H ? */

#ifndef GUCEF_VFS_CIVFSCANCELSOURCE_H
#include "gucefVFS_CIVfsCancelSource.h"
#define GUCEF_VFS_CIVFSCANCELSOURCE_H
#endif /* GUCEF_VFS_CIVFSCANCELSOURCE_H ? */

#ifndef GUCEF_VFS_CVFSEITHERCANCELSOURCE_H
#include "gucefVFS_CVfsEitherCancelSource.h"
#define GUCEF_VFS_CVFSEITHERCANCELSOURCE_H
#endif /* GUCEF_VFS_CVFSEITHERCANCELSOURCE_H ? */

#ifndef GUCEF_VFS_CVFSTASKCANCELSOURCE_H
#include "gucefVFS_CVfsTaskCancelSource.h"
#define GUCEF_VFS_CVFSTASKCANCELSOURCE_H
#endif /* GUCEF_VFS_CVFSTASKCANCELSOURCE_H ? */

#ifndef GUCEF_VFS_CVFSFETCHPROGRESSCANCELSCOPE_H
#include "gucefVFS_CVfsFetchProgressCancelScope.h"
#define GUCEF_VFS_CVFSFETCHPROGRESSCANCELSCOPE_H
#endif /* GUCEF_VFS_CVFSFETCHPROGRESSCANCELSCOPE_H ? */

#ifndef GUCEF_VFS_CVFSASYNCRESULT_H
#include "gucefVFS_CVfsAsyncResult.h"
#define GUCEF_VFS_CVFSASYNCRESULT_H
#endif /* GUCEF_VFS_CVFSASYNCRESULT_H ? */

#ifndef GUCEF_VFS_CIVFSASYNCCALL_H
#include "gucefVFS_CIVfsAsyncCall.h"
#define GUCEF_VFS_CIVFSASYNCCALL_H
#endif /* GUCEF_VFS_CIVFSASYNCCALL_H ? */

#ifndef GUCEF_VFS_CVFSASYNCCALLGATE_H
#include "gucefVFS_CVfsAsyncCallGate.h"
#define GUCEF_VFS_CVFSASYNCCALLGATE_H
#endif /* GUCEF_VFS_CVFSASYNCCALLGATE_H ? */

#ifndef GUCEF_VFS_CVFSASYNCCALLGATESCOPE_H
#include "gucefVFS_CVfsAsyncCallGateScope.h"
#define GUCEF_VFS_CVFSASYNCCALLGATESCOPE_H
#endif /* GUCEF_VFS_CVFSASYNCCALLGATESCOPE_H ? */

#ifndef GUCEF_VFS_CVFSASYNCCALLTASKDATA_H
#include "gucefVFS_CVfsAsyncCallTaskData.h"
#define GUCEF_VFS_CVFSASYNCCALLTASKDATA_H
#endif /* GUCEF_VFS_CVFSASYNCCALLTASKDATA_H ? */

#ifndef GUCEF_VFS_CVFSASYNCCALLTASKCONSUMER_H
#include "gucefVFS_CVfsAsyncCallTaskConsumer.h"
#define GUCEF_VFS_CVFSASYNCCALLTASKCONSUMER_H
#endif /* GUCEF_VFS_CVFSASYNCCALLTASKCONSUMER_H ? */

#ifndef GUCEF_VFS_CVFSPREFETCHWORK_H
#include "gucefVFS_CVfsPrefetchWork.h"
#define GUCEF_VFS_CVFSPREFETCHWORK_H
#endif /* GUCEF_VFS_CVFSPREFETCHWORK_H ? */

#ifndef GUCEF_VFS_CVFSASYNCVFSCALL_H
#include "gucefVFS_CVfsAsyncVfsCall.h"
#define GUCEF_VFS_CVFSASYNCVFSCALL_H
#endif /* GUCEF_VFS_CVFSASYNCVFSCALL_H ? */

#endif /* __cplusplus ? */
/*--------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 11-03-2004 :
       - Made the initial version of the library header. The classes here
         where originally part of one large GUCEF codebase without segmentation.

-----------------------------------------------------------------------------*/
