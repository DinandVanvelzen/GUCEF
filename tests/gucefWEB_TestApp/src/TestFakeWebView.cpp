/*
 *  gucefWEB_TestApp: tests of the gucefWEB module, its web views
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "TestFakeWebViewEngine.h"
#include "TestFakeWebView.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEBVIEWTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace WEB;

namespace {

const Int32 TitleHeight = 24;
const Int32 LineHeight = 14;
const Int32 ElementLeft = 16;
const Int32 ElementWidth = 300;
const Int32 ElementHeight = 28;
const Int32 ElementSpacing = 36;

/** JSON text of a string */
CORE::CString
JsonString( const CORE::CString& text )
{
    GUCEF::vector< char > out;
    out.push_back( '"' );
    const char* c = text.C_String();
    const UInt32 count = text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
    for ( UInt32 i=0; i<count; ++i )
    {
        const unsigned char b = (unsigned char) c[ i ];
        if ( '"' == b || '\\' == b )
        {
            out.push_back( '\\' );
            out.push_back( (char) b );
        }
        else
        if ( b < 0x20 )
        {
            const char* hex = "0123456789abcdef";
            const char escape[ 6 ] = { '\\', 'u', '0', '0', hex[ b >> 4 ], hex[ b & 0x0F ] };
            out.insert( out.end(), escape, escape + 6 );
        }
        else
            out.push_back( (char) b );
    }
    out.push_back( '"' );
    CORE::CString json;
    json.Set( &out[ 0 ], (UInt32) out.size() );
    return json;
}

/** The argument of a helper call: the string literal of "(function(s){...})("literal")" decoded; false when there is none */
bool
HelperArgument( const CORE::CString& script, CORE::CString& argument )
{
    const Int32 call = script.HasSubstr( "})(\"", false );
    if ( call < 0 )
        return false;
    const char* c = script.C_String() + call + 3;
    GUCEF::vector< char > out;
    for ( ++c; '\0' != *c && '"' != *c; ++c )
    {
        if ( '\\' != *c )
        {
            out.push_back( *c );
            continue;
        }
        ++c;
        if ( '\0' == *c )
            return false;
        if ( 'u' == *c )
        {
            UInt32 code = 0;
            for ( UInt32 n=0; n<4; ++n )
            {
                ++c;
                const char h = *c;
                if ( '\0' == h )
                    return false;
                code = code * 16 + (UInt32) ( h >= 'a' ? h - 'a' + 10 : ( h >= 'A' ? h - 'A' + 10 : h - '0' ) );
            }
            if ( code < 0x80 )
                out.push_back( (char) code );
            else
            if ( code < 0x800 )
            {
                out.push_back( (char) ( 0xC0 | ( code >> 6 ) ) );
                out.push_back( (char) ( 0x80 | ( code & 0x3F ) ) );
            }
            else
            {
                out.push_back( (char) ( 0xE0 | ( code >> 12 ) ) );
                out.push_back( (char) ( 0x80 | ( ( code >> 6 ) & 0x3F ) ) );
                out.push_back( (char) ( 0x80 | ( code & 0x3F ) ) );
            }
        }
        else
            out.push_back( 'n' == *c ? '\n' : ( 't' == *c ? '\t' : *c ) );
    }
    argument.Clear();
    if ( !out.empty() )
        argument.Set( &out[ 0 ], (UInt32) out.size() );
    return true;
}

/** The number in "name(<n>)" */
UInt32
NumberArgument( const CORE::CString& script )
{
    const Int32 open = script.HasChar( '(' );
    if ( open < 0 )
        return 0;
    return CORE::StringToUInt32( script.CutChars( (UInt32) open + 1, true ).SubstrToChar( ')', true ), 0 );
}

void
FillRect( IMAGE::CPixelMap& pixels, Int32 left, Int32 top, Int32 width, Int32 height, UInt8 r, UInt8 g, UInt8 b )
{
    CWebViewRect rect( left, top, width, height );
    if ( !rect.ClipTo( pixels.GetWidthInPixels(), pixels.GetHeightInPixels() ) )
        return;
    UInt8* data = static_cast< UInt8* >( pixels.GetDataPtr() );
    const UInt32 stride = pixels.GetWidthInPixels() * 4;
    for ( Int32 y=rect.top; y<rect.GetBottom(); ++y )
    {
        UInt8* p = data + (size_t) y * stride + (size_t) rect.left * 4;
        for ( Int32 x=0; x<rect.width; ++x, p+=4 )
        {
            p[ 0 ] = r;
            p[ 1 ] = g;
            p[ 2 ] = b;
            p[ 3 ] = 255;
        }
    }
}

/** Text as blocks: 5 x 7 cells per character, a pattern from the character code; 'cell' device pixels per cell */
void
DrawText( IMAGE::CPixelMap& pixels, Int32 left, Int32 top, const CORE::CString& text, Int32 cell, UInt8 r, UInt8 g, UInt8 b )
{
    const char* c = text.C_String();
    const UInt32 count = text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
    for ( UInt32 i=0; i<count; ++i )
    {
        const UInt32 code = (unsigned char) c[ i ];
        if ( ' ' == code )
            continue;
        UInt32 pattern = code * 2654435761u;
        pattern ^= pattern >> 13;
        for ( Int32 row=0; row<7; ++row )
            for ( Int32 column=0; column<5; ++column )
                if ( 0 != ( pattern & ( 1u << ( ( row * 5 + column ) % 32 ) ) ) )
                    FillRect( pixels, left + (Int32) i * 6 * cell + column * cell, top + row * cell, cell, cell, r, g, b );
    }
}

}

