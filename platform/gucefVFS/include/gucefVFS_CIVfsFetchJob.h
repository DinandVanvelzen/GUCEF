/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#ifndef GUCEF_VFS_CIVFSFETCHJOB_H
#define GUCEF_VFS_CIVFSFETCHJOB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_VFS_CVFSCALLRESULT_H
#include "gucefVFS_CVfsCallResult.h"
#define GUCEF_VFS_CVFSCALLRESULT_H
#endif /* GUCEF_VFS_CVFSCALLRESULT_H ? */

#ifndef GUCEF_VFS_CVFSARRIVALINFO_H
#include "gucefVFS_CVfsArrivalInfo.h"
#define GUCEF_VFS_CVFSARRIVALINFO_H
#endif /* GUCEF_VFS_CVFSARRIVALINFO_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A piece of background work of a remote archive that the CVfsFetchScheduler runs on a thread of its pool: fetching
 *  the metadata of a path, a listing, a run of blocks or a whole file. The job does its work in Run(), which may block on
 *  the remote (it runs on a pool thread, never on the thread that asked), and tells what came of it in OnEnded(), which the
 *  scheduler calls exactly once for every job it accepted and that was not dropped by CancelAll().
 *
 *  Two jobs with the same key are one job: the second is merged into the first. The key therefore has to name everything
 *  that the work depends on.
 */
class GUCEF_VFS_PUBLIC_CPP CIVfsFetchJob
{
    public:

    CIVfsFetchJob( void );

    virtual ~CIVfsFetchJob();

    virtual EVfsFetchJobKind GetKind( void ) const = 0;

    /** What identifies the work for the dedupe of the scheduler, for example "<kind>|<archive>|<path>|<first block>" */
    virtual CORE::CString GetKey( void ) const = 0;

    /** Who the job belongs to (an archive): CancelAll() takes the jobs of one owner. Only compared, never used */
    virtual const void* GetOwner( void ) const = 0;

    /**
     *  Does the work. 'cancelRequested' is set when the job is to stop early (CancelJob(), CancelAll()): a job that
     *  can stop between its steps looks at it. On success 'outcome' says what arrived, as the event of the job will tell.
     */
    virtual CVfsCallResult Run( const MT::CTAtomic< bool >& cancelRequested, CVfsArrivalInfo& outcome ) = 0;

    /**
     *  Called once when the job ended, on the thread that ran it (the thread that cancelled it for a job that never
     *  ran), with the result of Run() or why it did not run (FILESYSTEMACCESS_CANCELLED, FILESYSTEMACCESS_QUEUEING_FAILED).
     *  This is where the job raises its event; it is not called for a job that CancelAll() dropped.
     */
    virtual void OnEnded( const CVfsCallResult& result, const CVfsArrivalInfo& outcome ) = 0;

    private:

    CIVfsFetchJob( const CIVfsFetchJob& src );                  /**< not implemented */
    CIVfsFetchJob& operator=( const CIVfsFetchJob& src );       /**< not implemented */
};

typedef CORE::CTSharedPtr< CIVfsFetchJob, MT::CMutex > CIVfsFetchJobPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CIVFSFETCHJOB_H ? */
