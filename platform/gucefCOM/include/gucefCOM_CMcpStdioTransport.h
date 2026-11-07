/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#ifndef GUCEF_COM_CMCPSTDIOTRANSPORT_H
#define GUCEF_COM_CMCPSTDIOTRANSPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CSTANDARDINPUTREADER_H
#include "gucefCORE_CStandardInputReader.h"
#define GUCEF_CORE_CSTANDARDINPUTREADER_H
#endif /* GUCEF_CORE_CSTANDARDINPUTREADER_H ? */

#ifndef GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#include "gucefCORE_CStandardOutputWriter.h"
#define GUCEF_CORE_CSTANDARDOUTPUTWRITER_H
#endif /* GUCEF_CORE_CSTANDARDOUTPUTWRITER_H ? */

#ifndef GUCEF_COM_CIMCPTRANSPORT_H
#include "gucefCOM_CIMcpTransport.h"
#define GUCEF_COM_CIMCPTRANSPORT_H
#endif /* GUCEF_COM_CIMCPTRANSPORT_H ? */

#ifndef GUCEF_COM_CMCPSTREAMCHANNEL_H
#include "gucefCOM_CMcpStreamChannel.h"
#define GUCEF_COM_CMCPSTREAMCHANNEL_H
#endif /* GUCEF_COM_CMCPSTREAMCHANNEL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  MCP over the standard input and output of the process: the transport that a client which starts the server as a
 *  child process talks to. Messages are lines, a CORE::CStandardInputReader reads them on a thread of its own and a
 *  CORE::CStandardOutputWriter writes them on another, so that the protocol thread never blocks on either.
 *
 *  The one channel of the transport, kind CHANNEL_STREAM, has the principal "local:parent-process": the client started
 *  the process, which is all the authentication stdio has.
 *
 *  Start() claims standard output for the protocol (CStandardOutputWriter::ClaimExclusive()), so that the console
 *  loggers of the process stay off it, and refuses to start when it was claimed already. It does not alter the
 *  disposition of SIGPIPE unless ignoreSigPipe is set.
 *
 *  When the input ends (the client closed its end: the shutdown signal of the stdio binding) the transport closes
 *  its channel, which cancels the requests in flight, and, when the stopOnStdinClosed setting of the server is on,
 *  asks the server to raise ShutdownRequestedEvent. The application owns its main loop and exits on that event; a
 *  library never calls exit(). The same happens when the output pipe breaks or its queue overflows.
 *
 *  For tests and tools that talk over other streams, SetStreams() names the pipes to use instead of the standard
 *  streams; stdout is not claimed then.
 *
 *  Configuration keys (section entry of the server's Transports): ignoreSigPipe (false), scopes (comma separated, none),
 *  maxQueuedOutputBytes (64 MiB).
 */
class GUCEF_COM_PUBLIC_CPP CMcpStdioTransport : public CIMcpTransport  ,
                                                private CIMcpLineSink
{
    public:

    static const CORE::CString ConfigSectionName;           /**< "McpStdioTransport", when the settings are saved on their own */

    CMcpStdioTransport( void );

    virtual ~CMcpStdioTransport() GUCEF_VIRTUAL_OVERRIDE;

    /** The streams to use instead of the standard input and output of the process. They are not owned and have to outlive the transport. Before Start() */
    void SetStreams( CORE::CStandardInputReader::TStreamHandle input, CORE::CStandardOutputWriter::TStreamHandle output );

    /** Back to the standard input and output */
    void UseStandardStreams( void );

    void SetIgnoreSigPipe( bool ignoreSigPipe );

    bool GetIgnoreSigPipe( void ) const;

    void SetScopes( const CORE::CString::StringVector& scopes );

    void SetMaxQueuedOutputBytes( UInt32 maxQueuedBytes );

    /** The channel while the transport runs, for tests and applications that want to know about it */
    CMcpChannelPtr GetChannel( void ) const;

    /** True from the moment the input ended, the output broke or the transport stopped the channel itself */
    bool HasPeerGone( void ) const;

    virtual const CORE::CString& GetTransportName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Start( CMcpServer& server ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Stop( bool graceful ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetEndpointDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CMcpStdioTransport( const CMcpStdioTransport& src );
    CMcpStdioTransport& operator=( const CMcpStdioTransport& src );

    // CIMcpLineSink
    virtual bool SendMcpLine( const CORE::CString& line ) GUCEF_VIRTUAL_OVERRIDE;

    // events of the reader and the writer, on the protocol thread
    void OnLineReceived( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnLineTooLong( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnEndOfInput( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnReadError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnBrokenPipe( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    /** The peer is gone or the output failed: the channel closes, and the server is asked to stop when its settings say so */
    void OnPeerGone( const CORE::CString& reason );

    void Release( bool flush );

    private:

    CMcpServer* m_server;
    CORE::CStandardInputReader* m_reader;
    CORE::CStandardOutputWriter* m_writer;
    CORE::CTSGNotifier* m_relay;                    /**< brings the events of the two threads to the protocol thread */
    CMcpStreamChannel* m_channel;
    CMcpChannelPtr m_channelPtr;
    bool m_active;
    bool m_claimedStdout;
    bool m_peerGone;
    bool m_hasCustomStreams;
    CORE::CStandardInputReader::TStreamHandle m_inputStream;
    CORE::CStandardOutputWriter::TStreamHandle m_outputStream;
    bool m_ignoreSigPipe;
    CORE::CString::StringVector m_scopes;
    UInt32 m_maxQueuedOutputBytes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPSTDIOTRANSPORT_H ? */
