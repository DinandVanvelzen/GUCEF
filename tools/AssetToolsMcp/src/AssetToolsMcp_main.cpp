/*
 *  AssetToolsMcp: the asset tools as an MCP server over stdio, on a local asset store service
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

#include "gucef_essentials.h"
#include "CDynamicBuffer.h"
#include "CFileAccess.h"
#include "CPluginControl.h"
#include "CStdLogger.h"
#include "CObserver.h"
#include "dvcppfileutils.h"
#include "dvcppstringutils.h"
#include "DVCPPOSWRAP.h"
#include "gucefCORE_CCoreGlobal.h"
#include "CGUCEFApplication.h"
#include "gucefCORE_CPlatformNativeConsoleLogger.h"
#include "gucefCORE_CStandardOutputWriter.h"
#include "CDStoreCodecRegistry.h"
#include "gucefCORE_CTEventHandlerFunctor.h"
#include "CLogManager.h"
#include "gucefVFS_CVfsGlobal.h"
#include "gucefVFS_CVFS.h"
#include "gucefCOM_CComGlobal.h"
#include "gucefCOM_CMcpServer.h"
#include "gucefCOM_CMcpServerSettings.h"
#include "gucefCOM_CJsonRpcEndpoint.h"
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#include "gucefASSETSTORE_CAssetStoreServiceConfig.h"
#include "gucefASSETSTORE_CAssetToolSet.h"
#include "gucefASSETSTORE_CAssetToolSetSettings.h"
#include "AssetStoreConverterHook_CPipelineConverterHook.h"
#include "AssetStoreConverterHook_CConverterHookSettings.h"
#include "AssetStoreConverterHook_CPipelineOperationSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The options of the command line */
struct SOptions
{
    CORE::CString configFile;
    CORE::CString logFile;
    bool help;

    SOptions( void ) : configFile(), logFile(), help( false ) {}
};

bool
ParseOptions( int argc, char** argv, SOptions& options )
{GUCEF_TRACE;

    for ( int i=1; i<argc; ++i )
    {
        const CORE::CString argument = argv[ i ];
        if ( "--help" == argument || "-h" == argument )
            options.help = true;
        else
        if ( "--config" == argument && i + 1 < argc )
            options.configFile = argv[ ++i ];
        else
        if ( "--log-file" == argument && i + 1 < argc )
            options.logFile = argv[ ++i ];
        else
            return false;
    }
    if ( options.configFile.IsNULLOrEmpty() )
        options.configFile = CORE::GetEnv( "GUCEF_ASSETTOOLS_CONFIG" );
    return options.help || !options.configFile.IsNULLOrEmpty();
}

/** Loads a plugin of the folder of the program, the debug name first in a debug build */
bool
LoadPlugin( const CORE::CString& pluginName )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString firstPath = "$MODULEDIR$/" + pluginName + "_d";
    const CORE::CString secondPath = "$MODULEDIR$/" + pluginName;
    #else
    const CORE::CString firstPath = "$MODULEDIR$/" + pluginName;
    const CORE::CString secondPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return pluginControl.AddPluginFromDir( firstPath, "AssetToolsMcp", true ) || pluginControl.AddPluginFromDir( secondPath, "AssetToolsMcp", true );
}

bool
EnsureJsonCodec( void )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        return true;
    LoadPlugin( "dstorepluginJSONPARSER" );
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false );
}

/** The settings file: a JSON object with the sections below, nothing else */
bool
ReadSettingsFile( const CORE::CString& path, CORE::CDataNode& settings, CORE::CString& problem )
{GUCEF_TRACE;

    CORE::CDynamicBuffer content;
    if ( !CORE::FileExists( path ) || !content.LoadContentFromFile( path, 0, 4194304 ) )
    {
        problem = "the settings file " + path + " cannot be read";
        return false;
    }
    CORE::CString text;
    text.Set( (const char*) content.GetConstBufferPtr(), content.GetDataSize() );
    if ( !COM::CJsonRpcEndpoint::ParseJsonText( text, settings ) || GUCEF_DATATYPE_OBJECT != settings.GetNodeType() )
    {
        problem = "the settings file " + path + " is no JSON object";
        return false;
    }
    static const char* known[] = { "Plugins", "AssetToolSet", "AssetStoreService", "ConverterHook", "PipelineOperations", "McpServer" };
    CORE::CDataNode::const_iterator i = settings.ConstBegin();
    while ( i != settings.ConstEnd() )
    {
        bool isKnown = false;
        for ( UInt32 k=0; k<sizeof( known ) / sizeof( known[ 0 ] ); ++k )
            isKnown = isKnown || (*i)->GetName() == known[ k ];
        if ( !isKnown )
        {
            problem = "the settings file has the unknown section \"" + (*i)->GetName() + "\"";
            return false;
        }
        ++i;
    }
    if ( settings.GetAttCount() > 0 )
    {
        problem = "the settings file has members outside its sections";
        return false;
    }
    return true;
}

