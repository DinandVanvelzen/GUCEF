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

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

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

const UInt64 CSnapshotBuilder::DefaultMaxBytes = (UInt64) 500 * 1024 * 1024;
const char* CSnapshotBuilder::BranchPrefix = "scratch/";
const char* CSnapshotBuilder::BundleRef = "refs/heads/snapshot";

static const UInt32 GitTimeoutInMs = 600000;            /**< git add -A of a large working tree takes a while */
static const UInt32 PushTimeoutInMs = 1800000;          /**< the first push of a large history */
static const UInt32 MaxListedChanges = 40;
static const UInt32 MaxListedRefusals = 20;
static const UInt32 MaxErrorChars = 2000;

/** File names that hold secrets as a rule, matched case-insensitively against the last part of a path */
static const char* SecretNamePatterns[] = { "*.env", ".env.*", "secrets.*", "*.pem", "*.key", "id_rsa*", "id_dsa*", "id_ecdsa*",
                                            "id_ed25519*", ".netrc", "_netrc", "*.token", "*.p12", "*.pfx" };

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static MT::CMutex g_tempDirLock;
static UInt32 g_tempDirCounter = 0;

/*-------------------------------------------------------------------------*/

/** Matches text against a pattern with * (any characters) and ? (one character) */
static bool
WildcardMatch( const char* pattern, const char* text )
{
    const char* star = GUCEF_NULL;
    const char* resume = GUCEF_NULL;
    while ( '\0' != *text )
    {
        if ( '?' == *pattern || *pattern == *text )
        {
            ++pattern;
            ++text;
        }
        else
        if ( '*' == *pattern )
        {
            star = pattern++;
            resume = text;
        }
        else
        if ( GUCEF_NULL != star )
        {
            pattern = star + 1;
            text = ++resume;
        }
        else
            return false;
    }
    while ( '*' == *pattern )
        ++pattern;
    return '\0' == *pattern;
}

/*-------------------------------------------------------------------------*/

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

