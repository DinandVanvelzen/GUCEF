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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

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
#include "gucefMcpCi_CDirectCiService.h"

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

static const Int64 SweepIntervalInSecs = 6 * 3600;
static const UInt32 GitTimeoutInMs = 60000;
static const char RunIdSeparator = ':';
static const UInt32 MaxCommitChecks = 50;

const char* CDirectCiService::ReleaseBranchPrefix = "scratch/release-";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The top level directory of the working tree that holds 'repoDir' */
static bool
TopLevel( const CORE::CString& repoDir, CORE::CString& topLevel, CORE::CString& error )
{
    CORE::CProcess::TStringVector args;
    args.push_back( "rev-parse" );
    args.push_back( "--show-toplevel" );
    CORE::CString output;
    if ( !CORE::DirExists( repoDir ) || !CCiUtils::RunGit( repoDir, args, CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
    {
        error = "repo_dir is not a git working tree: " + repoDir;
        return false;
    }
    CORE::CString::StringVector lines = output.ReplaceSubstr( "\r", "" ).ParseElements( '\n', false );
    topLevel = lines.empty() ? repoDir : CCiUtils::Trimmed( lines[ lines.size()-1 ] );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDirectCiService::CDirectCiService( const CCiRegistry& registry, const CSnapshotBuilder& snapshotBuilder )
    : ICiService()
    , m_registry( registry )
    , m_snapshotBuilder( snapshotBuilder )
    , m_backends()
    , m_machineName()
    , m_pollIntervalInMs( 5000 )
    , m_lock()
    , m_lastSweeps()
    , m_repositoryDirectory()
    , m_repositoryLocks()
    , m_deployManager()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDirectCiService::~CDirectCiService()
{GUCEF_TRACE;

    for ( size_t i=0; i<m_backends.size(); ++i )
        GUCEF_DELETE m_backends[ i ];
    m_backends.clear();
    for ( TLockMap::iterator i = m_repositoryLocks.begin(); i != m_repositoryLocks.end(); ++i )
        GUCEF_DELETE (*i).second;
    m_repositoryLocks.clear();
}

/*-------------------------------------------------------------------------*/

void
CDirectCiService::SetRepositoryDirectory( const CORE::CString& directory )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_repositoryDirectory = directory;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDirectCiService::GetRepositoryPath( const CORE::CString& ciId ) const
{GUCEF_TRACE;

    const CORE::CString directory = m_repositoryDirectory.IsNULLOrEmpty() ? CORE::CombinePath( CORE::TempDir(), "gucefMcpCi/repos" ) : m_repositoryDirectory;
    return CORE::CombinePath( directory, CCiUtils::SafeName( ciId, 60 ) + ".git" );
}

/*-------------------------------------------------------------------------*/

MT::CMutex&
CDirectCiService::GetRepositoryLock( const CORE::CString& ciId )
{GUCEF_TRACE;

    // one per CI id of the registry: bounded by the registry
    MT::CScopeMutex lock( m_lock );
    TLockMap::iterator i = m_repositoryLocks.find( ciId );
    if ( i != m_repositoryLocks.end() )
        return *(*i).second;
    MT::CMutex* mutex = GUCEF_NEW MT::CMutex();
    m_repositoryLocks[ ciId ] = mutex;
    return *mutex;
}

/*-------------------------------------------------------------------------*/

CCiDeployManager&
CDirectCiService::GetDeployManager( void )
{GUCEF_TRACE;

    return m_deployManager;
}

/*-------------------------------------------------------------------------*/

void
CDirectCiService::AddBackend( ICiBackend* backend )
{GUCEF_TRACE;

    if ( GUCEF_NULL != backend )
        m_backends.push_back( backend );
}

/*-------------------------------------------------------------------------*/

void
CDirectCiService::SetMachineName( const CORE::CString& machineName )
{GUCEF_TRACE;

    m_machineName = machineName;
}

/*-------------------------------------------------------------------------*/

void
CDirectCiService::SetPollInterval( UInt32 pollIntervalInMs )
{GUCEF_TRACE;

    m_pollIntervalInMs = pollIntervalInMs > 0 ? pollIntervalInMs : 1000;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDirectCiService::ListCiIds( void ) const
{GUCEF_TRACE;

    return m_registry.ListIds();
}

/*-------------------------------------------------------------------------*/

const CCiDefinition*
CDirectCiService::FindCi( const CORE::CString& ciId, CORE::CString& error ) const
{GUCEF_TRACE;

    const CCiDefinition* ci = m_registry.FindById( ciId );
    if ( GUCEF_NULL == ci )
    {
        const CORE::CString known = m_registry.ListIds();
        error = "Unknown CI id \"" + ciId + "\". " + ( known.IsNULLOrEmpty() ? CORE::CString( "No CI is set up." ) : "Known CI ids: " + known + "." ) +
                " ci_resolve_repo gives the CI id of a repository.";
    }
    return ci;
}

/*-------------------------------------------------------------------------*/

ICiBackend*
CDirectCiService::FindBackend( const CCiDefinition& ci, CORE::CString& error ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_backends.size(); ++i )
        if ( m_backends[ i ]->GetBackendName() == ci.backend )
            return m_backends[ i ];
    error = "The CI id \"" + ci.id + "\" uses a kind of CI this server does not support";
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::SplitRunId( const CORE::CString& runId      ,
                              const CCiDefinition*& ci        ,
                              ICiBackend*& backend            ,
                              CORE::CString& backendRunId     ,
                              CORE::CString& error            ) const
{GUCEF_TRACE;

    const CORE::CString trimmed = CCiUtils::Trimmed( runId );
    const Int32 separator = trimmed.HasChar( RunIdSeparator, false );
    if ( separator <= 0 || (UInt32) separator + 1 >= trimmed.Length() )
    {
        error = "Unknown run id \"" + runId + "\": run ids look like <CI id>:<number>, as ci_submit returns them";
        return false;
    }
    const CORE::CString ciId = trimmed.SubstrToIndex( (UInt32) separator, true );
    backendRunId = trimmed.CutChars( (UInt32) separator + 1, true );
    ci = m_registry.FindById( ciId );
    if ( GUCEF_NULL == ci )
    {
        error = "Unknown run id \"" + runId + "\": the CI id \"" + ciId + "\" is not known";
        return false;
    }
    backend = FindBackend( *ci, error );
    return GUCEF_NULL != backend;
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::GetRemotes( const CORE::CString& repoDir, CORE::CString::StringVector& remotes, CORE::CString& error )
{GUCEF_TRACE;

    CORE::CProcess::TStringVector args;
    args.push_back( "remote" );
    args.push_back( "-v" );
    CORE::CString output;
    if ( !CCiUtils::RunGit( repoDir, args, CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
    {
        error = "Not a git repository, or git failed: " + repoDir;
        return false;
    }
    // origin<TAB>https://host/org/repo.git (fetch)
    CORE::CString::StringVector lines = output.ReplaceSubstr( "\r", "" ).ParseElements( '\n', false );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        CORE::CString line = lines[ i ].ReplaceChar( '\t', ' ' );
        CORE::CString::StringVector parts = line.ParseElements( ' ', false );
        if ( parts.size() < 2 )
            continue;
        const CORE::CString url = CCiUtils::Trimmed( parts[ 1 ] );
        bool listed = false;
        for ( size_t r=0; r<remotes.size() && !listed; ++r )
            listed = remotes[ r ] == url;
        if ( !listed && !url.IsNULLOrEmpty() )
            remotes.push_back( url );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::Resolve( const CORE::CString& repoDir                     ,
                           const CORE::CString::StringVector& remotes       ,
                           CORE::CString& ciId                              ,
                           CORE::CString& description                       ,
                           CORE::CString& matchedRemote                     ,
                           CORE::CString::StringVector& examinedRemotes     ,
                           CORE::CString& error                             )
{GUCEF_TRACE;

    examinedRemotes.clear();
    if ( !repoDir.IsNULLOrEmpty() )
    {
        if ( !CORE::DirExists( repoDir ) )
        {
            error = "repo_dir is not an existing directory: " + repoDir;
            return false;
        }
        if ( !GetRemotes( repoDir, examinedRemotes, error ) )
            return false;
    }
    for ( size_t i=0; i<remotes.size(); ++i )
        examinedRemotes.push_back( remotes[ i ] );
    if ( examinedRemotes.empty() )
    {
        error = repoDir.IsNULLOrEmpty() ? CORE::CString( "Give repo_dir or remotes" ) : "The repository has no remotes, so no CI can be set up for it: " + repoDir;
        return false;
    }

    const CCiDefinition* ci = m_registry.FindByRemotes( examinedRemotes, matchedRemote );
    if ( GUCEF_NULL == ci )
    {
        error = "No CI is set up for this repository. Only repositories the CI already knows can be built; this server knows " +
                ( m_registry.IsEmpty() ? CORE::CString( "none" ) : m_registry.ListIds() ) + ". Remotes looked at: " + CCiUtils::Join( examinedRemotes, ", " );
        return false;
    }
    ciId = ci->id;
    description = ci->description;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::Describe( const CORE::CString& ciId     ,
                            const CORE::CString& repoDir  ,
                            CCiDescription& description   ,
                            CORE::CString& error          )
{GUCEF_TRACE;

    const CCiDefinition* ci = FindCi( ciId, error );
    if ( GUCEF_NULL == ci )
        return false;
    ICiBackend* backend = FindBackend( *ci, error );
    if ( GUCEF_NULL == backend )
        return false;

    CORE::CString topLevel;
    if ( !repoDir.IsNULLOrEmpty() && !TopLevel( repoDir, topLevel, error ) )
        return false;
    return backend->Describe( *ci, topLevel, description, error );
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::CheckFilters( const CCiDefinition& ci, ICiBackend& backend, const CORE::CString& topLevel, const CCiSubmitOptions& options, CORE::CString& error ) const
{GUCEF_TRACE;

    if ( options.mode != "build" && options.mode != "test" )
    {
        error = "mode \"" + options.mode + "\" is not known: build or test";
        return false;
    }
    if ( options.toolchains.empty() && options.cppStandards.empty() && options.products.empty() )
        return true;

    CCiDescription description;
    CORE::CString problem;
    if ( !backend.Describe( ci, topLevel, description, problem ) || !description.hasOptions )
        return true;        // the CI does not say: passed on as they are
    return description.CheckFilters( options, ci.id, error );
}

/*-------------------------------------------------------------------------*/

CCiSubmitOptions
CDirectCiService::WithDefaults( const CCiDefinition& ci, const CCiSubmitOptions& options )
{GUCEF_TRACE;

    CCiSubmitOptions effective = options;
    if ( effective.mode.IsNULLOrEmpty() )
        effective.mode = ci.defaultMode;
    effective.mode = effective.mode.Lowercase();
    if ( effective.toolchains.empty() )
        effective.toolchains = ci.defaultToolchains;
    if ( effective.cppStandards.empty() )
        effective.cppStandards = ci.defaultCppStandards;
    if ( effective.products.empty() )
        effective.products = ci.defaultProducts;
    return effective;
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::Submit( const CORE::CString& ciId       ,
                          const CORE::CString& repoDir    ,
                          const CCiSubmitOptions& options ,
                          CCiSnapshot& snapshot           ,
                          CORE::CString& runId            ,
                          CORE::CString& error            ,
                          ICiProgress* progress           )
{GUCEF_TRACE;

    const CCiDefinition* ci = FindCi( ciId, error );
    if ( GUCEF_NULL == ci )
        return false;
    ICiBackend* backend = FindBackend( *ci, error );
    if ( GUCEF_NULL == backend )
        return false;
    if ( !options.commit.IsNULLOrEmpty() )
        return SubmitCommit( *ci, *backend, options, snapshot, runId, error );
    if ( repoDir.IsNULLOrEmpty() || !CORE::DirExists( repoDir ) )
    {
        error = "repo_dir is not an existing directory: " + repoDir;
        return false;
    }

    // only a checkout of the CI id is sent there
    CORE::CString::StringVector remotes;
    if ( !GetRemotes( repoDir, remotes, error ) )
        return false;
    CORE::CString matchedRemote;
    const CCiDefinition* ciOfRepo = m_registry.FindByRemotes( remotes, matchedRemote );
    if ( GUCEF_NULL == ciOfRepo || ciOfRepo->id != ci->id )
    {
        error = "repo_dir is not a checkout of CI id " + ci->id + " (its remotes: " + ( remotes.empty() ? CORE::CString( "none" ) : CCiUtils::Join( remotes, ", " ) ) +
                "). " + ( GUCEF_NULL == ciOfRepo ? CORE::CString( "No CI is set up for it." ) : "Its CI id is " + ciOfRepo->id + "." );
        return false;
    }

    const CCiSubmitOptions effective = WithDefaults( *ci, options );
    CORE::CString topLevel;
    if ( !TopLevel( repoDir, topLevel, error ) || !CheckFilters( *ci, *backend, topLevel, effective, error ) )
        return false;
    if ( GUCEF_NULL != progress )
        progress->OnProgress( 0.0, -1.0, "making the snapshot" );
    if ( !m_snapshotBuilder.Build( repoDir, effective.includeUncommitted, m_machineName, snapshot, error ) )
        return false;
    if ( effective.dryRun )
        return true;
    if ( GUCEF_NULL != progress )
    {
        if ( progress->IsCancelled() )
        {
            error = "Cancelled: the snapshot was made but not sent";
            return false;
        }
        progress->OnProgress( 0.0, -1.0, "sending the snapshot" );
    }

    CORE::CString backendRunId;
    if ( !backend->Submit( *ci, snapshot, effective, backendRunId, error ) )
        return false;
    runId = ci->id + RunIdSeparator + backendRunId;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Run " << runId << " started for " << snapshot.repoDir << " on " << snapshot.branchName );

    SweepIfDue( *ci, *backend );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDirectCiService::SweepIfDue( const CCiDefinition& ci, ICiBackend& backend )
{GUCEF_TRACE;

    if ( 0 == ci.retentionDays )
        return;
    const Int64 now = CCiUtils::NowUnix();
    {
        MT::CScopeMutex lock( m_lock );
        TSweepMap::iterator last = m_lastSweeps.find( ci.id );
        if ( last != m_lastSweeps.end() && now - (*last).second < SweepIntervalInSecs )
            return;
        m_lastSweeps[ ci.id ] = now;
    }
    CORE::CString summary;
    CORE::CString error;
    if ( backend.Cleanup( ci, ci.retentionDays, summary, error ) )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Retention of " << ci.id << ": " << summary );
    }
    else
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: The retention sweep of " << ci.id << " failed: " << error );
    }
}

/*-------------------------------------------------------------------------*/

const CCiRegistry&
CDirectCiService::GetRegistry( void ) const
{GUCEF_TRACE;

    return m_registry;
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::ListBaseCommits( const CORE::CString& ciId, CORE::CString::StringVector& commits, CORE::CString& error )
{GUCEF_TRACE;

    const CCiDefinition* ci = FindCi( ciId, error );
    if ( GUCEF_NULL == ci )
        return false;
    ICiBackend* backend = FindBackend( *ci, error );
    return GUCEF_NULL != backend && backend->ListBaseCommits( *ci, commits, error );
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::FetchPublished( const CORE::CString& ciId, const CORE::CString& bareRepoDir, CORE::CString& error )
{GUCEF_TRACE;

    const CCiDefinition* ci = FindCi( ciId, error );
    if ( GUCEF_NULL == ci )
        return false;
    ICiBackend* backend = FindBackend( *ci, error );
    return GUCEF_NULL != backend && backend->FetchPublished( *ci, bareRepoDir, error );
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::SendSnapshot( const CORE::CString& ciId, const CCiSnapshot& snapshot, CORE::CString& error )
{GUCEF_TRACE;

    const CCiDefinition* ci = FindCi( ciId, error );
    if ( GUCEF_NULL == ci )
        return false;
    ICiBackend* backend = FindBackend( *ci, error );
    return GUCEF_NULL != backend && backend->SendSnapshot( *ci, snapshot, error );
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::StartRun( const CORE::CString& ciId       ,
                            const CCiSnapshot& snapshot     ,
                            const CCiSubmitOptions& options ,
                            CORE::CString& runId            ,
                            CORE::CString& error            )
{GUCEF_TRACE;

    const CCiDefinition* ci = FindCi( ciId, error );
    if ( GUCEF_NULL == ci )
        return false;
    ICiBackend* backend = FindBackend( *ci, error );
    if ( GUCEF_NULL == backend )
        return false;
    const CCiSubmitOptions effective = WithDefaults( *ci, options );
    if ( !CheckFilters( *ci, *backend, CORE::CString::Empty, effective, error ) )
        return false;

    CORE::CString backendRunId;
    if ( !backend->StartRun( *ci, snapshot, effective, backendRunId, error ) )
        return false;
    runId = ci->id + RunIdSeparator + backendRunId;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Run " << runId << " started on " << snapshot.branchName );

    SweepIfDue( *ci, *backend );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::Status( const CORE::CString& runId      ,
                          UInt32 waitSeconds              ,
                          CCiRunStatus& status            ,
                          CORE::CString& error            )
{GUCEF_TRACE;

    const CCiDefinition* ci = GUCEF_NULL;
    ICiBackend* backend = GUCEF_NULL;
    CORE::CString backendRunId;
    if ( !SplitRunId( runId, ci, backend, backendRunId, error ) )
        return false;

    const UInt64 startTicks = MT::PrecisionTickCount();
    const UInt64 waitMs = (UInt64) waitSeconds * 1000;
    for ( ;; )
    {
        status = CCiRunStatus();
        bool isUnknown = false;
        if ( !backend->GetRun( *ci, backendRunId, status, isUnknown, error ) )
        {
            if ( isUnknown )
                error = "Unknown run id \"" + runId + "\"";
            return false;
        }
        status.runId = runId;
        status.ciId = ci->id;
        const UInt64 elapsed = CCiUtils::MilliSecsSince( startTicks );
        if ( status.IsFinished() || elapsed >= waitMs )
            return true;
        const UInt64 left = waitMs - elapsed;
        MT::ThreadDelay( (UInt32) ( left < m_pollIntervalInMs ? left : m_pollIntervalInMs ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::Report( const CORE::CString& runId      ,
                          CCiReport& report               ,
                          CORE::CString& error            )
{GUCEF_TRACE;

    const CCiDefinition* ci = GUCEF_NULL;
    ICiBackend* backend = GUCEF_NULL;
    CORE::CString backendRunId;
    if ( !SplitRunId( runId, ci, backend, backendRunId, error ) )
        return false;
    bool isUnknown = false;
    if ( !backend->GetReport( *ci, backendRunId, report, isUnknown, error ) )
    {
        if ( isUnknown )
            error = "Unknown run id \"" + runId + "\"";
        return false;
    }
    report.status.runId = runId;
    report.status.ciId = ci->id;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::JobDetails( const CORE::CString& runId      ,
                              const CORE::CString& jobId      ,
                              const CORE::CString& product    ,
                              CCiJobDetail& detail            ,
                              CORE::CString& error            )
{GUCEF_TRACE;

    const CCiDefinition* ci = GUCEF_NULL;
    ICiBackend* backend = GUCEF_NULL;
    CORE::CString backendRunId;
    if ( !SplitRunId( runId, ci, backend, backendRunId, error ) )
        return false;
    bool isUnknown = false;
    if ( !backend->GetJobDetail( *ci, backendRunId, jobId, product, detail, isUnknown, error ) )
    {
        if ( isUnknown && error == "Unknown run id" )
            error = "Unknown run id \"" + runId + "\"";
        return false;
    }
    detail.runId = runId;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::Cancel( const CORE::CString& runId      ,
                          CORE::CString& message          ,
                          CORE::CString& error            )
{GUCEF_TRACE;

    const CCiDefinition* ci = GUCEF_NULL;
    ICiBackend* backend = GUCEF_NULL;
    CORE::CString backendRunId;
    if ( !SplitRunId( runId, ci, backend, backendRunId, error ) )
        return false;
    bool isUnknown = false;
    if ( !backend->Cancel( *ci, backendRunId, message, isUnknown, error ) )
    {
        if ( isUnknown )
            error = "Unknown run id \"" + runId + "\"";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::Cleanup( const CORE::CString& ciId      ,
                           UInt32 olderThanDays           ,
                           CORE::CString& summary         ,
                           CORE::CString& error           )
{GUCEF_TRACE;

    const CCiDefinition* ci = FindCi( ciId, error );
    if ( GUCEF_NULL == ci )
        return false;
    ICiBackend* backend = FindBackend( *ci, error );
    if ( GUCEF_NULL == backend )
        return false;
    return backend->Cleanup( *ci, olderThanDays, summary, error );
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::SubmitCommit( const CCiDefinition& ci             ,
                                ICiBackend& backend                 ,
                                const CCiSubmitOptions& options     ,
                                CCiSnapshot& snapshot               ,
                                CORE::CString& runId                ,
                                CORE::CString& error                )
{GUCEF_TRACE;

    const CORE::CString commit = CCiUtils::Trimmed( options.commit ).Lowercase();
    if ( 40 != commit.Length() || !CCiUtils::IsHex( commit, 40 ) )
    {
        error = "commit must be the full 40 character id of a commit of the published repository";
        return false;
    }
    const CCiSubmitOptions effective = WithDefaults( ci, options );
    if ( !CheckFilters( ci, backend, CORE::CString::Empty, effective, error ) )
        return false;

    snapshot = CCiSnapshot();
    snapshot.repoDir = GetRepositoryPath( ci.id );
    snapshot.commit = commit;
    snapshot.baseCommit = commit;
    snapshot.includesUncommitted = false;
    snapshot.branchName = ReleaseBranchPrefix + commit.SubstrToIndex( 12, true );
    {
        // the commit must be on a branch of the published repository, as fetched now: never the word of the caller
        MT::CScopeMutex lock( GetRepositoryLock( ci.id ) );
        if ( !CCiDeployManager::EnsureBareRepository( snapshot.repoDir, error ) || !backend.FetchPublishedRepository( ci, snapshot.repoDir, error ) )
            return false;
        CORE::CProcess::TStringVector args;
        args.push_back( "for-each-ref" );
        args.push_back( "--count=" + CORE::UInt32ToString( MaxCommitChecks ) );
        args.push_back( "--format=%(refname)" );
        args.push_back( "--contains" );
        args.push_back( commit );
        args.push_back( CORE::CString( ICiBackend::PublishedRefPrefix ).CutChars( 1, false ) );
        args.push_back( ICiBackend::PublishedHeadRef );
        CORE::CString output;
        const bool known = CSnapshotBuilder::HasCommit( snapshot.repoDir, commit );
        if ( !known || !CCiUtils::RunGit( snapshot.repoDir, args, CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) || CCiUtils::Trimmed( output ).IsNULLOrEmpty() )
        {
            error = "The commit " + commit + " is " + ( known ? CORE::CString( "not on a branch of" ) : CORE::CString( "not in" ) ) + " the published repository " +
                    ci.owner + "/" + ci.repo + ": only published commits are built this way. Work that is not published goes with repo_dir as a snapshot.";
            return false;
        }
        if ( effective.dryRun )
            return true;
        if ( !backend.SendSnapshot( ci, snapshot, error ) )
            return false;
    }
    CORE::CString backendRunId;
    if ( !backend.StartRun( ci, snapshot, effective, backendRunId, error ) )
        return false;
    runId = ci.id + RunIdSeparator + backendRunId;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Run " << runId << " started for the published commit " << commit << " on " << snapshot.branchName <<
                      ( effective.images.empty() ? CORE::CString::Empty : ", images " + CCiUtils::Join( effective.images, "," ) ) );
    SweepIfDue( ci, backend );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::Images( const CORE::CString& runId      ,
                          CCiImageReport& report          ,
                          CORE::CString& error            )
{GUCEF_TRACE;

    const CCiDefinition* ci = GUCEF_NULL;
    ICiBackend* backend = GUCEF_NULL;
    CORE::CString backendRunId;
    if ( !SplitRunId( runId, ci, backend, backendRunId, error ) )
        return false;
    CCiRunStatus status;
    bool isUnknown = false;
    if ( !backend->GetRun( *ci, backendRunId, status, isUnknown, error ) )
    {
        if ( isUnknown )
            error = "Unknown run id \"" + runId + "\"";
        return false;
    }
    bool missing = false;
    if ( !backend->GetImageReport( *ci, backendRunId, report, missing, error ) )
    {
        if ( missing && error == "Unknown run id" )
            error = "Unknown run id \"" + runId + "\"";
        return false;
    }
    report.runId = runId;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::Deploy( const CORE::CString& ciId         ,
                          const CORE::CString& image        ,
                          const CORE::CString& target       ,
                          const CORE::CString& runOrCommit  ,
                          bool confirm                      ,
                          const CCiCaller& caller           ,
                          CCiDeployResult& result           ,
                          CORE::CString& error              ,
                          ICiProgress* progress             )
{GUCEF_TRACE;

    result = CCiDeployResult();
    const CCiDefinition* ci = FindCi( ciId, error );
    if ( GUCEF_NULL == ci )
    {
        result.action = "deploy";
        result.outcome = "refused";
        result.code = "unknown_ci";
        result.message = error;
        return false;
    }
    ICiBackend* backend = FindBackend( *ci, error );
    if ( GUCEF_NULL == backend )
        return false;
    return m_deployManager.Deploy( *ci, *backend, GetRepositoryPath( ci->id ), GetRepositoryLock( ci->id ), image, target, runOrCommit, confirm, caller,
                                   result, error, progress );
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::ReleaseStatus( const CORE::CString& ciId     ,
                                 const CORE::CString& target   ,
                                 CCiReleaseStatus& status      ,
                                 CORE::CString& error          )
{GUCEF_TRACE;

    const CCiDefinition* ci = FindCi( ciId, error );
    if ( GUCEF_NULL == ci )
        return false;
    return m_deployManager.ReleaseStatus( *ci, target, status, error );
}

/*-------------------------------------------------------------------------*/

bool
CDirectCiService::Rollback( const CORE::CString& ciId       ,
                            const CORE::CString& image      ,
                            const CORE::CString& target     ,
                            bool confirm                    ,
                            const CCiCaller& caller         ,
                            CCiDeployResult& result         ,
                            CORE::CString& error            ,
                            ICiProgress* progress           )
{GUCEF_TRACE;

    result = CCiDeployResult();
    const CCiDefinition* ci = FindCi( ciId, error );
    if ( GUCEF_NULL == ci )
    {
        result.action = "rollback";
        result.outcome = "refused";
        result.code = "unknown_ci";
        result.message = error;
        return false;
    }
    return m_deployManager.Rollback( *ci, image, target, confirm, caller, result, error, progress );
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
