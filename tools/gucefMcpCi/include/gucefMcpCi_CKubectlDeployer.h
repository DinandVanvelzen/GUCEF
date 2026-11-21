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

#ifndef GUCEF_MCPCI_CKUBECTLDEPLOYER_H
#define GUCEF_MCPCI_CKUBECTLDEPLOYER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#include "gucefMcpCi_ICiDeployer.h"

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
 *  Deploys with kubectl as a child process (CProcessRun, no shell, the arguments as a list) and the kubeconfig of a
 *  restricted ServiceAccount, never the admin kubeconfig of the cluster. The kubeconfig has one context per namespace,
 *  named like the namespace, so every call is
 *
 *      kubectl --kubeconfig <file> --context <namespace> -n <namespace> [--request-timeout=<n>s] <verb> ...
 *
 *  with only verbs the ServiceAccount may use (get and patch of the listed Deployments, list and watch of
 *  Deployments, ReplicaSets and Pods of the namespace):
 *
 *      state           get deployment <d> -o jsonpath=<image, pull policy, revision, change-cause, strategy, replicas>
 *      set the image   patch deployment/<d> --type=strategic -p <the image of the container by digest and the annotation
 *                      kubernetes.io/change-cause in one change, so that only the new revision carries the cause>
 *      rollout         rollout status deployment/<d> --timeout=<n>s, followed with progress and cancellation
 *      undo            rollout undo deployment/<d>
 *      history         rollout history deployment/<d>
 *
 *  The environment variable KUBECONFIG of the child is the same file, so nothing else is ever read. The content of the
 *  kubeconfig is never read by this class or shown; the output of kubectl is redacted (credentials after Bearer and
 *  Basic, sensitive name=value pairs, the secrets of AddSecret()) and bounded before it is returned. Image digests
 *  and commits stay readable. Every call has a time limit.
 */
class CKubectlDeployer : public ICiDeployer
{
    public:

    /** The kubeconfig the setup of the deployer writes on a gateway: /var/lib/gucef-ci/kubeconfig */
    static const char* DefaultKubeconfigPath;

    CKubectlDeployer( void );

    virtual ~CKubectlDeployer() GUCEF_VIRTUAL_OVERRIDE;

    /** The kubectl executable, a path or a name found through PATH; default "kubectl" */
    void SetKubectl( const GUCEF::CORE::CString& kubectl );

    void SetKubeconfig( const GUCEF::CORE::CString& kubeconfig );

    const GUCEF::CORE::CString& GetKubeconfig( void ) const;

    /** The time limit of the calls other than the rollout; default 60 seconds */
    void SetCommandTimeout( GUCEF::UInt32 timeoutSeconds );

    /** A secret that never leaves in what kubectl said; call before the deployer is used */
    void AddSecret( const GUCEF::CORE::CString& secret );

    virtual bool GetState( const CCiDeployImage::CTarget& target, CState& state, GUCEF::CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetImage( const CCiDeployImage::CTarget& target, const GUCEF::CORE::CString& imageReference, const GUCEF::CORE::CString& changeCause, GUCEF::CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE;

    virtual ERolloutResult WaitForRollout( const CCiDeployImage::CTarget& target ,
                                           GUCEF::UInt32 timeoutSeconds          ,
                                           ICiProgress* progress                 ,
                                           GUCEF::CORE::CString& detail          ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Undo( const CCiDeployImage::CTarget& target, GUCEF::CORE::CString& message, GUCEF::CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetHistory( const CCiDeployImage::CTarget& target, GUCEF::CORE::CString& history, GUCEF::CORE::CString& error ) GUCEF_VIRTUAL_OVERRIDE;

    /** The arguments of a call before its verb, for the target */
    GUCEF::CORE::CProcess::TStringVector MakeBaseArguments( const CCiDeployImage::CTarget& target, bool withRequestTimeout ) const;

    private:

    CKubectlDeployer( const CKubectlDeployer& src );                /**< not implemented */
    CKubectlDeployer& operator=( const CKubectlDeployer& src );     /**< not implemented */

    /** Runs kubectl to its end. @return whether it ran and exited with 0; 'output' is as kubectl wrote it, 'error' redacted */
    bool Run( const CCiDeployImage::CTarget& target                ,
              const GUCEF::CORE::CProcess::TStringVector& verbArgs ,
              GUCEF::CORE::CString& output                         ,
              GUCEF::CORE::CString& error                          ) const;

    /** Whether kubectl can be used at all; the reason in 'error' when not */
    bool CheckReady( GUCEF::CORE::CString& error ) const;

    GUCEF::CORE::CProcess::TStringMap MakeEnvironment( void ) const;

    /** Output of kubectl as it may be shown: redacted, bounded */
    GUCEF::CORE::CString Clean( const GUCEF::CORE::CString& output ) const;

    GUCEF::CORE::CString m_kubectl;
    GUCEF::CORE::CString m_kubeconfig;
    GUCEF::UInt32 m_commandTimeoutSeconds;
    GUCEF::CORE::CRedactor m_redactor;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CKUBECTLDEPLOYER_H ? */
