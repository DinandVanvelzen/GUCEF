/*
 *  gucefMcpCi_TestApp: tests of the gucefMcpCi MCP server
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

#include <stdlib.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_COREECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_COREECODECTYPES_H
#endif /* GUCEF_CORE_COREECODECTYPES_H ? */

#ifndef GUCEF_CORE_CICODECINSTANCE_H
#include "gucefCORE_CICodecInstance.h"
#define GUCEF_CORE_CICODECINSTANCE_H
#endif /* GUCEF_CORE_CICODECINSTANCE_H ? */

#ifndef GUCEF_TEST_CHILD_PROCESS_H
#include "gucef_test_child_process.h"
#define GUCEF_TEST_CHILD_PROCESS_H
#endif /* GUCEF_TEST_CHILD_PROCESS_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#include "TestMcpCiSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace MCPCITEST {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
FindServer( void )
{
    CORE::CString dir = CORE::RelativePath( "$MODULEDIR$" );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    const char* names[] = { "gucefMcpCi.exe", "gucefMcpCi_d.exe" };
    #else
    const char* names[] = { "gucefMcpCi", "gucefMcpCi_d" };
    #endif
    for ( int i=0; i<2; ++i )
    {
        CORE::CString candidate = CORE::CombinePath( dir, names[ i ] );
        if ( CORE::FileExists( candidate ) )
            return candidate;
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CProcess::TStringMap
ChildEnvironment( void )
{
    CORE::CProcess::TStringMap environment;
    environment[ "GUCEF_CONSOLE_LOGGER" ] = "stderr";
    TESTFW::KeepChildExitCodeUnderSanitizers( environment );
    return environment;
}

/*-------------------------------------------------------------------------*/

CORE::CString
Json( const CORE::CString& text )
{
    CORE::CString result = "\"";
    const char* c = text.C_String();
    for ( UInt32 i=0; '\0' != c[ i ]; ++i )
    {
        if ( '"' == c[ i ] || '\\' == c[ i ] )
            result += '\\';
        if ( '\n' == c[ i ] )
        {
            result += "\\n";
            continue;
        }
        if ( '\t' == c[ i ] )
        {
            result += "\\t";
            continue;
        }
        result += c[ i ];
    }
    return result + "\"";
}

/*-------------------------------------------------------------------------*/

CORE::CString
Trimmed( const CORE::CString& text )
{
    const char* c = text.C_String();
    UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    UInt32 start = 0;
    while ( start < size && ( ' ' == c[ start ] || '\t' == c[ start ] || '\r' == c[ start ] || '\n' == c[ start ] ) )
        ++start;
    while ( size > start && ( ' ' == c[ size-1 ] || '\t' == c[ size-1 ] || '\r' == c[ size-1 ] || '\n' == c[ size-1 ] ) )
        --size;
    return CORE::CString( c + start, size - start );
}

/*-------------------------------------------------------------------------*/

bool
Contains( const CORE::CString& text, const CORE::CString& part )
{
    return text.HasSubstr( part ) >= 0;
}

/*-------------------------------------------------------------------------*/

bool
WriteFile( const CORE::CString& path, const CORE::CString& content )
{
    CORE::CreatePathDirectories( path );
    return CORE::WriteFileAtomically( path, content.C_String(), content.ByteSize() > 0 ? content.ByteSize() - 1 : 0 );
}

/*-------------------------------------------------------------------------*/

bool
ReadFile( const CORE::CString& path, CORE::CString& content )
{
    if ( !CORE::FileExists( path ) )
        return false;
    return CORE::LoadTextFileAsString( path, content, false );
}

/*-------------------------------------------------------------------------*/

bool
WriteOwnerOnlyFile( const CORE::CString& path, const CORE::CString& content )
{
    if ( CORE::FileExists( path ) )
        CORE::DeleteFile( path );
    CORE::CString problem;
    return CORE::CreateOwnerOnlyFile( path, content.C_String(), content.ByteSize() > 0 ? content.ByteSize() - 1 : 0, problem );
}

/*-------------------------------------------------------------------------*/

Int32
RunCapture( const CORE::CString& exe, const CORE::CProcess::TStringVector& args, CORE::CString& output, const CORE::CString& workingDir, UInt32 timeoutInMs )
{
    output.Clear();
    CORE::CProcess process;
    CORE::CProcess::TStringMap env;
    env[ "GIT_TERMINAL_PROMPT" ] = "0";
    if ( !process.LaunchWithPipes( exe, args, true, true, true, workingDir, env ) )
        return -1;
    process.CloseStdinPipe();

    CORE::CProcessPipe* pipes[ 2 ] = { process.GetStdoutPipe(), process.GetStderrPipe() };
    bool open[ 2 ] = { GUCEF_NULL != pipes[ 0 ], GUCEF_NULL != pipes[ 1 ] };
    const UInt64 start = MT::PrecisionTickCount();
    const UInt64 resolution = MT::PrecisionTimerResolution();
    char buffer[ 8192 ];
    while ( open[ 0 ] || open[ 1 ] )
    {
        for ( int i=0; i<2; ++i )
        {
            if ( !open[ i ] )
                continue;
            if ( pipes[ i ]->WaitForData( 20 ) )
            {
                const UInt32 got = pipes[ i ]->ReadSome( buffer, sizeof( buffer ) );
                if ( 0 == got )
                    open[ i ] = false;
                else
                    output.Append( buffer, got );
            }
        }
        if ( resolution > 0 && ( ( MT::PrecisionTickCount() - start ) * 1000 ) / resolution > timeoutInMs )
        {
            process.Terminate( false, 0 );
            return -1;
        }
    }
    Int32 exitCode = -1;
    if ( !process.WaitForExit( 10000, exitCode ) )
    {
        process.Terminate( false, 0 );
        return -1;
    }
    return exitCode;
}

/*-------------------------------------------------------------------------*/

static CORE::CProcess::TStringVector
GitArgs( const CORE::CString& dir, const char* a, const char* b, const char* c, const char* d, const char* e, const char* f )
{
    CORE::CProcess::TStringVector args;
    args.push_back( "-C" );
    args.push_back( dir );
    const char* parts[] = { a, b, c, d, e, f };
    for ( int i=0; i<6; ++i )
        if ( GUCEF_NULL != parts[ i ] )
            args.push_back( parts[ i ] );
    return args;
}

/*-------------------------------------------------------------------------*/

Int32
Git( const CORE::CString& dir, const char* a, const char* b, const char* c, const char* d, const char* e, const char* f )
{
    CORE::CString output;
    return RunCapture( "git", GitArgs( dir, a, b, c, d, e, f ), output );
}

/*-------------------------------------------------------------------------*/

CORE::CString
GitOut( const CORE::CString& dir, const char* a, const char* b, const char* c, const char* d, const char* e, const char* f )
{
    CORE::CString output;
    if ( 0 != RunCapture( "git", GitArgs( dir, a, b, c, d, e, f ), output ) )
        return CORE::CString::Empty;
    return Trimmed( output );
}

/*-------------------------------------------------------------------------*/

bool
GitCommit( const CORE::CString& dir, const CORE::CString& message )
{
    CORE::CProcess::TStringVector args;
    args.push_back( "-C" );
    args.push_back( dir );
    args.push_back( "-c" );
    args.push_back( "user.name=test" );
    args.push_back( "-c" );
    args.push_back( "user.email=test@example.com" );
    args.push_back( "-c" );
    args.push_back( "commit.gpgSign=false" );
    args.push_back( "commit" );
    args.push_back( "-q" );
    args.push_back( "--no-verify" );
    args.push_back( "-m" );
    args.push_back( message );
    CORE::CString output;
    return 0 == RunCapture( "git", args, output );
}

/*-------------------------------------------------------------------------*/

bool
GitAvailable( void )
{
    CORE::CProcess::TStringVector args;
    args.push_back( "--version" );
    CORE::CString output;
    return 0 == RunCapture( "git", args, output );
}

/*-------------------------------------------------------------------------*/

bool
Gzip( const CORE::CString& text, CORE::CDynamicBuffer& compressed )
{
    CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
    if ( registry.TryGetCodec( CORE::CoreCodecTypes::CompressionCodec, "gzip" ).IsNULL() )
    {
        #ifdef GUCEF_DEBUG_MODE
        const CORE::CString first = "$MODULEDIR$/codecspluginZLIB_d";
        const CORE::CString second = "$MODULEDIR$/codecspluginZLIB";
        #else
        const CORE::CString first = "$MODULEDIR$/codecspluginZLIB";
        const CORE::CString second = "$MODULEDIR$/codecspluginZLIB_d";
        #endif
        CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
        if ( !pluginControl.AddPluginFromDir( first, "test", true ) )
            pluginControl.AddPluginFromDir( second, "test", true );
    }
    CORE::CCodecRegistry::TICodecPtr codec = registry.TryGetCodec( CORE::CoreCodecTypes::CompressionCodec, "gzip" );
    CORE::CICodecInstancePtr instance;
    if ( codec.IsNULL() || !codec->CreateInstance( CORE::CValueList(), instance ) || instance.IsNULL() )
        return false;
    compressed.Clear();
    return instance->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0, GUCEF_NULL, compressed );
}

/*-------------------------------------------------------------------------*/

Int32
McpHttpPost( const CORE::CString& url, const CORE::CString& token, const CORE::CString& body, const CORE::CString& caBundle, CORE::CString& responseText )
{
    WEB::CHTTPClient client;
    WEB::CHttpRequestData request;
    if ( !request.SetUrl( url ) )
        return 0;
    request.requestType = "POST";
    request.contentRepresentation = "application/json";
    request.resourceRepresentations.push_back( "application/json" );
    request.resourceRepresentations.push_back( "text/event-stream" );
    if ( !token.IsNULLOrEmpty() )
        request.otherHeaders[ "Authorization" ] = "Bearer " + token;
    request.otherHeaders[ "MCP-Protocol-Version" ] = "2025-11-25";
    request.content.CopyFrom( body.Length(), body.C_String() );
    WEB::CHttpClientRequestOptions options;
    options.caBundlePath = caBundle;
    options.totalTimeoutInMs = 60000;
    options.maxResponseBodySize = 4 * 1024 * 1024;
    WEB::CHttpResponseData response;
    if ( !client.Execute( request, options, response ) )
        return 0;
    responseText = CORE::CString( static_cast< const char* >( response.content.GetConstBufferPtr() ), response.content.GetDataSize() );
    return response.statusCode;
}

/*-------------------------------------------------------------------------*/

CORE::CString
PercentDecode( const CORE::CString& text )
{
    CORE::CString result;
    const char* c = text.C_String();
    for ( UInt32 i=0; '\0' != c[ i ]; ++i )
    {
        if ( '%' == c[ i ] && '\0' != c[ i+1 ] && '\0' != c[ i+2 ] )
        {
            char hex[ 3 ] = { c[ i+1 ], c[ i+2 ], '\0' };
            result += (char) ::strtol( hex, GUCEF_NULL, 16 );
            i += 2;
        }
        else
            result += c[ i ];
    }
    return result;
}

/*-------------------------------------------------------------------------*/

}; /* namespace MCPCITEST */

/*-------------------------------------------------------------------------*/
