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

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CGatewayTokens.h"

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

const char* CGatewayTokens::AdminMachineId = "admin";

/** The tokens file is read again at least this often, whatever its time and size say */
static const UInt64 MaxStalenessInMs = 30000;

/** A token is longer than this only by mistake: such headers are refused without hashing */
static const UInt32 MaxTokenLength = 4096;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static int
HexValue( char c )
{
    if ( c >= '0' && c <= '9' )
        return c - '0';
    if ( c >= 'a' && c <= 'f' )
        return c - 'a' + 10;
    if ( c >= 'A' && c <= 'F' )
        return c - 'A' + 10;
    return -1;
}

/*-------------------------------------------------------------------------*/

/** The characters of an RFC 6750 token68 */
static bool
IsToken68( const char* text, UInt32 size )
{
    if ( 0 == size )
        return false;
    for ( UInt32 i=0; i<size; ++i )
    {
        const char c = text[ i ];
        const bool ok = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) ||
                        '-' == c || '.' == c || '_' == c || '~' == c || '+' == c || '/' == c || '=' == c;
        if ( !ok )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGatewayTokens::CEntry::CEntry( void )
    : machineId()
    , scopes()
{GUCEF_TRACE;

    ::memset( digest, 0, sizeof( digest ) );
}

/*-------------------------------------------------------------------------*/

CGatewayTokens::CGatewayTokens( void )
    : m_tokensFile()
    , m_entries()
    , m_fileTimeInMs( 0 )
    , m_fileSize( 0 )
    , m_readAtInMs( 0 )
    , m_checkedAtInMs( 0 )
    , m_recheckIntervalInMs( 1000 )
    , m_hasStaticToken( false )
    , m_loadFailed( false )
    , m_lock()
{GUCEF_TRACE;

    ::memset( m_staticDigest, 0, sizeof( m_staticDigest ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGatewayTokens::DigestHex( const CORE::CString& token )
{GUCEF_TRACE;

    UInt8 digest[ 32 ];
    CORE::CSha256::Hash( token.C_String(), token.ByteSize() > 0 ? token.ByteSize() - 1 : 0, digest );
    static const char* digits = "0123456789abcdef";
    char text[ 65 ];
    for ( UInt32 i=0; i<32; ++i )
    {
        text[ i*2 ] = digits[ digest[ i ] >> 4 ];
        text[ i*2+1 ] = digits[ digest[ i ] & 0x0F ];
    }
    text[ 64 ] = '\0';
    ::memset( digest, 0, sizeof( digest ) );
    return CORE::CString( text, 64 );
}

/*-------------------------------------------------------------------------*/

bool
CGatewayTokens::ParseHexDigest( const CORE::CString& hex, UInt8* digest )
{GUCEF_TRACE;

    if ( 64 != hex.Length() )
        return false;
    const char* c = hex.C_String();
    for ( UInt32 i=0; i<32; ++i )
    {
        const int high = HexValue( c[ i*2 ] );
        const int low = HexValue( c[ i*2+1 ] );
        if ( high < 0 || low < 0 )
            return false;
        digest[ i ] = (UInt8) ( ( high << 4 ) | low );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGatewayTokens::SetStaticToken( const CORE::CString& token )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_hasStaticToken = !token.IsNULLOrEmpty();
    if ( m_hasStaticToken )
        CORE::CSha256::Hash( token.C_String(), token.ByteSize() - 1, m_staticDigest );
    else
        ::memset( m_staticDigest, 0, sizeof( m_staticDigest ) );
}

/*-------------------------------------------------------------------------*/

bool
CGatewayTokens::HasStaticToken( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_hasStaticToken;
}

/*-------------------------------------------------------------------------*/

void
CGatewayTokens::SetRecheckInterval( UInt32 intervalInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_recheckIntervalInMs = intervalInMs;
}

/*-------------------------------------------------------------------------*/

bool
CGatewayTokens::SetTokensFile( const CORE::CString& path, CORE::CString& problem )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_tokensFile = path;
    m_entries.clear();
    m_checkedAtInMs = (UInt64) CCiUtils::NowUnix() * 1000;
    return path.IsNULLOrEmpty() || Load( problem );
}

/*-------------------------------------------------------------------------*/

bool
CGatewayTokens::Load( CORE::CString& problem )
{GUCEF_TRACE;

    const UInt64 timeInMs = CORE::GetFileModificationTime( m_tokensFile ).ToUnixEpochBasedTicksInMillisecs();
    const UInt64 size = CORE::FileSize( m_tokensFile );
    CORE::CDynamicBuffer content;
    CORE::CString readProblem;
    m_fileTimeInMs = timeInMs;
    m_fileSize = size;
    m_readAtInMs = (UInt64) CCiUtils::NowUnix() * 1000;
    if ( !CORE::ReadOwnerOnlyFile( m_tokensFile, content, readProblem ) )
    {
        // fail closed: nobody of the file is accepted while it cannot be read
        m_entries.clear();
        problem = "the tokens file \"" + m_tokensFile + "\" cannot be used: " + readProblem;
        if ( !m_loadFailed )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: Gateway: " << problem << "; no machine is accepted until it can be read" );
        m_loadFailed = true;
        return false;
    }

    TEntryVector entries;
    const CORE::CString text( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() );
    CORE::CString::StringVector lines = text.ReplaceSubstr( "\r", "" ).ParseElements( '\n', false );
    UInt32 skipped = 0;
    CORE::CString::StringVector unknownScopes;
    for ( size_t i=0; i<lines.size(); ++i )
    {
        const CORE::CString line = CCiUtils::Trimmed( lines[ i ] );
        if ( line.IsNULLOrEmpty() || line.StartsWith( "#" ) )
            continue;
        // <digest><TAB><machine id>; spaces are taken as the separator too
        const CORE::CString normalized = line.ReplaceChar( ' ', '\t' );
        const Int32 tab = normalized.HasChar( '\t', true );
        CEntry entry;
        if ( tab != 64 || !ParseHexDigest( normalized.SubstrToIndex( 64, true ), entry.digest ) )
        {
            ++skipped;
            continue;
        }
        // <machine id>[<TAB><scopes>]; without a TAB a space ends the machine id too
        CORE::CString rest = CCiUtils::Trimmed( line.CutChars( 65, true ) );
        CORE::CString scopeList;
        Int32 separator = rest.HasChar( '\t', true );
        if ( separator < 0 )
            separator = rest.HasChar( ' ', true );
        if ( separator > 0 )
        {
            scopeList = CCiUtils::Trimmed( rest.CutChars( (UInt32) separator + 1, true ) );
            rest = CCiUtils::Trimmed( rest.SubstrToIndex( (UInt32) separator, true ) );
        }
        entry.machineId = rest;
        if ( entry.machineId.IsNULLOrEmpty() )
        {
            ++skipped;
            continue;
        }
        CORE::CString unknown;
        entry.scopes = CCiCaller::ParseScopes( scopeList, unknown );
        if ( !unknown.IsNULLOrEmpty() )
            unknownScopes.push_back( unknown + " of machine " + entry.machineId );
        entries.push_back( entry );
    }
    bool changed = entries.size() != m_entries.size();
    for ( size_t i=0; i<entries.size() && !changed; ++i )
        changed = entries[ i ].machineId != m_entries[ i ].machineId || entries[ i ].scopes != m_entries[ i ].scopes;
    m_entries = entries;
    if ( skipped > 0 )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Gateway: " << skipped << " line(s) of the tokens file " << m_tokensFile << " are not <sha256 hex><TAB><machine id>[<TAB><scopes>] and are ignored" );
    if ( m_loadFailed )
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Gateway: The tokens file can be read again" );
    m_loadFailed = false;
    if ( changed )
    {
        // said once per change of the file, not at every read
        for ( size_t i=0; i<unknownScopes.size(); ++i )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Gateway: The tokens file names the unknown scope(s) " << unknownScopes[ i ] <<
                               "; they are ignored (known: build, deploy)" );
        UInt32 deployers = 0;
        for ( size_t i=0; i<m_entries.size(); ++i )
            if ( m_entries[ i ].scopes.find( CCiCaller::ScopeDeploy ) != m_entries[ i ].scopes.end() )
                ++deployers;
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Gateway: " << (UInt32) m_entries.size() << " machine(s) in the tokens file, " << deployers << " of them may deploy" );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGatewayTokens::ReloadIfChanged( UInt64 nowInMs )
{GUCEF_TRACE;

    if ( m_tokensFile.IsNULLOrEmpty() || ( nowInMs >= m_checkedAtInMs && nowInMs - m_checkedAtInMs < m_recheckIntervalInMs ) )
        return;
    m_checkedAtInMs = nowInMs;

    // the time of a file has whole seconds: a file read in the second it was written is read again once more
    const UInt64 timeInMs = CORE::GetFileModificationTime( m_tokensFile ).ToUnixEpochBasedTicksInMillisecs();
    const UInt64 size = CORE::FileSize( m_tokensFile );
    const bool recentWhenRead = m_readAtInMs < m_fileTimeInMs + 2000;
    const bool stale = nowInMs < m_readAtInMs || nowInMs - m_readAtInMs >= MaxStalenessInMs;
    if ( timeInMs == m_fileTimeInMs && size == m_fileSize && !recentWhenRead && !m_loadFailed && !stale )
        return;
    CORE::CString problem;
    Load( problem );
}

/*-------------------------------------------------------------------------*/

UInt32
CGatewayTokens::GetMachineCount( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    ReloadIfChanged( (UInt64) CCiUtils::NowUnix() * 1000 );
    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

bool
CGatewayTokens::Authenticate( const CORE::CString& authorizationHeader, CORE::CString& machineId )
{GUCEF_TRACE;

    CCiCaller caller;
    const bool accepted = Authenticate( authorizationHeader, caller );
    machineId = accepted ? caller.machineId : CORE::CString::Empty;
    return accepted;
}

/*-------------------------------------------------------------------------*/

bool
CGatewayTokens::Authenticate( const CORE::CString& authorizationHeader, CCiCaller& caller )
{GUCEF_TRACE;

    caller = CCiCaller();
    CORE::CString& machineId = caller.machineId;

    // "Bearer" SP token68, the scheme without regard to case
    const char* text = authorizationHeader.C_String();
    const UInt32 size = authorizationHeader.ByteSize() > 0 ? authorizationHeader.ByteSize() - 1 : 0;
    if ( size < 8 || size > MaxTokenLength || ' ' != text[ 6 ] || !authorizationHeader.SubstrToIndex( 6, true ).Equals( "bearer", false ) )
        return false;
    UInt32 start = 7;
    while ( start < size && ' ' == text[ start ] )
        ++start;
    if ( !IsToken68( text + start, size - start ) )
        return false;

    UInt8 given[ 32 ];
    CORE::CSha256::Hash( text + start, size - start, given );

    MT::CScopeMutex lock( m_lock );
    ReloadIfChanged( (UInt64) CCiUtils::NowUnix() * 1000 );

    // every digest is compared, whatever matches first
    bool accepted = false;
    const CEntry* match = GUCEF_NULL;
    for ( size_t i=0; i<m_entries.size(); ++i )
    {
        const bool equal = CORE::CSha256::ConstantTimeEquals( given, m_entries[ i ].digest, 32 );
        if ( equal && GUCEF_NULL == match )
            match = &m_entries[ i ];
        accepted = accepted || equal;
    }
    const bool isStatic = m_hasStaticToken && CORE::CSha256::ConstantTimeEquals( given, m_staticDigest, 32 );
    ::memset( given, 0, sizeof( given ) );
    if ( isStatic )
    {
        machineId = AdminMachineId;
        return true;
    }
    if ( accepted && GUCEF_NULL != match )
    {
        machineId = match->machineId;
        caller.scopes = match->scopes;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
