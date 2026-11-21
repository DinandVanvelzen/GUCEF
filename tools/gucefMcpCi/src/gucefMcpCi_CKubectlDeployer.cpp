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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpLocalLlm_ProcessRun.h"
#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CKubectlDeployer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CKubectlDeployer::DefaultKubeconfigPath = "/var/lib/gucef-ci/kubeconfig";

static const UInt32 MaxOutputChars = 3000;
static const UInt32 MaxHistoryChars = 4000;
static const UInt32 MaxRolloutSeconds = 3600;
static const UInt32 RolloutMarginSeconds = 30;
static const UInt32 ProgressIntervalInMs = 2000;
static const UInt32 StateFieldCount = 9;
static const char* ChangeCauseAnnotation = "kubernetes.io/change-cause";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The last non-empty line of a text */
static CORE::CString
LastLine( const CORE::CString& text )
{
    CORE::CString::StringVector lines = text.ReplaceSubstr( "\r", "" ).ParseElements( '\n', false );
    for ( size_t i=lines.size(); i>0; --i )
    {
        const CORE::CString line = CCiUtils::Trimmed( lines[ i-1 ] );
        if ( !line.IsNULLOrEmpty() )
            return line;
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
EscapeJsonText( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString escaped;
    for ( UInt32 i=0; i<text.Length(); ++i )
    {
        const char c = text[ i ];
        if ( '"' == c || '\\' == c )
        {
            escaped += '\\';
            escaped += c;
        }
        else
        if ( (unsigned char) c < 0x20 )
            escaped += ' ';
        else
            escaped += c;
    }
    return escaped;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
DeploymentRef( const CCiDeployImage::CTarget& target )
{
    return "deployment/" + target.deployment;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKubectlDeployer::CKubectlDeployer( void )
    : ICiDeployer()
    , m_kubectl( "kubectl" )
    , m_kubeconfig( DefaultKubeconfigPath )
    , m_commandTimeoutSeconds( 60 )
    , m_redactor( CORE::REDACT_TEXT_AUTH_SCHEMES | CORE::REDACT_TEXT_KEY_VALUES )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKubectlDeployer::~CKubectlDeployer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CKubectlDeployer::SetKubectl( const CORE::CString& kubectl )
{GUCEF_TRACE;

    m_kubectl = kubectl.IsNULLOrEmpty() ? CORE::CString( "kubectl" ) : kubectl;
}

/*-------------------------------------------------------------------------*/

void
CKubectlDeployer::SetKubeconfig( const CORE::CString& kubeconfig )
{GUCEF_TRACE;

    m_kubeconfig = kubeconfig.IsNULLOrEmpty() ? CORE::CString( DefaultKubeconfigPath ) : kubeconfig;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CKubectlDeployer::GetKubeconfig( void ) const
{GUCEF_TRACE;

    return m_kubeconfig;
}

/*-------------------------------------------------------------------------*/

void
CKubectlDeployer::SetCommandTimeout( UInt32 timeoutSeconds )
{GUCEF_TRACE;

    m_commandTimeoutSeconds = timeoutSeconds > 0 && timeoutSeconds <= 600 ? timeoutSeconds : 60;
}

/*-------------------------------------------------------------------------*/

void
CKubectlDeployer::AddSecret( const CORE::CString& secret )
{GUCEF_TRACE;

    if ( !secret.IsNULLOrEmpty() )
        m_redactor.AddSecret( secret );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CKubectlDeployer::Clean( const CORE::CString& output ) const
{GUCEF_TRACE;

    return CCiUtils::Bound( CCiUtils::Trimmed( m_redactor.RedactText( output ) ), MaxOutputChars );
}

/*-------------------------------------------------------------------------*/

CORE::CProcess::TStringMap
CKubectlDeployer::MakeEnvironment( void ) const
{GUCEF_TRACE;

    // the kubeconfig of the option and no other, whatever the environment of this process says
    CORE::CProcess::TStringMap env;
    env[ "KUBECONFIG" ] = m_kubeconfig;
    return env;
}

/*-------------------------------------------------------------------------*/

CORE::CProcess::TStringVector
CKubectlDeployer::MakeBaseArguments( const CCiDeployImage::CTarget& target, bool withRequestTimeout ) const
{GUCEF_TRACE;

    CORE::CProcess::TStringVector args;
    args.push_back( "--kubeconfig" );
    args.push_back( m_kubeconfig );
    args.push_back( "--context" );
    args.push_back( target.nameSpace );
    args.push_back( "-n" );
    args.push_back( target.nameSpace );
    if ( withRequestTimeout )
        args.push_back( "--request-timeout=" + CORE::UInt32ToString( m_commandTimeoutSeconds ) + "s" );
    return args;
}

/*-------------------------------------------------------------------------*/

bool
CKubectlDeployer::CheckReady( CORE::CString& error ) const
{GUCEF_TRACE;

    if ( !CORE::FileExists( m_kubeconfig ) )
    {
        error = "The deployer has no kubeconfig at " + m_kubeconfig + " (--kubeconfig): the setup of the deployer of the cluster writes it, "
                "with the ServiceAccount that may change the Deployments of the deploy blocks";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKubectlDeployer::Run( const CCiDeployImage::CTarget& target                ,
                       const CORE::CProcess::TStringVector& verbArgs        ,
                       CORE::CString& output                                ,
                       CORE::CString& error                                 ) const
{GUCEF_TRACE;

    output.Clear();
    if ( !CheckReady( error ) )
        return false;
    CORE::CProcess::TStringVector args = MakeBaseArguments( target, true );
    args.insert( args.end(), verbArgs.begin(), verbArgs.end() );

    GUCEFMcpLocalLlm::CProcessRun run;
    if ( !run.Start( m_kubectl, args, CORE::CString::Empty, MakeEnvironment(), ( m_commandTimeoutSeconds + 15 ) * 1000 ) )
    {
        error = "kubectl could not be started (" + m_kubectl + "); it must be installed on the machine of the gateway (--kubectl)";
        return false;
    }
    run.RunToEnd();
    output = CORE::CString( run.GetOutput() );
    if ( run.TimedOut() )
    {
        error = "kubectl did not finish within " + CORE::UInt32ToString( m_commandTimeoutSeconds ) + " s";
        return false;
    }
    if ( 0 != run.GetExitCode() )
    {
        error = "kubectl failed (exit " + CORE::Int32ToString( run.GetExitCode() ) + "): " + ( output.IsNULLOrEmpty() ? CORE::CString( "no output" ) : Clean( output ) );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKubectlDeployer::GetState( const CCiDeployImage::CTarget& target, CState& state, CORE::CString& error )
{GUCEF_TRACE;

    state = CState();
    const CORE::CString container = "{.spec.template.spec.containers[?(@.name==\"" + target.container + "\")]";
    const CORE::CString jsonPath = "jsonpath=" + container + ".image}{\"\\t\"}" + container + ".imagePullPolicy}{\"\\t\"}"
                                   "{.metadata.annotations.deployment\\.kubernetes\\.io/revision}{\"\\t\"}"
                                   "{.metadata.annotations.kubernetes\\.io/change-cause}{\"\\t\"}{.spec.strategy.type}{\"\\t\"}"
                                   "{.spec.replicas}{\"\\t\"}{.status.readyReplicas}{\"\\t\"}{.status.updatedReplicas}{\"\\t\"}{.status.availableReplicas}";
    CORE::CProcess::TStringVector args;
    args.push_back( "get" );
    args.push_back( "deployment" );
    args.push_back( target.deployment );
    args.push_back( "-o" );
    args.push_back( jsonPath );
    CORE::CString output;
    if ( !Run( target, args, output, error ) )
    {
        error = "The Deployment " + target.GetKey() + " could not be read: " + error;
        return false;
    }

    // the line with the fields; kubectl may write warnings before it
    CORE::CString::StringVector lines = output.ReplaceSubstr( "\r", "" ).ParseElements( '\n', false );
    CORE::CString::StringVector fields;
    for ( size_t i=lines.size(); i>0 && fields.empty(); --i )
    {
        CORE::CString::StringVector candidate = lines[ i-1 ].ParseElements( '\t', true );
        if ( candidate.size() >= StateFieldCount )
            fields = candidate;
    }
    if ( fields.empty() )
    {
        error = "The Deployment " + target.GetKey() + " could not be read: kubectl answered something else than its state: " + Clean( output );
        return false;
    }
    state.image = CCiUtils::Trimmed( fields[ 0 ] );
    state.imagePullPolicy = CCiUtils::Trimmed( fields[ 1 ] );
    state.revision = CCiUtils::Trimmed( fields[ 2 ] );
    state.changeCause = CCiUtils::Trimmed( fields[ 3 ] );
    state.strategy = CCiUtils::Trimmed( fields[ 4 ] );
    state.replicas = CORE::StringToUInt32( CCiUtils::Trimmed( fields[ 5 ] ), 0 );
    state.readyReplicas = CORE::StringToUInt32( CCiUtils::Trimmed( fields[ 6 ] ), 0 );
    state.updatedReplicas = CORE::StringToUInt32( CCiUtils::Trimmed( fields[ 7 ] ), 0 );
    state.availableReplicas = CORE::StringToUInt32( CCiUtils::Trimmed( fields[ 8 ] ), 0 );
    if ( state.image.IsNULLOrEmpty() )
    {
        error = "The Deployment " + target.GetKey() + " has no container named \"" + target.container + "\"; the deploy block of ci.json names it";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKubectlDeployer::SetImage( const CCiDeployImage::CTarget& target, const CORE::CString& imageReference, const CORE::CString& changeCause, CORE::CString& error )
{GUCEF_TRACE;

    // One strategic merge patch holds the annotation and the image. The annotation belongs to the new revision only when
    // it is part of the same change: the deployment controller copies the annotations of the Deployment onto the
    // ReplicaSet of the current template, so a separate annotate beforehand would also relabel the revision that is
    // being replaced.
    CORE::CString patch = "{\"metadata\":{\"annotations\":{\"" + CORE::CString( ChangeCauseAnnotation ) + "\":\"" + EscapeJsonText( changeCause ) +
                          "\"}},\"spec\":{\"template\":{\"spec\":{\"containers\":[{\"name\":\"" + EscapeJsonText( target.container ) +
                          "\",\"image\":\"" + EscapeJsonText( imageReference ) + "\"}]}}}}";

    CORE::CProcess::TStringVector args;
    args.push_back( "patch" );
    args.push_back( DeploymentRef( target ) );
    args.push_back( "--type=strategic" );
    args.push_back( "-p" );
    args.push_back( patch );
    CORE::CString output;
    if ( !Run( target, args, output, error ) )
    {
        error = "The image of " + target.GetKey() + " could not be set: " + error;
        return false;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Deploy: " << target.GetKey() << " container " << target.container << " set to " << imageReference );
    return true;
}

/*-------------------------------------------------------------------------*/

ICiDeployer::ERolloutResult
CKubectlDeployer::WaitForRollout( const CCiDeployImage::CTarget& target ,
                                  UInt32 timeoutSeconds                 ,
                                  ICiProgress* progress                 ,
                                  CORE::CString& detail                 )
{GUCEF_TRACE;

    detail.Clear();
    if ( !CheckReady( detail ) )
        return ROLLOUT_ERROR;
    if ( 0 == timeoutSeconds || timeoutSeconds > MaxRolloutSeconds )
        timeoutSeconds = MaxRolloutSeconds;

    // rollout status watches the Deployment: no request timeout of its own, the --timeout of the rollout bounds it
    CORE::CProcess::TStringVector args = MakeBaseArguments( target, false );
    args.push_back( "rollout" );
    args.push_back( "status" );
    args.push_back( DeploymentRef( target ) );
    args.push_back( "--timeout=" + CORE::UInt32ToString( timeoutSeconds ) + "s" );

    GUCEFMcpLocalLlm::CProcessRun run;
    if ( !run.Start( m_kubectl, args, CORE::CString::Empty, MakeEnvironment(), ( timeoutSeconds + RolloutMarginSeconds ) * 1000 ) )
    {
        detail = "kubectl could not be started (" + m_kubectl + ")";
        return ROLLOUT_ERROR;
    }
    UInt64 lastReportMs = 0;
    while ( run.Pump( 250 ) )
    {
        if ( GUCEF_NULL == progress )
            continue;
        if ( progress->IsCancelled() )
        {
            run.Stop( true );
            detail = "Following the rollout was given up; the rollout itself goes on in the cluster. " + LastLine( Clean( CORE::CString( run.GetOutput() ) ) );
            return ROLLOUT_CANCELLED;
        }
        const UInt64 elapsed = run.GetElapsedMs();
        if ( elapsed - lastReportMs >= ProgressIntervalInMs )
        {
            lastReportMs = elapsed;
            const CORE::CString last = LastLine( Clean( CORE::CString( run.GetOutput() ) ) );
            progress->OnProgress( (Float64) ( elapsed / 1000 ), (Float64) timeoutSeconds, "rollout of " + target.GetKey() +
                                  ( last.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + last ) );
        }
    }

    const CORE::CString output = Clean( CORE::CString( run.GetOutput() ) );
    detail = output;
    if ( !run.TimedOut() && 0 == run.GetExitCode() )
        return ROLLOUT_COMPLETE;
    const CORE::CString lower = output.Lowercase();
    if ( run.TimedOut() || lower.HasSubstr( "timed out" ) >= 0 )
        return ROLLOUT_TIMEOUT;
    if ( lower.HasSubstr( "progress deadline" ) >= 0 || lower.HasSubstr( "failed" ) >= 0 )
        return ROLLOUT_FAILED;
    return ROLLOUT_ERROR;
}

/*-------------------------------------------------------------------------*/

bool
CKubectlDeployer::Undo( const CCiDeployImage::CTarget& target, CORE::CString& message, CORE::CString& error )
{GUCEF_TRACE;

    CORE::CProcess::TStringVector args;
    args.push_back( "rollout" );
    args.push_back( "undo" );
    args.push_back( DeploymentRef( target ) );
    CORE::CString output;
    if ( !Run( target, args, output, error ) )
    {
        error = "The rollback of " + target.GetKey() + " failed: " + error;
        return false;
    }
    message = Clean( output );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Deploy: " << target.GetKey() << " rolled back: " << LastLine( message ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKubectlDeployer::GetHistory( const CCiDeployImage::CTarget& target, CORE::CString& history, CORE::CString& error )
{GUCEF_TRACE;

    CORE::CProcess::TStringVector args;
    args.push_back( "rollout" );
    args.push_back( "history" );
    args.push_back( DeploymentRef( target ) );
    CORE::CString output;
    if ( !Run( target, args, output, error ) )
    {
        error = "The history of " + target.GetKey() + " could not be read: " + error;
        return false;
    }
    history = CCiUtils::Bound( CCiUtils::Trimmed( m_redactor.RedactText( output ) ), MaxHistoryChars );
    return true;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
