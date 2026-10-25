/*
 *  authpluginSTEAM: gucefAUTH plugin for logging in with the Steam client
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

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSYSTEMSTEAMHOSTVIEW_H
#include "authpluginSTEAM_CSystemSteamHostView.h"
#define GUCEF_AUTHPLUGIN_STEAM_CSYSTEMSTEAMHOSTVIEW_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_CSYSTEMSTEAMHOSTVIEW_H ? */

#include "authpluginSTEAM_CSteamLocalDetector.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace STEAM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CSteamLocalDetector::MaxFileBytes;
const UInt32 CSteamLocalDetector::MaxNameLength;
const UInt64 CSteamLocalDetector::SteamIdBase;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A folder and a name below it, joined with one separator; the separator of Windows paths is accepted by every call that follows */
static CORE::CString
Join( const CORE::CString& folder, const char* name )
{GUCEF_TRACE;

    size_t bytes = strlen( folder.C_String() );
    while ( bytes > 1 && ( '/' == folder.C_String()[ bytes - 1 ] || '\\' == folder.C_String()[ bytes - 1 ] ) )
        --bytes;
    return CORE::CString( folder.C_String(), (UInt32) bytes ) + "/" + name;
}

/*-------------------------------------------------------------------------*/

/** The text without the blanks, tabs and line ends at its start and its end */
static CORE::CString
TrimBlanks( const CORE::CString& text )
{GUCEF_TRACE;

    const char* chars = text.C_String();
    if ( GUCEF_NULL == chars )
        return CORE::CString::Empty;
    size_t begin = 0;
    size_t end = strlen( chars );
    while ( begin < end && ( ' ' == chars[ begin ] || '\t' == chars[ begin ] || '\r' == chars[ begin ] || '\n' == chars[ begin ] ) )
        ++begin;
    while ( end > begin && ( ' ' == chars[ end - 1 ] || '\t' == chars[ end - 1 ] || '\r' == chars[ end - 1 ] || '\n' == chars[ end - 1 ] ) )
        --end;
    return CORE::CString( chars + begin, (UInt32)( end - begin ) );
}

/*-------------------------------------------------------------------------*/

/** Digits only, at most 20 of them: the text of a 64 bit number */
static bool
ParseDecimal( const CORE::CString& text, UInt64& value )
{GUCEF_TRACE;

    const char* p = text.C_String();
    value = 0;
    if ( GUCEF_NULL == p || '\0' == *p )
        return false;
    UInt32 digits = 0;
    while ( '\0' != *p )
    {
        if ( *p < '0' || *p > '9' || ++digits > 20 )
            return false;
        const UInt64 digit = (UInt64)( *p - '0' );
        if ( value > ( 0xFFFFFFFFFFFFFFFFULL - digit ) / 10 )
            return false;
        value = value * 10 + digit;
        ++p;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
IsDigits( const CORE::CString& text )
{GUCEF_TRACE;

    UInt64 value = 0;
    return ParseDecimal( text, value );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSteamLocalUser::CSteamLocalUser( void )
    : steamId( 0 )
    , accountName()
    , personaName()
    , mostRecent( false )
    , timestamp( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CSteamLocalUser::Clear( void )
{GUCEF_TRACE;

    steamId = 0;
    accountName.Clear();
    personaName.Clear();
    mostRecent = false;
    timestamp = 0;
}

/*-------------------------------------------------------------------------*/

CSteamLocalInfo::CSteamLocalInfo( void )
    : installed( false )
    , running( false )
    , installPath()
    , hasUser( false )
    , user()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CSteamLocalInfo::Clear( void )
{GUCEF_TRACE;

    installed = false;
    running = false;
    installPath.Clear();
    hasUser = false;
    user.Clear();
}

/*-------------------------------------------------------------------------*/

CSteamLocalDetector::CSteamLocalDetector( const ISteamHostView* view )
    : m_view( view )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_view )
        m_view = &CSystemSteamHostView::Instance();
}

/*-------------------------------------------------------------------------*/

CSteamLocalDetector::~CSteamLocalDetector()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CSteamLocalDetector::SetHostView( const ISteamHostView* view )
{GUCEF_TRACE;

    m_view = GUCEF_NULL != view ? view : &CSystemSteamHostView::Instance();
}

/*-------------------------------------------------------------------------*/

const ISteamHostView&
CSteamLocalDetector::GetHostView( void ) const
{GUCEF_TRACE;

    return *m_view;
}

/*-------------------------------------------------------------------------*/

bool
CSteamLocalDetector::IsIndividualSteamId( UInt64 steamId )
{GUCEF_TRACE;

    return steamId >= SteamIdBase && steamId <= SteamIdBase + 0xFFFFFFFFULL;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSteamLocalDetector::MakeSafeName( const CORE::CString& name, UInt32 maxLength )
{GUCEF_TRACE;

    // bytes below 0x20 and 0x7F are controls; bytes of a multi byte character are all 0x80 or more and stay
    const char* p = name.C_String();
    if ( GUCEF_NULL == p )
        return CORE::CString::Empty;
    // at most 256 code points of at most 4 bytes: the name is made in a buffer of that size
    if ( maxLength > 256 )
        maxLength = 256;
    char buffer[ 256 * 4 + 1 ];
    const size_t inBytes = strlen( p );
    size_t out = 0;
    UInt32 codePoints = 0;
    for ( size_t i=0; i<inBytes && out < sizeof( buffer ) - 1; ++i )
    {
        const unsigned char c = (unsigned char) p[ i ];
        if ( c < 0x20 || 0x7F == c )
            continue;
        const bool continuation = 0x80 == ( c & 0xC0 );
        if ( !continuation )
        {
            if ( codePoints >= maxLength )
                break;
            ++codePoints;
        }
        buffer[ out++ ] = (char) c;
    }
    buffer[ out ] = '\0';
    CORE::CString result( buffer, (UInt32) out );
    return result.Trim( true ).Trim( false );
}

/*-------------------------------------------------------------------------*/

bool
CSteamLocalDetector::ParseLoginUsers( const CORE::CString& text, TSteamLocalUserVector& users )
{GUCEF_TRACE;

    users.clear();
    CSteamKeyValues values;
    const bool wellFormed = values.Parse( text );

    CORE::CString::StringVector path;
    path.push_back( "users" );
    const CSteamKeyValues::TIndex block = values.FindBlock( CSteamKeyValues::RootIndex, path );
    if ( CSteamKeyValues::NoIndex == block )
        return false;

    CSteamKeyValues::TIndexVector children;
    values.GetChildren( block, children );
    for ( size_t i=0; i<children.size(); ++i )
    {
        const CSteamKeyValues::TIndex entry = children[ i ];
        UInt64 steamId = 0;
        if ( !values.IsBlock( entry ) || !ParseDecimal( values.GetKey( entry ), steamId ) || !IsIndividualSteamId( steamId ) )
            continue;

        CSteamLocalUser user;
        user.steamId = steamId;
        user.accountName = MakeSafeName( values.GetChildValue( entry, "AccountName" ), MaxNameLength );
        user.personaName = MakeSafeName( values.GetChildValue( entry, "PersonaName" ), MaxNameLength );
        user.mostRecent = "1" == TrimBlanks( values.GetChildValue( entry, "MostRecent" ) );
        UInt64 timestamp = 0;
        if ( ParseDecimal( TrimBlanks( values.GetChildValue( entry, "Timestamp" ) ), timestamp ) )
            user.timestamp = timestamp;

        // the same id twice: the later entry wins
        bool replaced = false;
        for ( size_t u=0; u<users.size(); ++u )
        {
            if ( users[ u ].steamId == steamId )
            {
                users[ u ] = user;
                replaced = true;
                break;
            }
        }
        if ( !replaced )
            users.push_back( user );
    }
    return wellFormed;
}

/*-------------------------------------------------------------------------*/

bool
CSteamLocalDetector::ParseActiveProcess( const CORE::CString& text, UInt32& processId, UInt32& activeAccountId )
{GUCEF_TRACE;

    processId = 0;
    activeAccountId = 0;
    CSteamKeyValues values;
    values.Parse( text );

    CORE::CString::StringVector path;
    path.push_back( "Registry" );
    path.push_back( "HKCU" );
    path.push_back( "Software" );
    path.push_back( "Valve" );
    path.push_back( "Steam" );
    path.push_back( "ActiveProcess" );
    const CSteamKeyValues::TIndex block = values.FindBlock( CSteamKeyValues::RootIndex, path );
    if ( CSteamKeyValues::NoIndex == block )
        return false;

    UInt64 number = 0;
    if ( ParseDecimal( TrimBlanks( values.GetChildValue( block, "pid" ) ), number ) && number <= 0xFFFFFFFFULL )
        processId = (UInt32) number;
    if ( ParseDecimal( TrimBlanks( values.GetChildValue( block, "ActiveUser" ) ), number ) && number <= 0xFFFFFFFFULL )
        activeAccountId = (UInt32) number;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSteamLocalDetector::ChooseUser( const TSteamLocalUserVector& users, UInt32 activeAccountId, CSteamLocalUser& user )
{GUCEF_TRACE;

    user.Clear();
    if ( users.empty() )
        return false;

    if ( 0 != activeAccountId )
    {
        const UInt64 activeSteamId = SteamIdBase + activeAccountId;
        for ( size_t i=0; i<users.size(); ++i )
        {
            if ( users[ i ].steamId == activeSteamId )
            {
                user = users[ i ];
                return true;
            }
        }
    }

    // marked as the last one, the latest of those when the file marks several; then the latest of all
    bool found = false;
    for ( size_t pass=0; pass<2 && !found; ++pass )
    {
        for ( size_t i=0; i<users.size(); ++i )
        {
            if ( 0 == pass && !users[ i ].mostRecent )
                continue;
            if ( !found || users[ i ].timestamp > user.timestamp )
            {
                user = users[ i ];
                found = true;
            }
        }
    }
    return found;
}

/*-------------------------------------------------------------------------*/

void
CSteamLocalDetector::GetCandidateInstallFolders( CORE::CString::StringVector& folders ) const
{GUCEF_TRACE;

    folders.clear();
    switch ( m_view->GetPlatform() )
    {
        case STEAMHOST_LINUX:
        {
            const CORE::CString home = m_view->GetHomeDirectory();
            if ( !home.IsNULLOrEmpty() )
            {
                folders.push_back( Join( home, ".steam/steam" ) );
                folders.push_back( Join( home, ".local/share/Steam" ) );
                folders.push_back( Join( home, ".var/app/com.valvesoftware.Steam/.local/share/Steam" ) );
            }
            break;
        }
        case STEAMHOST_WINDOWS:
        {
            CORE::CString path;
            if ( m_view->ReadRegistryString( "Software\\Valve\\Steam", "SteamPath", path ) && !path.IsNULLOrEmpty() )
                folders.push_back( path );
            path = m_view->GetEnvironmentValue( "ProgramFiles(x86)" );
            if ( !path.IsNULLOrEmpty() )
                folders.push_back( Join( path, "Steam" ) );
            path = m_view->GetEnvironmentValue( "ProgramFiles" );
            if ( !path.IsNULLOrEmpty() )
                folders.push_back( Join( path, "Steam" ) );
            break;
        }
        case STEAMHOST_MACOS:
        {
            const CORE::CString home = m_view->GetHomeDirectory();
            if ( !home.IsNULLOrEmpty() )
                folders.push_back( Join( home, "Library/Application Support/Steam" ) );
            break;
        }
        default:
            break;
    }
}

/*-------------------------------------------------------------------------*/

bool
CSteamLocalDetector::LooksLikeSteam( const CORE::CString& folder ) const
{GUCEF_TRACE;

    // a folder that is there and has what the client puts in it, so that an empty or unrelated folder of that name is not taken for a client
    if ( !m_view->DirectoryExists( folder ) )
        return false;
    return m_view->DirectoryExists( Join( folder, "config" ) )     ||
           m_view->DirectoryExists( Join( folder, "steamapps" ) )  ||
           m_view->DirectoryExists( Join( folder, "ubuntu12_32" ) ) ||
           m_view->FileExists( Join( folder, "steam.sh" ) )        ||
           m_view->FileExists( Join( folder, "steam.exe" ) )       ||
           m_view->FileExists( Join( folder, "loginusers.vdf" ) );
}

/*-------------------------------------------------------------------------*/

bool
CSteamLocalDetector::FindInstallFolder( CORE::CString& folder, CORE::CString::StringVector& allFolders ) const
{GUCEF_TRACE;

    GetCandidateInstallFolders( allFolders );

    // the first folder that has the file with the users is the one the client uses; else the first that looks like a client
    CORE::CString firstFound;
    for ( size_t i=0; i<allFolders.size(); ++i )
    {
        if ( !LooksLikeSteam( allFolders[ i ] ) )
            continue;
        if ( firstFound.IsNULLOrEmpty() )
            firstFound = allFolders[ i ];
        if ( m_view->FileExists( Join( Join( allFolders[ i ], "config" ), "loginusers.vdf" ) ) )
        {
            folder = allFolders[ i ];
            return true;
        }
    }
    folder = firstFound;
    return !firstFound.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CSteamLocalDetector::IsClientRunning( const CORE::CString& installFolder, UInt32& activeAccountId ) const
{GUCEF_TRACE;

    activeAccountId = 0;
    const CORE::CString clientName = "steam";
    bool running = false;

    if ( STEAMHOST_WINDOWS == m_view->GetPlatform() )
    {
        UInt32 processId = 0;
        if ( m_view->ReadRegistryDword( "Software\\Valve\\Steam\\ActiveProcess", "pid", processId ) && m_view->IsProcessRunning( processId, clientName ) )
            running = true;
        if ( !running && m_view->ReadRegistryDword( "Software\\Valve\\Steam", "SteamPID", processId ) && m_view->IsProcessRunning( processId, clientName ) )
            running = true;
        UInt32 activeUser = 0;
        if ( running && m_view->ReadRegistryDword( "Software\\Valve\\Steam\\ActiveProcess", "ActiveUser", activeUser ) )
            activeAccountId = activeUser;
        return running;
    }

    // Linux and macOS: the files the client keeps its process id in
    const CORE::CString home = m_view->GetHomeDirectory();
    CORE::CString::StringVector pidFiles;
    CORE::CString::StringVector registryFiles;
    if ( STEAMHOST_LINUX == m_view->GetPlatform() && !home.IsNULLOrEmpty() )
    {
        pidFiles.push_back( Join( home, ".steam/steam.pid" ) );
        registryFiles.push_back( Join( home, ".steam/registry.vdf" ) );
    }
    registryFiles.push_back( Join( installFolder, "registry.vdf" ) );

    CORE::CString content;
    for ( size_t i=0; i<pidFiles.size() && !running; ++i )
    {
        UInt64 processId = 0;
        if ( m_view->ReadTextFile( pidFiles[ i ], 64, content ) && ParseDecimal( TrimBlanks( content ), processId ) && processId <= 0xFFFFFFFFULL )
            running = m_view->IsProcessRunning( (UInt32) processId, clientName );
    }
    for ( size_t i=0; i<registryFiles.size(); ++i )
    {
        UInt32 processId = 0, activeUser = 0;
        if ( !m_view->ReadTextFile( registryFiles[ i ], MaxFileBytes, content ) || !ParseActiveProcess( content, processId, activeUser ) )
            continue;
        if ( !running && 0 != processId )
            running = m_view->IsProcessRunning( processId, clientName );
        if ( running )
        {
            activeAccountId = activeUser;
            break;
        }
    }
    return running;
}

/*-------------------------------------------------------------------------*/

bool
CSteamLocalDetector::Detect( CSteamLocalInfo& info ) const
{GUCEF_TRACE;

    info.Clear();

    CORE::CString folder;
    CORE::CString::StringVector candidates;
    if ( !FindInstallFolder( folder, candidates ) )
        return false;
    info.installed = true;
    info.installPath = folder;

    UInt32 activeAccountId = 0;
    info.running = IsClientRunning( folder, activeAccountId );

    // the users of the client: the file of the folder in use
    CORE::CString content;
    TSteamLocalUserVector users;
    if ( m_view->ReadTextFile( Join( Join( folder, "config" ), "loginusers.vdf" ), MaxFileBytes, content ) )
        ParseLoginUsers( content, users );
    if ( ChooseUser( users, info.running ? activeAccountId : 0, info.user ) )
        info.hasUser = true;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STEAM */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
