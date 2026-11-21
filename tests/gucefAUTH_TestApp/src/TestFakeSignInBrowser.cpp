/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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
#ifndef GUCEF_CORE_CURLENCODER_H
#include "gucefCORE_CUrlEncoder.h"
#define GUCEF_CORE_CURLENCODER_H
#endif /* GUCEF_CORE_CURLENCODER_H ? */
#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#include "TestFakeSignInBrowser.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace AUTHTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
IsNameChar( char c )
{
    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '_' == c || '-' == c;
}

CORE::CString
Substring( const char* text, size_t start, size_t end )
{
    CORE::CString result;
    if ( end > start )
        result.Set( text + start, (UInt32) ( end - start ) );
    return result;
}

/** One part of a selector: what an element must have */
struct SSelectorPart
{
    CORE::CString tag;
    CORE::CString::StringVector ids;
    CORE::CString::StringVector classes;
    CORE::CString::StringMap attributes;
};

bool
ParseSelector( const CORE::CString& selector, SSelectorPart& part )
{
    const char* s = selector.C_String();
    const size_t n = selector.IsNULLOrEmpty() ? 0 : selector.ByteSize() - 1;
    if ( 0 == n )
        return false;
    size_t i = 0;
    size_t start = i;
    while ( i < n && IsNameChar( s[ i ] ) )
        ++i;
    part.tag = Substring( s, start, i ).Lowercase();
    while ( i < n )
    {
        const char kind = s[ i++ ];
        if ( '#' == kind || '.' == kind )
        {
            start = i;
            while ( i < n && IsNameChar( s[ i ] ) )
                ++i;
            if ( i == start )
                return false;
            if ( '#' == kind )
                part.ids.push_back( Substring( s, start, i ) );
            else
                part.classes.push_back( Substring( s, start, i ) );
        }
        else
        if ( '[' == kind )
        {
            start = i;
            while ( i < n && IsNameChar( s[ i ] ) )
                ++i;
            if ( i == start || i >= n )
                return false;
            const CORE::CString name = Substring( s, start, i ).Lowercase();
            CORE::CString value;
            bool hasValue = false;
            if ( '=' == s[ i ] )
            {
                ++i;
                hasValue = true;
                if ( i < n && ( '"' == s[ i ] || '\'' == s[ i ] ) )
                {
                    const char quote = s[ i++ ];
                    start = i;
                    while ( i < n && s[ i ] != quote )
                        ++i;
                    if ( i >= n )
                        return false;
                    value = Substring( s, start, i );
                    ++i;
                }
                else
                {
                    start = i;
                    while ( i < n && IsNameChar( s[ i ] ) )
                        ++i;
                    value = Substring( s, start, i );
                }
            }
            if ( i >= n || ']' != s[ i ] )
                return false;
            ++i;
            part.attributes[ name ] = hasValue ? value : CORE::CString( "\x01" );
        }
        else
            return false;
    }
    return !part.tag.IsNULLOrEmpty() || !part.ids.empty() || !part.classes.empty() || !part.attributes.empty();
}

CORE::CString
AttributeOf( const CORE::CString::StringMap& attributes, const char* name )
{
    CORE::CString::StringMap::const_iterator i = attributes.find( name );
    return i == attributes.end() ? CORE::CString::Empty : (*i).second;
}

bool
HasAttribute( const CORE::CString::StringMap& attributes, const char* name )
{
    return attributes.find( name ) != attributes.end();
}

CORE::CString
UrlEncode( const CORE::CString& text )
{
    CORE::CString encoded;
    CORE::CUrlEncoder::Encode( CORE::CUtf8String( text ), encoded );
    return encoded;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFakeSignInBrowser::CFakeSignInBrowser( void )
    : AUTH::CIWebSignInBrowser()
    , m_lock()
    , m_jar( GUCEF_NEW WEB::CHttpCookieJar() )
    , m_location()
    , m_elements()
    , m_text()
    , m_running( false )
    , m_windowClosed( false )
    , m_failStart( false )
    , m_startCount( 0 )
    , m_stopCount( 0 )
    , m_releaseCount( 0 )
    , m_fillCount( 0 )
    , m_clearCookiesCount( 0 )
    , m_lastSettings()
{
}

/*-------------------------------------------------------------------------*/

CFakeSignInBrowser::~CFakeSignInBrowser()
{
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInBrowser::Start( const WEB::CWebSignInBrowserSettings& settings, CORE::CString& problem )
{
    MT::CScopeMutex lock( m_lock );
    m_lastSettings = settings;
    if ( m_failStart )
    {
        problem = "The test browser does not start";
        return false;
    }
    ++m_startCount;
    m_running = true;
    m_windowClosed = false;
    m_location = "about:blank";
    m_elements.clear();
    m_text.Clear();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFakeSignInBrowser::Stop( void )
{
    MT::CScopeMutex lock( m_lock );
    if ( m_running )
        ++m_stopCount;
    m_running = false;
}

/*-------------------------------------------------------------------------*/

void
CFakeSignInBrowser::Release( void )
{
    MT::CScopeMutex lock( m_lock );
    if ( m_running )
        ++m_releaseCount;
    m_running = false;
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInBrowser::IsAlive( void )
{
    MT::CScopeMutex lock( m_lock );
    return m_running && !m_windowClosed;
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInBrowser::Navigate( const CORE::CString& url )
{
    MT::CScopeMutex lock( m_lock );
    if ( !m_running || m_windowClosed )
        return false;
    return LoadNoLock( "GET", url, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInBrowser::GetLocation( CORE::CString& url )
{
    MT::CScopeMutex lock( m_lock );
    if ( !m_running || m_windowClosed )
        return false;
    url = m_location;
    return true;
}

/*-------------------------------------------------------------------------*/

WEB::TWebElementState
CFakeSignInBrowser::GetElementState( const CORE::CString& selector )
{
    MT::CScopeMutex lock( m_lock );
    if ( !m_running || m_windowClosed )
        return WEB::WEBELEMENT_STATE_NO_ANSWER;
    bool invalid = false;
    const Int32 index = FindNoLock( selector, invalid );
    if ( invalid )
        return WEB::WEBELEMENT_STATE_INVALID_SELECTOR;
    if ( index < 0 )
        return WEB::WEBELEMENT_STATE_ABSENT;
    const SElement& element = m_elements[ index ];
    if ( HasAttribute( element.attributes, "hidden" ) || HasAttribute( element.attributes, "disabled" ) ||
         ( element.tag == "input" && AttributeOf( element.attributes, "type" ).Lowercase() == "hidden" ) )
        return WEB::WEBELEMENT_STATE_PRESENT;
    return WEB::WEBELEMENT_STATE_VISIBLE;
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInBrowser::ContainsText( const CORE::CString& text, bool& contains )
{
    MT::CScopeMutex lock( m_lock );
    if ( !m_running || m_windowClosed )
        return false;
    contains = m_text.HasSubstr( text, true ) >= 0;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInBrowser::FillField( const CORE::CString& selector, const AUTH::CSecretString& value )
{
    MT::CScopeMutex lock( m_lock );
    if ( !m_running || m_windowClosed )
        return false;
    ++m_fillCount;
    return FillNoLock( selector, value.Reveal() );
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInBrowser::Click( const CORE::CString& selector )
{
    MT::CScopeMutex lock( m_lock );
    if ( !m_running || m_windowClosed )
        return false;
    return ClickNoLock( selector );
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInBrowser::Submit( const CORE::CString& selector )
{
    MT::CScopeMutex lock( m_lock );
    if ( !m_running || m_windowClosed )
        return false;
    bool invalid = false;
    const Int32 index = FindNoLock( selector, invalid );
    if ( index < 0 )
        return false;
    const Int32 form = m_elements[ index ].tag == "form" ? index : m_elements[ index ].form;
    return form >= 0 && SubmitFormNoLock( form );
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInBrowser::GetCookies( AUTH::TWebSessionCookieVector& cookies )
{
    MT::CScopeMutex lock( m_lock );
    cookies.clear();
    if ( !m_running || m_windowClosed )
        return false;
    WEB::CHttpCookieJar::TCookieVector all;
    m_jar->GetAllCookies( all );
    for ( size_t i=0; i<all.size(); ++i )
        cookies.push_back( AUTH::CWebSessionCookie::FromHttpCookie( all[ i ] ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInBrowser::ClearCookies( void )
{
    MT::CScopeMutex lock( m_lock );
    ++m_clearCookiesCount;
    m_jar->Clear();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInBrowser::PersonFill( const CORE::CString& selector, const CORE::CString& value )
{
    MT::CScopeMutex lock( m_lock );
    return m_running && !m_windowClosed && FillNoLock( selector, value );
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInBrowser::PersonClick( const CORE::CString& selector )
{
    MT::CScopeMutex lock( m_lock );
    return m_running && !m_windowClosed && ClickNoLock( selector );
}

/*-------------------------------------------------------------------------*/

void
CFakeSignInBrowser::PersonClosesWindow( void )
{
    MT::CScopeMutex lock( m_lock );
    m_windowClosed = true;
}

/*-------------------------------------------------------------------------*/

void
CFakeSignInBrowser::AddCookie( const WEB::CHttpCookie& cookie )
{
    MT::CScopeMutex lock( m_lock );
    m_jar->SetCookie( cookie );
}

/*-------------------------------------------------------------------------*/

void
CFakeSignInBrowser::SetFailStart( bool failStart )
{
    MT::CScopeMutex lock( m_lock );
    m_failStart = failStart;
}

/*-------------------------------------------------------------------------*/

UInt32 CFakeSignInBrowser::GetStartCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_startCount; }
UInt32 CFakeSignInBrowser::GetStopCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_stopCount; }
UInt32 CFakeSignInBrowser::GetReleaseCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_releaseCount; }
UInt32 CFakeSignInBrowser::GetFillCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_fillCount; }
UInt32 CFakeSignInBrowser::GetClearCookiesCount( void ) const { MT::CScopeMutex lock( m_lock ); return m_clearCookiesCount; }
WEB::CWebSignInBrowserSettings CFakeSignInBrowser::GetLastSettings( void ) const { MT::CScopeMutex lock( m_lock ); return m_lastSettings; }

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeSignInBrowser::ResolveNoLock( const CORE::CString& reference ) const
{
    if ( reference.Lowercase().StartsWith( "http://" ) || reference.Lowercase().StartsWith( "https://" ) )
        return reference;
    const Int32 schemeEnd = m_location.HasSubstr( "://", true );
    if ( schemeEnd < 0 )
        return reference;
    const CORE::CString afterScheme = m_location.CutChars( (UInt32) schemeEnd + 3, true );
    const Int32 pathStart = afterScheme.HasChar( '/', true );
    const CORE::CString origin = m_location.SubstrToIndex( (UInt32) schemeEnd + 3, true ) +
                                 ( pathStart >= 0 ? afterScheme.SubstrToIndex( (UInt32) pathStart, true ) : afterScheme );
    if ( reference.IsNULLOrEmpty() )
        return m_location;
    return reference.StartsWith( "/" ) ? origin + reference : origin + "/" + reference;
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInBrowser::LoadNoLock( const CORE::CString& method, const CORE::CString& url, const CORE::CString& body )
{
    WEB::CHttpRequestData request;
    if ( !request.SetUrl( url ) )
        return false;
    request.requestType = method;
    if ( method == "POST" )
    {
        request.content.CopyFrom( body, 0, false );
        request.contentRepresentation = "application/x-www-form-urlencoded";
    }
    WEB::CHttpClientRequestOptions options;
    options.cookieJar = m_jar;
    options.followRedirects = true;
    options.maxRedirects = 10;
    options.connectTimeoutInMs = 5000;
    options.totalTimeoutInMs = 10000;

    WEB::CHTTPClient client;
    WEB::CHttpResponseData response;
    WEB::CHttpClientTransferInfo info;
    if ( !client.Execute( request, options, response, &info ) )
        return false;
    m_location = info.effectiveUrl.IsNULLOrEmpty() ? url : info.effectiveUrl;
    CORE::CString html;
    if ( response.content.GetDataSize() > 0 )
        html.Set( response.content.AsConstTypePtr< char >(), response.content.GetDataSize() );
    ParsePageNoLock( html );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFakeSignInBrowser::ParsePageNoLock( const CORE::CString& html )
{
    m_elements.clear();
    GUCEF::vector< char > text;
    const char* c = html.C_String();
    const size_t n = html.IsNULLOrEmpty() ? 0 : html.ByteSize() - 1;
    Int32 currentForm = -1;
    bool inBody = false;
    size_t i = 0;
    while ( i < n )
    {
        if ( '<' != c[ i ] )
        {
            if ( inBody )
                text.push_back( c[ i ] );
            ++i;
            continue;
        }
        size_t end = i + 1;
        while ( end < n && '>' != c[ end ] )
            ++end;
        if ( end >= n )
            break;
        const CORE::CString tagText = Substring( c, i + 1, end );
        i = end + 1;
        if ( tagText.StartsWith( "!" ) )
            continue;
        if ( tagText.StartsWith( "/" ) )
        {
            const CORE::CString name = tagText.CutChars( 1, true ).Trim( true ).Trim( false ).Lowercase();
            if ( name == "form" )
                currentForm = -1;
            text.push_back( ' ' );
            continue;
        }

        const char* t = tagText.C_String();
        const size_t tn = tagText.ByteSize() - 1;
        size_t p = 0;
        while ( p < tn && IsNameChar( t[ p ] ) )
            ++p;
        SElement element;
        element.tag = Substring( t, 0, p ).Lowercase();
        while ( p < tn )
        {
            while ( p < tn && ( ' ' == t[ p ] || '/' == t[ p ] ) )
                ++p;
            const size_t nameStart = p;
            while ( p < tn && IsNameChar( t[ p ] ) )
                ++p;
            if ( p == nameStart )
            {
                ++p;
                continue;
            }
            const CORE::CString name = Substring( t, nameStart, p ).Lowercase();
            CORE::CString value;
            if ( p < tn && '=' == t[ p ] )
            {
                ++p;
                if ( p < tn && ( '"' == t[ p ] || '\'' == t[ p ] ) )
                {
                    const char quote = t[ p++ ];
                    const size_t valueStart = p;
                    while ( p < tn && quote != t[ p ] )
                        ++p;
                    value = Substring( t, valueStart, p );
                    ++p;
                }
                else
                {
                    const size_t valueStart = p;
                    while ( p < tn && ' ' != t[ p ] )
                        ++p;
                    value = Substring( t, valueStart, p );
                }
            }
            element.attributes[ name ] = value;
        }
        if ( element.tag == "body" )
            inBody = true;
        element.form = currentForm;
        element.value = AttributeOf( element.attributes, "value" );
        m_elements.push_back( element );
        if ( element.tag == "form" )
        {
            currentForm = (Int32) m_elements.size() - 1;
            m_elements.back().form = currentForm;
        }
    }
    m_text.Clear();
    if ( !text.empty() )
        m_text.Set( &text[ 0 ], (UInt32) text.size() );
}

/*-------------------------------------------------------------------------*/

Int32
CFakeSignInBrowser::FindNoLock( const CORE::CString& selector, bool& invalid ) const
{
    SSelectorPart part;
    invalid = !ParseSelector( selector, part );
    if ( invalid )
        return -1;
    for ( size_t e=0; e<m_elements.size(); ++e )
    {
        const SElement& element = m_elements[ e ];
        if ( !part.tag.IsNULLOrEmpty() && part.tag != element.tag )
            continue;
        bool match = true;
        for ( size_t i=0; i<part.ids.size() && match; ++i )
            match = AttributeOf( element.attributes, "id" ) == part.ids[ i ];
        for ( size_t i=0; i<part.classes.size() && match; ++i )
        {
            const CORE::CString::StringVector classes = AttributeOf( element.attributes, "class" ).ParseElements( ' ', false );
            bool found = false;
            for ( size_t k=0; k<classes.size(); ++k )
                found = found || classes[ k ] == part.classes[ i ];
            match = found;
        }
        for ( CORE::CString::StringMap::const_iterator a = part.attributes.begin(); a != part.attributes.end() && match; ++a )
        {
            if ( !HasAttribute( element.attributes, (*a).first.C_String() ) )
                match = false;
            else
            if ( (*a).second != "\x01" )
                match = AttributeOf( element.attributes, (*a).first.C_String() ) == (*a).second;
        }
        if ( match )
            return (Int32) e;
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInBrowser::FillNoLock( const CORE::CString& selector, const CORE::CString& value )
{
    bool invalid = false;
    const Int32 index = FindNoLock( selector, invalid );
    if ( index < 0 || ( m_elements[ index ].tag != "input" && m_elements[ index ].tag != "textarea" ) )
        return false;
    m_elements[ index ].value = value;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInBrowser::ClickNoLock( const CORE::CString& selector )
{
    bool invalid = false;
    const Int32 index = FindNoLock( selector, invalid );
    if ( index < 0 )
        return false;
    const SElement& element = m_elements[ index ];
    const CORE::CString type = AttributeOf( element.attributes, "type" ).Lowercase();
    if ( ( element.tag == "button" && ( type.IsNULLOrEmpty() || type == "submit" ) ) || ( element.tag == "input" && type == "submit" ) )
        return element.form >= 0 ? SubmitFormNoLock( element.form ) : true;
    if ( element.tag == "a" && HasAttribute( element.attributes, "href" ) )
        return LoadNoLock( "GET", ResolveNoLock( AttributeOf( element.attributes, "href" ) ), CORE::CString::Empty );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeSignInBrowser::SubmitFormNoLock( Int32 formIndex )
{
    const SElement& form = m_elements[ formIndex ];
    const CORE::CString method = AttributeOf( form.attributes, "method" ).Lowercase() == "post" ? "POST" : "GET";
    const CORE::CString action = ResolveNoLock( AttributeOf( form.attributes, "action" ) );
    CORE::CString body;
    for ( size_t e=0; e<m_elements.size(); ++e )
    {
        const SElement& element = m_elements[ e ];
        if ( element.form != formIndex || (Int32) e == formIndex || !HasAttribute( element.attributes, "name" ) )
            continue;
        if ( element.tag != "input" && element.tag != "textarea" && element.tag != "select" )
            continue;
        if ( !body.IsNULLOrEmpty() )
            body += "&";
        body += UrlEncode( AttributeOf( element.attributes, "name" ) ) + "=" + UrlEncode( element.value );
    }
    if ( method == "POST" )
        return LoadNoLock( "POST", action, body );
    return LoadNoLock( "GET", body.IsNULLOrEmpty() ? action : action + "?" + body, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTHTEST */

/*-------------------------------------------------------------------------*/
