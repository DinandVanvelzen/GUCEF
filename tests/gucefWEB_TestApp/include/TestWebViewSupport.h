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

#ifndef GUCEF_WEBVIEWTEST_TESTWEBVIEWSUPPORT_H
#define GUCEF_WEBVIEWTEST_TESTWEBVIEWSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_WEB_WEBVIEW_H
#include "gucefWEB_WebView.h"
#define GUCEF_WEB_WEBVIEW_H
#endif /* GUCEF_WEB_WEBVIEW_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEBVIEWTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Waits for a command and gives its result (also of a failure); true for success */
inline bool
AwaitResult( const CORE::CFutureResult& future, WEB::CWebViewCommandResult& result, UInt32 timeoutMs = 10000 )
{
    WEB::CWebViewCommandResult::Await( future, timeoutMs, result );
    return result.IsSuccess();
}

/** The error a command ended with */
inline WEB::TWebViewError
AwaitError( const CORE::CFutureResult& future, UInt32 timeoutMs = 10000 )
{
    WEB::CWebViewCommandResult result;
    AwaitResult( future, result, timeoutMs );
    return result.error;
}

/** Waits for a command; true for success */
inline bool
AwaitOk( const CORE::CFutureResult& future, UInt32 timeoutMs = 10000 )
{
    WEB::CWebViewCommandResult result;
    return AwaitResult( future, result, timeoutMs );
}

/** The text a command ended with (a script result, a final address); empty on failure */
inline CORE::CString
AwaitText( const CORE::CFutureResult& future, UInt32 timeoutMs = 10000 )
{
    WEB::CWebViewCommandResult result;
    return AwaitResult( future, result, timeoutMs ) ? result.text : CORE::CString::Empty;
}

/** A view of the engine, NULL when it could not be made */
inline WEB::TWebViewPtr
CreateTestView( WEB::IWebViewEngine& engine, const WEB::CWebViewSettings& settings )
{
    WEB::CWebViewCommandResult result;
    if ( !AwaitResult( engine.CreateViewAsync( settings ), result ) )
        return WEB::TWebViewPtr();
    return result.view;
}

/** Settings of a test view: a profile, a small size, every host */
inline WEB::CWebViewSettings
MakeTestSettings( const CORE::CString& profileId = "test", UInt32 width = 320, UInt32 height = 240 )
{
    WEB::CWebViewSettings settings;
    settings.profile.profileId = profileId;
    settings.profile.proxyMode = WEB::WEBVIEWPROXY_NONE;
    settings.width = width;
    settings.height = height;
    return settings;
}

/** Settings of a sign-in view: only app.test, certificate errors refused */
inline WEB::CWebViewSettings
MakeSignInSettings( const CORE::CString& profileId = "signin-test" )
{
    WEB::CWebViewSettings settings = MakeTestSettings( profileId );
    settings.policy.navigation = WEB::WEBVIEWNAVIGATION_ALLOWLIST;
    settings.policy.allowList.AddEntry( "https://app.test" );
    settings.Sanitize( true );
    return settings;
}

/** Waits until the frame exchange of the view published at least 'count' frames */
inline bool
WaitForFrames( WEB::IWebView& view, UInt32 count, UInt32 timeoutMs = 5000 )
{
    const UInt64 start = MT::PrecisionTickCount();
    while ( view.GetFrameExchange().GetPublishedCount() < count )
    {
        if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) > (Float64) timeoutMs )
            return false;
        MT::PrecisionDelay( 2 );
    }
    return true;
}

/** The colour of a pixel of an RGBA pixel map */
inline const UInt8*
PixelOf( const IMAGE::CPixelMap& pixels, UInt32 x, UInt32 y )
{
    return static_cast< const UInt8* >( pixels.GetDataPtr() ) + ( (size_t) y * pixels.GetWidthInPixels() + x ) * 4;
}

inline bool
IsColour( const UInt8* pixel, UInt8 r, UInt8 g, UInt8 b, Int32 tolerance = 2 )
{
    const Int32 dr = (Int32) pixel[ 0 ] - r, dg = (Int32) pixel[ 1 ] - g, db = (Int32) pixel[ 2 ] - b;
    return dr >= -tolerance && dr <= tolerance && dg >= -tolerance && dg <= tolerance && db >= -tolerance && db <= tolerance;
}

/*
 *  The page queries of a sign-in, as the sign-in builds them (CWebPageScriptHelpers): fixed functions
 *  with one string literal argument. The fake engine knows them by their text, as a real engine runs them.
 */

inline CORE::CString
JsLiteral( const CORE::CString& text )
{
    CORE::CString literal = "\"";
    const char* c = text.C_String();
    for ( UInt32 i=0; !text.IsNULLOrEmpty() && i<text.ByteSize()-1; ++i )
    {
        if ( '"' == c[ i ] || '\\' == c[ i ] )
            literal += "\\";
        literal += CORE::CString( c + i, 1 );
    }
    return literal + "\"";
}

inline CORE::CString
ElementStateExpression( const CORE::CString& selector )
{
    return "(function(s){try{var e=document.querySelector(s);if(e===null){return 0;}var shown=e.getClientRects().length>0&&window.getComputedStyle(e).visibility!=='hidden';return (shown&&!e.disabled)?2:1;}catch(x){return -1;}})(" + JsLiteral( selector ) + ")";
}

inline CORE::CString
FocusAndClearExpression( const CORE::CString& selector )
{
    return "(function(s){try{var e=document.querySelector(s);if(e===null){return false;}e.scrollIntoView({block:'center'});e.focus();var d=Object.getOwnPropertyDescriptor(Object.getPrototypeOf(e),'value');if(d&&d.set){d.set.call(e,'');}else{e.value='';}e.dispatchEvent(new Event('input',{bubbles:true}));return document.activeElement===e;}catch(x){return false;}})(" + JsLiteral( selector ) + ")";
}

inline CORE::CString
ValueLengthExpression( const CORE::CString& selector )
{
    return "(function(s){try{var e=document.querySelector(s);if(e===null||typeof e.value!=='string'){return -1;}return e.value.length;}catch(x){return -1;}})(" + JsLiteral( selector ) + ")";
}

inline CORE::CString
ClickExpression( const CORE::CString& selector )
{
    return "(function(s){try{var e=document.querySelector(s);if(e===null){return false;}e.scrollIntoView({block:'center'});e.click();return true;}catch(x){return false;}})(" + JsLiteral( selector ) + ")";
}

inline CORE::CString
SubmitExpression( const CORE::CString& selector )
{
    return "(function(s){try{var e=document.querySelector(s);if(e===null){return false;}var f=(e.tagName==='FORM')?e:e.form;if(!f){return false;}if(typeof f.requestSubmit==='function'){f.requestSubmit();}else{f.submit();}return true;}catch(x){return false;}})(" + JsLiteral( selector ) + ")";
}

inline CORE::CString
ContainsTextExpression( const CORE::CString& text )
{
    return "(function(t){try{return document.body!==null&&document.body.innerText.indexOf(t)>=0;}catch(x){return false;}})(" + JsLiteral( text ) + ")";
}

inline CORE::CString
LocationExpression( void )
{
    return "(function(){try{return String(window.location.href);}catch(x){return '';}})()";
}

/** The isolated world the tests use */
inline const char*
TestWorld( void )
{
    return "gucef-test";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEBVIEWTEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEBVIEWTEST_TESTWEBVIEWSUPPORT_H ? */
