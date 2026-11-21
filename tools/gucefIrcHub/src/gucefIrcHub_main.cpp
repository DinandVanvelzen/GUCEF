/*
 *  gucefIrcHub: IRC hub with LLM bots for a family IRC server
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
#include <string.h>
#include <vector>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#include "gucefCORE_CPlatformNativeConsoleLogger.h"
#define GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#endif /* GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H ? */

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

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#include "gucefIrcHub_LlmBot.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEFIrcHub;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* DefaultConfigPath = "/etc/gucef-irc-hub/hub.json";
static const char* ThreadPoolName = "gucefIrcHub";
static const UInt32 StopWaitInMs = 5000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
EnsureJsonCodec( void )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        return true;
    const CORE::CString pluginName = "dstorepluginJSONPARSER";
    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "gucefIrcHub", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "gucefIrcHub", true );
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Attribute( const CORE::CDataNode& node, const char* name, const CORE::CString& defaultValue = CORE::CString::Empty )
{
    const CORE::CVariant& value = node.GetAttributeValue( name );
    return value.IsInitialized() ? value.AsString( defaultValue ) : defaultValue;
}

/*-------------------------------------------------------------------------*/

/** The value of the environment variable named by the attribute; empty when the attribute or the variable is not set */
static CORE::CString
SecretFromEnv( const CORE::CDataNode& node, const char* name )
{
    const CORE::CString variable = Attribute( node, name );
    return variable.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::GetEnv( variable );
}

/*-------------------------------------------------------------------------*/

static void
SplitList( const CORE::CString& text, CORE::CString::StringVector& items )
{
    items.clear();
    std::string current;
    const std::string all = text.STL_String() + ",";
    for ( size_t i=0; i<all.size(); ++i )
    {
        const char c = all[ i ];
        if ( ',' == c || ' ' == c )
        {
            if ( !current.empty() )
                items.push_back( CORE::CString( current ) );
            current.clear();
        }
        else
        {
            current += c;
        }
    }
}

/*-------------------------------------------------------------------------*/

struct SHubConfig
{
    COM::CIrcClientSettings server;
    CORE::CString operName;
    CORE::CString operPassword;
    UInt32 threads;
    std::vector< SBotSettings > bots;

    SHubConfig( void ) : server(), operName(), operPassword(), threads( 8 ), bots() {}
};

/*-------------------------------------------------------------------------*/

/**
 *  Reads the configuration, a JSON file:
 *  {
 *    "server": "ergo.irc.svc.cluster.local", "port": 6697,
 *    "tlsServerName": "name in the certificate of the server", "caFile": "certificate(s) to trust, PEM",
 *    "operName": "bot", "operPasswordEnv": "NAME_OF_ENV_VAR",
 *    "threads": 8,
 *    "bots": [ { "nick": "qwen", "passwordEnv": "NAME_OF_ENV_VAR", "displayName": "...", "room": "#qwen",
 *                "channels": "#family", "vendor": "llamacpp", "baseUrl": "http://...:8080/v1", "model": "",
 *                "apiKeyEnv": "NAME_OF_ENV_VAR", "effort": "", "maxTokens": 2048, "thinking": false,
 *                "dailyTokens": 0, "maxHistoryTurns": 20, "systemPrompt": "" } ]
 *  }
 *  Secrets are not in the file: the attributes ending in Env name the environment variables that hold them.
 *  A bot whose vendor needs a key that is not set is left out.
 */
