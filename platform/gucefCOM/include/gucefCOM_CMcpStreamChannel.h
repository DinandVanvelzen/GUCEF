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

#ifndef GUCEF_COM_CMCPSTREAMCHANNEL_H
#define GUCEF_COM_CMCPSTREAMCHANNEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COM_CMCPCHANNEL_H
#include "gucefCOM_CMcpChannel.h"
#define GUCEF_COM_CMCPCHANNEL_H
#endif /* GUCEF_COM_CMCPCHANNEL_H ? */

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
 *  Where a stream channel puts the lines it writes: the writer of stdout, a TCP connection. Implemented by the transport.
 */
class GUCEF_COM_PUBLIC_CPP CIMcpLineSink
{
    public:

    CIMcpLineSink( void ) {}

    virtual ~CIMcpLineSink() {}

    /**
     *  Takes one message, without its line end, and delivers it with the framing of the transport (a '\n' behind it).
     *  Must not block. @return false when the line could not be taken: the peer is gone or the queue is full
     */
    virtual bool SendMcpLine( const CORE::CString& line ) = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  The channel of a transport that carries lines, the stdio framing of MCP: stdio and TCP. One message per line,
 *  delimited by '\n' and never containing one; a '\r' in front of the '\n' is tolerated on input.
 *
 *  A transport that has bytes (TCP) gives them to ProcessIncomingBytes(), which splits them in lines and keeps what
 *  does not form a line yet. A transport that frames the lines itself (the reader thread of stdio) gives them to
 *  ProcessIncomingLine(). Both end at CMcpChannel::ProcessIncomingMessage(), which applies the size and nesting limits.
 *  Lines that are empty are ignored. A line longer than the limit of the channel is skipped without being kept, and
 *  answered with the error of a message that is too large, without an id: the id was never read.
 *
 *  The channel does not own the sink: the transport calls SetSink( GUCEF_NULL ) before it goes away.
 */
class GUCEF_COM_PUBLIC_CPP CMcpStreamChannel : public CMcpChannel
{
    public:

    CMcpStreamChannel( const CMcpPrincipal& principal, CIMcpLineSink* sink );

    virtual ~CMcpStreamChannel() GUCEF_VIRTUAL_OVERRIDE;

    void SetSink( CIMcpLineSink* sink );

    /** Bytes as they come in: lines are cut from them and processed in order. Stops when a handler of a line closes the channel */
    void ProcessIncomingBytes( const void* data, UInt32 size );

    /** One line of a transport that cuts them itself, without its line end. @return false when it was refused before it was parsed */
    bool ProcessIncomingLine( const CORE::CString& line );

    /** The number of bytes held for a line that has not ended yet */
    UInt32 GetPendingBytes( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /**
     *  Called for every complete line of ProcessIncomingBytes() before it is processed, so that a transport can refuse
     *  it, for example the TCP transport for text that is no JSON. The default accepts every line.
     *  @return false to drop the line; it is then up to the override to close the channel
     */
    virtual bool OnLineReceived( const CORE::CString& line );

    virtual bool SendMessageText( const CORE::CString& messageText ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CMcpStreamChannel( const CMcpStreamChannel& src );
    CMcpStreamChannel& operator=( const CMcpStreamChannel& src );

    void DeliverLine( const char* text, UInt32 size );

    private:

    CIMcpLineSink* m_sink;
    CORE::CDynamicBuffer m_partialLine;     /**< the start of a line that continues in the next bytes */
    bool m_skippingLine;
    UInt64 m_skippedBytes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPSTREAMCHANNEL_H ? */
