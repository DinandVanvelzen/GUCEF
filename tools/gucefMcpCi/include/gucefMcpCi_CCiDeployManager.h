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

#ifndef GUCEF_MCPCI_CCIDEPLOYMANAGER_H
#define GUCEF_MCPCI_CCIDEPLOYMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#include "gucefMcpCi_ICiBackend.h"
#include "gucefMcpCi_ICiDeployer.h"
#include "gucefMcpCi_CRegistryClient.h"
#include "gucefMcpCi_CDeployAudit.h"
#include "gucefMcpCi_CCiCaller.h"
#include "gucefMcpCi_CCiDeployResult.h"
#include "gucefMcpCi_CCiReleaseStatus.h"

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
 *  The deployments of the service (CDirectCiService), so that every mode has the same guards. A deployment of an image
 *  of a run to a target of the deploy block:
 *
 *    1. the image and the target are listed in the deploy block of the CI id (else refused, naming what is listed)
 *    2. the caller has the scope deploy
 *    3. confirm is true
 *    4. the run is finished and green: every job succeeded and an images job ran; its image report has the image with
 *       status ok and a digest
 *    5. a target without allowScratch: the commit of the run is an ancestor of the default branch of the published
 *       repository, as a git fetch into the bare repository of the CI id shows (merge-base --is-ancestor); a scratch
 *       snapshot or an unknown commit is refused
 *    6. the registry has the tag of the image with the digest of the report
 *    7. one deployment or rollback per Deployment at a time
 *    8. the Deployment does not pull with imagePullPolicy Never (still the image imported by hand)
 *    9. kubernetes.io/change-cause is set (CI id, run, commit, image, machine, time), then the image by digest, then the
 *       rollout is followed for at most the rollout timeout. A rollout that fails or times out is reported with what is
 *       known of the state; it is not undone.
 *
 *  Every deployment, rollback and refusal is a line of the audit. A rollback has guards 1 to 3 and 7.
 *
 *  Thread-safe.
 */
class CCiDeployManager
{
    public:

    CCiDeployManager( void );

    ~CCiDeployManager();

    /** The deployer; the manager deletes it. Without one every deployment is refused with the reason */
    void SetDeployer( ICiDeployer* deployer );

    CRegistryClient& GetRegistryClient( void );

    CDeployAudit& GetAudit( void );

    /** The longest wait for a rollout; default 600 seconds */
    void SetRolloutTimeout( GUCEF::UInt32 timeoutSeconds );

    GUCEF::UInt32 GetRolloutTimeout( void ) const;

    /**
     *  A deployment, see the class.
     *  @param bareRepoDir  the bare repository of the CI id on this machine, made when it is not there
     *  @param repoLock     the lock of that repository
     */
    bool Deploy( const CCiDefinition& ci                     ,
                 ICiBackend& backend                         ,
                 const GUCEF::CORE::CString& bareRepoDir     ,
                 GUCEF::MT::CMutex& repoLock                 ,
                 const GUCEF::CORE::CString& image           ,
                 const GUCEF::CORE::CString& target          ,
                 const GUCEF::CORE::CString& runOrCommit     ,
                 bool confirm                                ,
                 const CCiCaller& caller                     ,
                 CCiDeployResult& result                     ,
                 GUCEF::CORE::CString& error                 ,
                 ICiProgress* progress                       );

    bool Rollback( const CCiDefinition& ci                   ,
                   const GUCEF::CORE::CString& image         ,
                   const GUCEF::CORE::CString& target        ,
                   bool confirm                              ,
                   const CCiCaller& caller                   ,
                   CCiDeployResult& result                   ,
                   GUCEF::CORE::CString& error               ,
                   ICiProgress* progress                     );

    bool ReleaseStatus( const CCiDefinition& ci              ,
                        const GUCEF::CORE::CString& target   ,
                        CCiReleaseStatus& status             ,
                        GUCEF::CORE::CString& error          );

    /**
     *  Whether the commit is an ancestor of (or is) the default branch of the published repository, as last fetched
     *  into the bare repository. @param unknown    set when the repository does not have the commit at all
     */
    static bool IsOnPublishedHead( const GUCEF::CORE::CString& bareRepoDir, const GUCEF::CORE::CString& commit, bool& unknown, GUCEF::CORE::CString& error );

    /** Makes the bare repository when it is not there. @return false when it cannot be made or is not a bare repository */
    static bool EnsureBareRepository( const GUCEF::CORE::CString& bareRepoDir, GUCEF::CORE::CString& error );

    private:

    CCiDeployManager( const CCiDeployManager& src );                /**< not implemented */
    CCiDeployManager& operator=( const CCiDeployManager& src );     /**< not implemented */

    /** Ends a call with a result that is not a success: audited, the message in 'error'. @return false */
    bool Finish( CCiDeployResult& result, const char* outcome, const char* code, const GUCEF::CORE::CString& message,
                 const CCiCaller& caller, GUCEF::CORE::CString& error );

    /** The listed image and target, or a refusal */
    bool FindListed( const CCiDefinition& ci, const GUCEF::CORE::CString& image, const GUCEF::CORE::CString& target,
                     const CCiDeployImage*& listedImage, const CCiDeployImage::CTarget*& listedTarget, GUCEF::CORE::CString& problem ) const;

    /** Takes the Deployment for this call; false when another deployment of it runs */
    bool AcquireTarget( const GUCEF::CORE::CString& key );

    void ReleaseTarget( const GUCEF::CORE::CString& key );

    /** What a rollout that did not complete leaves, for the reader */
    GUCEF::CORE::CString DescribeIncompleteRollout( const CCiDeployImage::CTarget& target, ICiDeployer::ERolloutResult rollout,
                                                    const GUCEF::CORE::CString& detail, CCiDeployResult& result );

    ICiDeployer* m_deployer;
    CRegistryClient m_registry;
    CDeployAudit m_audit;
    GUCEF::UInt32 m_rolloutTimeoutSeconds;
    GUCEF::CORE::CString::StringSet m_busyTargets;
    mutable GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCIDEPLOYMANAGER_H ? */
