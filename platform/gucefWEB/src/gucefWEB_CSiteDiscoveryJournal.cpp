/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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
#include <stdlib.h>
#include <string.h>

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

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_WEB_CURLPATHTEMPLATE_H
#include "gucefWEB_CUrlPathTemplate.h"
#define GUCEF_WEB_CURLPATHTEMPLATE_H
#endif /* GUCEF_WEB_CURLPATHTEMPLATE_H ? */

#include "gucefWEB_CSiteDiscoveryJournal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The redactor in journal mode for the life of a scope, see CSiteDiscoveryRedactor::SetJournalMode() */
class CJournalModeScope
{
    public:

    explicit CJournalModeScope( CSiteDiscoveryRedactor& redactor )
        : m_redactor( redactor )
        , m_before( redactor.IsJournalMode() )
    {GUCEF_TRACE;

        m_redactor.SetJournalMode( true );
    }

    ~CJournalModeScope()
    {GUCEF_TRACE;

        m_redactor.SetJournalMode( m_before );
    }

    private:

    CJournalModeScope( const CJournalModeScope& src );              /**< not implemented */
    CJournalModeScope& operator=( const CJournalModeScope& src );   /**< not implemented */

    CSiteDiscoveryRedactor& m_redactor;
    bool m_before;
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

static CORE::CString
Quote( const CORE::CString& text )
{
    return "\"" + CUrlPathTemplate::EscapeJson( text ) + "\"";
}

/*-------------------------------------------------------------------------*/

static CORE::CString
QuoteList( const CORE::CString::StringVector& list )
{
    CORE::CString result = "[";
    for ( size_t i=0; i<list.size(); ++i )
    {
        if ( i > 0 )
            result += ',';
        result += Quote( list[ i ] );
    }
    result += "]";
    return result;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
FormatSeconds( Float64 seconds )
{
    char text[ 64 ];
    snprintf( text, sizeof( text ), "%.6f", seconds );
    return text;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
GetText( const CORE::CDataNode& node, const char* name )
{
    return node.GetAttributeValueOrChildValueByName( name ).AsString( CORE::CString::Empty, false );
}

/*-------------------------------------------------------------------------*/

static void
GetList( const CORE::CDataNode& node, const char* name, CORE::CString::StringVector& list )
{
    list.clear();
    const CORE::CDataNode* child = node.FindChild( name );
    if ( GUCEF_NULL == child )
        return;
    for ( CORE::CDataNode::const_iterator i = child->ConstBegin(); i != child->ConstEnd(); ++i )
        list.push_back( (*i)->GetValue().AsString() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSiteDiscoveryJournal::CSiteDiscoveryJournal( void )
    : CIWebObservationSink()
    , m_ownRedactor()
    , m_redactor( &m_ownRedactor )
    , m_file()
    , m_path()
    , m_nextSequence( 1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSiteDiscoveryJournal::~CSiteDiscoveryJournal()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryJournal::SetRedactor( CSiteDiscoveryRedactor* redactor )
{GUCEF_TRACE;

    m_redactor = GUCEF_NULL != redactor ? redactor : &m_ownRedactor;
}

/*-------------------------------------------------------------------------*/

CSiteDiscoveryRedactor&
CSiteDiscoveryJournal::GetRedactor( void ) const
{GUCEF_TRACE;

    return *m_redactor;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryJournal::ReadWholeFile( const CORE::CString& path, CORE::CString& content )
{GUCEF_TRACE;

    content.Clear();
    if ( !CORE::FileExists( path ) )
        return false;
    CORE::CFileAccess file( path, "rb" );
    if ( !file.Opened() )
        return false;
    const UInt64 size = file.GetSize();
    if ( 0 == size )
        return true;
    GUCEF::vector< char > buffer( (size_t) size );
    UInt64 done = 0;
    while ( done < size )
    {
        const UInt32 chunk = (UInt32) ( size - done > 16 * 1024 * 1024 ? 16 * 1024 * 1024 : size - done );
        if ( file.Read( &buffer[ (size_t) done ], 1, chunk ) != chunk )
            return false;
        done += chunk;
    }
    content = CORE::CString( &buffer[ 0 ], (UInt32) size );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CSiteDiscoveryJournal::GetNextSequenceOf( const CORE::CString& path )
{GUCEF_TRACE;

    if ( !CORE::FileExists( path ) )
        return 1;

    // the end of the file only, the journal gets large; the sequence only goes up so the last lines have the highest
    CORE::CFileAccess file( path, "rb" );
    if ( !file.Opened() )
        return 1;
    const UInt64 size = file.GetSize();
    const UInt32 tail = (UInt32) ( size < 262144 ? size : 262144 );
    if ( tail == 0 )
        return 1;
    file.Seek( -(Int64) tail, SEEK_END );
    GUCEF::vector< char > buffer( tail + 1 );
    if ( file.Read( &buffer[ 0 ], 1, tail ) != tail )
        return 1;
    buffer[ tail ] = '\0';

    UInt32 highest = 0;
    const char* pos = &buffer[ 0 ];
    while ( GUCEF_NULL != ( pos = strstr( pos, "{\"seq\":" ) ) )
    {
        const UInt32 value = (UInt32) strtoul( pos + 7, GUCEF_NULL, 10 );
        if ( value > highest )
            highest = value;
        pos += 7;
    }
    return highest + 1;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryJournal::Open( const CORE::CString& path )
{GUCEF_TRACE;

    Close();
    m_nextSequence = GetNextSequenceOf( path );

    const CORE::CString dir = CORE::StripFilename( path );
    if ( !dir.IsNULLOrEmpty() && !CORE::DirExists( dir ) )
    {
        if ( !CORE::CreateDirs( dir ) )
            return false;
        CORE::SetOwnerOnlyAccess( dir );
    }
    if ( !CORE::FileExists( path ) )
    {
        CORE::CString problem;
        if ( !CORE::CreateOwnerOnlyFile( path, "", 0, problem ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SiteDiscoveryJournal: Unable to create the journal " << path << ": " << problem );
            return false;
        }
    }
    else
        CORE::SetOwnerOnlyAccess( path );

    if ( !m_file.Open( path, "ab" ) )
        return false;
    m_path = path;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryJournal::Close( void )
{GUCEF_TRACE;

    m_file.Close();
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryJournal::IsOpen( void ) const
{GUCEF_TRACE;

    return m_file.Opened();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSiteDiscoveryJournal::GetPath( void ) const
{GUCEF_TRACE;

    return m_path;
}

/*-------------------------------------------------------------------------*/

UInt32
CSiteDiscoveryJournal::GetNextSequence( void ) const
{GUCEF_TRACE;

    return m_nextSequence;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryJournal::ToJournalLine( const CObservedWebRequest& input ) const
{GUCEF_TRACE;

    // what was observed: no value rules and no registered value but the credentials, those are for the report
    const CObservedWebRequest& observation = input;
    CJournalModeScope journalMode( *m_redactor );

    const CSiteDiscoverySettings& settings = m_redactor->GetSettings();
    const bool neverRecorded = observation.neverRecorded || m_redactor->IsNeverRecordUrl( observation.url );
    const bool examples = settings.recordExampleValues && !neverRecorded;

    CORE::CString line = "{\"seq\":" + CORE::UInt32ToString( observation.sequence );
    line += ",\"ts\":" + FormatSeconds( observation.timestamp );
    line += ",\"method\":" + Quote( observation.method );
    line += ",\"url\":" + Quote( m_redactor->RedactUrl( observation.url, neverRecorded ) );
    line += ",\"type\":" + Quote( observation.resourceType );
    line += ",\"status\":" + CORE::Int32ToString( observation.status );
    line += ",\"mime\":" + Quote( neverRecorded ? CORE::CString::Empty : observation.mimeType );
    line += ",\"size\":" + CORE::UInt64ToString( observation.encodedDataLength );
    line += CORE::CString( ",\"failed\":" ) + ( observation.failed ? "true" : "false" );
    line += ",\"error\":" + Quote( observation.errorText );
    line += ",\"bodyFile\":" + Quote( neverRecorded ? CORE::CString::Empty : observation.bodyFile );
    line += ",\"postData\":" + Quote( examples ? m_redactor->RedactBody( observation.postData ) : CORE::CString::Empty );
    line += ",\"requestHeaders\":{";
    if ( !neverRecorded )
    {
        bool first = true;
        for ( CObservedWebRequest::TStringMap::const_iterator h = observation.requestHeaders.begin(); h != observation.requestHeaders.end(); ++h )
        {
            line += ( first ? "" : "," ) + Quote( h->first ) + ":" + Quote( m_redactor->RedactHeaderValue( h->first, h->second ) );
            first = false;
        }
    }
    line += "}";
    line += ",\"action\":" + CORE::UInt32ToString( observation.actionId );
    if ( !neverRecorded )
    {
        if ( !observation.requestContentType.IsNULLOrEmpty() )
            line += ",\"requestContentType\":" + Quote( observation.requestContentType );
        if ( !observation.requestBodyShape.IsEmpty() )
            line += ",\"requestBodyShape\":" + QuoteList( observation.requestBodyShape.lines );
        if ( settings.recordEnumQueryValues )
        {
            CObservedWebRequest::TStringPairVector enums = observation.queryEnumValues;
            CUrlPathTemplate parsed;
            if ( enums.empty() && CUrlPathTemplate::Parse( observation.url, parsed ) )
                m_redactor->ExtractEnumQueryValues( parsed, enums );
            if ( !enums.empty() )
            {
                CORE::CString::StringVector flat;
                for ( size_t e=0; e<enums.size(); ++e )
                {
                    flat.push_back( enums[ e ].first );
                    flat.push_back( enums[ e ].second );
                }
                line += ",\"queryValues\":" + QuoteList( flat );
            }
        }
        if ( !observation.cookiesSent.empty() )
            line += ",\"cookiesSent\":" + QuoteList( observation.cookiesSent );
        if ( !observation.cookiesSet.empty() )
            line += ",\"cookiesSet\":" + QuoteList( observation.cookiesSet );
        if ( !observation.redirectTarget.IsNULLOrEmpty() )
            line += ",\"redirectTo\":" + Quote( observation.redirectTarget );
        if ( !observation.redirectedFromHosts.empty() )
            line += ",\"redirectedFrom\":" + QuoteList( observation.redirectedFromHosts );
        if ( observation.hasResponseShape )
        {
            line += ",\"responseShape\":" + QuoteList( observation.responseShape.lines );
            line += ",\"responsePaging\":" + QuoteList( observation.responseShape.paginationHints );
            line += ",\"responseUrls\":" + QuoteList( observation.responseShape.urlFields );
            line += CORE::CString( ",\"responseShapeTruncated\":" ) + ( observation.responseShape.truncated ? "true" : "false" );
        }
        if ( observation.isDownload )
        {
            line += ",\"download\":{\"disposition\":" + Quote( observation.contentDisposition ) + ",\"fileName\":" + Quote( observation.downloadFileName ) +
                    ",\"contentType\":" + Quote( observation.mimeType ) + ",\"size\":" + CORE::UInt64ToString( observation.encodedDataLength ) + "}";
        }
    }
    line += CORE::CString( ",\"neverRecorded\":" ) + ( neverRecorded ? "true" : "false" ) + "}";

    // a cookie, credential header or password value that stands somewhere else in the line: the value is masked, the line stays
    return m_redactor->ScrubKnownSecrets( line );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoveryJournal::ToJournalLine( const CObservedUserAction& input ) const
{GUCEF_TRACE;

    const CObservedUserAction& action = input;
    CJournalModeScope journalMode( *m_redactor );
    return m_redactor->ScrubKnownSecrets( "{\"action\":" + CORE::UInt32ToString( action.id ) + ",\"ts\":" + FormatSeconds( action.timestamp ) + ",\"kind\":" + Quote( action.kind ) +
                                          ",\"label\":" + Quote( action.label ) + ",\"page\":" + Quote( action.page ) + "}" );
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryJournal::WriteLine( const CORE::CString& line )
{GUCEF_TRACE;

    if ( !m_file.Opened() )
        return false;
    const CORE::CString withEnd = line + "\n";
    const UInt32 bytes = withEnd.ByteSize() - 1;
    const bool ok = m_file.Write( withEnd.C_String(), bytes, 1 ) == 1;
    m_file.Flush();
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryJournal::Append( const CObservedWebRequest& observation )
{GUCEF_TRACE;

    if ( !m_file.Opened() )
        return false;
    // every line is written as observed; what may not reach a report is decided when the report is made
    const bool ok = WriteLine( ToJournalLine( observation ) );
    if ( observation.sequence >= m_nextSequence )
        m_nextSequence = observation.sequence + 1;
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryJournal::AppendAction( const CObservedUserAction& action )
{GUCEF_TRACE;

    if ( !m_file.Opened() )
        return false;
    return WriteLine( ToJournalLine( action ) );
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryJournal::OnWebRequestObserved( const CObservedWebRequest& observation )
{GUCEF_TRACE;

    Append( observation );
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoveryJournal::OnUserActionObserved( const CObservedUserAction& action )
{GUCEF_TRACE;

    AppendAction( action );
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryJournal::FromJournalDocument( const CORE::CDataNode& document, CObservedWebRequest& observation )
{GUCEF_TRACE;

    observation = CObservedWebRequest();
    observation.url = GetText( document, "url" );
    if ( observation.url.IsNULLOrEmpty() )
        return false;
    observation.sequence = (UInt32) document.GetAttributeValue( "seq" ).AsUInt64( 0 );
    observation.timestamp = document.GetAttributeValue( "ts" ).AsFloat64( 0.0 );
    observation.method = GetText( document, "method" );
    observation.resourceType = GetText( document, "type" );
    observation.status = (Int32) document.GetAttributeValue( "status" ).AsInt64( 0 );
    observation.mimeType = GetText( document, "mime" );
    observation.encodedDataLength = document.GetAttributeValue( "size" ).AsUInt64( 0 );
    observation.failed = document.GetAttributeValue( "failed" ).AsBool( false );
    observation.errorText = GetText( document, "error" );
    observation.bodyFile = GetText( document, "bodyFile" );
    observation.postData = GetText( document, "postData" );
    observation.actionId = (UInt32) document.GetAttributeValue( "action" ).AsUInt64( 0 );
    observation.requestContentType = GetText( document, "requestContentType" );
    observation.neverRecorded = document.GetAttributeValue( "neverRecorded" ).AsBool( false );
    observation.redactedClass = GetText( document, "redacted" );

    const CORE::CDataNode* headers = document.FindChild( "requestHeaders" );
    if ( GUCEF_NULL != headers )
    {
        for ( UInt32 i=0; i<headers->GetAttCount(); ++i )
        {
            const CORE::CDataNode::TKeyValuePair* att = headers->GetAttribute( i );
            if ( GUCEF_NULL != att )
                observation.requestHeaders[ att->first ] = att->second.AsString();
        }
        for ( CORE::CDataNode::const_iterator i = headers->ConstBegin(); i != headers->ConstEnd(); ++i )
            observation.requestHeaders[ (*i)->GetName() ] = (*i)->GetValue().AsString();
    }
    GetList( document, "requestBodyShape", observation.requestBodyShape.lines );
    GetList( document, "cookiesSent", observation.cookiesSent );
    GetList( document, "cookiesSet", observation.cookiesSet );
    GetList( document, "redirectedFrom", observation.redirectedFromHosts );
    observation.redirectTarget = GetText( document, "redirectTo" );
    CORE::CString::StringVector enumFlat;
    GetList( document, "queryValues", enumFlat );
    for ( size_t e=0; e+1<enumFlat.size(); e+=2 )
        observation.queryEnumValues.push_back( std::make_pair( enumFlat[ e ], enumFlat[ e + 1 ] ) );
    if ( GUCEF_NULL != document.FindChild( "responseShape" ) )
    {
        observation.hasResponseShape = true;
        GetList( document, "responseShape", observation.responseShape.lines );
        GetList( document, "responsePaging", observation.responseShape.paginationHints );
        GetList( document, "responseUrls", observation.responseShape.urlFields );
        observation.responseShape.truncated = document.GetAttributeValue( "responseShapeTruncated" ).AsBool( false );
    }
    const CORE::CDataNode* download = document.FindChild( "download" );
    if ( GUCEF_NULL != download )
    {
        observation.isDownload = true;
        observation.contentDisposition = GetText( *download, "disposition" );
        observation.downloadFileName = GetText( *download, "fileName" );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryJournal::ActionFromJournalDocument( const CORE::CDataNode& document, CObservedUserAction& action )
{GUCEF_TRACE;

    action = CObservedUserAction();
    if ( !document.IsAttribute( "action" ) || document.IsAttribute( "seq" ) )
        return false;
    action.id = (UInt32) document.GetAttributeValue( "action" ).AsUInt64( 0 );
    action.timestamp = document.GetAttributeValue( "ts" ).AsFloat64( 0.0 );
    action.kind = GetText( document, "kind" );
    action.label = GetText( document, "label" );
    action.page = GetText( document, "page" );
    return 0 != action.id;
}

/*-------------------------------------------------------------------------*/

UInt32
CSiteDiscoveryJournal::Load( const CORE::CString& path, CIWebObservationSink& sink, UInt32* nextSequence ) const
{GUCEF_TRACE;

    UInt32 loaded = 0;
    UInt32 maxSequence = 0;
    const CORE::CString dir = CORE::StripFilename( path );
    const CSiteDiscoverySettings& settings = m_redactor->GetSettings();

    CORE::CString text;
    if ( ReadWholeFile( path, text ) && !text.IsNULLOrEmpty() )
    {
        const char* t = text.C_String();
        const UInt32 len = text.ByteSize() - 1;
        UInt32 start = 0;
        while ( start < len )
        {
            UInt32 end = start;
            while ( end < len && t[ end ] != '\n' )
                ++end;
            UInt32 lineEnd = end;
            if ( lineEnd > start && t[ lineEnd-1 ] == '\r' )
                --lineEnd;
            const CORE::CString line( t + start, lineEnd - start );
            start = end + 1;
            if ( line.IsNULLOrEmpty() )
                continue;

            CORE::CDataNode document;
            if ( !COM::CJsonRpcEndpoint::ParseJsonText( line, document ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "SiteDiscoveryJournal: Skipping a journal line that cannot be read" );
                continue;
            }
            CObservedUserAction action;
            if ( ActionFromJournalDocument( document, action ) )
            {
                sink.OnUserActionObserved( action );
                continue;
            }
            CObservedWebRequest observation;
            if ( !FromJournalDocument( document, observation ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "SiteDiscoveryJournal: Skipping a journal line that is not a request" );
                continue;
            }

            // a saved response of a journal that kept no shapes
            if ( !observation.hasResponseShape && !observation.bodyFile.IsNULLOrEmpty() && sink.WantsResponseShape( observation ) )
            {
                CORE::CString body;
                CORE::CDataNode bodyDocument;
                if ( ReadWholeFile( CORE::CombinePath( dir, observation.bodyFile ), body ) && COM::CJsonRpcEndpoint::ParseJsonText( body, bodyDocument ) )
                {
                    CWebBodyShape described;
                    CWebBodyShape::Describe( bodyDocument, described, settings.maxShapeDepth, settings.maxShapeLines,
                                             settings.recordExampleValues, &m_redactor->GetNameRedactor() );
                    if ( sink.IsNewResponseShape( observation, described ) )
                    {
                        observation.responseShape = described;
                        observation.hasResponseShape = true;
                    }
                }
            }
            sink.OnWebRequestObserved( observation );
            ++loaded;
            if ( observation.sequence > maxSequence )
                maxSequence = observation.sequence;
        }
    }
    if ( GUCEF_NULL != nextSequence )
        *nextSequence = maxSequence + 1;
    return loaded;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoveryJournal::Scrub( const CORE::CString& path, SScrubResult& result, CORE::CString& problem ) const
{GUCEF_TRACE;

    result = SScrubResult();
    if ( m_file.Opened() && m_path == path )
    {
        problem = "The journal is open for appending";
        return false;
    }
    CORE::CString text;
    if ( !CORE::FileExists( path ) || !ReadWholeFile( path, text ) )
    {
        problem = "The journal " + path + " cannot be read";
        return false;
    }

    // the journal as it was, before anything is changed
    CORE::CString backup = path + ".bak";
    for ( UInt32 n=1; CORE::FileExists( backup ) && n<1000; ++n )
        backup = path + ".bak." + CORE::UInt32ToString( n );
    if ( CORE::FileExists( backup ) || !CORE::CopyFile( path, backup, false ) || !CORE::SetOwnerOnlyAccess( backup ) )
    {
        problem = "The backup " + backup + " of the journal could not be made";
        return false;
    }
    result.backupPath = backup;

    const CSiteDiscoverySettings& settings = m_redactor->GetSettings();
    CORE::CString out;
    const char* t = text.IsNULLOrEmpty() ? "" : text.C_String();
    const UInt32 len = text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
    UInt32 start = 0;
    while ( start < len )
    {
        UInt32 end = start;
        while ( end < len && t[ end ] != '\n' )
            ++end;
        UInt32 lineEnd = end;
        if ( lineEnd > start && t[ lineEnd-1 ] == '\r' )
            --lineEnd;
        const CORE::CString line( t + start, lineEnd - start );
        start = end + 1;
        if ( line.IsNULLOrEmpty() )
            continue;
        ++result.lines;

        CORE::CString replacement = line;
        bool keep = true;
        CORE::CDataNode document;
        if ( !COM::CJsonRpcEndpoint::ParseJsonText( line, document ) )
            ++result.unreadableLines;
        else
        {
            CObservedUserAction action;
            CObservedWebRequest observation;
            if ( ActionFromJournalDocument( document, action ) )
            {
                if ( !action.page.IsNULLOrEmpty() && m_redactor->IsNeverRecordUrl( action.page ) )
                {
                    action.label = "{redacted}";
                    action.page = CORE::CString::Empty;
                    replacement = ToJournalLine( action );
                    ++result.stubbedActions;
                }
                else
                if ( m_redactor->RedactAction( action ) )
                {
                    replacement = ToJournalLine( action );
                    ++result.maskedActions;
                }
            }
            else
            if ( FromJournalDocument( document, observation ) && observation.redactedClass.IsNULLOrEmpty() )
            {
                if ( !observation.neverRecorded && m_redactor->IsNeverRecordUrl( observation.url ) )
                {
                    if ( settings.noteNeverRecorded )
                    {
                        observation.neverRecorded = true;
                        replacement = ToJournalLine( observation );
                        ++result.stubbedRequests;
                    }
                    else
                    {
                        keep = false;
                        ++result.droppedRequests;
                    }
                }
                else
                if ( m_redactor->RedactObservation( observation ) )
                {
                    // the value rules of today: values of the user that the line still held
                    replacement = ToJournalLine( observation );
                    ++result.maskedRequests;
                }
            }
        }
        if ( keep )
            out += replacement + "\n";
    }

    if ( !CORE::WriteFileAtomically( path, out.IsNULLOrEmpty() ? "" : out.C_String(), out.IsNULLOrEmpty() ? 0 : out.ByteSize() - 1 ) )
    {
        problem = "The scrubbed journal could not be written; the journal is as it was, the backup is " + backup;
        return false;
    }
    CORE::SetOwnerOnlyAccess( path );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