/*-------------------------------------------------------------------------*/

CFakeWebView::CFakeWebView( CFakeWebViewEngine& engine, UInt32 viewId, const CWebViewSettings& settings, const CWebViewCapabilities& capabilities )
    : CWebViewBase( engine, viewId, settings, capabilities )
    , m_fakeEngine( engine )
    , m_page()
    , m_url( "about:blank" )
    , m_history()
    , m_historyIndex( -1 )
    , m_pixels( GUCEF_NULL, 1, 1, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 )
    , m_pressed( -1 )
    , m_hovered( -1 )
    , m_scrollY( 0 )
    , m_nextDialog( 0 )
    , m_openDialog( 0 )
    , m_pendingCertificateUrl()
    , m_certificateDecision( 0 )
    , m_decisionActions()
    , m_afterLoad()
    , m_ephemeralJar()
    , m_testLock()
    , m_recorded()
    , m_verified( false )
    , m_focused( -1 )
    , m_answers()
    , m_layout()
    , m_renders( 0 )
    , m_visible( true )
    , m_pageFocus( false )
    , m_zoom( 1.0f )
{GUCEF_TRACE;

    m_page.title = "";
}

/*-------------------------------------------------------------------------*/

CFakeWebView::~CFakeWebView()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

GUCEF::vector< CWebViewInputEvent >
CFakeWebView::GetRecordedInput( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_testLock );
    return m_recorded;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeWebView::GetRecordedInputCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_testLock );
    return (UInt32) m_recorded.size();
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebView::IsVerified( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_testLock );
    return m_verified;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeWebView::GetFocusedElement( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_testLock );
    return m_focused >= 0 && m_focused < (Int32) m_page.elements.size() ? m_page.elements[ m_focused ].id : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CFakeWebView::GetAnswers( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_testLock );
    return m_answers;
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebView::GetElementRect( const CORE::CString& id, CWebViewRect& rect ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_testLock );
    for ( UInt32 i=0; i<(UInt32)m_page.elements.size() && i<(UInt32)m_layout.size(); ++i )
    {
        if ( m_page.elements[ i ].id == id )
        {
            rect = m_layout[ i ];
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeWebView::GetRenderCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_testLock );
    return m_renders;
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebView::IsPageVisible( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_testLock );
    return m_visible;
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebView::HasPageFocus( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_testLock );
    return m_pageFocus;
}

/*-------------------------------------------------------------------------*/

Float32
CFakeWebView::GetZoom( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_testLock );
    return m_zoom;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFakeWebView::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::WEBVIEWTEST::CFakeWebView";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