/** Mounts the folder of every root that has one, where the tools and the converter expect it; the mount names are kept */
bool
MountRoots( const ASSETSTORE::TAssetToolRootVector& roots, CORE::CString::StringVector& mounted, CORE::CString& problem )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( size_t i=0; i<roots.size(); ++i )
    {
        const ASSETSTORE::CAssetToolRoot& root = roots[ i ];
        if ( root.diskPath.IsNULLOrEmpty() )
            continue;
        const CORE::CString name = "assettools-root-" + root.id;
        bool already = false;
        for ( size_t m=0; m<mounted.size(); ++m )
            already = already || mounted[ m ] == name;
        if ( already )
            continue;
        CORE::CreateDirs( root.diskPath );
        const bool writable = root.AllowsWrites() || ASSETSTORE::CAssetToolRoot::KindIntermediate == root.kind ||
                              ASSETSTORE::CAssetToolRoot::KindOutput == root.kind || ASSETSTORE::CAssetToolRoot::KindWork == root.kind;
        if ( !vfs.AddRoot( root.diskPath, name, writable, false, root.vfsPath ) )
        {
            problem = "the folder of the root " + root.id + " cannot be mounted";
            return false;
        }
        mounted.push_back( name );
    }
    return true;
}

/** Ends the main loop of the application when the server asks to stop (stdin closed) */
class CShutdownListener : public CORE::CObserver
{
    public:

    explicit CShutdownListener( COM::CMcpServer& server )
    {
        typedef CORE::CTEventHandlerFunctor< CShutdownListener > TEventCallback;
        TEventCallback callback( this, &CShutdownListener::OnShutdownRequested );
        SubscribeTo( &server, COM::CMcpServer::ShutdownRequestedEvent, callback );
    }

    virtual ~CShutdownListener() GUCEF_VIRTUAL_OVERRIDE
    {
        UnsubscribeAllFromObserver();
    }

    void OnShutdownRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {GUCEF_TRACE;

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetToolsMcp: Stopping" );
        CORE::CCoreGlobal::Instance()->GetApplication().Stop();
    }
};

void
PrintUsage( void )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetToolsMcp: the asset tools as an MCP server over stdio. Options: --config FILE (or "
                                             "GUCEF_ASSETTOOLS_CONFIG), --log-file FILE, --help. The settings file is a JSON object with the "
                                             "sections Plugins (names of plugins next to the program), AssetToolSet, AssetStoreService, "
                                             "ConverterHook, PipelineOperations and McpServer. Exit codes: 0 after the input ended, 2 bad "
                                             "options, 3 no JSON codec or settings refused, 4 the service or the transport did not start" );
}

