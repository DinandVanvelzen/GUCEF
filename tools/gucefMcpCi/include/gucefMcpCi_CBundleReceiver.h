/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_MCPCI_CBUNDLERECEIVER_H
#define GUCEF_MCPCI_CBUNDLERECEIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#include "gucefMcpCi_CDirectCiService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Takes the snapshots that arrive at a gateway as git bundles into a bare repository per CI id (never into a working
 *  tree), from where the backend gets them as in direct mode:
 *
 *    1. the bundle must carry exactly one ref, CSnapshotBuilder::BundleRef, the commit of the snapshot
 *    2. git bundle verify; when the bundle needs commits the bare repository lacks, what the backend published is
 *       fetched (CDirectCiService::FetchPublished()) and the bundle is verified again
 *    3. git fetch of the bundle into refs/heads/<scratch branch> of the bare repository
 *
 *  Local scratch branches older than the retention of the CI id (14 days when it has none) are removed afterwards,
 *  so the bare repository does not grow without bound. One snapshot at a time per CI id, under the repository lock of
 *  the service (CDirectCiService::GetRepositoryLock()), which its other users of the repository hold too.
 *
 *  Thread-safe.
 */
class CBundleReceiver
{
    public:

    enum EResult
    {
        BUNDLE_OK = 0,
        BUNDLE_INVALID,             /**< not a bundle, or not one of a snapshot */
        BUNDLE_UNKNOWN_BASE,        /**< made against commits the CI does not have */
        BUNDLE_FAILED               /**< git or the backend failed */
    };

    /** The service must outlive the receiver */
    explicit CBundleReceiver( CDirectCiService& service );

    ~CBundleReceiver();

    /** The directory of the bare repositories */
    void SetDirectory( const GUCEF::CORE::CString& directory );

    /**
     *  Takes the snapshot of the bundle into the bare repository of the CI id.
     *  @param machineName  the machine the snapshot comes from, part of the name of its scratch branch
     *  @param repoDir      the bare repository
     *  @param commit       the commit of the snapshot
     *  @param branchName   its scratch branch, CSnapshotBuilder::MakeBranchName()
     */
    EResult Receive( const GUCEF::CORE::CString& ciId         ,
                     const GUCEF::CORE::CString& bundlePath   ,
                     const GUCEF::CORE::CString& machineName  ,
                     GUCEF::CORE::CString& repoDir            ,
                     GUCEF::CORE::CString& commit             ,
                     GUCEF::CORE::CString& branchName         ,
                     GUCEF::CORE::CString& error              );

    private:

    CBundleReceiver( const CBundleReceiver& src );                /**< not implemented */
    CBundleReceiver& operator=( const CBundleReceiver& src );     /**< not implemented */

    GUCEF::MT::CMutex& LockOf( const GUCEF::CORE::CString& ciId );

    bool EnsureRepo( const GUCEF::CORE::CString& repoDir, GUCEF::CORE::CString& error );

    /** git bundle verify. @param missingBase  set when it failed for lack of commits */
    bool Verify( const GUCEF::CORE::CString& repoDir, const GUCEF::CORE::CString& bundlePath, bool& missingBase, GUCEF::CORE::CString& error );

    void PruneOldBranches( const GUCEF::CORE::CString& repoDir, GUCEF::UInt32 retentionDays );

    CDirectCiService& m_service;
    GUCEF::CORE::CString m_directory;
    GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CBUNDLERECEIVER_H ? */
