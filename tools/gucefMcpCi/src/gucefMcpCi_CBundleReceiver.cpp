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
#include "gucefMcpCi_CSnapshotBuilder.h"
#include "gucefMcpCi_CBundleReceiver.h"

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

static const UInt32 GitTimeoutInMs = 600000;
static const UInt32 FetchTimeoutInMs = 1800000;
static const UInt32 DefaultLocalRetentionDays = 14;
static const UInt32 MaxErrorChars = 1500;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CProcess::TStringVector
Args( const char* a, const char* b = GUCEF_NULL, const char* c = GUCEF_NULL, const char* d = GUCEF_NULL, const char* e = GUCEF_NULL )
{
    CORE::CProcess::TStringVector args;
    const char* parts[] = { a, b, c, d, e };
    for ( int i=0; i<5; ++i )
        if ( GUCEF_NULL != parts[ i ] )
            args.push_back( parts[ i ] );
    return args;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
GitProblem( const CORE::CString& output )
{
    UInt32 totalLines = 0;
    return CCiUtils::Bound( CCiUtils::Trimmed( CCiUtils::TailLines( output, 8, 0, totalLines ) ), MaxErrorChars );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CBundleReceiver::CBundleReceiver( CDirectCiService& service )
    : m_service( service )
    , m_directory()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBundleReceiver::~CBundleReceiver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CBundleReceiver::SetDirectory( const CORE::CString& directory )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_directory = directory;
}

/*-------------------------------------------------------------------------*/

MT::CMutex&
CBundleReceiver::LockOf( const CORE::CString& ciId )
{GUCEF_TRACE;

    // the lock of the service: the check of published commits and the release submits work in the same repository
    return m_service.GetRepositoryLock( ciId );
}

/*-------------------------------------------------------------------------*/

bool
CBundleReceiver::EnsureRepo( const CORE::CString& repoDir, CORE::CString& error )
{GUCEF_TRACE;

    CORE::CString output;
    if ( CORE::DirExists( repoDir ) &&
         CCiUtils::RunGit( repoDir, Args( "rev-parse", "--is-bare-repository" ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) &&
         CCiUtils::Trimmed( output ) == "true" )
        return true;
    if ( CORE::DirExists( repoDir ) )
        CORE::DeleteDir( repoDir, true );
    CORE::CreateDirs( repoDir );
    if ( !CCiUtils::RunGit( repoDir, Args( "init", "-q", "--bare" ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
    {
        error = "The gateway could not make its repository: " + GitProblem( output );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBundleReceiver::Verify( const CORE::CString& repoDir, const CORE::CString& bundlePath, bool& missingBase, CORE::CString& error )
{GUCEF_TRACE;

    missingBase = false;
    CORE::CString output;
    // without --quiet: with it git does not say which commits are missing
    if ( CCiUtils::RunGit( repoDir, Args( "bundle", "verify", bundlePath.C_String() ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
        return true;
    missingBase = output.HasSubstr( "prerequisite" ) >= 0;
    error = GitProblem( output );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CBundleReceiver::PruneOldBranches( const CORE::CString& repoDir, UInt32 retentionDays )
{GUCEF_TRACE;

    const UInt32 days = retentionDays > 0 ? retentionDays : DefaultLocalRetentionDays;
    const Int64 oldest = CCiUtils::NowUnix() - (Int64) days * 86400;
    CORE::CString output;
    CORE::CString prefix = CORE::CString( "refs/heads/" ) + CSnapshotBuilder::BranchPrefix;
    if ( !CCiUtils::RunGit( repoDir, Args( "for-each-ref", "--format=%(refname)", prefix.C_String() ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
        return;
    CORE::CString::StringVector refs = output.ReplaceSubstr( "\r", "" ).ParseElements( '\n', false );
    UInt32 removed = 0;
    for ( size_t i=0; i<refs.size(); ++i )
    {
        // scratch/<machine>-<utc>-<shortsha>: the time is the part before the last '-'
        const CORE::CString ref = CCiUtils::Trimmed( refs[ i ] );
        CORE::CString::StringVector parts = ref.ParseElements( '-', false );
        if ( parts.size() < 3 )
            continue;
        Int64 time = 0;
        if ( !CCiUtils::ParseCompactUtcToUnix( parts[ parts.size()-2 ], time ) || time >= oldest )
            continue;
        if ( CCiUtils::RunGit( repoDir, Args( "update-ref", "-d", ref.C_String() ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
            ++removed;
    }
    if ( removed > 0 )
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Gateway: Removed " << removed << " local scratch branch(es) older than " << days << " day(s) from " << repoDir );
}

/*-------------------------------------------------------------------------*/

CBundleReceiver::EResult
CBundleReceiver::Receive( const CORE::CString& ciId         ,
                          const CORE::CString& bundlePath   ,
                          const CORE::CString& machineName  ,
                          CORE::CString& repoDir            ,
                          CORE::CString& commit             ,
                          CORE::CString& branchName         ,
                          CORE::CString& error              )
{GUCEF_TRACE;

    const CCiDefinition* ci = m_service.GetRegistry().FindById( ciId );
    if ( GUCEF_NULL == ci )
    {
        error = "Unknown CI id \"" + ciId + "\"";
        return BUNDLE_FAILED;
    }
    {
        MT::CScopeMutex lock( m_lock );
        repoDir = CORE::CombinePath( m_directory, CCiUtils::SafeName( ciId, 60 ) + ".git" );
    }

    MT::CScopeMutex ciLock( LockOf( ciId ) );
    if ( !EnsureRepo( repoDir, error ) )
        return BUNDLE_FAILED;

    // exactly one ref, the snapshot
    CORE::CString output;
    if ( !CCiUtils::RunGit( repoDir, Args( "bundle", "list-heads", bundlePath.C_String() ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
    {
        error = "What arrived is not a git bundle: " + GitProblem( output );
        return BUNDLE_INVALID;
    }
    CORE::CString::StringVector heads = output.ReplaceSubstr( "\r", "" ).ParseElements( '\n', false );
    commit.Clear();
    for ( size_t i=0; i<heads.size(); ++i )
    {
        const CORE::CString line = CCiUtils::Trimmed( heads[ i ] );
        if ( line.IsNULLOrEmpty() )
            continue;
        const Int32 space = line.HasChar( ' ', true );
        const CORE::CString id = space > 0 ? line.SubstrToIndex( (UInt32) space, true ) : CORE::CString::Empty;
        const CORE::CString ref = space > 0 ? CCiUtils::Trimmed( line.CutChars( (UInt32) space + 1, true ) ) : CORE::CString::Empty;
        if ( ref != CSnapshotBuilder::BundleRef || !CCiUtils::IsHex( id, 40 ) || !commit.IsNULLOrEmpty() )
        {
            error = "The bundle is not one of a snapshot: it must carry exactly the ref " + CORE::CString( CSnapshotBuilder::BundleRef );
            return BUNDLE_INVALID;
        }
        commit = id;
    }
    if ( commit.IsNULLOrEmpty() )
    {
        error = "The bundle carries no snapshot";
        return BUNDLE_INVALID;
    }

    // what the bundle builds on must be here: when it is not, what the CI published is fetched once
    bool missingBase = false;
    CORE::CString problem;
    if ( !Verify( repoDir, bundlePath, missingBase, problem ) )
    {
        if ( !missingBase )
        {
            error = "The bundle is damaged: " + problem;
            return BUNDLE_INVALID;
        }
        CORE::CString fetchProblem;
        if ( !m_service.FetchPublished( ciId, repoDir, fetchProblem ) )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Gateway: Fetching the scratch repository of " << ciId << " failed: " << fetchProblem );
        if ( !Verify( repoDir, bundlePath, missingBase, problem ) )
        {
            if ( missingBase )
            {
                error = "The snapshot was made against commits the CI does not have (an unknown base). Send it again against the commits "
                        "GET /v1/ci names, or as a full bundle. git says: " + problem;
                return BUNDLE_UNKNOWN_BASE;
            }
            error = "The bundle is damaged: " + problem;
            return BUNDLE_INVALID;
        }
    }

    branchName = CSnapshotBuilder::MakeBranchName( machineName, commit );
    CORE::CProcess::TStringVector args;
    args.push_back( "-c" );
    args.push_back( "gc.auto=0" );
    args.push_back( "fetch" );
    args.push_back( "--quiet" );
    args.push_back( "--no-tags" );
    args.push_back( bundlePath );
    args.push_back( CORE::CString( "+" ) + CSnapshotBuilder::BundleRef + ":refs/heads/" + branchName );
    if ( !CCiUtils::RunGit( repoDir, args, CORE::CProcess::TStringMap(), FetchTimeoutInMs, output ) )
    {
        error = "The bundle could not be taken in (git fetch): " + GitProblem( output );
        return BUNDLE_FAILED;
    }
    if ( !CCiUtils::RunGit( repoDir, Args( "cat-file", "-t", commit.C_String() ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) ||
         CCiUtils::Trimmed( output ) != "commit" )
    {
        error = "The snapshot of the bundle is not a commit";
        return BUNDLE_INVALID;
    }

    PruneOldBranches( repoDir, ci->retentionDays );
    return BUNDLE_OK;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
