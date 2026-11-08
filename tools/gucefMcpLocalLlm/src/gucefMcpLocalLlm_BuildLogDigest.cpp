/*
 *  gucefMcpLocalLlm: MCP server that delegates coding tasks to local LLMs
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

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "gucefMcpLocalLlm_BuildLogDigest.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpLocalLlm {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const size_t MaxMessageChars = 300;
static const size_t MaxListedNames = 15;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static std::string
Trim( const std::string& text )
{
    size_t start = 0;
    while ( start < text.size() && isspace( (unsigned char) text[ start ] ) )
        ++start;
    size_t end = text.size();
    while ( end > start && isspace( (unsigned char) text[ end-1 ] ) )
        --end;
    return text.substr( start, end - start );
}

/*-------------------------------------------------------------------------*/

/** Removes terminal colour codes: ESC [ ... letter */
static std::string
StripAnsi( const std::string& text )
{
    std::string result;
    result.reserve( text.size() );
    for ( size_t i=0; i<text.size(); ++i )
    {
        if ( '\x1b' == text[ i ] && i+1 < text.size() && '[' == text[ i+1 ] )
        {
            i += 2;
            while ( i < text.size() && !isalpha( (unsigned char) text[ i ] ) )
                ++i;
            continue;
        }
        if ( '\r' != text[ i ] )
            result += text[ i ];
    }
    return result;
}

/*-------------------------------------------------------------------------*/