static bool
LoadHubConfig( const CORE::CString& path, SHubConfig& config, CORE::CString& problem )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
    {
        problem = "the JSON codec is not available";
        return false;
    }
    if ( !CORE::FileExists( path ) )
    {
        problem = "the configuration file " + path + " does not exist";
        return false;
    }
    CORE::CDataNode root;
    if ( !codec->BuildDataTree( &root, path ) )
    {
        problem = "the configuration file " + path + " is not valid JSON";
        return false;
    }

    config.server.server = Attribute( root, "server" );
    config.server.port = (UInt16) root.GetAttributeValue( "port" ).AsUInt32( 6697 );
    config.server.useTls = root.GetAttributeValue( "tls" ).AsBool( true );
    const CORE::CString caFile = Attribute( root, "caFile" );
    if ( !caFile.IsNULLOrEmpty() )
        config.server.tls.SetCaBundlePath( caFile );
    const CORE::CString tlsServerName = Attribute( root, "tlsServerName" );
    if ( !tlsServerName.IsNULLOrEmpty() )
        config.server.tls.SetServerName( tlsServerName );
    config.operName = Attribute( root, "operName" );
    config.operPassword = SecretFromEnv( root, "operPasswordEnv" );
    config.threads = root.GetAttributeValue( "threads" ).AsUInt32( 8 );
    if ( config.server.server.IsNULLOrEmpty() )
    {
        problem = "the configuration has no server";
        return false;
    }

    const CORE::CDataNode* bots = root.FindChild( "bots" );
    const CORE::CDataNode* botNode = GUCEF_NULL != bots ? bots->GetFirstChild() : GUCEF_NULL;
    while ( GUCEF_NULL != botNode )
    {
        SBotSettings bot;
        bot.nick = Attribute( *botNode, "nick" );
        const CORE::CString passwordEnv = Attribute( *botNode, "passwordEnv" );
        bot.password = passwordEnv.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::GetEnv( passwordEnv );
        bot.displayName = Attribute( *botNode, "displayName", bot.nick );
        bot.room = Attribute( *botNode, "room" );
        SplitList( Attribute( *botNode, "channels" ), bot.channels );
        bot.systemPrompt = Attribute( *botNode, "systemPrompt" );
        bot.llm.vendor = Attribute( *botNode, "vendor" );
        bot.llm.baseUrl = Attribute( *botNode, "baseUrl" );
        bot.llm.model = Attribute( *botNode, "model" );
        bot.llm.apiKey = SecretFromEnv( *botNode, "apiKeyEnv" );
        bot.llm.effort = Attribute( *botNode, "effort" );
        bot.llm.maxTokens = botNode->GetAttributeValue( "maxTokens" ).AsUInt32( bot.llm.maxTokens );
        bot.llm.thinking = botNode->GetAttributeValue( "thinking" ).AsBool( false );
        bot.llm.timeoutInMs = botNode->GetAttributeValue( "timeoutSecs" ).AsUInt32( bot.llm.timeoutInMs / 1000 ) * 1000;
        bot.dailyTokensPerAccount = botNode->GetAttributeValue( "dailyTokens" ).AsUInt32( 0 );
        bot.maxHistoryTurns = botNode->GetAttributeValue( "maxHistoryTurns" ).AsUInt32( bot.maxHistoryTurns );
        bot.maxChannelReplyLines = botNode->GetAttributeValue( "maxChannelReplyLines" ).AsUInt32( bot.maxChannelReplyLines );
        bot.maxPrivateReplyLines = botNode->GetAttributeValue( "maxPrivateReplyLines" ).AsUInt32( bot.maxPrivateReplyLines );
        botNode = botNode->GetNext();

        if ( bot.nick.IsNULLOrEmpty() )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefIrcHub: A bot without a nick is left out" );
            continue;
        }
        if ( !IsKnownVendor( bot.llm.vendor ) || bot.llm.baseUrl.IsNULLOrEmpty() )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefIrcHub: Bot " + bot.nick + " is left out: vendor \"" + bot.llm.vendor +
                               "\" is not one of anthropic, openai, openai-compatible, llamacpp, or it has no baseUrl" );
            continue;
        }
        if ( bot.password.IsNULLOrEmpty() )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefIrcHub: Bot " + bot.nick + " is left out: the environment variable " +
                               passwordEnv + " with its password is not set" );
            continue;
        }
        if ( bot.llm.apiKey.IsNULLOrEmpty() && "llamacpp" != bot.llm.vendor )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefIrcHub: Bot " + bot.nick + " is left out: no API key for " + bot.llm.vendor );
            continue;
        }
        config.bots.push_back( bot );
    }
    if ( config.bots.empty() )
    {
        problem = "the configuration has no usable bots";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static void
PrintUsage( void )
{
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefIrcHub: LLM bots for an IRC server. Options: --config PATH (default " +
                      CORE::CString( DefaultConfigPath ) + ", or the environment variable IRC_HUB_CONFIG)" );
}

/*-------------------------------------------------------------------------*/

/** SIGTERM and SIGINT, the way Kubernetes stops the container: only the async-signal-safe stop request */
static void
StopSignalHandler( int signal )
{
    CORE::CGUCEFApplication::RequestStopFromSignal( signal );
}

/*-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    GUCEF_OSMAIN_SIGNAL_HANDLER( StopSignalHandler );

    CORE::CString configPath = CORE::GetEnv( "IRC_HUB_CONFIG" );
    bool help = false;
    bool optionsValid = true;
    for ( int i=1; i<argc; ++i )
    {
        if ( 0 == strcmp( argv[ i ], "--config" ) && i + 1 < argc )
            configPath = argv[ ++i ];
        else
        if ( 0 == strcmp( argv[ i ], "--help" ) || 0 == strcmp( argv[ i ], "-h" ) )
            help = true;
        else
            optionsValid = false;
    }
    if ( configPath.IsNULLOrEmpty() )
        configPath = DefaultConfigPath;

    // a service: log lines on stderr, which is what kubectl logs shows, unless GUCEF_CONSOLE_LOGGER says otherwise
    if ( GUCEF_NULL == getenv( "GUCEF_CONSOLE_LOGGER" ) )
        setenv( "GUCEF_CONSOLE_LOGGER", "stderr", 0 );
    CORE::CPlatformNativeConsoleLogger console;
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

    int exitCode = 0;
    SHubConfig config;
    CORE::CString problem;
    if ( !optionsValid || help )
    {
        PrintUsage();
        exitCode = help ? 0 : 2;
    }
    else
    if ( !EnsureJsonCodec() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefIrcHub: The JSON codec plugin dstorepluginJSONPARSER is not available" );
        exitCode = 3;
    }
    else
    if ( !LoadHubConfig( configPath, config, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefIrcHub: " + problem );
        exitCode = 2;
    }
    else
    {
        COM::CComGlobal::Instance();            // the IRC client and its transports
        WEB::CWebGlobal::Instance();            // the HTTP client for the models

        // a request to a model takes up a thread for as long as the model takes
        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( ThreadPoolName, true );
        if ( !threadPool.IsNULL() )
        {
            threadPool->SetDesiredMinNrOfWorkerThreads( config.threads );
            threadPool->SetDesiredMaxTotalNrOfThreads( (Int32) config.threads );
        }

        CORE::CString::StringVector botNicks;
        for ( size_t i=0; i<config.bots.size(); ++i )
            botNicks.push_back( config.bots[ i ].nick );

        std::vector< CLlmBot* > bots;
        for ( size_t i=0; i<config.bots.size(); ++i )
        {
            CORE::CString::StringVector otherBots;
            for ( size_t n=0; n<botNicks.size(); ++n )
            {
                if ( n != i )
                    otherBots.push_back( botNicks[ n ] );
            }
            CLlmBot* bot = GUCEF_NEW CLlmBot( config.bots[ i ], config.server, config.operName, config.operPassword, otherBots, ThreadPoolName );
            bots.push_back( bot );
            bot->Start();
        }

        exitCode = CORE::CCoreGlobal::Instance()->GetApplication().main( argc, argv, true );

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefIrcHub: Stopping" );
        for ( size_t i=0; i<bots.size(); ++i )
            bots[ i ]->Stop( StopWaitInMs );
        for ( size_t i=0; i<bots.size(); ++i )
            GUCEF_DELETE bots[ i ];
    }

    CORE::CLogStreamScope::FlushLogs();
    return exitCode;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------*/
