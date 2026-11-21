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

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_WEB_CURLPATHTEMPLATE_H
#include "gucefWEB_CUrlPathTemplate.h"
#define GUCEF_WEB_CURLPATHTEMPLATE_H
#endif /* GUCEF_WEB_CURLPATHTEMPLATE_H ? */

#include "gucefWEB_CBrowserDownloadCapture.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CBrowserDownloadCapture::DownloadStartedEvent = "GUCEF::WEB::CBrowserDownloadCapture::DownloadStartedEvent";
const CORE::CEvent CBrowserDownloadCapture::DownloadFinishedEvent = "GUCEF::WEB::CBrowserDownloadCapture::DownloadFinishedEvent";
const CORE::CEvent CBrowserDownloadCapture::WaitTimedOutEvent = "GUCEF::WEB::CBrowserDownloadCapture::WaitTimedOutEvent";
const CORE::CString CBrowserDownloadCapture::IncomingDirectoryName = "incoming";

/** Responses remembered for their content type */
static const size_t MaxSeenResponses = 64;

/*-------------------------------------------------------------------------*/

static UInt32
ByteLength( const CORE::CString& text )
{
    return text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
GetText( const CORE::CDataNode& node, const char* name )
{
    return node.GetAttributeValue( name ).AsString( CORE::CString::Empty, false );
}

/*-------------------------------------------------------------------------*/

void
CBrowserDownloadCapture::RegisterEvents( void )
{GUCEF_TRACE;

    DownloadStartedEvent.Initialize();
    DownloadFinishedEvent.Initialize();
    WaitTimedOutEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CBrowserDownloadCapture::CBrowserDownloadCapture( CChromeDevToolsClient& browserClient )
    : CORE::CObservingNotifier()
    , m_client( browserClient )
    , m_settings()
    , m_runDirectory()
    , m_incomingDirectory()
    , m_capturing( false )
    , m_downloads()
    , m_results()
    , m_seenResponses()
    , m_finishedCount( 0 )
    , m_waitState( WAIT_IDLE )
    , m_waitDeadline( 0 )
    , m_waitBaseline( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    TEventCallback callback( this, &CBrowserDownloadCapture::OnProtocolEvent );
    SubscribeTo( &m_client, CChromeDevToolsClient::ProtocolEvent, callback );
}

/*-------------------------------------------------------------------------*/

CBrowserDownloadCapture::~CBrowserDownloadCapture()
{GUCEF_TRACE;

    UnsubscribeFrom( &m_client );
}

/*-------------------------------------------------------------------------*/

void
CBrowserDownloadCapture::SetSettings( const CBrowserDownloadSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

const CBrowserDownloadSettings&
CBrowserDownloadCapture::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

UInt64
CBrowserDownloadCapture::NowInMs( void ) const
{GUCEF_TRACE;

    return (UInt64) ( MT::PrecisionTickCount() / ( MT::PrecisionTimerResolution() / 1000.0 ) );
}

/*-------------------------------------------------------------------------*/

bool
CBrowserDownloadCapture::Begin( const CORE::CString& runDirectory )
{GUCEF_TRACE;

    if ( runDirectory.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "BrowserDownloadCapture: A run folder is needed" );
        return false;
    }
    const CORE::CString incoming = CORE::CombinePath( runDirectory, IncomingDirectoryName );
    if ( ( !CORE::DirExists( runDirectory ) && !CORE::CreateDirs( runDirectory ) ) || !CORE::SetOwnerOnlyAccess( runDirectory ) ||
         ( !CORE::DirExists( incoming ) && !CORE::CreateDirs( incoming ) ) || !CORE::SetOwnerOnlyAccess( incoming ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "BrowserDownloadCapture: Unable to make the owner-only run folder " << runDirectory );
        return false;
    }

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "behavior", CORE::CVariant( CORE::CString( "allowAndName" ) ) );
    params.SetAttribute( "downloadPath", CORE::CVariant( incoming ) );
    params.SetAttribute( "eventsEnabled", true );
    CORE::CDataNode result;
    if ( !m_client.SendCommandOnSessionAndWait( CORE::CString::Empty, "Browser.setDownloadBehavior", &params, result ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "BrowserDownloadCapture: The browser would not save downloads into the run folder" );
        return false;
    }
    m_runDirectory = runDirectory;
    m_incomingDirectory = incoming;
    m_capturing = true;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BrowserDownloadCapture: Downloads go to " << runDirectory );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CBrowserDownloadCapture::CancelDownload( const CORE::CString& guid )
{GUCEF_TRACE;

    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "guid", CORE::CVariant( guid ) );
    UInt64 commandId = 0;
    m_client.SendCommandOnSession( CORE::CString::Empty, "Browser.cancelDownload", &params, commandId );
}

/*-------------------------------------------------------------------------*/

bool
CBrowserDownloadCapture::End( bool denyDownloads )
{GUCEF_TRACE;

    for ( TDownloadMap::iterator i = m_downloads.begin(); i != m_downloads.end(); ++i )
    {
        if ( !i->second.browserFinished && i->second.refuseReason.IsNULLOrEmpty() )
        {
            i->second.refuseReason = CBrowserDownloadResult::ReasonCanceled;
            CancelDownload( i->first );
        }
    }
    m_capturing = false;
    if ( !m_client.IsConnected() )
        return false;
    CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
    params.SetAttribute( "behavior", CORE::CVariant( CORE::CString( denyDownloads ? "deny" : "default" ) ) );
    CORE::CDataNode result;
    return m_client.SendCommandOnSessionAndWait( CORE::CString::Empty, "Browser.setDownloadBehavior", &params, result );
}

/*-------------------------------------------------------------------------*/

bool
CBrowserDownloadCapture::IsCapturing( void ) const
{GUCEF_TRACE;

    return m_capturing;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CBrowserDownloadCapture::GetRunDirectory( void ) const
{GUCEF_TRACE;

    return m_runDirectory;
}

/*-------------------------------------------------------------------------*/

bool
CBrowserDownloadCapture::WatchSession( const CORE::CString& sessionId )
{GUCEF_TRACE;

    // only the content types of responses are looked at. No buffer sizes are given: the session may be shared with a
    // recorder or a response tap, whose sizes must stay
    CORE::CDataNode result;
    return m_client.SendCommandOnSessionAndWait( sessionId, "Network.enable", GUCEF_NULL, result );
}

/*-------------------------------------------------------------------------*/

bool
CBrowserDownloadCapture::ExpectDownload( UInt32 timeoutInMs )
{GUCEF_TRACE;

    if ( !m_capturing )
        return false;
    m_waitState = WAIT_PENDING;
    m_waitBaseline = m_finishedCount;
    m_waitDeadline = NowInMs() + timeoutInMs;
    return true;
}

/*-------------------------------------------------------------------------*/

CBrowserDownloadCapture::EWaitState
CBrowserDownloadCapture::GetWaitState( void ) const
{GUCEF_TRACE;

    return m_waitState;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CBrowserDownloadCapture::MakeUrlTemplate( const CORE::CString& url )
{GUCEF_TRACE;

    const Int32 colon = url.HasChar( ':', true );
    const CORE::CString scheme = colon > 0 ? url.SubstrToIndex( (UInt32) colon, true ).Lowercase() : CORE::CString::Empty;
    CUrlPathTemplate parsed;
    if ( ( scheme == "http" || scheme == "https" ) && CUrlPathTemplate::Parse( url, parsed ) )
        return parsed.scheme + "://" + parsed.host + parsed.pathTemplate;
    return scheme.IsNULLOrEmpty() ? CORE::CString::Empty : scheme + ":";
}

/*-------------------------------------------------------------------------*/

void
CBrowserDownloadCapture::OnProtocolEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    const SChromeProtocolEvent& event = static_cast< TChromeProtocolEventData* >( eventData )->GetData();
    if ( !event.hasParams )
        return;
    if ( event.method == "Browser.downloadWillBegin" && event.sessionId.IsNULLOrEmpty() )
        HandleWillBegin( event );
    else
    if ( event.method == "Browser.downloadProgress" && event.sessionId.IsNULLOrEmpty() )
        HandleProgress( event );
    else
    if ( event.method == "Network.responseReceived" )
        HandleResponse( event );
}

/*-------------------------------------------------------------------------*/

void
CBrowserDownloadCapture::HandleResponse( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    // the content type of every response that could become a download: documents and other loads, attachments
    const CORE::CDataNode* response = event.params.FindChild( "response" );
    if ( GUCEF_NULL == response )
        return;
    const CORE::CString type = GetText( event.params, "type" );
    if ( type != "Document" && type != "Other" && type != "XHR" && type != "Fetch" )
        return;
    const CORE::CString url = GetText( *response, "url" );
    const CORE::CString mimeType = GetText( *response, "mimeType" ).Lowercase();
    if ( url.IsNULLOrEmpty() || mimeType.IsNULLOrEmpty() )
        return;
    if ( m_seenResponses.size() >= MaxSeenResponses )
        m_seenResponses.erase( m_seenResponses.begin() );
    m_seenResponses.push_back( TUrlMimePair( url, mimeType ) );
}

/*-------------------------------------------------------------------------*/

void
CBrowserDownloadCapture::HandleWillBegin( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    const CORE::CString guid = GetText( event.params, "guid" );
    if ( !m_capturing || guid.IsNULLOrEmpty() )
        return;

    SDownload& download = m_downloads[ guid ];
    download.result.guid = guid;
    download.result.suggestedFilename = GetText( event.params, "suggestedFilename" );
    download.result.frameId = GetText( event.params, "frameId" );
    const CORE::CString url = GetText( event.params, "url" );
    download.result.urlTemplate = MakeUrlTemplate( url );
    for ( size_t i=m_seenResponses.size(); i>0; --i )
    {
        if ( m_seenResponses[ i-1 ].first == url )
        {
            download.result.mimeType = m_seenResponses[ i-1 ].second;
            download.result.mimeTypeFromServer = true;
            break;
        }
    }
    download.stagedPath = CORE::CombinePath( m_incomingDirectory, guid );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BrowserDownloadCapture: A download begins from " << download.result.urlTemplate );

    TBrowserDownloadResultEventData eventData( download.result );
    NotifyObservers( DownloadStartedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CBrowserDownloadCapture::HandleProgress( const SChromeProtocolEvent& event )
{GUCEF_TRACE;

    TDownloadMap::iterator i = m_downloads.find( GetText( event.params, "guid" ) );
    if ( i == m_downloads.end() || i->second.browserFinished )
        return;
    SDownload& download = i->second;
    const UInt64 received = (UInt64) event.params.GetAttributeValue( "receivedBytes" ).AsFloat64( 0.0 );
    const UInt64 total = (UInt64) event.params.GetAttributeValue( "totalBytes" ).AsFloat64( 0.0 );
    download.result.bytes = received;
    const CORE::CString state = GetText( event.params, "state" );

    if ( state == "inProgress" )
    {
        if ( 0 != m_settings.maxBytes && ( received > m_settings.maxBytes || total > m_settings.maxBytes ) && download.refuseReason.IsNULLOrEmpty() )
        {
            download.refuseReason = CBrowserDownloadResult::ReasonTooLarge;
            CancelDownload( download.result.guid );
        }
        return;
    }
    if ( state == "completed" )
    {
        download.browserCompleted = true;
        const CORE::CString filePath = GetText( event.params, "filePath" );
        if ( !filePath.IsNULLOrEmpty() )
            download.stagedPath = filePath;
    }
    download.browserFinished = true;     // completed or canceled: the file work happens in Process()
}

/*-------------------------------------------------------------------------*/

CORE::CString
CBrowserDownloadCapture::GetExtension( const CORE::CString& fileName )
{GUCEF_TRACE;

    const Int32 dot = fileName.HasChar( '.', false );
    if ( dot < 0 )
        return CORE::CString::Empty;
    const CORE::CString extension = fileName.CutChars( (UInt32) dot + 1, true ).Lowercase();
    return extension.HasChar( '/' ) >= 0 || extension.HasChar( '\\' ) >= 0 ? CORE::CString::Empty : extension;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CBrowserDownloadCapture::SanitizeFileName( const CORE::CString& suggestedName, UInt32 maxLength )
{GUCEF_TRACE;

    // the name part only
    CORE::CString name = suggestedName;
    const Int32 slash = name.HasChar( '/', false );
    if ( slash >= 0 )
        name = name.CutChars( (UInt32) slash + 1, true );
    const Int32 backslash = name.HasChar( '\\', false );
    if ( backslash >= 0 )
        name = name.CutChars( (UInt32) backslash + 1, true );

    CORE::CString clean;
    const char* t = name.C_String();
    const UInt32 len = ByteLength( name );
    for ( UInt32 i=0; i<len; ++i )
    {
        const char c = t[ i ];
        const bool keep = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || c == '.' || c == '-' || c == '_';
        const char put = keep ? c : '_';
        const UInt32 cleanLength = ByteLength( clean );
        if ( put == '_' && cleanLength > 0 && clean.C_String()[ cleanLength - 1 ] == '_' )
            continue;
        clean += put;
    }

    // no hidden file, no name that ends in a dot or an underscore
    while ( !clean.IsNULLOrEmpty() && ( clean.C_String()[ 0 ] == '.' || clean.C_String()[ 0 ] == '_' ) )
        clean = clean.CutChars( 1, true );
    while ( !clean.IsNULLOrEmpty() && ( clean.C_String()[ ByteLength( clean ) - 1 ] == '.' || clean.C_String()[ ByteLength( clean ) - 1 ] == '_' ) )
        clean = clean.CutChars( 1, false );
    if ( clean.IsNULLOrEmpty() )
        clean = "download";

    // the device names of Windows are no file names there
    const Int32 firstDot = clean.HasChar( '.', true );
    const CORE::CString stemUpper = ( firstDot >= 0 ? clean.SubstrToIndex( (UInt32) firstDot, true ) : clean ).Uppercase();
    static const char* const reserved[] = { "CON", "PRN", "AUX", "NUL", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
                                            "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9", GUCEF_NULL };
    for ( UInt32 i=0; GUCEF_NULL != reserved[ i ]; ++i )
        if ( stemUpper == reserved[ i ] )
            clean = "file-" + clean;

    // shorten the stem, keep a short extension
    if ( maxLength < 8 )
        maxLength = 8;
    if ( ByteLength( clean ) > maxLength )
    {
        const CORE::CString extension = GetExtension( clean );
        const UInt32 extensionLength = ByteLength( extension );
        if ( extensionLength > 0 && extensionLength <= 10 && extensionLength + 2 < maxLength )
            clean = clean.SubstrToIndex( maxLength - extensionLength - 1, true ) + "." + extension;
        else
            clean = clean.SubstrToIndex( maxLength, true );
    }
    return clean;
}

/*-------------------------------------------------------------------------*/

static bool
StartsWithBytes( const UInt8* head, UInt32 headSize, const char* prefix, bool ignoreCase )
{
    const UInt32 len = (UInt32) strlen( prefix );
    if ( headSize < len )
        return false;
    for ( UInt32 i=0; i<len; ++i )
    {
        char c = (char) head[ i ];
        if ( ignoreCase && c >= 'A' && c <= 'Z' )
            c = (char) ( c - 'A' + 'a' );
        if ( c != prefix[ i ] )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CBrowserDownloadCapture::SniffMimeType( const CORE::CString& fileName, const void* headData, UInt32 headSize )
{GUCEF_TRACE;

    const UInt8* head = static_cast< const UInt8* >( headData );
    const CORE::CString extension = GetExtension( fileName );
    if ( GUCEF_NULL == head )
        headSize = 0;

    if ( StartsWithBytes( head, headSize, "%PDF-", false ) )
        return "application/pdf";
    if ( headSize >= 4 && head[ 0 ] == 'P' && head[ 1 ] == 'K' && head[ 2 ] == 3 && head[ 3 ] == 4 )
    {
        if ( extension == "xlsx" )
            return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
        if ( extension == "docx" )
            return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
        return "application/zip";
    }
    if ( headSize >= 8 && head[ 0 ] == 0xD0 && head[ 1 ] == 0xCF && head[ 2 ] == 0x11 && head[ 3 ] == 0xE0 )
        return extension == "xls" ? "application/vnd.ms-excel" : "application/x-ole-storage";

    // text: a byte order mark and white space are skipped
    UInt32 start = 0;
    if ( headSize >= 3 && head[ 0 ] == 0xEF && head[ 1 ] == 0xBB && head[ 2 ] == 0xBF )
        start = 3;
    while ( start < headSize && ( head[ start ] == ' ' || head[ start ] == '\t' || head[ start ] == '\r' || head[ start ] == '\n' ) )
        ++start;
    const UInt8* text = head + start;
    const UInt32 textSize = headSize - start;
    if ( StartsWithBytes( text, textSize, "<!doctype html", true ) || StartsWithBytes( text, textSize, "<html", true ) ||
         StartsWithBytes( text, textSize, "<head", true ) || StartsWithBytes( text, textSize, "<body", true ) ||
         StartsWithBytes( text, textSize, "<!--", true ) )
        return "text/html";
    if ( StartsWithBytes( text, textSize, "<?xml", true ) )
        return "application/xml";
    if ( textSize > 0 && ( text[ 0 ] == '{' || text[ 0 ] == '[' ) && extension != "csv" )
        return "application/json";

    bool printable = true;
    for ( UInt32 i=start; i<headSize && printable; ++i )
        printable = head[ i ] >= 32 || head[ i ] == '\t' || head[ i ] == '\r' || head[ i ] == '\n';
    if ( printable )
    {
        if ( extension == "csv" )
            return "text/csv";
        if ( extension == "tsv" )
            return "text/tab-separated-values";
        if ( extension == "json" )
            return "application/json";
        if ( extension == "ofx" || extension == "qfx" )
            return "application/x-ofx";
        return "text/plain";
    }
    return "application/octet-stream";
}

/*-------------------------------------------------------------------------*/

bool
CBrowserDownloadCapture::IsMimeTypeAllowed( const CORE::CString::StringVector& allowed, const CORE::CString& mimeType )
{GUCEF_TRACE;

    if ( allowed.empty() )
        return true;
    CORE::CString bare = mimeType;
    const Int32 semicolon = bare.HasChar( ';', true );
    if ( semicolon >= 0 )
        bare = bare.SubstrToIndex( (UInt32) semicolon, true );
    bare = bare.Trim( true ).Trim( false ).Lowercase();
    for ( size_t i=0; i<allowed.size(); ++i )
    {
        const CORE::CString pattern = allowed[ i ].Trim( true ).Trim( false ).Lowercase();
        if ( pattern == bare || CORE::CGlobPattern::IsMatch( pattern, bare, false ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CBrowserDownloadCapture::HashFile( const CORE::CString& path, CORE::CString& hexDigest )
{GUCEF_TRACE;

    CORE::CFileAccess file;
    if ( !file.Open( path, "rb" ) )
        return false;
    CORE::CSha256 sha;
    UInt8 buffer[ 16384 ];
    UInt32 read = 0;
    while ( 0 != ( read = file.Read( buffer, 1, sizeof( buffer ) ) ) )
        sha.Update( buffer, read );
    file.Close();
    UInt8 digest[ CORE::CSha256::DigestSize ];
    sha.Final( digest );
    static const char* const hexDigits = "0123456789abcdef";
    hexDigest.Clear();
    for ( UInt32 i=0; i<CORE::CSha256::DigestSize; ++i )
    {
        hexDigest += hexDigits[ digest[ i ] >> 4 ];
        hexDigest += hexDigits[ digest[ i ] & 15 ];
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CBrowserDownloadCapture::Finish( SDownload& download )
{GUCEF_TRACE;

    CBrowserDownloadResult& result = download.result;
    const CORE::CString staged = download.stagedPath;
    result.state = CBrowserDownloadResult::STATE_FAILED;

    if ( !download.refuseReason.IsNULLOrEmpty() )
    {
        result.state = download.refuseReason == CBrowserDownloadResult::ReasonCanceled ? CBrowserDownloadResult::STATE_CANCELED : CBrowserDownloadResult::STATE_REFUSED;
        result.failureReason = download.refuseReason;
    }
    else
    if ( !download.browserCompleted )
    {
        result.state = CBrowserDownloadResult::STATE_CANCELED;
        result.failureReason = CBrowserDownloadResult::ReasonCanceled;
    }
    else
    if ( !CORE::FileExists( staged ) )
        result.failureReason = CBrowserDownloadResult::ReasonFileMissing;
    else
    {
        result.bytes = CORE::FileSize( staged );
        const CORE::CString finalName = SanitizeFileName( result.suggestedFilename, m_settings.maxFileNameLength );
        const CORE::CString extension = GetExtension( finalName );

        UInt8 head[ 512 ];
        UInt32 headSize = 0;
        {
            CORE::CFileAccess file;
            if ( file.Open( staged, "rb" ) )
            {
                headSize = file.Read( head, 1, sizeof( head ) );
                file.Close();
            }
        }
        result.sniffedMimeType = SniffMimeType( finalName, head, headSize );
        if ( !result.mimeTypeFromServer )
            result.mimeType = result.sniffedMimeType;

        bool extensionOk = m_settings.allowedExtensions.empty();
        for ( size_t i=0; i<m_settings.allowedExtensions.size() && !extensionOk; ++i )
            extensionOk = m_settings.allowedExtensions[ i ].Lowercase() == extension;

        result.state = CBrowserDownloadResult::STATE_REFUSED;
        if ( 0 != m_settings.maxBytes && result.bytes > m_settings.maxBytes )
            result.failureReason = CBrowserDownloadResult::ReasonTooLarge;
        else
        if ( result.bytes < m_settings.minBytes )
            result.failureReason = CBrowserDownloadResult::ReasonTooSmall;
        else
        if ( !extensionOk )
            result.failureReason = CBrowserDownloadResult::ReasonExtensionNotAllowed;
        else
        if ( !m_settings.allowHtml && ( result.sniffedMimeType == "text/html" || IsMimeTypeAllowed( CORE::CString::StringVector( 1, "text/html" ), result.mimeType ) ) )
            result.failureReason = CBrowserDownloadResult::ReasonHtmlInsteadOfFile;
        else
        if ( !IsMimeTypeAllowed( m_settings.allowedMimeTypes, result.mimeType ) )
            result.failureReason = CBrowserDownloadResult::ReasonMimeTypeNotAllowed;
        else
        {
            // owner-only, hashed, then moved in one step under a name that is not taken
            CORE::SetOwnerOnlyAccess( staged );
            HashFile( staged, result.sha256 );
            CORE::CString target = CORE::CombinePath( m_runDirectory, finalName );
            const Int32 dot = finalName.HasChar( '.', false );
            const CORE::CString stem = dot > 0 ? finalName.SubstrToIndex( (UInt32) dot, true ) : finalName;
            const CORE::CString suffix = dot > 0 ? finalName.CutChars( (UInt32) dot, true ) : CORE::CString::Empty;
            for ( UInt32 n=2; CORE::FileExists( target ) && n<1000; ++n )
                target = CORE::CombinePath( m_runDirectory, stem + "-" + CORE::UInt32ToString( n ) + suffix );
            if ( !CORE::FileExists( target ) && CORE::MoveFileAtomically( staged, target, false ) )
            {
                result.state = CBrowserDownloadResult::STATE_COMPLETED;
                result.finalPath = target;
            }
            else
            {
                result.state = CBrowserDownloadResult::STATE_FAILED;
                result.failureReason = CBrowserDownloadResult::ReasonMoveFailed;
                result.sha256.Clear();
            }
        }
    }

    // nothing that did not pass stays behind
    if ( result.state != CBrowserDownloadResult::STATE_COMPLETED )
    {
        if ( !staged.IsNULLOrEmpty() && CORE::FileExists( staged ) )
            CORE::DeleteFile( staged );
        const CORE::CString partial = staged + ".crdownload";
        if ( !staged.IsNULLOrEmpty() && CORE::FileExists( partial ) )
            CORE::DeleteFile( partial );
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BrowserDownloadCapture: The download from " << result.urlTemplate << " is " <<
               CBrowserDownloadResult::StateToString( result.state ) << ( result.failureReason.IsNULLOrEmpty() ? CORE::CString::Empty : " (" + result.failureReason + ")" ) <<
               ", " << result.bytes << " bytes" );
}

/*-------------------------------------------------------------------------*/

UInt32
CBrowserDownloadCapture::Process( void )
{GUCEF_TRACE;

    UInt32 finished = 0;
    CORE::CString::StringVector done;
    for ( TDownloadMap::iterator i = m_downloads.begin(); i != m_downloads.end(); ++i )
        if ( i->second.browserFinished )
            done.push_back( i->first );
    for ( size_t n=0; n<done.size(); ++n )
    {
        TDownloadMap::iterator i = m_downloads.find( done[ n ] );
        if ( i == m_downloads.end() )
            continue;
        Finish( i->second );
        m_results.push_back( i->second.result );
        ++m_finishedCount;
        ++finished;
        TBrowserDownloadResultEventData eventData( i->second.result );
        m_downloads.erase( i );
        NotifyObservers( DownloadFinishedEvent, &eventData );
    }

    if ( WAIT_PENDING == m_waitState )
    {
        if ( m_finishedCount > m_waitBaseline )
            m_waitState = WAIT_DONE;
        else
        if ( NowInMs() >= m_waitDeadline )
        {
            m_waitState = WAIT_TIMED_OUT;
            NotifyObservers( WaitTimedOutEvent );
        }
    }
    return finished;
}

/*-------------------------------------------------------------------------*/

bool
CBrowserDownloadCapture::PopResult( CBrowserDownloadResult& result )
{GUCEF_TRACE;

    if ( m_results.empty() )
        return false;
    result = m_results.front();
    m_results.erase( m_results.begin() );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CBrowserDownloadCapture::GetFinishedCount( void ) const
{GUCEF_TRACE;

    return m_finishedCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CBrowserDownloadCapture::GetInProgressCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_downloads.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
