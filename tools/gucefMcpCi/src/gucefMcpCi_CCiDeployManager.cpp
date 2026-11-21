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

#include <time.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

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

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CCiDeployManager.h"

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

static const UInt32 GitTimeoutInMs = 120000;
static const UInt32 DefaultRolloutTimeoutSeconds = 600;
static const UInt32 MaxRolloutTimeoutSeconds = 3600;
static const UInt32 MaxImageLogLines = 15;
static const char RunIdSeparator = ':';

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
NowIsoUtc( void )
{
    char buffer[ 32 ];
    time_t now = ::time( GUCEF_NULL );
    struct tm parts;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    ::gmtime_s( &parts, &now );
    #else
    ::gmtime_r( &now, &parts );
    #endif
    ::strftime( buffer, sizeof( buffer ), "%Y-%m-%dT%H:%M:%SZ", &parts );
    return buffer;
}

/*-------------------------------------------------------------------------*/

static bool
IsDigits( const CORE::CString& text )
{
    if ( text.IsNULLOrEmpty() )
        return false;
    const char* c = text.C_String();
    for ( UInt32 i=0; '\0' != c[ i ]; ++i )
        if ( c[ i ] < '0' || c[ i ] > '9' )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** The first characters of a commit, for names and sentences */
static CORE::CString
ShortCommit( const CORE::CString& commit )
{
    return commit.Length() > 12 ? commit.SubstrToIndex( 12, true ) : commit;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCiDeployManager::CCiDeployManager( void )
    : m_deployer( GUCEF_NULL )
    , m_registry()
    , m_audit()
    , m_rolloutTimeoutSeconds( DefaultRolloutTimeoutSeconds )
    , m_busyTargets()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCiDeployManager::~CCiDeployManager()
{GUCEF_TRACE;

    GUCEF_DELETE m_deployer;
    m_deployer = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CCiDeployManager::SetDeployer( ICiDeployer* deployer )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( deployer != m_deployer )
        GUCEF_DELETE m_deployer;
    m_deployer = deployer;
}

/*-------------------------------------------------------------------------*/

CRegistryClient&
CCiDeployManager::GetRegistryClient( void )
{GUCEF_TRACE;

    return m_registry;
}

/*-------------------------------------------------------------------------*/

CDeployAudit&
CCiDeployManager::GetAudit( void )
{GUCEF_TRACE;

    return m_audit;
}

/*-------------------------------------------------------------------------*/

void
CCiDeployManager::SetRolloutTimeout( UInt32 timeoutSeconds )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_rolloutTimeoutSeconds = 0 == timeoutSeconds ? DefaultRolloutTimeoutSeconds : ( timeoutSeconds > MaxRolloutTimeoutSeconds ? MaxRolloutTimeoutSeconds : timeoutSeconds );
}

/*-------------------------------------------------------------------------*/

UInt32
CCiDeployManager::GetRolloutTimeout( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_rolloutTimeoutSeconds;
}

/*-------------------------------------------------------------------------*/

bool
CCiDeployManager::EnsureBareRepository( const CORE::CString& bareRepoDir, CORE::CString& error )
{GUCEF_TRACE;

    CORE::CProcess::TStringVector args;
    CORE::CString output;
    if ( CORE::DirExists( bareRepoDir ) )
    {
        args.push_back( "rev-parse" );
        args.push_back( "--is-bare-repository" );
        if ( CCiUtils::RunGit( bareRepoDir, args, CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) && CCiUtils::Trimmed( output ) == "true" )
            return true;
        // a directory that is not a bare repository is left alone
        error = "The repository directory " + bareRepoDir + " of this machine is not a bare git repository";
        return false;
    }
    CORE::CreateDirs( bareRepoDir );
    args.clear();
    args.push_back( "init" );
    args.push_back( "-q" );
    args.push_back( "--bare" );
    if ( !CCiUtils::RunGit( bareRepoDir, args, CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
    {
        error = "The repository " + bareRepoDir + " could not be made: " + CCiUtils::Bound( CCiUtils::Trimmed( output ), 500 );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCiDeployManager::IsOnPublishedHead( const CORE::CString& bareRepoDir, const CORE::CString& commit, bool& unknown, CORE::CString& error )
{GUCEF_TRACE;

    unknown = false;
    CORE::CProcess::TStringVector args;
    CORE::CString output;
    args.push_back( "cat-file" );
    args.push_back( "-e" );
    args.push_back( commit + "^{commit}" );
    if ( !CCiUtils::IsHex( commit, 40 ) ||
         !CCiUtils::RunGit( bareRepoDir, args, CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
    {
        unknown = true;
        return false;
    }
    args.clear();
    args.push_back( "merge-base" );
    args.push_back( "--is-ancestor" );
    args.push_back( commit );
    args.push_back( ICiBackend::PublishedHeadRef );
    Int32 exitCode = -1;
    if ( CCiUtils::RunGit( bareRepoDir, args, CORE::CProcess::TStringMap(), GitTimeoutInMs, output, &exitCode ) )
        return true;
    if ( 1 != exitCode )
        error = "git merge-base failed (exit " + CORE::Int32ToString( exitCode ) + "): " + CCiUtils::Bound( CCiUtils::Trimmed( output ), 500 );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CCiDeployManager::Finish( CCiDeployResult& result, const char* outcome, const char* code, const CORE::CString& message,
                          const CCiCaller& caller, CORE::CString& error )
{GUCEF_TRACE;

    result.outcome = outcome;
    result.code = code;
    result.message = message;
    error = message;
    m_audit.Record( result, caller.machineId );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Deploy: " << result.action << " of " << result.image << " to " << result.target << " of " <<
                      result.ciId << " for " << caller.machineId << ": " << outcome << " (" << code << "): " << message );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CCiDeployManager::FindListed( const CCiDefinition& ci, const CORE::CString& image, const CORE::CString& target,
                              const CCiDeployImage*& listedImage, const CCiDeployImage::CTarget*& listedTarget, CORE::CString& problem ) const
{GUCEF_TRACE;

    listedImage = GUCEF_NULL;
    listedTarget = GUCEF_NULL;
    if ( ci.deployImages.empty() )
    {
        problem = "Nothing of CI id " + ci.id + " may be deployed: its entry in ci.json has no deploy block";
        return false;
    }
    if ( image.IsNULLOrEmpty() )
    {
        // only unambiguous: the one image that has the target
        for ( size_t i=0; i<ci.deployImages.size(); ++i )
        {
            if ( GUCEF_NULL == ci.deployImages[ i ].FindTarget( target ) )
                continue;
            if ( GUCEF_NULL != listedImage )
            {
                problem = "More than one image of CI id " + ci.id + " has the target \"" + target + "\": name the image. Deployable: " + ci.ListDeployables();
                listedImage = GUCEF_NULL;
                return false;
            }
            listedImage = &ci.deployImages[ i ];
        }
    }
    else
        listedImage = ci.FindDeployImage( image );
    if ( GUCEF_NULL == listedImage )
    {
        problem = ( image.IsNULLOrEmpty() ? "No image of CI id " + ci.id + " has the target \"" + target + "\"" :
                                            "The image \"" + image + "\" is not in the deploy block of CI id " + ci.id ) +
                  ". Deployable: " + ci.ListDeployables();
        return false;
    }
    listedTarget = listedImage->FindTarget( target );
    if ( GUCEF_NULL == listedTarget )
    {
        problem = "The image \"" + listedImage->name + "\" of CI id " + ci.id + " has no target \"" + target + "\"; its targets: " + listedImage->ListTargets();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCiDeployManager::AcquireTarget( const CORE::CString& key )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_busyTargets.find( key ) != m_busyTargets.end() )
        return false;
    m_busyTargets.insert( key );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCiDeployManager::ReleaseTarget( const CORE::CString& key )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_busyTargets.erase( key );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiDeployManager::DescribeIncompleteRollout( const CCiDeployImage::CTarget& target, ICiDeployer::ERolloutResult rollout,
                                             const CORE::CString& detail, CCiDeployResult& result )
{GUCEF_TRACE;

    CORE::CString text = "The rollout of " + target.GetKey() + " " + CORE::CString( ICiDeployer::RolloutResultToString( rollout ) ) +
                         ( detail.IsNULLOrEmpty() ? CORE::CString( "." ) : ": " + detail ) + "\n";
    ICiDeployer::CState after;
    CORE::CString stateProblem;
    if ( m_deployer->GetState( target, after, stateProblem ) )
    {
        result.revision = after.revision;
        result.replicas = after.DescribeReplicas();
        result.strategy = after.strategy;
        text += "State now: the Deployment is set to " + after.image + " (revision " + ( after.revision.IsNULLOrEmpty() ? CORE::CString( "unknown" ) : after.revision ) +
                "), " + after.DescribeReplicas() + ".\n";
        if ( after.strategy == "Recreate" )
            text += "The strategy is Recreate: the old pods were stopped before the new ones started, so the service may be down until a revision runs.\n";
        else
            text += "The strategy is " + ( after.strategy.IsNULLOrEmpty() ? CORE::CString( "not known" ) : after.strategy ) +
                    ": the pods of the previous revision that were ready keep serving while the new ones are not ready.\n";
    }
    else
        text += "The state of the Deployment could not be read afterwards: " + stateProblem + "\n";
    text += "Nothing was rolled back. Tell the user; ci_release_status shows the state, ci_rollback returns the previous revision when the user asks for it.";
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CCiDeployManager::Deploy( const CCiDefinition& ci                 ,
                          ICiBackend& backend                     ,
                          const CORE::CString& bareRepoDir        ,
                          MT::CMutex& repoLock                    ,
                          const CORE::CString& image              ,
                          const CORE::CString& target             ,
                          const CORE::CString& runOrCommit        ,
                          bool confirm                            ,
                          const CCiCaller& caller                 ,
                          CCiDeployResult& result                 ,
                          CORE::CString& error                    ,
                          ICiProgress* progress                   )
{GUCEF_TRACE;

    result = CCiDeployResult();
    result.action = "deploy";
    result.ciId = ci.id;
    result.image = image;
    result.target = target;
    result.runId = runOrCommit;

    // 1. listed
    const CCiDeployImage* listedImage = GUCEF_NULL;
    const CCiDeployImage::CTarget* listedTarget = GUCEF_NULL;
    CORE::CString problem;
    if ( image.IsNULLOrEmpty() || !FindListed( ci, image, target, listedImage, listedTarget, problem ) )
        return Finish( result, "refused", "not_listed", image.IsNULLOrEmpty() ? "Name the image to deploy. Deployable: " + ci.ListDeployables() : problem, caller, error );
    result.nameSpace = listedTarget->nameSpace;
    result.deployment = listedTarget->deployment;
    result.container = listedTarget->container;

    // 2. scope, 3. confirm
    if ( !caller.HasScope( CCiCaller::ScopeDeploy ) )
        return Finish( result, "refused", "scope", "The machine " + caller.machineId + " may not deploy: its token has no scope deploy (lablink hub, "
                       "--ci-deploy-machines). It may read ci_release_status and ci_images.", caller, error );
    if ( !confirm )
        return Finish( result, "refused", "confirm", "A deployment needs confirm=true. Deploy only when the user asked for it explicitly; show the user "
                       "first what changes (ci_release_status for what runs now, ci_images for what the run built).", caller, error );

    MT::CScopeMutex deployerLock( m_lock );
    ICiDeployer* deployer = m_deployer;
    const UInt32 rolloutTimeout = m_rolloutTimeoutSeconds;
    deployerLock.EarlyUnlock();
    if ( GUCEF_NULL == deployer )
        return Finish( result, "refused", "kubectl", "This server has no deployer set up", caller, error );

    // the run: given, or the newest of a commit
    CORE::CString backendRunId;
    const CORE::CString given = CCiUtils::Trimmed( runOrCommit );
    const Int32 separator = given.HasChar( RunIdSeparator, false );
    if ( separator > 0 )
    {
        if ( given.SubstrToIndex( (UInt32) separator, true ) != ci.id )
            return Finish( result, "refused", "bad_request", "The run " + given + " is not one of CI id " + ci.id, caller, error );
        backendRunId = given.CutChars( (UInt32) separator + 1, true );
        if ( !IsDigits( backendRunId ) )
            return Finish( result, "refused", "unknown_run", "Unknown run id \"" + given + "\"", caller, error );
    }
    else
    if ( 40 == given.Length() && CCiUtils::IsHex( given, 40 ) )
    {
        if ( !backend.FindRunOfCommit( ci, given.Lowercase(), backendRunId, problem ) )
            return Finish( result, "refused", "unknown_run", problem, caller, error );
    }
    else
        return Finish( result, "refused", "bad_request", "Give a run id as ci_submit returns it (" + ci.id + ":<number>) or the full 40 character commit", caller, error );
    result.runId = ci.id + RunIdSeparator + backendRunId;

    // 4. green, with the image
    CCiRunStatus status;
    bool unknownRun = false;
    if ( !backend.GetRun( ci, backendRunId, status, unknownRun, problem ) )
        return Finish( result, "refused", unknownRun ? "unknown_run" : "backend", unknownRun ? "Unknown run id \"" + result.runId + "\"" : "The run could not be read: " + problem, caller, error );
    result.commit = status.commit;
    if ( !status.IsFinished() )
        return Finish( result, "refused", "not_green", "The run " + result.runId + " is " + CCiRunStatus::StateToString( status.state ) + ": only a finished, green run can be deployed", caller, error );
    bool imagesJobSeen = false;
    CORE::CString::StringVector notGreen;
    for ( size_t i=0; i<status.jobs.size(); ++i )
    {
        if ( CCiImageReport::IsImagesJob( status.jobs[ i ].id ) )
            imagesJobSeen = true;
        if ( CCiRunStatus::STATE_SUCCEEDED != status.jobs[ i ].state )
            notGreen.push_back( status.jobs[ i ].id + " (" + CCiRunStatus::StateToString( status.jobs[ i ].state ) + ")" );
    }
    if ( CCiRunStatus::STATE_SUCCEEDED != status.state || !notGreen.empty() )
        return Finish( result, "refused", "not_green", "The run " + result.runId + " is not green (" + CCiRunStatus::StateToString( status.state ) + ")" +
                       ( notGreen.empty() ? CORE::CString::Empty : "; not succeeded: " + CCiUtils::Join( notGreen, ", " ) ) + ". Only a run whose every job succeeded is deployed.", caller, error );
    if ( !imagesJobSeen )
        return Finish( result, "refused", "no_images", "The run " + result.runId + " has no images job: it built no images. Submit with images to build them.", caller, error );

    CCiImageReport report;
    bool missing = false;
    if ( !backend.GetImageReport( ci, backendRunId, report, missing, problem ) )
        return Finish( result, "refused", "no_images", problem, caller, error );
    if ( !report.commit.IsNULLOrEmpty() && !status.commit.IsNULLOrEmpty() && report.commit.Lowercase() != status.commit.Lowercase() )
        return Finish( result, "refused", "no_images", "The image report of run " + result.runId + " names the commit " + report.commit + ", the run built " + status.commit, caller, error );
    if ( result.commit.IsNULLOrEmpty() )
        result.commit = report.commit;
    const CCiImageReport::CImage* built = report.FindImage( listedImage->name );
    if ( GUCEF_NULL == built )
    {
        CORE::CString::StringVector names;
        for ( size_t i=0; i<report.images.size(); ++i )
            names.push_back( report.images[ i ].name );
        return Finish( result, "refused", "no_images", "The run " + result.runId + " did not build the image " + listedImage->name + "; it built: " +
                       ( names.empty() ? CORE::CString( "none" ) : CCiUtils::Join( names, ", " ) ), caller, error );
    }
    if ( !built->IsOk() )
    {
        UInt32 lines = 0;
        const CORE::CString tail = CCiUtils::TailLines( built->logTail, MaxImageLogLines, 0, lines );
        return Finish( result, "refused", "image_failed", "The image " + built->name + " of run " + result.runId + " was not built and pushed (status " +
                       ( built->status.IsNULLOrEmpty() ? CORE::CString( "none" ) : built->status ) + ( CCiImageReport::IsDigest( built->digest ) ? CORE::CString::Empty : CORE::CString( ", no digest" ) ) + ")" +
                       ( tail.IsNULLOrEmpty() ? CORE::CString::Empty : ". The end of its log:\n" + tail ), caller, error );
    }
    result.digest = built->digest;

    // 5. published: the gateway looks itself, the word of the client is not taken
    if ( !listedTarget->allowScratch )
    {
        if ( !CCiUtils::IsHex( result.commit, 40 ) )
            return Finish( result, "refused", "unpublished", "The commit of run " + result.runId + " is not known, so it cannot be shown to be published", caller, error );
        bool unknown = false;
        bool published = false;
        {
            MT::CScopeMutex lock( repoLock );
            if ( !EnsureBareRepository( bareRepoDir, problem ) || !backend.FetchPublishedRepository( ci, bareRepoDir, problem ) )
                return Finish( result, "refused", "unpublished", "Whether the commit " + result.commit + " is published could not be checked: " + problem, caller, error );
            published = IsOnPublishedHead( bareRepoDir, result.commit.Lowercase(), unknown, problem );
        }
        if ( !published )
        {
            if ( !problem.IsNULLOrEmpty() )
                return Finish( result, "refused", "unpublished", "Whether the commit " + result.commit + " is published could not be checked: " + problem, caller, error );
            return Finish( result, "refused", "unpublished", "The commit " + result.commit + " of run " + result.runId + " is " +
                           ( unknown ? CORE::CString( "not in the published repository at all (a scratch snapshot, or not pushed)" ) : CORE::CString( "not on the default branch of the published repository" ) ) +
                           ", and the target \"" + target + "\" takes published commits only. Build a published commit with ci_submit commit and images, then deploy that run.", caller, error );
        }
    }

    // 6. the registry has it, with that digest
    const CORE::CString repository = listedImage->registryOwner + "/" + listedImage->registryName;
    const CORE::CString tag = built->tag.IsNULLOrEmpty() ? "sha-" + ShortCommit( result.commit ) : built->tag;
    CORE::CString registryDigest;
    bool notFound = false;
    if ( !m_registry.ResolveDigest( ci.GetRegistryUrl(), ci.registryUser, repository, tag, registryDigest, notFound, problem ) )
        return Finish( result, "refused", "registry", problem, caller, error );
    if ( registryDigest != built->digest )
        return Finish( result, "refused", "registry", "The registry has " + repository + ":" + tag + " as " + registryDigest + ", the run reported " + built->digest +
                       "; the tag was moved or the report is stale, nothing is deployed", caller, error );
    result.imageReference = ci.GetRegistryHostForImages() + "/" + repository + "@" + built->digest;

    // 7. one at a time
    const CORE::CString key = listedTarget->GetKey();
    if ( !AcquireTarget( key ) )
        return Finish( result, "refused", "busy", "Another deployment or rollback of " + key + " is running; wait for it and look with ci_release_status", caller, error );

    // 8. the Deployment pulls from a registry
    ICiDeployer::CState before;
    if ( !deployer->GetState( *listedTarget, before, problem ) )
    {
        ReleaseTarget( key );
        return Finish( result, "failed", "kubectl", problem, caller, error );
    }
    result.previousImage = before.image;
    result.previousDigest = before.GetDigest();
    result.strategy = before.strategy;
    if ( before.imagePullPolicy == "Never" )
    {
        ReleaseTarget( key );
        return Finish( result, "refused", "pull_policy", "The Deployment " + key + " uses imagePullPolicy Never: it runs an image imported into the nodes by hand "
                       "and cannot pull from the registry. The owner switches the service to the registry once (IMAGE_REGISTRY in its service.env and one run "
                       "of its deploy.sh, which sets imagePullPolicy IfNotPresent); after that it can be deployed from here.", caller, error );
    }
    if ( before.image == result.imageReference )
    {
        ReleaseTarget( key );
        result.outcome = "unchanged";
        result.revision = before.revision;
        result.replicas = before.DescribeReplicas();
        result.message = key + " already runs " + result.imageReference + " (revision " + before.revision + ", " + before.DescribeReplicas() + "); nothing was changed.";
        m_audit.Record( result, caller.machineId );
        return true;
    }

    // 9. the image by digest and the change cause of the new revision in one change, then the rollout
    const CORE::CString changeCause = "ci " + ci.id + " run " + result.runId + " commit " + result.commit + " image " + listedImage->name +
                                      " by " + caller.machineId + " at " + NowIsoUtc();
    if ( !deployer->SetImage( *listedTarget, result.imageReference, changeCause, problem ) )
    {
        ReleaseTarget( key );
        return Finish( result, "failed", "kubectl", problem + ". The image was not changed.", caller, error );
    }

    CORE::CString detail;
    const ICiDeployer::ERolloutResult rollout = deployer->WaitForRollout( *listedTarget, rolloutTimeout, progress, detail );
    if ( ICiDeployer::ROLLOUT_COMPLETE != rollout )
    {
        const CORE::CString text = DescribeIncompleteRollout( *listedTarget, rollout, detail, result );
        ReleaseTarget( key );
        const char* outcome = ICiDeployer::ROLLOUT_TIMEOUT == rollout ? "rollout_timeout" : ( ICiDeployer::ROLLOUT_FAILED == rollout ? "rollout_failed" :
                              ( ICiDeployer::ROLLOUT_CANCELLED == rollout ? "cancelled" : "rollout_unknown" ) );
        return Finish( result, outcome, "rollout", text, caller, error );
    }
    ICiDeployer::CState after;
    if ( deployer->GetState( *listedTarget, after, problem ) )
    {
        result.revision = after.revision;
        result.replicas = after.DescribeReplicas();
    }
    ReleaseTarget( key );
    result.outcome = "deployed";
    result.message = key + " runs " + result.imageReference + " (run " + result.runId + ", commit " + result.commit + ")" +
                     ( result.revision.IsNULLOrEmpty() ? CORE::CString::Empty : ", revision " + result.revision ) +
                     ( result.replicas.IsNULLOrEmpty() ? CORE::CString::Empty : ", " + result.replicas ) + ". Before: " + result.previousImage + ".";
    m_audit.Record( result, caller.machineId );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Deploy: " << result.message << " By " << caller.machineId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCiDeployManager::Rollback( const CCiDefinition& ci             ,
                            const CORE::CString& image          ,
                            const CORE::CString& target         ,
                            bool confirm                        ,
                            const CCiCaller& caller             ,
                            CCiDeployResult& result             ,
                            CORE::CString& error                ,
                            ICiProgress* progress               )
{GUCEF_TRACE;

    result = CCiDeployResult();
    result.action = "rollback";
    result.ciId = ci.id;
    result.image = image;
    result.target = target;

    const CCiDeployImage* listedImage = GUCEF_NULL;
    const CCiDeployImage::CTarget* listedTarget = GUCEF_NULL;
    CORE::CString problem;
    if ( !FindListed( ci, image, target, listedImage, listedTarget, problem ) )
        return Finish( result, "refused", "not_listed", problem, caller, error );
    result.image = listedImage->name;
    result.nameSpace = listedTarget->nameSpace;
    result.deployment = listedTarget->deployment;
    result.container = listedTarget->container;
    if ( !caller.HasScope( CCiCaller::ScopeDeploy ) )
        return Finish( result, "refused", "scope", "The machine " + caller.machineId + " may not roll back: its token has no scope deploy", caller, error );
    if ( !confirm )
        return Finish( result, "refused", "confirm", "A rollback needs confirm=true. Roll back only when the user asked for it explicitly; ci_release_status "
                       "shows the revision history first.", caller, error );

    MT::CScopeMutex deployerLock( m_lock );
    ICiDeployer* deployer = m_deployer;
    const UInt32 rolloutTimeout = m_rolloutTimeoutSeconds;
    deployerLock.EarlyUnlock();
    if ( GUCEF_NULL == deployer )
        return Finish( result, "refused", "kubectl", "This server has no deployer set up", caller, error );

    const CORE::CString key = listedTarget->GetKey();
    if ( !AcquireTarget( key ) )
        return Finish( result, "refused", "busy", "Another deployment or rollback of " + key + " is running; wait for it and look with ci_release_status", caller, error );

    ICiDeployer::CState before;
    if ( !deployer->GetState( *listedTarget, before, problem ) )
    {
        ReleaseTarget( key );
        return Finish( result, "failed", "kubectl", problem, caller, error );
    }
    result.previousImage = before.image;
    result.previousDigest = before.GetDigest();
    result.strategy = before.strategy;

    CORE::CString undoMessage;
    if ( !deployer->Undo( *listedTarget, undoMessage, problem ) )
    {
        ReleaseTarget( key );
        return Finish( result, "failed", "kubectl", problem + ". Nothing was changed.", caller, error );
    }
    CORE::CString detail;
    const ICiDeployer::ERolloutResult rollout = deployer->WaitForRollout( *listedTarget, rolloutTimeout, progress, detail );
    if ( ICiDeployer::ROLLOUT_COMPLETE != rollout )
    {
        const CORE::CString text = DescribeIncompleteRollout( *listedTarget, rollout, detail, result );
        ReleaseTarget( key );
        const char* outcome = ICiDeployer::ROLLOUT_TIMEOUT == rollout ? "rollout_timeout" : ( ICiDeployer::ROLLOUT_FAILED == rollout ? "rollout_failed" :
                              ( ICiDeployer::ROLLOUT_CANCELLED == rollout ? "cancelled" : "rollout_unknown" ) );
        return Finish( result, outcome, "rollout", "The rollback was started. " + text, caller, error );
    }
    ICiDeployer::CState after;
    if ( deployer->GetState( *listedTarget, after, problem ) )
    {
        result.imageReference = after.image;
        result.digest = after.GetDigest();
        result.revision = after.revision;
        result.replicas = after.DescribeReplicas();
    }
    ReleaseTarget( key );
    result.outcome = "rolled_back";
    result.message = key + " was returned to its previous revision" + ( result.revision.IsNULLOrEmpty() ? CORE::CString::Empty : " (now revision " + result.revision + ")" ) +
                     ": it runs " + ( result.imageReference.IsNULLOrEmpty() ? CORE::CString( "an image that could not be read" ) : result.imageReference ) +
                     ( result.replicas.IsNULLOrEmpty() ? CORE::CString::Empty : ", " + result.replicas ) + ". Before: " + result.previousImage + "." +
                     ( after.changeCause.IsNULLOrEmpty() ? CORE::CString::Empty : " Change cause of that revision: " + after.changeCause );
    m_audit.Record( result, caller.machineId );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Deploy: " << result.message << " By " << caller.machineId );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCiDeployManager::ReleaseStatus( const CCiDefinition& ci            ,
                                 const CORE::CString& target        ,
                                 CCiReleaseStatus& status           ,
                                 CORE::CString& error               )
{GUCEF_TRACE;

    status = CCiReleaseStatus();
    status.ciId = ci.id;
    status.target = target;
    for ( size_t i=0; i<ci.deployImages.size(); ++i )
    {
        const CCiDeployImage::CTarget* listed = ci.deployImages[ i ].FindTarget( target );
        if ( GUCEF_NULL == listed )
            continue;
        CCiReleaseStatus::CEntry entry;
        entry.image = ci.deployImages[ i ].name;
        entry.nameSpace = listed->nameSpace;
        entry.deployment = listed->deployment;
        entry.container = listed->container;
        entry.allowScratch = listed->allowScratch;
        status.entries.push_back( entry );
    }
    if ( status.entries.empty() )
    {
        error = ci.deployImages.empty() ? "Nothing of CI id " + ci.id + " may be deployed: its entry in ci.json has no deploy block"
                                        : "No image of CI id " + ci.id + " has the target \"" + target + "\". Deployable: " + ci.ListDeployables();
        return false;
    }

    MT::CScopeMutex deployerLock( m_lock );
    ICiDeployer* deployer = m_deployer;
    deployerLock.EarlyUnlock();
    if ( GUCEF_NULL == deployer )
    {
        error = "This server has no deployer set up";
        return false;
    }
    for ( size_t i=0; i<status.entries.size(); ++i )
    {
        CCiReleaseStatus::CEntry& entry = status.entries[ i ];
        CCiDeployImage::CTarget listed;
        listed.name = target;
        listed.nameSpace = entry.nameSpace;
        listed.deployment = entry.deployment;
        listed.container = entry.container;
        ICiDeployer::CState state;
        CORE::CString problem;
        if ( !deployer->GetState( listed, state, problem ) )
        {
            entry.problem = problem;
            continue;
        }
        entry.imageReference = state.image;
        entry.digest = state.GetDigest();
        entry.imagePullPolicy = state.imagePullPolicy;
        entry.revision = state.revision;
        entry.changeCause = state.changeCause;
        entry.strategy = state.strategy;
        entry.replicas = state.DescribeReplicas();
        if ( !deployer->GetHistory( listed, entry.history, problem ) )
            entry.history = "(the history could not be read: " + problem + ")";
    }
    return true;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
