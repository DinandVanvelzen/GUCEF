/*
 *  SocketSink: Simple socket app to dump incoming traffic to disk for analysis
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_PROCESSARGUMENTS_H
#include "gucefCORE_ProcessArguments.h"
#define GUCEF_CORE_PROCESSARGUMENTS_H
#endif /* GUCEF_CORE_PROCESSARGUMENTS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CONFIGSTORE_H
#endif /* GUCEF_CORE_CONFIGSTORE_H ? */

#ifndef GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H
#include "gucefCORE_CPlatformNativeConsoleWindow.h"
#define GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H
#endif /* GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H ? */

#ifndef GUCEF_CORE_CGLOBALCONFIGVALUELIST_H
#include "gucefCORE_CGlobalConfigValueList.h"
#define GUCEF_CORE_CGLOBALCONFIGVALUELIST_H
#endif /* GUCEF_CORE_CGLOBALCONFIGVALUELIST_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

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

class SocketSink : public CORE::CObserver
{
    private:

    typedef CORE::CTEventHandlerFunctor< SocketSink > TEventCallback;

    COMCORE::CUDPSocket m_udpSocket;
    COMCORE::CHostAddress m_udpFwdAddr;
    COMCORE::CTCPServerSocket m_tcpServerSocket;
    CORE::CFileAccess m_udpSinkFile;
    CORE::CFileAccess m_tcpSinkFile;
    bool m_doUdpForwarding;
    bool m_udpAddNewLine;
    CORE::Int64 m_flushThreshold;
    CORE::CDynamicBuffer m_udpFileBuffer;
    CORE::CTimer m_udpFlushTimer;

    void
    OnUDPSocketError( CORE::CNotifier* notifier   ,
                      const CORE::CEvent& eventID ,
                      CORE::CICloneable* evenData )
    {GUCEF_TRACE;

        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: UDP Socket experienced an error" );
    }

    void
    OnUDPSocketClosed( CORE::CNotifier* notifier   ,
                       const CORE::CEvent& eventID ,
                       CORE::CICloneable* evenData )
    {GUCEF_TRACE;

        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: UDP Socket has been closed" );
    }

    void
    OnUDPSocketOpened( CORE::CNotifier* notifier   ,
                       const CORE::CEvent& eventID ,
                       CORE::CICloneable* evenData )
    {GUCEF_TRACE;

        // We don't want to lose UDP packets, thus update faster then we normally would
        CORE::CCoreGlobal::Instance()->GetApplication().GetPulseGenerator()->RequestPeriodicPulses( this, 1 );
        
        m_udpFlushTimer.SetEnabled( true );

        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: UDP Socket has been opened" );
    }

    void
    FlushUdpBuffer( void )
    {
        // flush memory buffer
        if ( m_udpSinkFile.IsValid() )
        {
            m_udpSinkFile.Write( m_udpFileBuffer.GetConstBufferPtr(), m_udpFileBuffer.GetDataSize(), 1 );
            m_udpFileBuffer.Clear( true );
            m_udpSinkFile.Flush();
        }
    }