/** Undoes the quoting git applies to unusual paths: "a\tb" */
static CORE::CString
UnquoteGitPath( const CORE::CString& path )
{
    if ( !path.StartsWith( "\"" ) || !path.EndsWith( "\"" ) || path.Length() < 2 )
        return path;
    const char* c = path.C_String();
    const UInt32 size = path.ByteSize() - 1;
    CORE::CString result;
    for ( UInt32 i=1; i+1<size; ++i )
    {
        if ( '\\' != c[ i ] || i+2 >= size )
        {
            result += c[ i ];
            continue;
        }
        const char next = c[ ++i ];
        if ( next >= '0' && next <= '7' && i+2 < size )
        {
            const int value = ( ( c[ i ] - '0' ) << 6 ) | ( ( c[ i+1 ] - '0' ) << 3 ) | ( c[ i+2 ] - '0' );
            result += (char) value;
            i += 2;
        }
        else
        if ( 't' == next )
            result += '\t';
        else
        if ( 'n' == next )
            result += '\n';
        else
            result += next;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

/** The non-empty lines of git output */
static CORE::CString::StringVector
Lines( const CORE::CString& output )
{
    CORE::CString::StringVector lines = output.ReplaceSubstr( "\r", "" ).ParseElements( '\n', false );
    CORE::CString::StringVector result;
    for ( size_t i=0; i<lines.size(); ++i )
        if ( !lines[ i ].IsNULLOrEmpty() )
            result.push_back( lines[ i ] );
    return result;
}

/*-------------------------------------------------------------------------*/

/** The first number in the output of a git command that prints one */
static bool
FirstNumber( const CORE::CString& output, UInt64& number )
{
    CORE::CString::StringVector lines = Lines( output );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        const CORE::CString line = CCiUtils::Trimmed( lines[ i ] );
        bool digits = !line.IsNULLOrEmpty();
        const char* c = line.C_String();
        for ( UInt32 n=0; '\0' != c[ n ] && digits; ++n )
            digits = c[ n ] >= '0' && c[ n ] <= '9';
        if ( digits )
        {
            number = CORE::StringToUInt64( line );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** The end of git output for an error message */
static CORE::CString
GitProblem( const CORE::CString& output )
{
    UInt32 totalLines = 0;
    return CCiUtils::Bound( CCiUtils::Trimmed( CCiUtils::TailLines( output, 12, 0, totalLines ) ), MaxErrorChars );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSnapshotBuilder::CSnapshotBuilder( void )
    : m_maxBytes( DefaultMaxBytes )
    , m_workDir()
    , m_commitName( "gucefMcpCi" )
    , m_commitEmail( "gucefmcpci@localhost" )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CSnapshotBuilder::SetMaxBytes( UInt64 maxBytes )
{GUCEF_TRACE;

    m_maxBytes = maxBytes;
}

/*-------------------------------------------------------------------------*/

UInt64
CSnapshotBuilder::GetMaxBytes( void ) const
{GUCEF_TRACE;

    return m_maxBytes;
}

/*-------------------------------------------------------------------------*/

void
CSnapshotBuilder::SetWorkDir( const CORE::CString& workDir )
{GUCEF_TRACE;

    m_workDir = workDir;
}

/*-------------------------------------------------------------------------*/

void
CSnapshotBuilder::SetCommitIdentity( const CORE::CString& name, const CORE::CString& email )
{GUCEF_TRACE;

    m_commitName = name;
    m_commitEmail = email;
}

/*-------------------------------------------------------------------------*/

bool
CSnapshotBuilder::IsSecretLookingPath( const CORE::CString& path )
{GUCEF_TRACE;

    const CORE::CString name = CORE::ExtractFilename( path.ReplaceChar( '\\', '/' ) ).Lowercase();
    if ( name.IsNULLOrEmpty() )
        return false;
    for ( size_t i=0; i<sizeof( SecretNamePatterns ) / sizeof( SecretNamePatterns[ 0 ] ); ++i )
        if ( WildcardMatch( SecretNamePatterns[ i ], name.C_String() ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSnapshotBuilder::MakeBranchName( const CORE::CString& machineName, const CORE::CString& commit )
{GUCEF_TRACE;

    CORE::CString machine = CCiUtils::SafeName( machineName, 40 );
    if ( machine.IsNULLOrEmpty() )
        machine = "machine";
    return CORE::CString( BranchPrefix ) + machine + "-" + CCiUtils::FormatCompactUtcNow() + "-" + commit.SubstrToIndex( 10, true );
}

/*-------------------------------------------------------------------------*/

bool
CSnapshotBuilder::GitTakesConfigFromEnvironment( CORE::CString& version )
{GUCEF_TRACE;

    CORE::CString output;
    if ( !CCiUtils::RunGit( CORE::CString::Empty, Args( "--version" ), CORE::CProcess::TStringMap(), 30000, output ) )
        return false;
    // "git version 2.53.0" or "git version 2.39.2.windows.1"
    version = CCiUtils::Trimmed( output );
    const Int32 at = version.HasSubstr( "version ", true );
    if ( at < 0 )
        return false;
    CORE::CString::StringVector parts = version.CutChars( (UInt32) at + 8, true ).ParseElements( '.', false );
    if ( parts.size() < 2 )
        return false;
    const UInt32 major = CORE::StringToUInt32( parts[ 0 ] );
    const UInt32 minor = CORE::StringToUInt32( parts[ 1 ] );
    return major > 2 || ( 2 == major && minor >= 31 );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSnapshotBuilder::MakeTempDir( void ) const
{GUCEF_TRACE;

    UInt32 number = 0;
    {
        MT::CScopeMutex lock( g_tempDirLock );
        number = ++g_tempDirCounter;
    }
    const CORE::CString base = m_workDir.IsNULLOrEmpty() ? CORE::CombinePath( CORE::TempDir(), "gucefMcpCi" ) : m_workDir;
    const CORE::CString dir = CORE::CombinePath( base, "snapshot-" + CORE::UInt32ToString( MT::GetProcessID() ) + "-" + CORE::UInt32ToString( number ) );
    if ( CORE::DirExists( dir ) )
        CORE::DeleteDir( dir, true );
    CORE::CreateDirs( dir );
    return dir;
}

/*-------------------------------------------------------------------------*/

void
CSnapshotBuilder::EstimateSendSize( const CORE::CString& topLevel, CCiSnapshot& snapshot ) const
{GUCEF_TRACE;

    // only meaningful against what the remotes are known to have; a repository without remote tracking branches
    // would count its whole history
    CORE::CString output;
    if ( !CCiUtils::RunGit( topLevel, Args( "for-each-ref", "--count=1", "--format=%(refname)", "refs/remotes" ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) ||
         CCiUtils::Trimmed( output ).IsNULLOrEmpty() )
        return;
    // --disk-usage needs git 2.38; an older git fails and there is no estimate
    UInt64 bytes = 0;
    CORE::CProcess::TStringVector args;
    args.push_back( "rev-list" );
    args.push_back( "--objects" );
    args.push_back( "--disk-usage" );
    args.push_back( snapshot.commit );
    args.push_back( "--not" );
    args.push_back( "--remotes" );
    if ( CCiUtils::RunGit( topLevel, args, CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) && FirstNumber( output, bytes ) )
    {
        snapshot.hasSendEstimate = true;
        snapshot.sendBytesEstimate = bytes;
    }
}

/*-------------------------------------------------------------------------*/

bool
CSnapshotBuilder::Build( const CORE::CString& repoDir      ,
                         bool includeUncommitted           ,
                         const CORE::CString& machineName  ,
                         CCiSnapshot& snapshot             ,
                         CORE::CString& error              ) const
{GUCEF_TRACE;

    snapshot = CCiSnapshot();
    if ( repoDir.IsNULLOrEmpty() || !CORE::DirExists( repoDir ) )
    {
        error = "repo_dir is not an existing directory: " + repoDir;
        return false;
    }

    CORE::CString output;
    if ( !CCiUtils::RunGit( repoDir, Args( "rev-parse", "--show-toplevel" ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
    {
        error = "Not a git working tree: " + repoDir + " (" + GitProblem( output ) + ")";
        return false;
    }
    CORE::CString::StringVector lines = Lines( output );
    const CORE::CString topLevel = lines.empty() ? CORE::CString::Empty : CCiUtils::Trimmed( lines[ lines.size()-1 ] );
    if ( topLevel.IsNULLOrEmpty() )
    {
        error = "The top level of the repository of " + repoDir + " is not known";
        return false;
    }
    snapshot.repoDir = topLevel;

    if ( !CCiUtils::RunGit( topLevel, Args( "rev-parse", "--verify", "HEAD^{commit}" ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) ||
         CCiUtils::FindObjectId( output ).IsNULLOrEmpty() )
    {
        error = "The repository has no commit yet; commit something first: " + topLevel;
        return false;
    }
    snapshot.baseCommit = CCiUtils::FindObjectId( output );

    UInt64 count = 0;
    if ( CCiUtils::RunGit( topLevel, Args( "rev-list", "--count", "HEAD", "--not", "--remotes" ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) &&
         FirstNumber( output, count ) )
        snapshot.unpublishedCommits = (UInt32) count;

    if ( includeUncommitted )
    {
        if ( !BuildWithWorkingTree( topLevel, snapshot, error ) )
            return false;
    }
    else
    {
        snapshot.commit = snapshot.baseCommit;
        if ( CCiUtils::RunGit( topLevel, Args( "ls-tree", "-r", "--name-only", "HEAD" ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
            snapshot.filesInSnapshot = (UInt32) Lines( output ).size();
    }

    EstimateSendSize( topLevel, snapshot );
    const UInt64 size = snapshot.hasSendEstimate && snapshot.sendBytesEstimate > snapshot.changedBytes ? snapshot.sendBytesEstimate : snapshot.changedBytes;
    if ( size > m_maxBytes )
    {
        error = "The snapshot is " + CCiUtils::FormatBytes( size ) + ", more than the limit of " + CCiUtils::FormatBytes( m_maxBytes ) +
                " (--max-snapshot-mb). Large files that are not meant for the CI belong in .gitignore.\n" + snapshot.Describe();
        return false;
    }
    snapshot.branchName = MakeBranchName( machineName, snapshot.commit );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSnapshotBuilder::BuildWithWorkingTree( const CORE::CString& topLevel ,
                                        CCiSnapshot& snapshot         ,
                                        CORE::CString& error          ) const
{GUCEF_TRACE;

    snapshot.includesUncommitted = true;
    const CORE::CString tempDir = MakeTempDir();
    const CORE::CString tempIndex = CORE::CombinePath( tempDir, "index" );
    CORE::CProcess::TStringMap env;
    env[ "GIT_INDEX_FILE" ] = tempIndex;
    CORE::CString output;
    bool ok = true;

    // the tree of HEAD in the temporary index; the copy of the index of the user keeps the file information, so that
    // git add does not read unchanged files again. Without it, or when that index is in the middle of a merge, a
    // plain read-tree does the same more slowly
    CORE::CString userIndex;
    if ( CCiUtils::RunGit( topLevel, Args( "rev-parse", "--git-path", "index" ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
    {
        CORE::CString::StringVector lines = Lines( output );
        if ( !lines.empty() )
        {
            userIndex = CCiUtils::Trimmed( lines[ lines.size()-1 ] );
            if ( !CCiUtils::IsAbsolute( userIndex ) )
                userIndex = CORE::CombinePath( topLevel, userIndex );
        }
    }
    bool haveTree = false;
    if ( !userIndex.IsNULLOrEmpty() && CORE::FileExists( userIndex ) && CORE::CopyFile( userIndex, tempIndex, true ) )
        haveTree = CCiUtils::RunGit( topLevel, Args( "read-tree", "-m", "HEAD" ), env, GitTimeoutInMs, output );
    if ( !haveTree )
    {
        CORE::DeleteFile( tempIndex );
        haveTree = CCiUtils::RunGit( topLevel, Args( "read-tree", "HEAD" ), env, GitTimeoutInMs, output );
    }
    if ( !haveTree )
    {
        error = "git read-tree HEAD failed: " + GitProblem( output );
        ok = false;
    }

    if ( ok && !CCiUtils::RunGit( topLevel, Args( "-c", "gc.auto=0", "add", "-A" ), env, GitTimeoutInMs, output ) )
    {
        error = "git add -A failed: " + GitProblem( output );
        ok = false;
    }

    // what differs from HEAD
    CORE::CString::StringVector added;
    if ( ok )
    {
        CORE::CProcess::TStringVector args;
        args.push_back( "-c" );
        args.push_back( "core.quotePath=false" );
        args.push_back( "diff" );
        args.push_back( "--cached" );
        args.push_back( "--name-status" );
        args.push_back( "--no-renames" );
        args.push_back( "HEAD" );
        if ( !CCiUtils::RunGit( topLevel, args, env, GitTimeoutInMs, output ) )
        {
            error = "git diff --cached failed: " + GitProblem( output );
            ok = false;
        }
        else
        {
            CORE::CString::StringVector lines = Lines( output );
            for ( size_t i=0; i<lines.size(); ++i )
            {
                const CORE::CString& line = lines[ i ];
                const char* c = line.C_String();
                if ( line.ByteSize() < 4 || '\t' != c[ 1 ] )
                    continue;           // not an entry, a warning of git
                const char status = c[ 0 ];
                const CORE::CString path = UnquoteGitPath( line.CutChars( 2, true ) );
                if ( 'A' == status )
                {
                    ++snapshot.addedFiles;
                    added.push_back( path );
                }
                else
                if ( 'D' == status )
                    ++snapshot.deletedFiles;
                else
                if ( 'M' == status || 'T' == status )
                    ++snapshot.modifiedFiles;
                else
                    continue;
                if ( 'D' != status )
                    snapshot.changedBytes += CORE::FileSize( CORE::CombinePath( topLevel, path ) );
                if ( snapshot.changedPaths.size() < MaxListedChanges )
                    snapshot.changedPaths.push_back( CORE::CString( status ) + " " + path );
                else
                    snapshot.changedPathsTruncated = true;
            }
        }
    }

    // the added files that the index of the user does not have either are untracked; those that look like
    // secrets are refused
    if ( ok && !added.empty() )
    {
        CORE::CString::StringSet untracked;
        CORE::CProcess::TStringVector args;
        args.push_back( "-c" );
        args.push_back( "core.quotePath=false" );
        args.push_back( "ls-files" );
        args.push_back( "--others" );
        args.push_back( "--exclude-standard" );
        if ( CCiUtils::RunGit( topLevel, args, CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
        {
            CORE::CString::StringVector lines = Lines( output );
            for ( size_t i=0; i<lines.size(); ++i )
                untracked.insert( UnquoteGitPath( lines[ i ] ) );
        }
        CORE::CString::StringVector refused;
        for ( size_t i=0; i<added.size(); ++i )
        {
            if ( untracked.find( added[ i ] ) == untracked.end() )
                continue;
            ++snapshot.untrackedFiles;
            if ( IsSecretLookingPath( added[ i ] ) )
                refused.push_back( added[ i ] );
        }
        if ( !refused.empty() )
        {
            CORE::CString list;
            for ( size_t i=0; i<refused.size() && i<MaxListedRefusals; ++i )
                list += "  " + refused[ i ] + "\n";
            if ( refused.size() > MaxListedRefusals )
                list += "  ...\n";
            error = "Refused: " + CORE::UInt32ToString( (UInt32) refused.size() ) + " untracked file(s) look like secrets and are not sent:\n" + list +
                    "Add them to .gitignore, or track them with git add if they really belong in the repository, then submit again.";
            ok = false;
        }
    }

    CORE::CString tree;
    if ( ok )
    {
        if ( !CCiUtils::RunGit( topLevel, Args( "write-tree" ), env, GitTimeoutInMs, output ) || ( tree = CCiUtils::FindObjectId( output ) ).IsNULLOrEmpty() )
        {
            error = "git write-tree failed: " + GitProblem( output );
            ok = false;
        }
    }
    if ( ok && CCiUtils::RunGit( topLevel, Args( "ls-files" ), env, GitTimeoutInMs, output ) )
        snapshot.filesInSnapshot = (UInt32) Lines( output ).size();

    // the commit; when nothing differs from HEAD, HEAD itself
    if ( ok )
    {
        CORE::CString headTree;
        if ( CCiUtils::RunGit( topLevel, Args( "rev-parse", "HEAD^{tree}" ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
            headTree = CCiUtils::FindObjectId( output );
        if ( headTree == tree )
        {
            snapshot.commit = snapshot.baseCommit;
        }
        else
        {
            CORE::CProcess::TStringVector args;
            args.push_back( "-c" );
            args.push_back( "user.name=" + m_commitName );
            args.push_back( "-c" );
            args.push_back( "user.email=" + m_commitEmail );
            args.push_back( "-c" );
            args.push_back( "gc.auto=0" );
            args.push_back( "-c" );
            args.push_back( "commit.gpgSign=false" );
            args.push_back( "commit-tree" );
            args.push_back( tree );
            args.push_back( "-p" );
            args.push_back( snapshot.baseCommit );
            args.push_back( "-m" );
            args.push_back( "CI scratch snapshot" );
            if ( !CCiUtils::RunGit( topLevel, args, env, GitTimeoutInMs, output ) || ( snapshot.commit = CCiUtils::FindObjectId( output ) ).IsNULLOrEmpty() )
            {
                error = "git commit-tree failed: " + GitProblem( output );
                ok = false;
            }
        }
    }

    CORE::DeleteDir( tempDir, true );
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CSnapshotBuilder::Push( const CCiSnapshot& snapshot                ,
                        const CORE::CString& remoteUrl             ,
                        const CORE::CString& authorizationHeader   ,
                        CORE::CString& error                       ) const
{GUCEF_TRACE;

    if ( snapshot.commit.IsNULLOrEmpty() || snapshot.branchName.IsNULLOrEmpty() || remoteUrl.IsNULLOrEmpty() )
    {
        error = "Nothing to push: the snapshot or the scratch repository is missing";
        return false;
    }

    // the credential reaches git through its environment only: never the URL, the arguments or the process list
    CORE::CProcess::TStringMap env;
    if ( !CredentialEnvironment( authorizationHeader, env, error ) )
        return false;

    CORE::CProcess::TStringVector args;
    args.push_back( "push" );
    args.push_back( "--no-verify" );
    args.push_back( "--recurse-submodules=no" );
    args.push_back( "--porcelain" );
    args.push_back( remoteUrl );
    args.push_back( snapshot.commit + ":refs/heads/" + snapshot.branchName );
    CORE::CString output;
    Int32 exitCode = 0;
    if ( !CCiUtils::RunGit( snapshot.repoDir, args, env, PushTimeoutInMs, output, &exitCode ) )
    {
        CORE::CString problem = GitProblem( output );
        if ( !authorizationHeader.IsNULLOrEmpty() )
            problem = problem.ReplaceSubstr( authorizationHeader, "***" );
        error = "Sending the snapshot failed (git push, exit " + CORE::Int32ToString( exitCode ) + "): " + problem;
        return false;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Pushed " << snapshot.commit << " as " << snapshot.branchName );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSnapshotBuilder::CredentialEnvironment( const CORE::CString& authorizationHeader, CORE::CProcess::TStringMap& env, CORE::CString& error )
{GUCEF_TRACE;

    if ( authorizationHeader.IsNULLOrEmpty() )
        return true;
    CORE::CString version;
    if ( !GitTakesConfigFromEnvironment( version ) )
    {
        error = "git 2.31 or later is needed to send the snapshot without putting the credential on the command line; this machine has: " +
                ( version.IsNULLOrEmpty() ? CORE::CString( "no git" ) : version );
        return false;
    }
    env[ "GIT_CONFIG_COUNT" ] = "2";
    env[ "GIT_CONFIG_KEY_0" ] = "http.extraHeader";
    env[ "GIT_CONFIG_VALUE_0" ] = "Authorization: " + authorizationHeader;
    env[ "GIT_CONFIG_KEY_1" ] = "credential.helper";
    env[ "GIT_CONFIG_VALUE_1" ] = "";
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSnapshotBuilder::Fetch( const CORE::CString& repoDir                 ,
                         const CORE::CString& remoteUrl               ,
                         const CORE::CString& authorizationHeader     ,
                         const CORE::CString& refspec                 ,
                         CORE::CString& error                         ) const
{GUCEF_TRACE;

    if ( repoDir.IsNULLOrEmpty() || remoteUrl.IsNULLOrEmpty() )
    {
        error = "Nothing to fetch: the repository or the scratch repository is missing";
        return false;
    }
    CORE::CProcess::TStringMap env;
    if ( !CredentialEnvironment( authorizationHeader, env, error ) )
        return false;

    CORE::CProcess::TStringVector args;
    args.push_back( "-c" );
    args.push_back( "gc.auto=0" );
    args.push_back( "fetch" );
    args.push_back( "--quiet" );
    args.push_back( "--prune" );
    args.push_back( "--no-tags" );
    args.push_back( "--recurse-submodules=no" );
    args.push_back( remoteUrl );
    args.push_back( refspec );
    CORE::CString output;
    Int32 exitCode = 0;
    if ( !CCiUtils::RunGit( repoDir, args, env, PushTimeoutInMs, output, &exitCode ) )
    {
        CORE::CString problem = GitProblem( output );
        if ( !authorizationHeader.IsNULLOrEmpty() )
            problem = problem.ReplaceSubstr( authorizationHeader, "***" );
        error = "Fetching from the scratch repository failed (git fetch, exit " + CORE::Int32ToString( exitCode ) + "): " + problem;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSnapshotBuilder::HasCommit( const CORE::CString& repoDir, const CORE::CString& commit )
{GUCEF_TRACE;

    if ( !CCiUtils::IsHex( commit, 40 ) )
        return false;
    CORE::CString output;
    return CCiUtils::RunGit( repoDir, Args( "cat-file", "-e", ( commit + "^{commit}" ).C_String() ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output );
}

/*-------------------------------------------------------------------------*/

bool
CSnapshotBuilder::CreateBundle( const CCiSnapshot& snapshot                     ,
                                const CORE::CString::StringVector& excludedCommits ,
                                const CORE::CString& bundlePath                 ,
                                CORE::CString& error                            ) const
{GUCEF_TRACE;

    if ( snapshot.commit.IsNULLOrEmpty() || snapshot.repoDir.IsNULLOrEmpty() )
    {
        error = "Nothing to bundle: the snapshot is missing";
        return false;
    }

    // the object store of the repository, which the temporary repository borrows
    CORE::CString output;
    CORE::CString objects;
    if ( CCiUtils::RunGit( snapshot.repoDir, Args( "rev-parse", "--git-path", "objects" ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
    {
        CORE::CString::StringVector lines = Lines( output );
        if ( !lines.empty() )
        {
            objects = CCiUtils::Trimmed( lines[ lines.size()-1 ] );
            if ( !CCiUtils::IsAbsolute( objects ) )
                objects = CORE::CombinePath( snapshot.repoDir, objects );
        }
    }
    if ( objects.IsNULLOrEmpty() || !CORE::DirExists( objects ) )
    {
        error = "The object store of the repository is not found: " + snapshot.repoDir;
        return false;
    }

    const CORE::CString tempDir = MakeTempDir();
    const CORE::CString tempRepo = CORE::CombinePath( tempDir, "bundle.git" );
    bool ok = CCiUtils::RunGit( CORE::CString::Empty, Args( "init", "-q", "--bare", tempRepo.C_String() ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output );
    if ( !ok )
        error = "git init of a temporary repository failed: " + GitProblem( output );
    if ( ok )
    {
        const CORE::CString alternates = CORE::CombinePath( tempRepo, "objects/info/alternates" );
        const CORE::CString line = objects.ReplaceChar( '\\', '/' ) + "\n";
        CORE::CreatePathDirectories( alternates );
        ok = CORE::WriteFileAtomically( alternates, line.C_String(), line.ByteSize() - 1 );
        if ( !ok )
            error = "The temporary repository for the bundle could not be made in " + tempDir;
    }
    if ( ok && !CCiUtils::RunGit( tempRepo, Args( "update-ref", BundleRef, snapshot.commit.C_String() ), CORE::CProcess::TStringMap(), GitTimeoutInMs, output ) )
    {
        error = "The snapshot commit is not in the repository: " + GitProblem( output );
        ok = false;
    }
    if ( ok )
    {
        CORE::CProcess::TStringVector args;
        args.push_back( "-c" );
        args.push_back( "gc.auto=0" );
        args.push_back( "bundle" );
        args.push_back( "create" );
        args.push_back( "--quiet" );
        args.push_back( bundlePath );
        args.push_back( BundleRef );
        for ( size_t i=0; i<excludedCommits.size(); ++i )
            if ( CCiUtils::IsHex( excludedCommits[ i ], 40 ) && excludedCommits[ i ] != snapshot.commit )
                args.push_back( "^" + excludedCommits[ i ] );
        if ( CORE::FileExists( bundlePath ) )
            CORE::DeleteFile( bundlePath );
        if ( !CCiUtils::RunGit( tempRepo, args, CORE::CProcess::TStringMap(), PushTimeoutInMs, output ) || !CORE::FileExists( bundlePath ) )
        {
            error = "git bundle create failed: " + GitProblem( output );
            ok = false;
        }
    }
    CORE::DeleteDir( tempDir, true );
    return ok;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
