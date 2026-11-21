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

#include <string.h>

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
  #include <sys/stat.h>
#endif

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefLabLink_Protocol.h"
#include "gucefLabLink_CiFiles.h"
#include "gucefLabLink_CiGatewayTokens.h"

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

/** A machine id as the access endpoint takes it: 16 to 64 characters without white space or tabs */
static bool
IsMachineId( const CORE::CString& machineId )
{
    if ( machineId.Length() < 16 || machineId.Length() > 64 )
        return false;
    // printable ASCII: the id is a field of tab separated lines
    for ( UInt32 i=0; i<machineId.Length(); ++i )
    {
        const UInt32 c = machineId[ i ];
        if ( c <= ' ' || c > '~' || '#' == c )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** A token as MakeGatewayToken() makes it: 64 hex digits */
static bool
IsGatewayToken( const CORE::CString& token )
{
    if ( 64 != token.Length() )
        return false;
    for ( UInt32 i=0; i<token.Length(); ++i )
    {
        const UInt32 c = token[ i ];
        if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCiGatewayTokens::CCiGatewayTokens( CStore& store                     ,
                                    const CORE::CString& hashFilePath ,
                                    const CORE::CString& gatewayUser  )
    : m_store( store )
    , m_hashFilePath( hashFilePath )
    , m_gatewayUser( StripText( gatewayUser ) )
    , m_reported()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiGatewayTokens::GetHashFilePath( void ) const
{GUCEF_TRACE;

    if ( !m_hashFilePath.IsNULLOrEmpty() )
        return m_hashFilePath;
    return CORE::CombinePath( CORE::CombinePath( m_store.GetStateDir(), "ci" ), "gateway-tokens" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiGatewayTokens::HashFileContent( const TTokenMap& tokens )
{GUCEF_TRACE;

    CORE::CString content = "# The tokens of the CI gateway, written by gucefLabLink on the hub: <SHA-256 of the token as hex><TAB><machine id>\n"
                            "# Replaced whenever a token is made or taken away; do not edit.\n";
    for ( TTokenMap::const_iterator i = tokens.begin(); i != tokens.end(); ++i )
        content += Sha256Hex( (*i).second ) + "\t" + (*i).first + "\n";
    return content;
}

/*-------------------------------------------------------------------------*/

bool
CCiGatewayTokens::Load( TTokenMap& tokens, CORE::CString& problem ) const
{GUCEF_TRACE;

    tokens.clear();
    CORE::CString content;
    bool missing = false;
    if ( !m_store.ReadCiGatewayTokens( content, missing, problem ) )
    {
        if ( missing )
        {
            problem.Clear();
            return true;
        }
        return false;
    }
    CORE::CString::StringVector lines = content.ParseElements( '\n', false );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        const CORE::CString line = StripText( lines[ i ] );
        if ( line.IsNULLOrEmpty() || '#' == line[ 0 ] )
            continue;
        CORE::CString::StringVector fields = line.ParseElements( '\t', true );
        if ( fields.size() < 2 )
            continue;
        const CORE::CString machineId = StripText( fields[ 0 ] );
        const CORE::CString token = StripText( fields[ 1 ] );
        if ( IsMachineId( machineId ) && IsGatewayToken( token ) )
            tokens[ machineId ] = token;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCiGatewayTokens::Save( const TTokenMap& tokens, CORE::CString& problem )
{GUCEF_TRACE;

    CORE::CString content;
    for ( TTokenMap::const_iterator i = tokens.begin(); i != tokens.end(); ++i )
        content += (*i).first + "\t" + (*i).second + "\n";
    if ( !m_store.WriteCiGatewayTokens( content, problem ) )
        return false;
    return WriteHashFile( tokens, problem );
}

/*-------------------------------------------------------------------------*/

bool
CCiGatewayTokens::WriteHashFile( const TTokenMap& tokens, CORE::CString& problem )
{GUCEF_TRACE;

    const CORE::CString path = GetHashFilePath();
    const CORE::CString content = HashFileContent( tokens );

    // no tokens and no file: nothing to say
    if ( tokens.empty() && !CORE::FileExists( path ) )
        return true;

    // the directory: made when it is part of the state directory, else the installer's
    CORE::CString directory = CORE::StripFilename( path );
    while ( directory.Length() > 1 && ( directory.EndsWith( "/" ) || directory.EndsWith( "\\" ) ) )
        directory = directory.CutChars( 1, false );
    if ( !CORE::DirExists( directory ) )
    {
        CORE::CString stateDir = m_store.GetStateDir();
        while ( stateDir.Length() > 1 && ( stateDir.EndsWith( "/" ) || stateDir.EndsWith( "\\" ) ) )
            stateDir = stateDir.CutChars( 1, false );
        const bool inStateDir = directory.StartsWith( stateDir + "/" ) || directory.StartsWith( stateDir + "\\" );
        if ( !inStateDir )
        {
            problem = "the directory " + directory + " of the CI gateway tokens does not exist: make it, so that the gateway user can enter it";
            ReportOnce( problem, true );
            return false;
        }
        if ( !CORE::CreateDirs( directory ) && !CORE::DirExists( directory ) )
        {
            problem = "cannot make " + directory;
            ReportOnce( problem, true );
            return false;
        }
        #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
        // the gateway user may enter it; the file itself is its own only
        ::chmod( directory.C_String(), 0755 );
        #endif
    }

    // mode 0600, given to the gateway user, as CORE::ReadOwnerOnlyFile() of the gateway wants it; only what differs is written
    bool changed = false;
    CORE::CString ownerProblem;
    const bool written = CCiFiles::WriteOwnerOnlyFile( path, content, m_gatewayUser, changed, ownerProblem, problem );
    if ( !ownerProblem.IsNULLOrEmpty() )
        ReportOnce( "the CI gateway tokens " + path + " stay with the account of the hub: " + ownerProblem, false );
    if ( !written )
        ReportOnce( problem, true );
    return written;
}

/*-------------------------------------------------------------------------*/

void
CCiGatewayTokens::ReportOnce( const CORE::CString& problem, bool error )
{GUCEF_TRACE;

    if ( m_reported.find( problem ) != m_reported.end() )
        return;
    if ( m_reported.size() >= 64 )
        m_reported.clear();
    m_reported.insert( problem );
    if ( error )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefLabLink: " << problem );
    }
    else
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: " << problem );
    }
}

/*-------------------------------------------------------------------------*/

bool
CCiGatewayTokens::GetOrMake( const CORE::CString& machineId, CORE::CString& token, bool& made, bool& full, CORE::CString& problem )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    made = false;
    full = false;
    token.Clear();
    if ( !IsMachineId( machineId ) )
    {
        problem = "not a machine id";
        return false;
    }
    TTokenMap tokens;
    if ( !Load( tokens, problem ) )
        return false;
    TTokenMap::iterator found = tokens.find( machineId );
    if ( found != tokens.end() )
    {
        token = (*found).second;
        // the hash file follows the tokens, also when it went missing
        return WriteHashFile( tokens, problem );
    }
    if ( tokens.size() >= MaxMachines )
    {
        full = true;
        problem = "there are gateway tokens for " + CORE::ToString( MaxMachines ) + " machines, the most there can be";
        return false;
    }
    const CORE::CString fresh = MakeGatewayToken();
    if ( !IsGatewayToken( fresh ) )
    {
        problem = "no randomness for a gateway token";
        return false;
    }
    tokens[ machineId ] = fresh;
    if ( !Save( tokens, problem ) )
        return false;
    token = fresh;
    made = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCiGatewayTokens::Remove( const CORE::CString::StringVector& machineIds, UInt32& removed, CORE::CString& problem )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    removed = 0;
    TTokenMap tokens;
    if ( !Load( tokens, problem ) )
        return false;
    for ( size_t i=0; i<machineIds.size(); ++i )
        removed += (UInt32) tokens.erase( machineIds[ i ] );
    if ( removed > 0 )
        return Save( tokens, problem );
    return WriteHashFile( tokens, problem );
}

/*-------------------------------------------------------------------------*/

bool
CCiGatewayTokens::Reconcile( const GUCEF::vector< SApproval >& approvals, UInt32& removed, CORE::CString& problem )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    removed = 0;
    TTokenMap tokens;
    if ( !Load( tokens, problem ) )
        return false;
    CORE::CString::StringSet approved;
    for ( size_t i=0; i<approvals.size(); ++i )
        if ( approvals[ i ].state == AccessApproved )
            approved.insert( approvals[ i ].machineId );
    TTokenMap kept;
    for ( TTokenMap::iterator i = tokens.begin(); i != tokens.end(); ++i )
    {
        if ( approved.find( (*i).first ) != approved.end() )
            kept[ (*i).first ] = (*i).second;
        else
            ++removed;
    }
    if ( removed > 0 )
        return Save( kept, problem );
    // nothing to take away: the hash file only when it is not what it should be
    return WriteHashFile( kept, problem );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
