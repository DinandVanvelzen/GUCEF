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

#ifndef GUCEF_MCPCI_ICIDEPLOYER_H
#define GUCEF_MCPCI_ICIDEPLOYER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefMcpCi_CCiDeployImage.h"
#include "gucefMcpCi_ICiProgress.h"

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
 *  What runs a workload: the Deployments of the deploy block of a CI id, with the least a deployment needs. Every
 *  call names the target (namespace, Deployment, container) and touches nothing else. Messages may be shown to the
 *  model as they are: an implementation never puts credentials in them and bounds them.
 *
 *  The calls are made from several threads at the same time, never two at a time for one target.
 */
class ICiDeployer
{
    public:

    /** What a Deployment runs now */
    class CState
    {
        public:

        GUCEF::CORE::CString image;             /**< the image reference of the container */
        GUCEF::CORE::CString imagePullPolicy;   /**< of the container: Always, IfNotPresent or Never */
        GUCEF::CORE::CString revision;          /**< deployment.kubernetes.io/revision */
        GUCEF::CORE::CString changeCause;       /**< kubernetes.io/change-cause */
        GUCEF::CORE::CString strategy;          /**< RollingUpdate or Recreate */
        GUCEF::UInt32 replicas;
        GUCEF::UInt32 readyReplicas;
        GUCEF::UInt32 updatedReplicas;
        GUCEF::UInt32 availableReplicas;

        CState( void );

        /** The digest of the image reference ("sha256:..."), empty when it names a tag */
        GUCEF::CORE::CString GetDigest( void ) const;

        /** The replicas for a person: "1 of 1 ready, 1 updated, 1 available" */
        GUCEF::CORE::CString DescribeReplicas( void ) const;
    };

    enum ERolloutResult
    {
        ROLLOUT_COMPLETE = 0,
        ROLLOUT_FAILED,             /**< the rollout reported a failure, such as its progress deadline */
        ROLLOUT_TIMEOUT,            /**< not done within the time given */
        ROLLOUT_CANCELLED,          /**< the caller gave up */
        ROLLOUT_ERROR               /**< the state could not be followed at all */
    };

    virtual ~ICiDeployer();

    /** What the Deployment runs now */
    virtual bool GetState( const CCiDeployImage::CTarget& target, CState& state, GUCEF::CORE::CString& error ) = 0;

    /** Sets the image of the container and, in the same change, kubernetes.io/change-cause, which the revision history shows
     *  for the new revision only; the Deployment then rolls out */
    virtual bool SetImage( const CCiDeployImage::CTarget& target, const GUCEF::CORE::CString& imageReference, const GUCEF::CORE::CString& changeCause, GUCEF::CORE::CString& error ) = 0;

    /**
     *  Waits until the rollout is done, at most timeoutSeconds, telling 'progress' how it goes and stopping when it
     *  asks to. @param detail  what the rollout said last
     */
    virtual ERolloutResult WaitForRollout( const CCiDeployImage::CTarget& target ,
                                           GUCEF::UInt32 timeoutSeconds          ,
                                           ICiProgress* progress                 ,
                                           GUCEF::CORE::CString& detail          ) = 0;

    /** Returns the Deployment to its previous revision */
    virtual bool Undo( const CCiDeployImage::CTarget& target, GUCEF::CORE::CString& message, GUCEF::CORE::CString& error ) = 0;

    /** The revision history, bounded */
    virtual bool GetHistory( const CCiDeployImage::CTarget& target, GUCEF::CORE::CString& history, GUCEF::CORE::CString& error ) = 0;

    static const char* RolloutResultToString( ERolloutResult result );
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_ICIDEPLOYER_H ? */