/** SIGTERM and SIGINT: only the async-signal-safe stop request */
void
StopSignalHandler( int signal )
{
    CORE::CGUCEFApplication::RequestStopFromSignal( signal );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    GUCEF_OSMAIN_SIGNAL_HANDLER( StopSignalHandler );

    SOptions options;
    const bool optionsValid = ParseOptions( argc, argv, options );

    // standard output belongs to the protocol: the claim makes the console logger write to standard error
    CORE::CStandardOutputWriter::ClaimExclusive();
    CORE::CPlatformNativeConsoleLogger console;
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );
    CORE::CStandardOutputWriter::ReleaseClaim();
    CORE::CFileAccess* logFile = GUCEF_NULL;
    CORE::CStdLogger* fileLogger = GUCEF_NULL;
    if ( !options.logFile.IsNULLOrEmpty() )
    {
        logFile = GUCEF_NEW CORE::CFileAccess( options.logFile, "w" );
        fileLogger = GUCEF_NEW CORE::CStdLogger( *logFile );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( fileLogger );
    }
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

    int exitCode = 0;
    CORE::CDataNode settings;
    CORE::CString problem;
    if ( !optionsValid || options.help )
    {
        PrintUsage();
        exitCode = options.help ? 0 : 2;
    }
    else
    if ( !EnsureJsonCodec() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetToolsMcp: The JSON codec plugin dstorepluginJSONPARSER is not available" );
        exitCode = 3;
    }
    else
    if ( !ReadSettingsFile( options.configFile, settings, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetToolsMcp: " << problem );
        exitCode = 3;
    }
    else
    {
        COM::CComGlobal::Instance();
        ASSETSTORE::CAssetStoreGlobal::Instance();
        CORE::CDataNodeSerializableSettings serializerSettings;

        // the plugins the settings name
        CORE::CString::StringVector plugins;
        settings.GetValuesOfChildByName( "Plugins", plugins );
        for ( size_t p=0; p<plugins.size(); ++p )
            if ( !LoadPlugin( plugins[ p ] ) )
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetToolsMcp: The plugin " << plugins[ p ] << " was not loaded" );

        // the settings of every part, the tool set strictly
        ASSETSTORE::CAssetToolSetSettings toolSetSettings;
        const CORE::CDataNode* toolSetNode = settings.FindChild( "AssetToolSet" );
        ASSETSTORECONVERTER::CConverterHookSettings hookSettings;
        const CORE::CDataNode* hookNode = settings.FindChild( "ConverterHook" );
        ASSETSTORECONVERTER::CPipelineOperationSettings operationSettings;
        const CORE::CDataNode* operationNode = settings.FindChild( "PipelineOperations" );
        ASSETSTORE::CAssetStoreServiceConfig serviceConfig;
        const CORE::CDataNode* serviceNode = settings.FindChild( "AssetStoreService" );
        COM::CMcpServerSettings serverSettings;
        serverSettings.serverName = "gucef-asset-tools";
        const CORE::CDataNode* serverNode = settings.FindChild( "McpServer" );
        bool settingsValid = true;
        if ( GUCEF_NULL != toolSetNode && !toolSetSettings.LoadStrict( *toolSetNode, problem ) )
            settingsValid = false;
        if ( settingsValid && GUCEF_NULL != hookNode && !hookSettings.Deserialize( *hookNode, serializerSettings ) )
        {
            problem = "the section ConverterHook is refused";
            settingsValid = false;
        }
        if ( settingsValid && GUCEF_NULL != operationNode && ( !operationSettings.Deserialize( *operationNode, serializerSettings ) || !operationSettings.IsValid( problem ) ) )
        {
            if ( problem.IsNULLOrEmpty() )
                problem = "the section PipelineOperations is refused";
            settingsValid = false;
        }
        if ( settingsValid && GUCEF_NULL != serviceNode && !serviceConfig.Deserialize( *serviceNode, serializerSettings ) )
        {
            problem = "the section AssetStoreService is refused";
            settingsValid = false;
        }
        if ( settingsValid && GUCEF_NULL != serverNode && !serverSettings.LoadConfig( *serverNode ) )
        {
            problem = "the section McpServer is refused";
            settingsValid = false;
        }

        CORE::CString::StringVector mounts;
        if ( !settingsValid )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetToolsMcp: The settings are refused: " << problem );
            exitCode = 3;
        }
        else
        if ( !MountRoots( toolSetSettings.roots, mounts, problem ) || !MountRoots( operationSettings.roots, mounts, problem ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetToolsMcp: " << problem );
            exitCode = 4;
        }
        else
        {
            // the service of this process with the converter pipeline as its converter
            ASSETSTORECONVERTER::CPipelineConverterHook hook( hookSettings );
            hook.SetOperationSettings( operationSettings );
            serviceConfig.implementationId = ASSETSTORE::CAssetStoreServiceConfig::LocalImplementationId;
            serviceConfig.converterHook = &hook;
            CORE::CString serviceError;
            ASSETSTORE::TAssetStoreServicePtr service = ASSETSTORE::CAssetStoreGlobal::Instance()->CreateService( serviceConfig, serviceError );
            if ( service.IsNULL() || ASSETSTORE::ASSETSTORE_STATUS_OK != service->Open() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetToolsMcp: The asset store service did not open: " <<
                                 ( service.IsNULL() ? serviceError : service->GetStatusMessage() ) );
                exitCode = 4;
            }
            else
            {
                COM::CMcpServer server;
                server.SetSettings( serverSettings );
                COM::CMcpImplementationInfo info;
                info.name = serverSettings.serverName;
                info.title = "GUCEF asset tools";
                info.version = "0.1.0";
                server.SetServerInfo( info );

                ASSETSTORE::CAssetToolSet toolSet( service, toolSetSettings );
                CORE::CString registerError;
                COM::CIMcpTransportPtr transport = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( "stdio" );
                CORE::CDataNode transportConfig( GUCEF_DATATYPE_OBJECT );
                transportConfig.SetAttribute( "type", CORE::CVariant( "stdio" ) );
                if ( !toolSet.RegisterInto( server, registerError ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetToolsMcp: The asset tools were not registered: " << registerError );
                    exitCode = 4;
                }
                else
                if ( transport.IsNULL() || !transport->LoadConfig( transportConfig ) || !server.AddTransport( transport ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetToolsMcp: The stdio transport cannot be made" );
                    exitCode = 4;
                }
                else
                {
                    CShutdownListener shutdownListener( server );
                    if ( server.Start() )
                    {
                        exitCode = CORE::CCoreGlobal::Instance()->GetApplication().main( argc, argv, true );
                        server.Stop( true );
                    }
                    else
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetToolsMcp: The stdio transport did not start" );
                        exitCode = 4;
                    }
                }
                toolSet.Unregister( 5000 );
                service->Close();
            }
            service.Unlink();
        }
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        for ( size_t m=0; m<mounts.size(); ++m )
            vfs.UnmountArchiveByName( mounts[ m ] );
    }

    CORE::CLogStreamScope::FlushLogs();
    if ( GUCEF_NULL != fileLogger )
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( fileLogger );
        GUCEF_DELETE fileLogger;
        GUCEF_DELETE logFile;
    }
    return exitCode;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------*/