TWebViewCookieVector&
CFakeWebView::GetJar( void )
{GUCEF_TRACE;

    if ( GetSettings().profile.ephemeral )
        return m_ephemeralJar;
    return m_fakeEngine.GetCookieJar( GetSettings().profile.profileId );
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebView::HasSession( void ) const
{GUCEF_TRACE;

    const TWebViewCookieVector& jar = const_cast< CFakeWebView* >( this )->GetJar();
    for ( TWebViewCookieVector::const_iterator i=jar.begin(); i!=jar.end(); ++i )
        if ( "sid" == (*i).name && (*i).MatchesUrl( CFakeWebSite::Url( "/account" ) ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::AddAnswer( const CORE::CString& answer )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_testLock );
    m_answers.push_back( answer );
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::OnCreated( void )
{GUCEF_TRACE;

    Render( GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::Layout( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_testLock );
    m_layout.clear();
    Int32 y = TitleHeight + 8 + (Int32) m_page.lines.size() * LineHeight + 8 - m_scrollY;
    for ( UInt32 i=0; i<(UInt32)m_page.elements.size(); ++i )
    {
        m_layout.push_back( CWebViewRect( (Int32) ( ElementLeft * m_zoom ), (Int32) ( y * m_zoom ), (Int32) ( ElementWidth * m_zoom ), (Int32) ( ElementHeight * m_zoom ) ) );
        y += ElementSpacing;
    }
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::Render( const CWebViewRect* logicalDirty )
{GUCEF_TRACE;

    UInt32 width = 0, height = 0;
    Float32 scale = 1.0f;
    GetSize( width, height, scale );
    UInt32 deviceWidth = 0, deviceHeight = 0;
    CWebViewSettings::ToDeviceSize( width, height, scale, deviceWidth, deviceHeight );
    bool whole = GUCEF_NULL == logicalDirty;
    if ( m_pixels.GetWidthInPixels() != deviceWidth || m_pixels.GetHeightInPixels() != deviceHeight )
    {
        m_pixels.Assign( GUCEF_NULL, deviceWidth, deviceHeight, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 );
        whole = true;
    }
    Layout();

    IMAGE::CPixelMap& p = m_pixels;
    const Int32 w = (Int32) deviceWidth;
    const Int32 h = (Int32) deviceHeight;
    if ( m_page.colourBlocks )
    {
        const Int32 halfW = w / 2;
        const Int32 halfH = h / 2;
        const UInt8 (*q)[ 3 ] = CFakeWebSite::QuadrantColours;
        FillRect( p, 0, 0, halfW, halfH, q[ 0 ][ 0 ], q[ 0 ][ 1 ], q[ 0 ][ 2 ] );
        FillRect( p, halfW, 0, w - halfW, halfH, q[ 1 ][ 0 ], q[ 1 ][ 1 ], q[ 1 ][ 2 ] );
        FillRect( p, 0, halfH, halfW, h - halfH, q[ 2 ][ 0 ], q[ 2 ][ 1 ], q[ 2 ][ 2 ] );
        FillRect( p, halfW, halfH, w - halfW, h - halfH, q[ 3 ][ 0 ], q[ 3 ][ 1 ], q[ 3 ][ 2 ] );
    }
    else
    {
        const Float32 s = scale * m_zoom;
        const Int32 cell = s < 1.0f ? 1 : (Int32) ( s + 0.5f );
        FillRect( p, 0, 0, w, h, 255, 255, 255 );
        FillRect( p, 0, 0, w, (Int32) ( TitleHeight * s ), 220, 222, 228 );
        DrawText( p, (Int32) ( 8 * s ), (Int32) ( 6 * s ), m_page.title, cell, 40, 40, 60 );
        for ( UInt32 i=0; i<(UInt32)m_page.lines.size(); ++i )
            DrawText( p, (Int32) ( 8 * s ), (Int32) ( ( TitleHeight + 8 + (Int32) i * LineHeight - m_scrollY ) * s ), m_page.lines[ i ], cell, 20, 20, 20 );
        MT::CScopeMutex lock( m_testLock );
        for ( UInt32 i=0; i<(UInt32)m_page.elements.size() && i<(UInt32)m_layout.size(); ++i )
        {
            const CFakeWebSite::SElement& e = m_page.elements[ i ];
            const CWebViewRect& r = m_layout[ i ];
            const Int32 x = (Int32) ( r.left * scale ), y = (Int32) ( r.top * scale );
            const Int32 ew = (Int32) ( r.width * scale ), eh = (Int32) ( r.height * scale );
            const bool focused = (Int32) i == m_focused;
            FillRect( p, x, y, ew, eh, focused ? 30 : 120, focused ? 110 : 120, focused ? 220 : 130 );
            if ( CFakeWebSite::ELEMENT_BUTTON == e.kind )
            {
                FillRect( p, x + 1, y + 1, ew - 2, eh - 2, 60, 110, 200 );
                DrawText( p, x + (Int32) ( 8 * s ), y + (Int32) ( 10 * s ), e.label, cell, 255, 255, 255 );
            }
            else
            if ( CFakeWebSite::ELEMENT_LINK == e.kind )
            {
                FillRect( p, x + 1, y + 1, ew - 2, eh - 2, 255, 255, 255 );
                DrawText( p, x + (Int32) ( 8 * s ), y + (Int32) ( 10 * s ), e.label, cell, 30, 60, 200 );
            }
            else
            {
                FillRect( p, x + 1, y + 1, ew - 2, eh - 2, 245, 245, 245 );
                // a password shows one mark per character, never the characters
                CORE::CString shown = e.value;
                if ( CFakeWebSite::ELEMENT_PASSWORD == e.kind )
                {
                    shown.Clear();
                    for ( UInt32 n=0; n<e.value.Length(); ++n )
                        shown += "*";
                }
                DrawText( p, x + (Int32) ( 8 * s ), y + (Int32) ( 10 * s ), shown.IsNULLOrEmpty() ? e.label : shown, cell, shown.IsNULLOrEmpty() ? 150 : 10, shown.IsNULLOrEmpty() ? 150 : 10, shown.IsNULLOrEmpty() ? 150 : 10 );
            }
        }
    }
    {
        MT::CScopeMutex lock( m_testLock );
        ++m_renders;
        if ( !m_visible )
            return;
    }

    CWebViewRect dirty = CWebViewRect::Whole( deviceWidth, deviceHeight );
    if ( !whole )
    {
        dirty = CWebViewRect( (Int32) ( logicalDirty->left * scale ), (Int32) ( logicalDirty->top * scale ),
                              (Int32) ( logicalDirty->width * scale + 0.999f ), (Int32) ( logicalDirty->height * scale + 0.999f ) );
    }
    PublishFrame( m_pixels, dirty );
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::RedrawElement( Int32 element )
{GUCEF_TRACE;

    CWebViewRect rect;
    {
        MT::CScopeMutex lock( m_testLock );
        if ( element < 0 || element >= (Int32) m_layout.size() )
            return;
        rect = m_layout[ element ];
    }
    Render( &rect );
}

/*-------------------------------------------------------------------------*/

Int32
CFakeWebView::HitTest( Int32 x, Int32 y ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_testLock );
    for ( UInt32 i=0; i<(UInt32)m_layout.size(); ++i )
        if ( m_layout[ i ].Contains( x, y ) )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CFakeWebView::FindElement( const CORE::CString& selector ) const
{GUCEF_TRACE;

    CORE::CString id = selector;
    if ( selector.StartsWith( "#" ) )
        id = selector.CutChars( 1, true );
    else
    if ( selector.StartsWith( "[name=" ) )
        id = selector.CutChars( 6, true ).SubstrToChar( ']', true ).ReplaceChar( '"', ' ' ).Trim( true ).Trim( false );
    for ( UInt32 i=0; i<(UInt32)m_page.elements.size(); ++i )
        if ( m_page.elements[ i ].id == id )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeWebView::GetPageText( void ) const
{GUCEF_TRACE;

    CORE::CString text = m_page.title;
    for ( UInt32 i=0; i<(UInt32)m_page.lines.size(); ++i )
        text += "\n" + m_page.lines[ i ];
    for ( UInt32 i=0; i<(UInt32)m_page.elements.size(); ++i )
        if ( CFakeWebSite::ELEMENT_BUTTON == m_page.elements[ i ].kind || CFakeWebSite::ELEMENT_LINK == m_page.elements[ i ].kind )
            text += "\n" + m_page.elements[ i ].label;
    return text;
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::Focus( Int32 element )
{GUCEF_TRACE;

    Int32 previous = -1;
    {
        MT::CScopeMutex lock( m_testLock );
        previous = m_focused;
        m_focused = element;
    }
    if ( previous != element )
    {
        RedrawElement( previous );
        RedrawElement( element );
    }
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::Click( Int32 element, bool trusted )
{GUCEF_TRACE;

    if ( element < 0 || element >= (Int32) m_page.elements.size() )
        return;
    CFakeWebSite::SElement& e = m_page.elements[ element ];
    if ( e.disabled || ( e.trustedOnly && !trusted ) )
        return;
    switch ( e.kind )
    {
        case CFakeWebSite::ELEMENT_LINK:
        {
            if ( WEBVIEWDECISION_ALLOW == CheckNavigation( e.target, true, trusted, false ) )
                NavigateInternal( e.target, false, trusted );
            return;
        }
        case CFakeWebSite::ELEMENT_BUTTON:
        {
            if ( "verify" == e.id )
            {
                {
                    MT::CScopeMutex lock( m_testLock );
                    m_verified = true;
                }
                m_page.lines.push_back( "Verified" );
                Render( GUCEF_NULL );
                return;
            }
            Submit( element );
            return;
        }
        default:
        {
            Focus( element );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::Submit( Int32 element )
{GUCEF_TRACE;

    if ( !m_page.signsIn )
        return;
    // the form is the page: both fields filled signs in
    const Int32 user = FindElement( "#user" );
    const Int32 password = FindElement( "#password" );
    if ( user < 0 || password < 0 || m_page.elements[ user ].value.IsNULLOrEmpty() || m_page.elements[ password ].value.IsNULLOrEmpty() )
    {
        m_page.lines.push_back( "Fill in both fields" );
        Render( GUCEF_NULL );
        return;
    }
    CWebViewCookie cookie;
    cookie.name = "sid";
    cookie.SetValue( "fake-session-" + CORE::UInt32ToString( m_fakeEngine.NextSessionNumber() ) );
    cookie.domain = "app.test";
    cookie.path = "/";
    cookie.secure = true;
    cookie.httpOnly = true;
    cookie.sameSite = WEBVIEWSAMESITE_LAX;
    TWebViewCookieVector& jar = GetJar();
    for ( TWebViewCookieVector::iterator i=jar.begin(); i!=jar.end(); ++i )
    {
        if ( (*i).IsSameCookie( cookie ) )
        {
            jar.erase( i );
            break;
        }
    }
    jar.push_back( cookie );
    CORE::CString target = CFakeWebSite::Url( "/account" );
    for ( UInt32 i=0; i<(UInt32)m_page.elements.size(); ++i )
        if ( "submit" == m_page.elements[ i ].id && !m_page.elements[ i ].target.IsNULLOrEmpty() )
            target = m_page.elements[ i ].target;
    if ( WEBVIEWDECISION_ALLOW == CheckNavigation( target, true, true, false ) )
        NavigateInternal( target, false, true );
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::OnNavigate( const CORE::CString& url )
{GUCEF_TRACE;

    NavigateInternal( url, false, false );
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::NavigateInternal( const CORE::CString& startUrl, bool fromHistory, bool userGesture )
{GUCEF_TRACE;

    NotifyLoadStarted( startUrl );
    NotifyProgress( 0.1f );
    m_afterLoad.clear();

    if ( startUrl.Equals( "about:blank", false ) )
    {
        CFakeWebSite::TPage blank;
        CommitPage( startUrl, blank, fromHistory );
        NotifyLoadFinished( startUrl );
        return;
    }

    CORE::CString url = startUrl;
    CFakeWebSite::TPage page;
    for ( UInt32 hop=0; hop<6; ++hop )
    {
        bool knownHost = false, certificateError = false;
        m_fakeEngine.GetSite().FindPage( url, page, knownHost, certificateError );
        if ( !knownHost )
        {
            NotifyLoadFailed( url, WEBVIEWERROR_NETWORK, "The host is not known (no request leaves the machine)" );
            return;
        }
        if ( certificateError )
        {
            CWebViewCertificateError report;
            report.url = url;
            CORE::CString scheme;
            UInt16 port = 0;
            bool userInfo = false;
            CWebViewHostAllowList::SplitUrl( url, scheme, report.host, port, userInfo );
            report.errorFlags = 1;
            report.errorDescription = "The certificate is not trusted";
            report.issuer = "Fake Test Issuer";
            report.subject = report.host;
            report.validFrom = "2026-01-01";
            report.validUntil = "2027-01-01";
            UInt32 decisionId = 0;
            const TWebViewDecision decision = ReportCertificateError( report, decisionId );
            if ( WEBVIEWDECISION_ASK == decision )
            {
                m_pendingCertificateUrl = url;
                m_certificateDecision = decisionId;
                return;
            }
            NotifyLoadFailed( url, WEBVIEWERROR_CERTIFICATE, "The certificate of " + report.host + " is not trusted" );
            return;
        }
        if ( page.requiresSession && !HasSession() )
            page.redirectTo = CFakeWebSite::Url( "/login" );
        if ( page.redirectTo.IsNULLOrEmpty() )
            break;
        url = page.redirectTo;
        if ( WEBVIEWDECISION_ALLOW != CheckNavigation( url, true, false, false ) )
            return;
    }

    if ( page.loops )
    {
        // a page that keeps the engine busy: the watchdog of the host notices
        MT::PrecisionDelay( m_fakeEngine.GetLoopMs() );
    }
    if ( page.crashes )
    {
        ReportProcessTerminated( WEBVIEWTERMINATION_CRASH );
        return;
    }
    CommitPage( url, page, fromHistory );
    RunPageLoadActions();
    NotifyLoadFinished( url );

    // a popup allowed into the same view replaces the page once it loaded
    CORE::CString::StringVector afterLoad;
    afterLoad.swap( m_afterLoad );
    for ( UInt32 i=0; i<(UInt32)afterLoad.size(); ++i )
        NavigateInternal( afterLoad[ i ], false, false );
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::CommitPage( const CORE::CString& url, const CFakeWebSite::TPage& page, bool fromHistory )
{GUCEF_TRACE;

    m_page = page;
    m_url = url;
    m_scrollY = 0;
    m_pressed = -1;
    m_hovered = -1;
    m_nextDialog = 0;
    m_openDialog = 0;
    {
        MT::CScopeMutex lock( m_testLock );
        m_focused = -1;
        m_verified = false;
    }
    if ( !fromHistory )
    {
        if ( m_historyIndex + 1 < (Int32) m_history.size() )
            m_history.erase( m_history.begin() + ( m_historyIndex + 1 ), m_history.end() );
        m_history.push_back( url );
        m_historyIndex = (Int32) m_history.size() - 1;
    }
    NotifyLoadCommitted( url );
    NotifyTitleChanged( m_page.title );
    NotifyProgress( 0.5f );
    NotifyCursorChanged( WEBVIEWCURSOR_ARROW );
    Render( GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::RunPageLoadActions( void )
{GUCEF_TRACE;

    if ( !m_page.subframeUrl.IsNULLOrEmpty() )
    {
        const bool allowed = WEBVIEWDECISION_ALLOW == CheckNavigation( m_page.subframeUrl, false, false, false );
        m_page.lines.push_back( allowed ? "Frame loaded" : "Frame refused" );
        Render( GUCEF_NULL );
    }
    if ( !m_page.popupUrl.IsNULLOrEmpty() )
    {
        UInt32 decisionId = 0;
        const TWebViewDecision decision = RequestPopup( m_page.popupUrl, decisionId );
        if ( WEBVIEWDECISION_ALLOW == decision )
            m_afterLoad.push_back( m_page.popupUrl );
        else
        if ( WEBVIEWDECISION_ASK == decision )
            m_decisionActions[ decisionId ] = m_page.popupUrl;
        else
            AddAnswer( "popup:refused" );
    }
    if ( !m_page.downloadUrl.IsNULLOrEmpty() )
    {
        UInt32 decisionId = 0;
        if ( WEBVIEWDECISION_REFUSE == RequestDownload( m_page.downloadUrl, m_page.downloadName, decisionId ) )
            AddAnswer( "download:refused" );
    }
    for ( UInt32 i=0; i<(UInt32)m_page.permissions.size(); ++i )
    {
        UInt32 decisionId = 0;
        if ( WEBVIEWDECISION_REFUSE == RequestPermission( (TWebViewPermissionKind) m_page.permissions[ i ], "https://app.test", decisionId ) )
            AddAnswer( CORE::CString( "permission:" ) + WebViewPermissionKindToString( (TWebViewPermissionKind) m_page.permissions[ i ] ) + ":denied" );
    }
    if ( !m_page.consoleText.IsNULLOrEmpty() )
        ReportConsoleMessage( WEBVIEWCONSOLE_INFO, m_page.consoleText, m_url, 1 );
    ShowNextDialog();
    if ( m_page.closesWindow )
        RequestClose();
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::ShowNextDialog( void )
{GUCEF_TRACE;

    while ( m_nextDialog < (UInt32) m_page.dialogs.size() )
    {
        const TWebViewScriptDialogKind kind = (TWebViewScriptDialogKind) m_page.dialogs[ m_nextDialog++ ];
        UInt32 decisionId = 0;
        const CORE::CString message = CORE::CString( "A " ) + WebViewScriptDialogKindToString( kind ) + " of the page";
        const TWebViewDecision decision = ShowScriptDialog( kind, message, WEBVIEWDIALOG_PROMPT == kind ? "suggested" : "", decisionId );
        if ( WEBVIEWDECISION_ASK == decision )
        {
            m_openDialog = decisionId;
            return;
        }
        AddAnswer( CORE::CString( WebViewScriptDialogKindToString( kind ) ) + ":refused" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebView::OnReload( void )
{GUCEF_TRACE;

    if ( m_historyIndex < 0 )
        return false;
    NavigateInternal( m_history[ m_historyIndex ], true, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebView::OnStop( void )
{GUCEF_TRACE;

    m_afterLoad.clear();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebView::OnGoBack( void )
{GUCEF_TRACE;

    if ( m_historyIndex <= 0 )
        return false;
    --m_historyIndex;
    NavigateInternal( m_history[ m_historyIndex ], true, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebView::OnGoForward( void )
{GUCEF_TRACE;

    if ( m_historyIndex + 1 >= (Int32) m_history.size() )
        return false;
    ++m_historyIndex;
    NavigateInternal( m_history[ m_historyIndex ], true, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebView::EvaluateHelper( const CORE::CString& script, bool pageWorld, CORE::CString& json, CORE::CString& error )
{GUCEF_TRACE;

    // the page's own world of the hostile page has a document.querySelector that finds nothing
    const bool sabotaged = pageWorld && m_page.redefinesQuerySelector;
    CORE::CString argument;
    if ( script.HasSubstr( "window.location.href" ) >= 0 )
    {
        json = JsonString( m_url );
        return true;
    }
    if ( !HelperArgument( script, argument ) )
        return false;
    const Int32 element = sabotaged ? -1 : FindElement( argument );

    if ( script.HasSubstr( "getClientRects" ) >= 0 )
    {
        if ( element < 0 )
            json = "0";
        else
            json = m_page.elements[ element ].disabled ? "1" : "2";
        return true;
    }
    if ( script.HasSubstr( "set.call(e,'')" ) >= 0 )
    {
        if ( element < 0 )
        {
            json = "false";
            return true;
        }
        m_page.elements[ element ].value.Clear();
        Focus( element );
        RedrawElement( element );
        json = "true";
        return true;
    }
    if ( script.HasSubstr( "e.value.length" ) >= 0 )
    {
        const bool isField = element >= 0 && ( CFakeWebSite::ELEMENT_INPUT == m_page.elements[ element ].kind || CFakeWebSite::ELEMENT_PASSWORD == m_page.elements[ element ].kind );
        json = isField ? CORE::Int32ToString( (Int32) m_page.elements[ element ].value.Length() ) : CORE::CString( "-1" );
        return true;
    }
    if ( script.HasSubstr( "e.click();return true" ) >= 0 )
    {
        if ( element < 0 )
        {
            json = "false";
            return true;
        }
        Click( element, false );        // a script click is not trusted
        json = "true";
        return true;
    }
    if ( script.HasSubstr( "requestSubmit" ) >= 0 )
    {
        if ( element < 0 )
        {
            json = "false";
            return true;
        }
        Submit( element );
        json = "true";
        return true;
    }
    if ( script.HasSubstr( "innerText.indexOf(t)" ) >= 0 )
    {
        json = GetPageText().HasSubstr( argument ) >= 0 ? "true" : "false";
        return true;
    }
    error = "ReferenceError: the fake engine does not know this helper";
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::OnEvaluateScript( const CORE::CString& script, const CORE::CString& world, CWebViewCommand& command )
{GUCEF_TRACE;

    const CORE::CString trimmed = script.Trim( true ).Trim( false );
    if ( trimmed.StartsWith( "throw " ) )
    {
        FailCommand( command, WEBVIEWERROR_SCRIPT, "Error: " + trimmed.CutChars( 6, true ) );
        return;
    }
    if ( "document.title" == trimmed )
    {
        CompleteScript( command, JsonString( m_page.title ) );
        return;
    }
    if ( trimmed.StartsWith( "fake:repeat(" ) )
    {
        const UInt32 count = NumberArgument( trimmed );
        GUCEF::vector< char > text( count + 2, 'x' );
        text[ 0 ] = '"';
        text[ count + 1 ] = '"';
        CORE::CString json;
        json.Set( &text[ 0 ], (UInt32) text.size() );
        CompleteScript( command, json );
        return;
    }
    if ( trimmed.StartsWith( "fake:nested(" ) )
    {
        const UInt32 depth = NumberArgument( trimmed );
        CORE::CString json;
        for ( UInt32 i=0; i<depth; ++i )
            json += "[";
        for ( UInt32 i=0; i<depth; ++i )
            json += "]";
        CompleteScript( command, json );
        return;
    }
    if ( trimmed.StartsWith( "fake:value(" ) )
    {
        const Int32 element = FindElement( trimmed.CutChars( 11, true ).SubstrToChar( ')', true ) );
        CompleteScript( command, element < 0 ? CORE::CString( "null" ) : JsonString( m_page.elements[ element ].value ) );
        return;
    }
    if ( "fake:state" == trimmed )
    {
        CompleteScript( command, CORE::CString( "{\"verified\":" ) + ( IsVerified() ? "true" : "false" ) + ",\"focused\":" + JsonString( GetFocusedElement() ) + "}" );
        return;
    }
    CORE::CString json, error;
    if ( !EvaluateHelper( trimmed, world.IsNULLOrEmpty(), json, error ) )
    {
        FailCommand( command, WEBVIEWERROR_SCRIPT, "SyntaxError: the fake engine evaluates its known expressions only" );
        return;
    }
    if ( !error.IsNULLOrEmpty() )
        FailCommand( command, WEBVIEWERROR_SCRIPT, error );
    else
        CompleteScript( command, json );
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::OnGetCookies( const CORE::CString& urlFilter, CWebViewCommand& command )
{GUCEF_TRACE;

    TWebViewCookieVector matches;
    const TWebViewCookieVector& jar = GetJar();
    for ( TWebViewCookieVector::const_iterator i=jar.begin(); i!=jar.end(); ++i )
        if ( (*i).MatchesFilter( urlFilter ) )
            matches.push_back( (*i) );
    CompleteCookies( command, matches );
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::OnClearCookies( CWebViewCommand& command )
{GUCEF_TRACE;

    GetJar().clear();
    CompleteCommand( command );
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::OnSetCookie( const CWebViewCookie& cookie, CWebViewCommand& command )
{GUCEF_TRACE;

    TWebViewCookieVector& jar = GetJar();
    for ( TWebViewCookieVector::iterator i=jar.begin(); i!=jar.end(); ++i )
    {
        if ( (*i).IsSameCookie( cookie ) )
        {
            jar.erase( i );
            break;
        }
    }
    jar.push_back( cookie );
    CompleteCommand( command );
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::OnSetSize( UInt32 logicalWidth, UInt32 logicalHeight, Float32 deviceScale )
{GUCEF_TRACE;

    Render( GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::OnSetVisible( bool visible )
{GUCEF_TRACE;

    bool wasVisible = true;
    {
        MT::CScopeMutex lock( m_testLock );
        wasVisible = m_visible;
        m_visible = visible;
    }
    if ( visible && !wasVisible )
        Render( GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::OnSetFocus( bool focused )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_testLock );
    m_pageFocus = focused;
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::OnInjectInput( const CWebViewInputEvent& inputEvent )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_testLock );
        m_recorded.push_back( inputEvent );
    }
    switch ( inputEvent.kind )
    {
        case WEBVIEWINPUT_POINTER_MOVE:
        {
            const Int32 hit = HitTest( inputEvent.x, inputEvent.y );
            m_hovered = hit;
            TWebViewCursorShape shape = WEBVIEWCURSOR_ARROW;
            if ( hit >= 0 )
            {
                const CFakeWebSite::EElementKind kind = m_page.elements[ hit ].kind;
                shape = ( CFakeWebSite::ELEMENT_BUTTON == kind || CFakeWebSite::ELEMENT_LINK == kind ) ? WEBVIEWCURSOR_HAND : WEBVIEWCURSOR_TEXT;
            }
            NotifyCursorChanged( shape );
            return;
        }
        case WEBVIEWINPUT_POINTER_DOWN:
        {
            if ( WEBMOUSEBUTTON_LEFT != inputEvent.button )
                return;
            // a press focuses what it is on (fields, buttons and links take the focus), or nothing
            m_pressed = HitTest( inputEvent.x, inputEvent.y );
            Focus( m_pressed );
            return;
        }
        case WEBVIEWINPUT_POINTER_UP:
        {
            if ( WEBMOUSEBUTTON_LEFT != inputEvent.button )
                return;
            const Int32 hit = HitTest( inputEvent.x, inputEvent.y );
            const Int32 pressed = m_pressed;
            m_pressed = -1;
            if ( hit >= 0 && hit == pressed )
                Click( hit, true );
            return;
        }
        case WEBVIEWINPUT_POINTER_LEAVE:
        {
            m_hovered = -1;
            m_pressed = -1;
            NotifyCursorChanged( WEBVIEWCURSOR_ARROW );
            return;
        }
        case WEBVIEWINPUT_WHEEL:
        {
            m_scrollY -= (Int32) ( inputEvent.wheelNotchesY * 40.0f );
            if ( m_scrollY < 0 )
                m_scrollY = 0;
            Render( GUCEF_NULL );
            return;
        }
        case WEBVIEWINPUT_KEY_DOWN:
        {
            Int32 focused = -1;
            {
                MT::CScopeMutex lock( m_testLock );
                focused = m_focused;
            }
            const bool backwards = 0 != ( inputEvent.modifiers & WEBMODIFIER_SHIFT );
            // the keys act by their key value, as in a browser: a layout may put them elsewhere
            if ( WEBNAMEDKEY_TAB == inputEvent.namedKey )
            {
                const Int32 count = (Int32) m_page.elements.size();
                const Int32 next = backwards ? ( focused < 0 ? count - 1 : focused - 1 ) : focused + 1;
                if ( next < 0 || next >= count )
                {
                    // past the last (or first) element: the focus leaves the page
                    Focus( -1 );
                    RequestFocusLeave( !backwards );
                }
                else
                    Focus( next );
                return;
            }
            if ( focused < 0 )
                return;
            CFakeWebSite::SElement& e = m_page.elements[ focused ];
            if ( WEBNAMEDKEY_BACKSPACE == inputEvent.namedKey && e.value.Length() > 0 )
            {
                e.value = e.value.CutChars( 1, false );
                RedrawElement( focused );
                return;
            }
            if ( WEBNAMEDKEY_ENTER == inputEvent.namedKey )
            {
                if ( CFakeWebSite::ELEMENT_INPUT == e.kind || CFakeWebSite::ELEMENT_PASSWORD == e.kind )
                    Submit( focused );
                else
                    Click( focused, true );
                return;
            }
            if ( inputEvent.IsCharacterKey() && ' ' == inputEvent.keyText[ 0 ] && '\0' == inputEvent.keyText[ 1 ] && ( CFakeWebSite::ELEMENT_BUTTON == e.kind || CFakeWebSite::ELEMENT_LINK == e.kind ) )
                Click( focused, true );
            return;
        }
        case WEBVIEWINPUT_TEXT:
        {
            Int32 focused = -1;
            {
                MT::CScopeMutex lock( m_testLock );
                focused = m_focused;
            }
            if ( focused < 0 )
                return;
            CFakeWebSite::SElement& e = m_page.elements[ focused ];
            if ( CFakeWebSite::ELEMENT_INPUT != e.kind && CFakeWebSite::ELEMENT_PASSWORD != e.kind )
                return;
            // injected input is trusted input, also in fields that take nothing else
            e.value += inputEvent.text;
            RedrawElement( focused );
            return;
        }
        case WEBVIEWINPUT_FOCUS_IN:
        case WEBVIEWINPUT_FOCUS_OUT:
        {
            MT::CScopeMutex lock( m_testLock );
            m_pageFocus = WEBVIEWINPUT_FOCUS_IN == inputEvent.kind;
            return;
        }
        default:
            return;
    }
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebView::OnSetZoom( Float32 factor )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_testLock );
        m_zoom = factor;
    }
    Render( GUCEF_NULL );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::OnDecisionResolved( UInt32 decisionId, TWebViewDecisionKind kind, bool allow, const CORE::CString& text, bool timedOut )
{GUCEF_TRACE;

    const CORE::CString outcome = timedOut ? CORE::CString( "timedOut" ) : ( allow ? CORE::CString( "allowed" ) : CORE::CString( "refused" ) );
    switch ( kind )
    {
        case WEBVIEWDECISIONKIND_POPUP:
        {
            TDecisionActionMap::iterator i = m_decisionActions.find( decisionId );
            CORE::CString url;
            if ( i != m_decisionActions.end() )
            {
                url = (*i).second;
                m_decisionActions.erase( i );
            }
            AddAnswer( "popup:" + outcome );
            if ( allow && !url.IsNULLOrEmpty() && WEBVIEWDECISION_ALLOW == CheckNavigation( url, true, false, false ) )
                NavigateInternal( url, false, false );
            return;
        }
        case WEBVIEWDECISIONKIND_DOWNLOAD:
        {
            AddAnswer( "download:" + outcome );
            return;
        }
        case WEBVIEWDECISIONKIND_PERMISSION:
        {
            AddAnswer( "permission:" + outcome );
            return;
        }
        case WEBVIEWDECISIONKIND_CERTIFICATE_ERROR:
        {
            AddAnswer( "certificate:" + outcome );
            if ( decisionId != m_certificateDecision )
                return;
            const CORE::CString url = m_pendingCertificateUrl;
            m_pendingCertificateUrl.Clear();
            m_certificateDecision = 0;
            if ( !allow )
            {
                NotifyLoadFailed( url, WEBVIEWERROR_CERTIFICATE, "The certificate was not accepted" );
                return;
            }
            bool knownHost = false, certificateError = false;
            CFakeWebSite::TPage page;
            m_fakeEngine.GetSite().FindPage( url, page, knownHost, certificateError );
            CommitPage( url, page, false );
            NotifySecurityState( WEBVIEWSECURITY_CERTIFICATE_ERROR );
            NotifyLoadFinished( url );
            return;
        }
        case WEBVIEWDECISIONKIND_SCRIPT_DIALOG:
        {
            if ( decisionId != m_openDialog )
                return;
            m_openDialog = 0;
            const UInt32 index = m_nextDialog > 0 ? m_nextDialog - 1 : 0;
            const TWebViewScriptDialogKind dialogKind = index < (UInt32) m_page.dialogs.size() ? (TWebViewScriptDialogKind) m_page.dialogs[ index ] : WEBVIEWDIALOG_ALERT;
            CORE::CString answer = WebViewScriptDialogKindToString( dialogKind );
            if ( WEBVIEWDIALOG_PROMPT == dialogKind && allow )
                answer += ":" + text;
            else
            if ( WEBVIEWDIALOG_ALERT != dialogKind || timedOut )
                answer += ":" + outcome;
            AddAnswer( answer );
            ShowNextDialog();
            return;
        }
        default:
            return;
    }
}

/*-------------------------------------------------------------------------*/

bool
CFakeWebView::OnEditCommand( TWebViewEditCommand command, CORE::CString& copiedText )
{GUCEF_TRACE;

    Int32 focused = -1;
    {
        MT::CScopeMutex lock( m_testLock );
        focused = m_focused;
    }
    if ( focused < 0 )
        return WEBVIEWEDIT_SELECTALL == command;
    CFakeWebSite::SElement& e = m_page.elements[ focused ];
    switch ( command )
    {
        case WEBVIEWEDIT_COPY:
        {
            // a page does not let a password be copied
            copiedText = CFakeWebSite::ELEMENT_PASSWORD == e.kind ? CORE::CString::Empty : e.value;
            return true;
        }
        case WEBVIEWEDIT_CUT:
        {
            copiedText = CFakeWebSite::ELEMENT_PASSWORD == e.kind ? CORE::CString::Empty : e.value;
            e.value.Clear();
            RedrawElement( focused );
            return true;
        }
        case WEBVIEWEDIT_SELECTALL:
            return true;
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CFakeWebView::OnClose( void )
{GUCEF_TRACE;

    m_decisionActions.clear();
    m_afterLoad.clear();
    m_ephemeralJar.clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEBVIEWTEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