static bool
IsDigits( const std::string& text )
{
    if ( text.empty() )
        return false;
    for ( size_t i=0; i<text.size(); ++i )
        if ( !isdigit( (unsigned char) text[ i ] ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
StartsWith( const std::string& text, const char* prefix )
{
    return 0 == text.compare( 0, strlen( prefix ), prefix );
}

/*-------------------------------------------------------------------------*/

/** file:line or file:line:col, the form GCC and Clang use */
static bool
IsFileLocation( const std::string& location )
{
    size_t lastColon = location.rfind( ':' );
    if ( std::string::npos == lastColon || !IsDigits( location.substr( lastColon+1 ) ) )
        return false;
    size_t previousColon = location.rfind( ':', lastColon-1 );
    if ( std::string::npos != previousColon && previousColon > 1 && IsDigits( location.substr( previousColon+1, lastColon-previousColon-1 ) ) )
        lastColon = previousColon;
    // a Windows drive letter is the only colon allowed in the file part
    const std::string file = location.substr( 0, lastColon );
    return !file.empty() && file.find( ' ' ) == std::string::npos;
}

/*-------------------------------------------------------------------------*/

static std::string
Shorten( const std::string& message )
{
    std::string text = Trim( message );
    if ( text.size() > MaxMessageChars )
        text = text.substr( 0, MaxMessageChars ) + "...";
    return text;
}

/*-------------------------------------------------------------------------*/

static std::string
ToString( UInt32 value )
{
    char buffer[ 16 ];
    snprintf( buffer, sizeof( buffer ), "%u", value );
    return buffer;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CBuildLogDigest::CBuildLogDigest( const CORE::CString& baseDir )
    : m_baseDirs()
    , m_basePrefixes()
    , m_errors()
    , m_warnings()
    , m_errorIndex()
    , m_warningIndex()
    , m_failedTargets()
    , m_failedTests()
    , m_testSummary()
    , m_errorCount( 0 )
    , m_warningCount( 0 )
    , m_inCMakeMessage( false )
    , m_cmakeIsError( false )
    , m_cmakeLocation()
    , m_cmakeText()
    , m_inFailedTests( false )
{
    std::string dir = baseDir.STL_String();
    for ( size_t i=0; i<dir.size(); ++i )
        if ( '\\' == dir[ i ] )
            dir[ i ] = '/';
    while ( dir.size() > 1 && '/' == dir[ dir.size()-1 ] )
        dir.erase( dir.size()-1 );

    // the base directory, then its parent and grandparent; not the root of a drive or the file system
    std::string prefix;
    for ( int level=0; level<3 && !dir.empty(); ++level )
    {
        const size_t slash = dir.rfind( '/' );
        if ( std::string::npos == slash || 0 == slash || ( 2 == slash && ':' == dir[ 1 ] ) )
            break;
        m_baseDirs.push_back( dir + '/' );
        m_basePrefixes.push_back( prefix );
        dir = dir.substr( 0, slash );
        prefix += "../";
    }
}

/*-------------------------------------------------------------------------*/

std::string
CBuildLogDigest::ShortenPath( const std::string& path ) const
{
    if ( m_baseDirs.empty() )
        return path;
    std::string normalized = path;
    for ( size_t i=0; i<normalized.size(); ++i )
        if ( '\\' == normalized[ i ] )
            normalized[ i ] = '/';
    for ( size_t i=0; i<m_baseDirs.size(); ++i )
    {
        const std::string& base = m_baseDirs[ i ];
        if ( normalized.size() > base.size() && 0 == normalized.compare( 0, base.size(), base ) )
            return m_basePrefixes[ i ] + normalized.substr( base.size() );
    }
    return path;
}

/*-------------------------------------------------------------------------*/

void
CBuildLogDigest::AddIssue( bool isError, const std::string& location, const std::string& message )
{
    const std::string shortLocation = ShortenPath( Trim( location ) );
    const std::string shortMessage = Shorten( message );
    if ( shortMessage.empty() )
        return;
    const std::string key = shortLocation + '\n' + shortMessage;

    TIssueVector& issues = isError ? m_errors : m_warnings;
    TIndexMap& index = isError ? m_errorIndex : m_warningIndex;
    ( isError ? m_errorCount : m_warningCount )++;

    TIndexMap::iterator found = index.find( key );
    if ( found != index.end() )
    {
        ++issues[ (*found).second ].count;
        return;
    }
    SIssue issue;
    issue.location = shortLocation;
    issue.message = shortMessage;
    issue.count = 1;
    index[ key ] = issues.size();
    issues.push_back( issue );
}

/*-------------------------------------------------------------------------*/

void
CBuildLogDigest::FlushCMakeMessage( void )
{
    if ( !m_inCMakeMessage )
        return;
    m_inCMakeMessage = false;
    AddIssue( m_cmakeIsError, m_cmakeLocation, m_cmakeText.empty() ? std::string( "CMake reported a problem" ) : m_cmakeText );
    m_cmakeLocation.clear();
    m_cmakeText.clear();
}

/*-------------------------------------------------------------------------*/

void
CBuildLogDigest::ParseLine( const std::string& rawLine, bool& consumed )
{
    consumed = true;
    const std::string line = StripAnsi( rawLine );
    const std::string trimmed = Trim( line );

    // the indented lines of a CMake message
    if ( m_inCMakeMessage )
    {
        if ( !line.empty() && ( ' ' == line[ 0 ] || '\t' == line[ 0 ] ) )
        {
            if ( !m_cmakeText.empty() )
                m_cmakeText += ' ';
            m_cmakeText += trimmed;
            return;
        }
        if ( trimmed.empty() )
            return;     // blank lines separate the parts of a message; the next line that is not indented ends it
        FlushCMakeMessage();
    }

    // the CTest list of failed tests ends at a blank line
    if ( m_inFailedTests )
    {
        if ( trimmed.empty() || StartsWith( trimmed, "Errors while running" ) )
        {
            m_inFailedTests = false;
            return;
        }
        if ( m_failedTests.size() < 200 )
            m_failedTests.push_back( trimmed );
        return;
    }

    if ( trimmed.empty() )
        return;

    // CMake
    if ( StartsWith( trimmed, "CMake Error" ) || StartsWith( trimmed, "CMake Warning" ) || StartsWith( trimmed, "CMake Deprecation Warning" ) )
    {
        m_inCMakeMessage = true;
        m_cmakeIsError = StartsWith( trimmed, "CMake Error" );
        m_cmakeLocation.clear();
        m_cmakeText.clear();
        const size_t at = trimmed.find( " at " );
        if ( std::string::npos != at )
        {
            std::string where = trimmed.substr( at + 4 );
            const size_t paren = where.find( " (" );
            if ( std::string::npos != paren )
                where = where.substr( 0, paren );
            if ( !where.empty() && ':' == where[ where.size()-1 ] )
                where.erase( where.size()-1 );
            m_cmakeLocation = where;
        }
        else
        {
            const size_t colon = trimmed.find( ": " );
            if ( std::string::npos != colon )
                m_cmakeText = trimmed.substr( colon + 2 );
        }
        return;
    }

    // CTest
    if ( StartsWith( trimmed, "The following tests FAILED:" ) )
    {
        m_inFailedTests = true;
        return;
    }
    if ( std::string::npos != trimmed.find( "tests passed," ) && std::string::npos != trimmed.find( "out of" ) )
    {
        m_testSummary = trimmed;
        return;
    }

    // GUCEF test framework, after the prefix of the log line
    size_t marker = trimmed.find( "Test END: " );
    if ( std::string::npos != marker && std::string::npos != trimmed.find( " - FAILED" ) )
    {
        if ( m_failedTests.size() < 200 )
            m_failedTests.push_back( trimmed.substr( marker + 10 ) );
        return;
    }
    const char* testMarkers[] = { "ASSERTION FAILED: ", "EXCEPTION: " };
    for ( int i=0; i<2; ++i )
    {
        marker = trimmed.find( testMarkers[ i ] );
        if ( std::string::npos == marker )
            continue;
        std::string text = trimmed.substr( marker );
        std::string location;
        const size_t at = text.rfind( " at " );
        if ( std::string::npos != at && IsFileLocation( text.substr( at + 4 ) ) )
        {
            location = text.substr( at + 4 );
            text = text.substr( 0, at );
        }
        AddIssue( true, location, text );
        return;
    }

    // ninja and make
    if ( StartsWith( trimmed, "FAILED: " ) )
    {
        if ( m_failedTargets.size() < 200 )
            m_failedTargets.push_back( Shorten( trimmed.substr( 8 ) ) );
        return;
    }
    marker = trimmed.find( "*** [" );
    if ( std::string::npos != marker && std::string::npos != trimmed.find( "] Error" ) )
    {
        const size_t end = trimmed.find( ']', marker );
        if ( m_failedTargets.size() < 200 )
            m_failedTargets.push_back( ShortenPath( trimmed.substr( marker + 5, end - marker - 5 ) ) );
        return;
    }

    // linkers
    marker = trimmed.find( "undefined reference to " );
    if ( std::string::npos != marker )
    {
        std::string location;
        const size_t objectCode = trimmed.find( ":(" );
        if ( std::string::npos != objectCode && objectCode < marker )
        {
            location = trimmed.substr( 0, objectCode );
            const size_t toolPrefix = location.rfind( ": " );
            if ( std::string::npos != toolPrefix )
                location = location.substr( toolPrefix + 2 );
        }
        AddIssue( true, location, trimmed.substr( marker ) );
        return;
    }
    if ( ( StartsWith( trimmed, "/usr/bin/ld: " ) || StartsWith( trimmed, "ld: " ) || StartsWith( trimmed, "ld.lld: " ) ) &&
         ( std::string::npos != trimmed.find( "cannot find" ) || std::string::npos != trimmed.find( "error" ) ) )
    {
        AddIssue( true, std::string(), trimmed );
        return;
    }

    // MSVC and MSBuild: file(line,col): error C1234: message [project]
    std::string msvcLine = trimmed;
    if ( !msvcLine.empty() && ']' == msvcLine[ msvcLine.size()-1 ] )
    {
        const size_t bracket = msvcLine.rfind( " [" );
        if ( std::string::npos != bracket && std::string::npos != msvcLine.find( "proj", bracket ) )
            msvcLine = msvcLine.substr( 0, bracket );
    }
    const char* msvcKinds[] = { "): fatal error ", "): error ", "): warning " };
    for ( int i=0; i<3; ++i )
    {
        marker = msvcLine.find( msvcKinds[ i ] );
        if ( std::string::npos == marker )
            continue;
        const size_t open = msvcLine.rfind( '(', marker );
        if ( std::string::npos == open )
            break;
        std::string position = msvcLine.substr( open + 1, marker - open - 1 );
        bool numeric = !position.empty();
        for ( size_t c=0; c<position.size(); ++c )
        {
            if ( ',' == position[ c ] )
                position[ c ] = ':';
            else
            if ( !isdigit( (unsigned char) position[ c ] ) )
                numeric = false;
        }
        if ( !numeric )
            break;
        // "error C2065: ..." under the heading Errors: the code stays, the kind goes
        const std::string kind = msvcKinds[ i ] + 3;
        std::string message = msvcLine.substr( marker + 3 + kind.size() );
        if ( 0 == i )
            message = "fatal " + message;
        AddIssue( 2 != i, msvcLine.substr( 0, open ) + ":" + position, message );
        return;
    }
    const char* msvcToolKinds[] = { " : fatal error ", " : error ", " : warning " };
    for ( int i=0; i<3; ++i )
    {
        marker = msvcLine.find( msvcToolKinds[ i ] );
        if ( std::string::npos == marker )
            continue;
        AddIssue( 2 != i, msvcLine.substr( 0, marker ), msvcLine.substr( marker + 3 ) );
        return;
    }

    // GCC and Clang: file:line:col: error: message
    const char* gccKinds[] = { ": fatal error: ", ": error: ", ": warning: " };
    for ( int i=0; i<3; ++i )
    {
        marker = trimmed.find( gccKinds[ i ] );
        if ( std::string::npos == marker )
            continue;
        const std::string location = trimmed.substr( 0, marker );
        // the kind is in the heading (Errors, Warnings); only "fatal" adds information
        std::string message = trimmed.substr( marker + strlen( gccKinds[ i ] ) );
        if ( 0 == i )
            message = "fatal: " + message;
        if ( IsFileLocation( location ) )
            AddIssue( 2 != i, location, message );
        else
            AddIssue( 2 != i, std::string(), trimmed );     // a tool without a location: "clang: error: linker command failed"
        return;
    }

    // exceptions of Python, Java, .NET and others: SomeError: message, some.package.SomeException: message
    const size_t exceptionColon = trimmed.find( ": " );
    if ( std::string::npos != exceptionColon && exceptionColon > 4 )
    {
        const std::string name = trimmed.substr( 0, exceptionColon );
        const bool isName = std::string::npos == name.find_first_not_of( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.$" );
        const bool endsAsException = ( name.size() >= 5 && 0 == name.compare( name.size()-5, 5, "Error" ) ) ||
                                     ( name.size() >= 9 && 0 == name.compare( name.size()-9, 9, "Exception" ) );
        if ( isName && endsAsException && isupper( (unsigned char) name[ name.rfind( '.' ) == std::string::npos ? 0 : name.rfind( '.' ) + 1 ] ) )
        {
            AddIssue( true, std::string(), trimmed );
            return;
        }
    }

    // other tools
    if ( StartsWith( trimmed, "error:" ) || StartsWith( trimmed, "ERROR:" ) || StartsWith( trimmed, "Error:" ) || StartsWith( trimmed, "fatal:" ) )
    {
        AddIssue( true, std::string(), trimmed );
        return;
    }
    if ( StartsWith( trimmed, "warning:" ) || StartsWith( trimmed, "WARNING:" ) )
    {
        AddIssue( false, std::string(), trimmed );
        return;
    }
    consumed = false;
}

/*-------------------------------------------------------------------------*/

void
CBuildLogDigest::Parse( const std::string& log )
{
    size_t start = 0;
    while ( start <= log.size() )
    {
        size_t end = log.find( '\n', start );
        if ( std::string::npos == end )
            end = log.size();
        bool consumed = false;
        ParseLine( log.substr( start, end - start ), consumed );
        start = end + 1;
    }
    FlushCMakeMessage();
}

/*-------------------------------------------------------------------------*/

const CBuildLogDigest::TIssueVector& CBuildLogDigest::GetErrors( void ) const { return m_errors; }
const CBuildLogDigest::TIssueVector& CBuildLogDigest::GetWarnings( void ) const { return m_warnings; }
const CBuildLogDigest::TStringVector& CBuildLogDigest::GetFailedTargets( void ) const { return m_failedTargets; }
const CBuildLogDigest::TStringVector& CBuildLogDigest::GetFailedTests( void ) const { return m_failedTests; }
const std::string& CBuildLogDigest::GetTestSummary( void ) const { return m_testSummary; }
UInt32 CBuildLogDigest::GetErrorCount( void ) const { return m_errorCount; }
UInt32 CBuildLogDigest::GetWarningCount( void ) const { return m_warningCount; }

/*-------------------------------------------------------------------------*/

bool
CBuildLogDigest::HasFailures( void ) const
{
    return !m_errors.empty() || !m_failedTargets.empty() || !m_failedTests.empty();
}

/*-------------------------------------------------------------------------*/

static void
AppendIssues( std::string& text, const char* title, const CBuildLogDigest::TIssueVector& issues, UInt32 total, UInt32 maxShown )
{
    if ( issues.empty() )
        return;
    text += std::string( title ) + ": " + ToString( total ) + " (" + ToString( (UInt32) issues.size() ) + " distinct)\n";
    const size_t shown = issues.size() < maxShown ? issues.size() : maxShown;
    for ( size_t i=0; i<shown; ++i )
    {
        const CBuildLogDigest::SIssue& issue = issues[ i ];
        text += "  ";
        if ( !issue.location.empty() )
            text += issue.location + ": ";
        text += issue.message;
        if ( issue.count > 1 )
            text += " [x" + ToString( issue.count ) + "]";
        text += "\n";
    }
    if ( issues.size() > shown )
        text += "  ... " + ToString( (UInt32) ( issues.size() - shown ) ) + " more distinct\n";
}

/*-------------------------------------------------------------------------*/

static void
AppendNames( std::string& text, const char* title, const CBuildLogDigest::TStringVector& names )
{
    if ( names.empty() )
        return;
    text += std::string( title ) + " (" + ToString( (UInt32) names.size() ) + "):\n";
    const size_t shown = names.size() < MaxListedNames ? names.size() : MaxListedNames;
    for ( size_t i=0; i<shown; ++i )
        text += "  " + names[ i ] + "\n";
    if ( names.size() > shown )
        text += "  ... " + ToString( (UInt32) ( names.size() - shown ) ) + " more\n";
}

/*-------------------------------------------------------------------------*/

std::string
CBuildLogDigest::Format( UInt32 maxErrors, UInt32 maxWarnings ) const
{
    std::string text;
    AppendIssues( text, "Errors", m_errors, m_errorCount, maxErrors );
    AppendNames( text, "Failed targets", m_failedTargets );
    AppendNames( text, "Failed tests", m_failedTests );
    if ( !m_testSummary.empty() )
        text += "Tests: " + m_testSummary + "\n";
    AppendIssues( text, "Warnings", m_warnings, m_warningCount, maxWarnings );
    return text;
}

/*-------------------------------------------------------------------------*/

std::string
CBuildLogDigest::Tail( const std::string& log, UInt32 maxLines, UInt32 maxChars )
{
    std::string clean = StripAnsi( log );
    size_t end = clean.size();
    while ( end > 0 && ( '\n' == clean[ end-1 ] || ' ' == clean[ end-1 ] ) )
        --end;
    size_t start = end;
    UInt32 lines = 0;
    while ( start > 0 && lines < maxLines && end - start < maxChars )
    {
        --start;
        if ( '\n' == clean[ start ] )
            ++lines;
    }
    if ( start > 0 )
        ++start;
    if ( end - start > maxChars )
        start = end - maxChars;
    return clean.substr( start, end - start );
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpLocalLlm */

/*-------------------------------------------------------------------------*/
