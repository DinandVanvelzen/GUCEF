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

#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpLocalLlm_ProcessRun.h"
#include "gucefMcpCi_CCiUtils.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Days since 1970-01-01 of a date of the proleptic Gregorian calendar */
static Int64
DaysFromCivil( Int64 year, Int64 month, Int64 day )
{
    year -= month <= 2 ? 1 : 0;
    const Int64 era = ( year >= 0 ? year : year - 399 ) / 400;
    const Int64 yearOfEra = year - era * 400;
    const Int64 dayOfYear = ( 153 * ( month + ( month > 2 ? -3 : 9 ) ) + 2 ) / 5 + day - 1;
    const Int64 dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
    return era * 146097 + dayOfEra - 719468;
}

/*-------------------------------------------------------------------------*/

/** The date of a number of days since 1970-01-01 */
static void
CivilFromDays( Int64 days, Int64& year, Int64& month, Int64& day )
{
    days += 719468;
    const Int64 era = ( days >= 0 ? days : days - 146096 ) / 146097;
    const Int64 dayOfEra = days - era * 146097;
    const Int64 yearOfEra = ( dayOfEra - dayOfEra / 1460 + dayOfEra / 36524 - dayOfEra / 146096 ) / 365;
    const Int64 dayOfYear = dayOfEra - ( 365 * yearOfEra + yearOfEra / 4 - yearOfEra / 100 );
    const Int64 monthPart = ( 5 * dayOfYear + 2 ) / 153;
    day = dayOfYear - ( 153 * monthPart + 2 ) / 5 + 1;
    month = monthPart + ( monthPart < 10 ? 3 : -9 );
    year = yearOfEra + era * 400 + ( month <= 2 ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

/** Reads 'count' digits at 'text'; false when they are not all digits */
static bool
ReadDigits( const char* text, UInt32 count, Int64& value )
{
    value = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( text[ i ] < '0' || text[ i ] > '9' )
            return false;
        value = value * 10 + ( text[ i ] - '0' );
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CCiUtils::RunGit( const CORE::CString& repoDir                     ,
                  const CORE::CProcess::TStringVector& args        ,
                  const CORE::CProcess::TStringMap& environment    ,
                  UInt32 timeoutInMs                               ,
                  CORE::CString& output                            ,
                  Int32* exitCode                                  )
{GUCEF_TRACE;

    CORE::CProcess::TStringVector gitArgs;
    if ( !repoDir.IsNULLOrEmpty() )
    {
        gitArgs.push_back( "-C" );
        gitArgs.push_back( repoDir );
    }
    gitArgs.insert( gitArgs.end(), args.begin(), args.end() );

    CORE::CProcess::TStringMap env = environment;
    env[ "GIT_TERMINAL_PROMPT" ] = "0";
    env[ "GIT_OPTIONAL_LOCKS" ] = "0";

    GUCEFMcpLocalLlm::CProcessRun run;
    if ( !run.Start( "git", gitArgs, CORE::CString::Empty, env, timeoutInMs ) )
    {
        output = "git could not be started";
        if ( GUCEF_NULL != exitCode )
            *exitCode = -1;
        return false;
    }
    run.RunToEnd();
    output = CORE::CString( run.GetOutput() );
    if ( GUCEF_NULL != exitCode )
        *exitCode = run.GetExitCode();
    return !run.TimedOut() && 0 == run.GetExitCode();
}

/*-------------------------------------------------------------------------*/

bool
CCiUtils::IsHex( const CORE::CString& text, UInt32 minLength )
{GUCEF_TRACE;

    if ( text.ByteSize() - 1 < minLength || text.IsNULLOrEmpty() )
        return false;
    const char* c = text.C_String();
    for ( UInt32 i=0; '\0' != c[ i ]; ++i )
    {
        const char ch = c[ i ];
        if ( !( ( ch >= '0' && ch <= '9' ) || ( ch >= 'a' && ch <= 'f' ) || ( ch >= 'A' && ch <= 'F' ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiUtils::FindObjectId( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString found;
    CORE::CString::StringVector lines = text.ParseElements( '\n', false );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        const CORE::CString line = Trimmed( lines[ i ] );
        const UInt32 length = line.Length();
        if ( ( 40 == length || 64 == length ) && IsHex( line, 40 ) )
            found = line;
    }
    return found;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CCiUtils::GetStringArray( const CORE::CDataNode& node, const char* name )
{GUCEF_TRACE;

    CORE::CString::StringVector values;
    const CORE::CDataNode* array = node.FindChild( name );
    if ( GUCEF_NULL == array )
    {
        // a single value given as a string is a list of one
        const CORE::CString single = Trimmed( node.GetAttributeValue( name ).AsString() );
        if ( !single.IsNULLOrEmpty() )
            values.push_back( single );
        return values;
    }
    CORE::CDataNode::TVariantVector items = array->GetChildrenValues();
    for ( size_t i=0; i<items.size(); ++i )
    {
        const CORE::CString value = Trimmed( items[ i ].AsString() );
        if ( !value.IsNULLOrEmpty() )
            values.push_back( value );
    }
    return values;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiUtils::Join( const CORE::CString::StringVector& elements, const char* separator )
{GUCEF_TRACE;

    CORE::CString text;
    for ( size_t i=0; i<elements.size(); ++i )
    {
        if ( i > 0 )
            text += separator;
        text += elements[ i ];
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiUtils::Trimmed( const CORE::CString& text )
{GUCEF_TRACE;

    // CString::Trim() leaves line ends: every white space character goes here
    const char* c = text.C_String();
    UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    UInt32 start = 0;
    while ( start < size && ( ' ' == c[ start ] || '\t' == c[ start ] || '\r' == c[ start ] || '\n' == c[ start ] ) )
        ++start;
    while ( size > start && ( ' ' == c[ size-1 ] || '\t' == c[ size-1 ] || '\r' == c[ size-1 ] || '\n' == c[ size-1 ] ) )
        --size;
    if ( 0 == start && size + 1 == text.ByteSize() )
        return text;
    return CORE::CString( c + start, size - start );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
PercentEncodeKeeping( const CORE::CString& text, char keep )
{
    static const char* hex = "0123456789ABCDEF";
    CORE::CString result;
    const char* c = text.C_String();
    const UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        const unsigned char ch = (unsigned char) c[ i ];
        if ( ( ch >= 'A' && ch <= 'Z' ) || ( ch >= 'a' && ch <= 'z' ) || ( ch >= '0' && ch <= '9' ) ||
             '-' == ch || '.' == ch || '_' == ch || '~' == ch || ( '\0' != keep && keep == (char) ch ) )
        {
            result += (char) ch;
        }
        else
        {
            result += '%';
            result += hex[ ch >> 4 ];
            result += hex[ ch & 0x0F ];
        }
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiUtils::PercentEncode( const CORE::CString& text )
{GUCEF_TRACE;

    return PercentEncodeKeeping( text, '\0' );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiUtils::PercentEncodePath( const CORE::CString& text )
{GUCEF_TRACE;

    return PercentEncodeKeeping( text, '/' );
}

/*-------------------------------------------------------------------------*/

bool
CCiUtils::ReadFile( const CORE::CString& path, CORE::CString& content )
{GUCEF_TRACE;

    if ( !CORE::FileExists( path ) )
        return false;
    return CORE::LoadTextFileAsString( path, content, false );
}

/*-------------------------------------------------------------------------*/

bool
CCiUtils::ParseJson( const CORE::CDynamicBuffer& buffer, CORE::CDataNode& tree )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) || codec.IsNULL() )
        return false;
    if ( 0 == buffer.GetDataSize() )
        return false;
    CORE::CDynamicBuffer copy( buffer );
    CORE::CDynamicBufferAccess access( copy );
    tree.Clear();
    return codec->BuildDataTree( &tree, &access );
}

/*-------------------------------------------------------------------------*/

bool
CCiUtils::ParseJson( const CORE::CString& text, CORE::CDataNode& tree )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( text.ByteSize() > 0 ? text.ByteSize() - 1 : 0, text.C_String() );
    return ParseJson( buffer, tree );
}

/*-------------------------------------------------------------------------*/

bool
CCiUtils::ToJson( const CORE::CDataNode& tree, CORE::CString& text )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) || codec.IsNULL() )
        return false;
    CORE::CDynamicBuffer buffer;
    CORE::CDynamicBufferAccess access( buffer );
    if ( !codec->StoreDataTree( &tree, &access ) )
        return false;
    text = CORE::CString( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCiUtils::ParseIsoToUnix( const CORE::CString& text, Int64& unixSeconds )
{GUCEF_TRACE;

    const CORE::CString trimmed = Trimmed( text );
    const char* c = trimmed.C_String();
    if ( trimmed.ByteSize() < 20 )
        return false;
    Int64 year = 0, month = 0, day = 0, hours = 0, minutes = 0, seconds = 0;
    if ( !ReadDigits( c, 4, year ) || '-' != c[ 4 ] || !ReadDigits( c + 5, 2, month ) || '-' != c[ 7 ] ||
         !ReadDigits( c + 8, 2, day ) || ( 'T' != c[ 10 ] && ' ' != c[ 10 ] && 't' != c[ 10 ] ) ||
         !ReadDigits( c + 11, 2, hours ) || ':' != c[ 13 ] || !ReadDigits( c + 14, 2, minutes ) || ':' != c[ 16 ] ||
         !ReadDigits( c + 17, 2, seconds ) )
        return false;
    if ( month < 1 || month > 12 || day < 1 || day > 31 )
        return false;

    // a fraction of a second, then Z or an offset
    const char* rest = c + 19;
    if ( '.' == *rest )
    {
        ++rest;
        while ( *rest >= '0' && *rest <= '9' )
            ++rest;
    }
    Int64 offsetSeconds = 0;
    if ( '+' == *rest || '-' == *rest )
    {
        Int64 offsetHours = 0, offsetMinutes = 0;
        if ( !ReadDigits( rest + 1, 2, offsetHours ) )
            return false;
        const char* minutesAt = ':' == rest[ 3 ] ? rest + 4 : rest + 3;
        if ( '\0' != *minutesAt && !ReadDigits( minutesAt, 2, offsetMinutes ) )
            return false;
        offsetSeconds = ( offsetHours * 3600 + offsetMinutes * 60 ) * ( '+' == *rest ? 1 : -1 );
    }
    unixSeconds = DaysFromCivil( year, month, day ) * 86400 + hours * 3600 + minutes * 60 + seconds - offsetSeconds;
    return true;
}

/*-------------------------------------------------------------------------*/

Int64
CCiUtils::NowUnix( void )
{GUCEF_TRACE;

    return (Int64) ::time( GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiUtils::FormatCompactUtcNow( void )
{GUCEF_TRACE;

    const Int64 now = NowUnix();
    Int64 days = now / 86400;
    Int64 secondsOfDay = now % 86400;
    if ( secondsOfDay < 0 )
    {
        secondsOfDay += 86400;
        --days;
    }
    Int64 year = 0, month = 0, day = 0;
    CivilFromDays( days, year, month, day );
    char buffer[ 32 ];
    ::sprintf( buffer, "%04d%02d%02dT%02d%02d%02dZ", (int) year, (int) month, (int) day,
               (int) ( secondsOfDay / 3600 ), (int) ( ( secondsOfDay / 60 ) % 60 ), (int) ( secondsOfDay % 60 ) );
    return buffer;
}

/*-------------------------------------------------------------------------*/

bool
CCiUtils::ParseCompactUtcToUnix( const CORE::CString& text, Int64& unixSeconds )
{GUCEF_TRACE;

    const char* c = text.C_String();
    if ( 16 != text.Length() || 'T' != c[ 8 ] || 'Z' != c[ 15 ] )
        return false;
    Int64 year = 0, month = 0, day = 0, hours = 0, minutes = 0, seconds = 0;
    if ( !ReadDigits( c, 4, year ) || !ReadDigits( c + 4, 2, month ) || !ReadDigits( c + 6, 2, day ) ||
         !ReadDigits( c + 9, 2, hours ) || !ReadDigits( c + 11, 2, minutes ) || !ReadDigits( c + 13, 2, seconds ) )
        return false;
    if ( month < 1 || month > 12 || day < 1 || day > 31 )
        return false;
    unixSeconds = DaysFromCivil( year, month, day ) * 86400 + hours * 3600 + minutes * 60 + seconds;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiUtils::FormatDuration( UInt64 seconds )
{GUCEF_TRACE;

    char buffer[ 64 ];
    if ( seconds < 60 )
        ::sprintf( buffer, "%us", (unsigned int) seconds );
    else
    if ( seconds < 3600 )
        ::sprintf( buffer, "%um%02us", (unsigned int) ( seconds / 60 ), (unsigned int) ( seconds % 60 ) );
    else
        ::sprintf( buffer, "%uh%02um", (unsigned int) ( seconds / 3600 ), (unsigned int) ( ( seconds / 60 ) % 60 ) );
    return buffer;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiUtils::FormatBytes( UInt64 bytes )
{GUCEF_TRACE;

    char buffer[ 64 ];
    if ( bytes < 1024 )
        ::sprintf( buffer, "%u B", (unsigned int) bytes );
    else
    if ( bytes < 1024 * 1024 )
        ::sprintf( buffer, "%.1f KiB", (double) bytes / 1024.0 );
    else
    if ( bytes < (UInt64) 1024 * 1024 * 1024 )
        ::sprintf( buffer, "%.1f MiB", (double) bytes / ( 1024.0 * 1024.0 ) );
    else
        ::sprintf( buffer, "%.2f GiB", (double) bytes / ( 1024.0 * 1024.0 * 1024.0 ) );
    return buffer;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiUtils::Bound( const CORE::CString& text, UInt32 maxChars )
{GUCEF_TRACE;

    const UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    if ( size <= maxChars )
        return text;
    const char* c = text.C_String();
    UInt32 cut = maxChars;
    while ( cut > maxChars / 2 && '\n' != c[ cut - 1 ] )
        --cut;
    if ( cut <= maxChars / 2 )
        cut = maxChars;
    // never in the middle of a UTF-8 sequence
    while ( cut > 0 && 0x80 == ( ( (unsigned char) c[ cut ] ) & 0xC0 ) )
        --cut;
    return CORE::CString( c, cut ) + "\n[... cut at " + CORE::UInt32ToString( maxChars ) + " characters]\n";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiUtils::TailLines( const CORE::CString& text, UInt32 maxLines, UInt32 skipFromEnd, UInt32& totalLines )
{GUCEF_TRACE;

    const char* c = text.C_String();
    const UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;

    // the line starts, a last line without a line end included
    GUCEF::vector< UInt32 > starts;
    UInt32 start = 0;
    while ( start < size )
    {
        starts.push_back( start );
        const char* end = static_cast< const char* >( ::memchr( c + start, '\n', size - start ) );
        if ( GUCEF_NULL == end )
            break;
        start = (UInt32) ( end - c ) + 1;
    }
    totalLines = (UInt32) starts.size();
    if ( 0 == totalLines || skipFromEnd >= totalLines )
        return CORE::CString::Empty;

    const UInt32 lastLine = totalLines - skipFromEnd;           // one past the last line shown
    const UInt32 firstLine = lastLine > maxLines ? lastLine - maxLines : 0;
    if ( firstLine >= lastLine )
        return CORE::CString::Empty;
    const UInt32 from = starts[ firstLine ];
    const UInt32 to = lastLine < totalLines ? starts[ lastLine ] : size;
    return CORE::CString( c + from, to - from );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiUtils::SafeName( const CORE::CString& text, UInt32 maxLength )
{GUCEF_TRACE;

    CORE::CString result;
    const char* c = text.C_String();
    bool lastWasDash = true;
    for ( UInt32 i=0; '\0' != c[ i ] && result.Length() < maxLength; ++i )
    {
        char ch = c[ i ];
        if ( ch >= 'A' && ch <= 'Z' )
            ch = (char) ( ch - 'A' + 'a' );
        if ( ( ch >= 'a' && ch <= 'z' ) || ( ch >= '0' && ch <= '9' ) )
        {
            result += ch;
            lastWasDash = false;
        }
        else
        if ( !lastWasDash )
        {
            result += '-';
            lastWasDash = true;
        }
    }
    while ( result.EndsWith( "-" ) )
        result = result.CutChars( 1, false );
    return result;
}

/*-------------------------------------------------------------------------*/

UInt64
CCiUtils::MonotonicMs( void )
{GUCEF_TRACE;

    // divided first: the ticks times 1000 would overflow after months of running
    const UInt64 resolution = MT::PrecisionTimerResolution();
    const UInt64 ticks = MT::PrecisionTickCount();
    if ( 0 == resolution )
        return 0;
    return resolution >= 1000 ? ticks / ( resolution / 1000 ) : ticks * 1000 / resolution;
}

/*-------------------------------------------------------------------------*/

UInt64
CCiUtils::MilliSecsSince( UInt64 startTicks )
{GUCEF_TRACE;

    return GUCEFMcpLocalLlm::MilliSecsSince( startTicks );
}

/*-------------------------------------------------------------------------*/

bool
CCiUtils::IsAbsolute( const CORE::CString& path )
{GUCEF_TRACE;

    const char* c = path.C_String();
    if ( path.IsNULLOrEmpty() )
        return false;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return '\\' == c[ 0 ] || '/' == c[ 0 ] || ( path.Length() > 2 && ':' == c[ 1 ] );
    #else
    return '/' == c[ 0 ];
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiUtils::MakeAbsolute( const CORE::CString& path )
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() || IsAbsolute( path ) )
        return path;
    return CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), path );
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