    void
    OnUDPPacketsRecieved( CORE::CNotifier* notifier   ,
                          const CORE::CEvent& eventID ,
                          CORE::CICloneable* evenData )
    {GUCEF_TRACE;

        COMCORE::CUDPSocket::UDPPacketsRecievedEventData* udpPacketData = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( evenData );
        if ( GUCEF_NULL != udpPacketData )
        {
            const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = udpPacketData->GetData();

            for ( CORE::UInt32 i=0; i<data.packetsReceived; ++i )
            {
                const CORE::CDynamicBuffer& udpPacketBuffer = data.packets[ i ].dataBuffer.GetData();

                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: UDP Socket received a packet from " + data.packets[ i ].sourceAddress.AddressAndPortAsString() );

                bool writeToFile = true;
                if ( m_flushThreshold > 0 ) 
                {
                    if ( m_udpFileBuffer.GetDataSize() < m_flushThreshold ) 
                    {
                        // add to in-memory buffer
                        m_udpFileBuffer.Append( udpPacketBuffer.GetConstBufferPtr(), udpPacketBuffer.GetDataSize(), true );
                        if ( m_udpAddNewLine )
                        {
                            static const char newLine = '\n';
                            m_udpFileBuffer.Append( &newLine, 1, true );
                        }
                        writeToFile = false;
                    }
                    else
                    {
                        // flush memory buffer
                        FlushUdpBuffer();
                    }
                }

                if ( writeToFile && m_udpSinkFile.IsValid() )
                {
                    m_udpSinkFile.Write( udpPacketBuffer.GetConstBufferPtr(), udpPacketBuffer.GetDataSize(), 1 );
                    if ( m_udpAddNewLine )
                    {
                        static const char newLine = '\n';
                        m_udpSinkFile.Write( &newLine, 1, 1 );
                    }
                    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "SocketSink: Wrote UDP packet to file: " + CORE::UInt32ToString( udpPacketBuffer.GetDataSize() ) + " bytes in payload" );
                }

                if ( m_doUdpForwarding )
                {
                    m_udpSocket.SendPacketTo( m_udpFwdAddr.GetFirstIPv4Address(), udpPacketBuffer.GetConstBufferPtr(), udpPacketBuffer.GetDataSize() );
                    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "SocketSink: Forwarded UDP packet" );
                }
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: UDP Socket has a data received event but no data was provided" );
        }
    }

    void
    OnTCPServerSocketOpened( CORE::CNotifier* notifier   ,
                             const CORE::CEvent& eventID ,
                             CORE::CICloneable* evenData )
    {GUCEF_TRACE;

        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: TCP Server Socket opened" );
    }

    void
    OnTCPServerSocketClosed( CORE::CNotifier* notifier   ,
                             const CORE::CEvent& eventID ,
                             CORE::CICloneable* evenData )
    {GUCEF_TRACE;

        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: TCP Server Socket closed" );
    }

    void
    OnTCPServerSocketError( CORE::CNotifier* notifier   ,
                            const CORE::CEvent& eventID ,
                            CORE::CICloneable* evenData )
    {GUCEF_TRACE;

        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: TCP Server Socket experienced an error" );
    }

    void
    OnTCPServerSocketMaxConnectionsChanged( CORE::CNotifier* notifier   ,
                                            const CORE::CEvent& eventID ,
                                            CORE::CICloneable* evenData )
    {GUCEF_TRACE;

        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: TCP Server Socket changed max client connections" );
    }

    void
    OnTCPServerSocketClientConnected( CORE::CNotifier* notifier   ,
                                      const CORE::CEvent& eventID ,
                                      CORE::CICloneable* evenData )
    {GUCEF_TRACE;

        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: TCP Server Socket has a new client connection" );
    }

    void
    OnTCPServerSocketClientDisconnected( CORE::CNotifier* notifier   ,
                                         const CORE::CEvent& eventID ,
                                         CORE::CICloneable* evenData )
    {GUCEF_TRACE;

        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: TCP Server Socket disconnected a client connection" );
    }

    void
    OnTCPServerSocketClientError( CORE::CNotifier* notifier   ,
                                  const CORE::CEvent& eventID ,
                                  CORE::CICloneable* evenData )
    {GUCEF_TRACE;

        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: TCP Server Socket experienced an error on a client connection" );
    }

    void
    OnUdpFlushTimerUpdate( CORE::CNotifier* notifier   ,
                           const CORE::CEvent& eventID ,
                           CORE::CICloneable* evenData )
    {GUCEF_TRACE;

        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: Flush timer interval triggered" );
        FlushUdpBuffer();
    }

    void
    RegisterEventHandlers( void )
    {GUCEF_TRACE;

        // Register UDP socket event handlers
        TEventCallback callback( this, &SocketSink::OnUDPSocketError );
        SubscribeTo( &m_udpSocket                             ,
                     COMCORE::CUDPSocket::UDPSocketErrorEvent ,
                     callback                                 );
        TEventCallback callback2( this, &SocketSink::OnUDPSocketClosed );
        SubscribeTo( &m_udpSocket                              ,
                     COMCORE::CUDPSocket::UDPSocketClosedEvent ,
                     callback2                                 );
        TEventCallback callback3( this, &SocketSink::OnUDPSocketOpened );
        SubscribeTo( &m_udpSocket                              ,
                     COMCORE::CUDPSocket::UDPSocketOpenedEvent ,
                     callback3                                 );
        TEventCallback callback4( this, &SocketSink::OnUDPPacketsRecieved );
        SubscribeTo( &m_udpSocket                                 ,
                     COMCORE::CUDPSocket::UDPPacketsRecievedEvent ,
                     callback4                                    );

        // Register TCP server socket event handlers
        TEventCallback callback5( this, &SocketSink::OnTCPServerSocketOpened );
        SubscribeTo( &m_tcpServerSocket                                 ,
                     COMCORE::CTCPServerSocket::ServerSocketOpenedEvent ,
                     callback5                                          );
        TEventCallback callback6( this, &SocketSink::OnTCPServerSocketClosed );
        SubscribeTo( &m_tcpServerSocket                                 ,
                     COMCORE::CTCPServerSocket::ServerSocketClosedEvent ,
                     callback6                                          );
        TEventCallback callback7( this, &SocketSink::OnTCPServerSocketError );
        SubscribeTo( &m_tcpServerSocket                                ,
                     COMCORE::CTCPServerSocket::ServerSocketErrorEvent ,
                     callback7                                         );
        TEventCallback callback8( this, &SocketSink::OnTCPServerSocketMaxConnectionsChanged );
        SubscribeTo( &m_tcpServerSocket                                                ,
                     COMCORE::CTCPServerSocket::ServerSocketMaxConnectionsChangedEvent ,
                     callback8                                                         );
        TEventCallback callback9( this, &SocketSink::OnTCPServerSocketClientConnected );
        SubscribeTo( &m_tcpServerSocket                              ,
                     COMCORE::CTCPServerSocket::ClientConnectedEvent ,
                     callback9                                       );
        TEventCallback callback10( this, &SocketSink::OnTCPServerSocketClientDisconnected );
        SubscribeTo( &m_tcpServerSocket                                 ,
                     COMCORE::CTCPServerSocket::ClientDisconnectedEvent ,
                     callback10                                         );
        TEventCallback callback11( this, &SocketSink::OnTCPServerSocketClientError );
        SubscribeTo( &m_tcpServerSocket                          ,
                     COMCORE::CTCPServerSocket::ClientErrorEvent ,
                     callback11                                  );

        // Register timer event handler
        TEventCallback callback12( this, &SocketSink::OnUdpFlushTimerUpdate );
        SubscribeTo( &m_udpFlushTimer               ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback12                     );
    }

    public:

    virtual const CORE::CString& GetClassTypeName( void ) const
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "SocketSink";
        return classTypeName;
    }

    bool Setup( const CORE::CValueList& keyValueList )
    {GUCEF_TRACE;

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "SocketSink: Setup starting. " + CORE::UInt32ToString( keyValueList.GetKeyCount() ) + " Config keys were provided" );

        CORE::CPlatformNativeConsoleWindow consoleWindow;
        bool provideConsoleWindow = false;
        CORE::CVariant value;
        if ( keyValueList.TryGetValue( "ProvideConsoleWindow", value ) )
        {
            provideConsoleWindow = value.AsBool( provideConsoleWindow );
            if ( provideConsoleWindow )
            {
                consoleWindow.CreateConsole();
            }
        }

        bool enableUdp = false;
        if ( keyValueList.TryGetValue( "EnableUdp", value ) )
        {
            enableUdp = value.AsBool( enableUdp );
        }
        bool enableTcp = false;
        if ( keyValueList.TryGetValue( "EnableTcp", value ) )
        {
            enableTcp = value.AsBool( enableTcp );
        }

        CORE::Int32 minLogLevel = CORE::LOGLEVEL_BELOW_NORMAL;
        if ( keyValueList.TryGetValue( "MinimalLogLevel", value ) )
        {
            minLogLevel = value.AsInt32( minLogLevel );
            CORE::CCoreGlobal::Instance()->GetLogManager().SetMinLogLevel( minLogLevel );
        }

        CORE::UInt32 flushTimerInterval = 360000;
        if ( keyValueList.TryGetValue( "UdpFlushTimerInterval", value ) )
        {
            minLogLevel = value.AsUInt32( flushTimerInterval );
            m_udpFlushTimer.SetInterval( flushTimerInterval );
        }

        if ( enableUdp )
        {
            CORE::CString udpOutputFilePath = CORE::RelativePath( keyValueList.GetValueAlways( "UdpOutputFile" ) );
            if ( !udpOutputFilePath.IsNULLOrEmpty() && !m_udpSinkFile.Open( udpOutputFilePath, "wb" ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: Failed to open udp output file at: " + udpOutputFilePath );
            }

            if ( keyValueList.TryGetValue( "UdpBufferSize", value ) )
            {
                m_udpSocket.SetRecievedDataBufferSize( value.AsUInt32() );
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: Set UDP buffer size as " + value.AsString() + ". For non-payload-delimited packets this should match the packet size" );
            }

            m_udpAddNewLine = false;
            if ( keyValueList.TryGetValue( "AddNewLineAfterUdpData", value ) )
            {
                m_udpAddNewLine = value.AsBool( m_udpAddNewLine );
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: Add new line is true for UDP: Make sure you are sending text!" );
            }

            m_flushThreshold = -1;
            if ( keyValueList.TryGetValue( "UdpFlushThreshold", value ) )
            {
                m_flushThreshold = value.AsInt64( m_flushThreshold );
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: Set UDP flush threshold to " + value.AsString() );
            }

            m_udpFileBuffer.SetAutoEnlarge( true );
            if ( keyValueList.TryGetValue( "UdpFileBufferSize", value ) )
            {
                m_udpFileBuffer.SetBufferSize( value.AsUInt32() );
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: Set UDP file buffer size to: " + value.AsString() );
            }

            if ( keyValueList.TryGetValue( "UdpPort", value ) )
            {
                if ( m_udpSocket.Open( value.AsUInt16() ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: Opened UDP port " + value.AsString() );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: Failed to open udp port " + value.AsString() );
                }
            }

            m_doUdpForwarding = false;
            if ( keyValueList.TryGetValue( "DoUdpForwarding", value ) )
            {
                m_doUdpForwarding = value.AsBool( m_doUdpForwarding );
            }

            CORE::UInt16 fwdPort = CORE::StringToUInt16( keyValueList.GetValueAlways( "UdpFwdPort" ) );
            m_udpFwdAddr.SetHostnameAndPort( keyValueList.GetValueAlways( "UdpFwdAddr" ), fwdPort );
        }

        if ( enableTcp )
        {
            CORE::CString tcpOutputFilePath = CORE::RelativePath( keyValueList.GetValueAlways( "TcpOutputFile" ) );
            if ( !tcpOutputFilePath.IsNULLOrEmpty() && !m_tcpSinkFile.Open( tcpOutputFilePath, "wb" ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: Failed to open udp output file at: " + tcpOutputFilePath );
            }

            CORE::CString tcpPortStr = keyValueList.GetValueAlways( "TcpPort" );
            if ( !tcpPortStr.IsNULLOrEmpty() )
            {
                if ( m_tcpServerSocket.ListenOnPort( CORE::StringToUInt16( tcpPortStr ) ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: Opened TCP server port " + tcpPortStr );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: Failed to open tcp server port " + tcpPortStr );
                }
            }
        }

        return m_udpSocket.IsActive() || m_tcpServerSocket.IsActive();
    }

    SocketSink()
        : CORE::CObserver()          ,
          m_udpSocket( false )       ,
          m_tcpServerSocket( false ) ,
          m_doUdpForwarding( false )
    {GUCEF_TRACE;

        RegisterEventHandlers();
    }

    virtual ~SocketSink()
    {GUCEF_TRACE;

        m_udpSocket.Close( true );
        m_tcpServerSocket.Close();

        FlushUdpBuffer();
    }
};

/*-------------------------------------------------------------------------*/

CORE::CString
LookForConfigFile( const CORE::CString& configFile )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFile );
    if ( !CORE::FileExists( configFile ) )
    {
        CORE::CString configFilePath = CORE::CombinePath( "$CURWORKDIR$", configFile );
        configFilePath = CORE::RelativePath( configFilePath );

        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFilePath );
        if ( !CORE::FileExists( configFilePath ) )
        {
            configFilePath = CORE::CombinePath( "$MODULEDIR$", configFile );
            configFilePath = CORE::RelativePath( configFilePath );

            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFilePath );
            if ( !FileExists( configFilePath ) )
            {            
                configFilePath = CORE::CombinePath( "$TEMPDIR$", configFile );
                configFilePath = CORE::RelativePath( configFilePath );

                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFilePath );
                if ( !FileExists( configFilePath ) )
                {                                        
                    return CORE::CString::Empty;
                }
            }
        }

        return configFilePath; 
    }

    return configFile;
}

/*-------------------------------------------------------------------------*/

bool
LoadConfig( const CORE::CString& bootstrapConfigPath   ,
            const CORE::CString& configPath            ,
            CORE::CValueList& keyValueList             ,
            CORE::CDataNode* loadedConfig = GUCEF_NULL )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString bootstrapConfigFile = "SocketSink_bootstrap_d.ini";
    const CORE::CString configFile = "SocketSink_d.ini";
    #else
    const CORE::CString bootstrapConfigFile = "SocketSink_bootstrap.ini";
    const CORE::CString configFile = "SocketSink.ini";
    #endif

    CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();
    
    CORE::CString bootstrapConfigFilePath = LookForConfigFile( bootstrapConfigFile );
    CORE::CString configFilePath = LookForConfigFile( configFile );

    if ( !bootstrapConfigFilePath.IsNULLOrEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Located bootstrap config file @ " + bootstrapConfigFilePath );
        configStore.SetBootstrapConfigFile( bootstrapConfigFilePath );
    }
    if ( !configFilePath.IsNULLOrEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Located config file @ " + configFilePath );
        configStore.SetConfigFile( configFilePath );
    }
    if ( bootstrapConfigFilePath.IsNULLOrEmpty() && configFilePath.IsNULLOrEmpty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Unable to locate any config file, will rely on params only" );
    }

    CORE::CGlobalConfigValueList globalCfg;
    globalCfg.SetConfigNamespace( "Main/AppArgs" );    
    globalCfg.SetAllowDuplicates( false );
    globalCfg.SetAllowMultipleValues( true );

    bool loadSuccess = configStore.LoadConfig( loadedConfig );

    keyValueList = globalCfg;
    return loadSuccess;
}

/*-------------------------------------------------------------------------*/

void
ParseParams( const int argc                 ,
             char* argv[]                   ,
             CORE::CValueList& keyValueList )
{GUCEF_TRACE;

    CORE::ParseApplicationArguments( argc, argv, keyValueList, '*' );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Application parameters: " << CORE::ToString( keyValueList ) );
}

/*-------------------------------------------------------------------------*/

static void
GucefAppSignalHandler( int signal )
{GUCEF_TRACE; 

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "GucefAppSignalHandler: Received signal " << signal );

    // signal context: only the async-signal-safe stop request, the application stops at its next pulse
    ::GUCEF::CORE::CGUCEFApplication::RequestStopFromSignal( signal );
}

/*-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    // a service: a stop signal during the start (while the config loads) is kept and handed to the stop handler once
    // it is installed, instead of ending the process at once (or being ignored, for process 1 of a container)
    GUCEF_OSMAIN_DEFER_STOP_SIGNALS;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "This tool was compiled on: " __DATE__ " @ " __TIME__ );

    CORE::CCoreGlobal::Instance();
    COMCORE::CComCoreGlobal::Instance();

    // Check for config param first
    CORE::CValueList keyValueList;
    ParseParams( argc, argv, keyValueList );
    CORE::CString bootstrapConfigPathParam = keyValueList.GetValueAlways( "BootstrapConfigPath" );
    CORE::CString configPathParam = keyValueList.GetValueAlways( "ConfigPath" );
    keyValueList.Clear();

    // Load settings from a config file (if any) and then override with params (if any)
    LoadConfig( bootstrapConfigPathParam, configPathParam, keyValueList );
    ParseParams( argc, argv, keyValueList );

    CORE::CString outputDir = keyValueList.GetValueAlways( "outputDir" );
    if ( outputDir.IsNULLOrEmpty() )
    {
        outputDir = CORE::RelativePath( "$CURWORKDIR$" );
    }
    CORE::CreateDirs( outputDir );

    CORE::CString logFilename = CORE::CombinePath( outputDir, "SocketSink_Log.txt" );

    keyValueList.Set( "logfile", logFilename );

    CORE::CFileAccess logFileAccess( logFilename, "w" );
    CORE::CStdLogger logger( logFileAccess );
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

    CORE::CPlatformNativeConsoleLogger console;
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

    SocketSink socketSink;
    if ( socketSink.Setup( keyValueList ) )
    {
        GUCEF_OSMAIN_SIGNAL_HANDLER( GucefAppSignalHandler );
        return CORE::CCoreGlobal::Instance()->GetApplication().main( argc, argv, true );
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketSink: Exiting because setup failed" );
    return -1;
}
GUCEF_OSMAIN_END

/*---------------------------------------------------------------------------*/
