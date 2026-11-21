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

#ifndef GUCEF_VFS_CVFSREMOTEFETCHJOB_H
#define GUCEF_VFS_CVFSREMOTEFETCHJOB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFS_CIVFSFETCHJOB_H
#include "gucefVFS_CIVfsFetchJob.h"
#define GUCEF_VFS_CIVFSFETCHJOB_H
#endif /* GUCEF_VFS_CIVFSFETCHJOB_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEENTRY_H
#include "gucefVFS_CVfsRemoteEntry.h"
#define GUCEF_VFS_CVFSREMOTEENTRY_H
#endif /* GUCEF_VFS_CVFSREMOTEENTRY_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEARCHIVECORE_H
#include "gucefVFS_CVfsRemoteArchiveCore.h"
#define GUCEF_VFS_CVFSREMOTEARCHIVECORE_H
#endif /* GUCEF_VFS_CVFSREMOTEARCHIVECORE_H ? */

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
 *  The background job of a remote archive (CVfsRemoteArchiveCore): the work a miss of a thread that must not wait starts.
 *  It does the same work a thread that waits does (CVfsRemoteArchiveCore::RunJob()), on a thread of the pool of the
 *  scheduler, and raises the event that ends it (CVfsRemoteArchiveCore::OnJobEnded()). It belongs to its core and holds it.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsRemoteFetchJob : public CIVfsFetchJob
{
    public:

    /**
     *  @param core         the archive the job works for; the job keeps it alive
     *  @param kind         what the job does
     *  @param path         the file or directory, relative to the archive
     *  @param entry        for BLOCKS and WHOLE: size, revision and digest of the file the job fetches
     *  @param firstBlock   for BLOCKS: the first block of the run
     *  @param blockCount   for BLOCKS: the number of blocks
     */
    CVfsRemoteFetchJob( const CVfsRemoteArchiveCorePtr& core ,
                        EVfsFetchJobKind kind                ,
                        const CORE::CString& path            ,
                        const CVfsRemoteEntry& entry         ,
                        UInt32 firstBlock                    ,
                        UInt32 blockCount                    );

    virtual ~CVfsRemoteFetchJob() GUCEF_VIRTUAL_OVERRIDE;

    virtual EVfsFetchJobKind GetKind( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetKey( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const void* GetOwner( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CVfsCallResult Run( const MT::CTAtomic< bool >& cancelRequested, CVfsArrivalInfo& outcome ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnEnded( const CVfsCallResult& result, const CVfsArrivalInfo& outcome ) GUCEF_VIRTUAL_OVERRIDE;

    /** The key a job of this kind, archive, path and first block has */
    static CORE::CString MakeKey( EVfsFetchJobKind kind, const void* owner, const CORE::CString& path, UInt32 firstBlock );

    private:

    CVfsRemoteArchiveCorePtr m_core;
    EVfsFetchJobKind m_kind;
    CORE::CString m_path;
    CVfsRemoteEntry m_entry;
    UInt32 m_firstBlock;
    UInt32 m_blockCount;
    bool m_silent;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSREMOTEFETCHJOB_H ? */
