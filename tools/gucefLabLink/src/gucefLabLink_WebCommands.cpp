/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
  #include <termios.h>
  #include <unistd.h>
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <windows.h>
  #include <io.h>
#endif

#include "gucefLabLink_Protocol.h"
#include "gucefLabLink_PasswordHash.h"
#include "gucefLabLink_WebUsers.h"
#include "gucefLabLink_WebCommands.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The longest line a password is read from */
static const UInt32 MaxPasswordLineBytes = 1024;

/*-------------------------------------------------------------------------*/

static bool
IsTerminal( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
    return 0 != ::isatty( 0 );
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return 0 != ::_isatty( ::_fileno( stdin ) );
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------*/

/** Reads a line of standard input; on a terminal without echo. The line break is not part of it */
static bool
ReadLine( const char* prompt, bool hidden, CORE::CString& text )
{GUCEF_TRACE;

    text.Clear();
    if ( GUCEF_NULL != prompt )
    {
        fprintf( stderr, "%s", prompt );
        fflush( stderr );
    }

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
    struct termios saved;
    const bool echoOff = hidden && 0 == ::tcgetattr( 0, &saved );
    if ( echoOff )
    {
        struct termios quiet = saved;
        quiet.c_lflag &= ~( (tcflag_t) ECHO );
        ::tcsetattr( 0, TCSANOW, &quiet );
    }
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    HANDLE input = ::GetStdHandle( STD_INPUT_HANDLE );
    DWORD savedMode = 0;
    const bool echoOff = hidden && INVALID_HANDLE_VALUE != input && 0 != ::GetConsoleMode( input, &savedMode );
    if ( echoOff )
        ::SetConsoleMode( input, savedMode & ~( (DWORD) ENABLE_ECHO_INPUT ) );
    #endif

    char line[ MaxPasswordLineBytes + 16 ];
    const bool read = GUCEF_NULL != fgets( line, (int) sizeof( line ), stdin );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
    if ( echoOff )
    {
        ::tcsetattr( 0, TCSANOW, &saved );
        fprintf( stderr, "\n" );
    }
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    if ( echoOff )
    {
        ::SetConsoleMode( input, savedMode );
        fprintf( stderr, "\n" );
    }
    #endif

    if ( read )
    {
        size_t length = ::strlen( line );
        while ( length > 0 && ( '\n' == line[ length-1 ] || '\r' == line[ length-1 ] ) )
            --length;
        text = CORE::CString( line, (UInt32) length );
    }
    // the copy on the stack goes, as far as this program can make it
    volatile char* bytes = line;
    for ( size_t i=0; i<sizeof( line ); ++i )
        bytes[ i ] = '\0';
    return read;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
DateOf( UInt64 unixSecs )
{GUCEF_TRACE;

    if ( 0 == unixSecs )
        return "-";
    const time_t seconds = (time_t) unixSecs;
    struct tm parts;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    if ( 0 != ::gmtime_s( &parts, &seconds ) )
        return "-";
    #else
    if ( GUCEF_NULL == ::gmtime_r( &seconds, &parts ) )
        return "-";
    #endif
    char text[ 32 ];
    ::strftime( text, sizeof( text ), "%Y-%m-%d %H:%M UTC", &parts );
    return text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CWebCommands::IsFlag( const CORE::CString& arg )
{GUCEF_TRACE;

    return arg == "--web" || arg == "--web-allow-plain-http" || arg == "--list-users";
}

/*-------------------------------------------------------------------------*/

bool
CWebCommands::ParseOption( const CORE::CString& arg, const CORE::CString& value, SWebSettings& settings, CORE::CString& command,
                           CORE::CString& commandArgument )
{GUCEF_TRACE;

    if ( arg == "--web" ) settings.enabled = true;
    else if ( arg == "--web-port" ) settings.port = (UInt16) CORE::StringToUInt32( value, DefaultWebPort );
    else if ( arg == "--web-listen" ) settings.listenAddress = value;
    else if ( arg == "--web-cert" ) settings.certificateFile = value;
    else if ( arg == "--web-key" ) settings.privateKeyFile = value;
    else if ( arg == "--web-allow-plain-http" ) settings.allowPlainHttp = true;
    else if ( arg == "--web-token-file" ) settings.tokenFile = value;
    else if ( arg == "--list-users" ) command = arg;
    else if ( arg == "--add-user" || arg == "--set-password" || arg == "--remove-user" || arg == "--disable-user" || arg == "--enable-user" )
    {
        command = arg;
        commandArgument = value;
    }
    else
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebCommands::IsCommand( const CORE::CString& command )
{GUCEF_TRACE;

    return command == "--list-users" || command == "--add-user" || command == "--set-password" || command == "--remove-user" ||
           command == "--disable-user" || command == "--enable-user";
}

/*-------------------------------------------------------------------------*/

const char*
CWebCommands::Usage( void )
{GUCEF_TRACE;

    return "Website (cluster): --web, --web-port N, --web-listen ADDR, --web-cert PATH, --web-key PATH, --web-allow-plain-http, --web-token-file PATH.\n"
           "Users of the website: --add-user NAME --role admin|viewer, --set-password NAME, --disable-user NAME, --enable-user NAME,\n"
           "  --remove-user NAME, --list-users. Passwords are asked on the terminal or read from standard input, never given as an option.\n";
}

/*-------------------------------------------------------------------------*/

bool
CWebCommands::ReadNewPassword( const CORE::CString& user, UInt32 minLength, CORE::CString& password, CORE::CString& problem )
{GUCEF_TRACE;

    password.Clear();
    const bool terminal = IsTerminal();
    const CORE::CString prompt = "Password for " + user + " (not shown): ";
    if ( !ReadLine( terminal ? prompt.C_String() : GUCEF_NULL, terminal, password ) || password.IsNULLOrEmpty() )
    {
        problem = "no password was given";
        CPasswordHash::Wipe( password );
        return false;
    }
    if ( password.Length() < minLength || password.Length() > MaxPasswordLineBytes )
    {
        problem = "a password has " + CORE::ToString( minLength ) + " to " + CORE::ToString( MaxPasswordLineBytes ) + " characters";
        CPasswordHash::Wipe( password );
        return false;
    }
    if ( terminal )
    {
        CORE::CString again;
        const bool same = ReadLine( "The same again: ", true, again ) && again == password;
        CPasswordHash::Wipe( again );
        if ( !same )
        {
            problem = "the two passwords differ";
            CPasswordHash::Wipe( password );
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

int
CWebCommands::Run( CStore& store, const SWebSettings& settings, const CORE::CString& command, const CORE::CString& name,
                   const CORE::CString& role )
{GUCEF_TRACE;

    CWebUserStore users( store );
    CORE::CString problem;
    if ( !users.Load( problem ) )
    {
        fprintf( stderr, "gucefLabLink: %s\n", problem.C_String() );
        return 1;
    }

    if ( command == "--list-users" )
    {
        GUCEF::vector< SWebUser > all;
        users.GetAll( all );
        if ( all.empty() )
            fprintf( stdout, "The website has no users. Add one with: gucefLabLink --add-user NAME --role admin\n" );
        for ( size_t i=0; i<all.size(); ++i )
        {
            SPasswordParams params;
            CORE::CDynamicBuffer salt, hash;
            const CORE::CString hashText = CPasswordHash::ParseRecord( all[ i ].record, params, salt, hash ) ? params.algorithm : CORE::CString( "no password" );
            fprintf( stdout, "%-24s %-7s %-9s %-22s %s\n", all[ i ].name.C_String(), all[ i ].role.C_String(), all[ i ].disabled ? "disabled" : "enabled",
                     DateOf( all[ i ].createdInUnixSecs ).C_String(), hashText.C_String() );
        }
        return 0;
    }

    if ( !CWebUserStore::IsValidName( name ) )
    {
        fprintf( stderr, "gucefLabLink: a user name is 1 to 64 letters, digits, '.', '_', '-' or '@'\n" );
        return 2;
    }
    SWebUser existing;
    const bool exists = users.Find( name, existing );

    if ( command == "--add-user" || command == "--set-password" )
    {
        if ( command == "--add-user" && !CWebUserStore::IsValidRole( role ) )
        {
            fprintf( stderr, "gucefLabLink: --add-user needs --role admin or --role viewer\n" );
            return 2;
        }
        if ( command == "--add-user" && exists )
        {
            fprintf( stderr, "gucefLabLink: there is a user %s already; change the password with --set-password %s\n", name.C_String(), name.C_String() );
            return 1;
        }
        if ( command == "--set-password" && !exists )
        {
            fprintf( stderr, "gucefLabLink: there is no user %s (see --list-users)\n", name.C_String() );
            return 1;
        }
        CORE::CString password;
        if ( !ReadNewPassword( name, settings.minPasswordLength, password, problem ) )
        {
            fprintf( stderr, "gucefLabLink: %s\n", problem.C_String() );
            return 1;
        }
        CORE::CString record;
        const bool hashed = CPasswordHash::MakeRecord( password, CPasswordHash::Effective( settings.passwordParams ), record );
        CPasswordHash::Wipe( password );
        if ( !hashed )
        {
            fprintf( stderr, "gucefLabLink: the password cannot be hashed (no randomness or no hash function)\n" );
            return 1;
        }
        if ( command == "--add-user" )
        {
            SWebUser user;
            user.name = name;
            user.role = role;
            user.createdInUnixSecs = NowInUnixSecs();
            user.record = record;
            if ( !users.Add( user, problem ) )
            {
                fprintf( stderr, "gucefLabLink: %s\n", problem.C_String() );
                return 1;
            }
            fprintf( stdout, "%s is a user of the website now (%s).\n", name.C_String(), role.C_String() );
            return 0;
        }
        if ( !users.SetRecord( name, record, problem ) )
        {
            fprintf( stderr, "gucefLabLink: %s\n", problem.C_String() );
            return 1;
        }
        fprintf( stdout, "The password of %s is changed; the sessions of %s on the website end.\n", name.C_String(), name.C_String() );
        return 0;
    }

    if ( !exists )
    {
        fprintf( stderr, "gucefLabLink: there is no user %s (see --list-users)\n", name.C_String() );
        return 1;
    }
    bool done = false;
    if ( command == "--remove-user" )
        done = users.Remove( name, problem );
    else
        done = users.SetDisabled( name, command == "--disable-user", problem );
    if ( !done )
    {
        fprintf( stderr, "gucefLabLink: %s\n", problem.C_String() );
        return 1;
    }
    if ( command == "--enable-user" )
        fprintf( stdout, "%s can log in to the website again.\n", name.C_String() );
    else
        fprintf( stdout, "%s is %s; the sessions of %s on the website end.\n", name.C_String(), command == "--remove-user" ? "removed" : "disabled", name.C_String() );
    return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
