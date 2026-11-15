/*
 *  gucef common header: header-only in-memory MCP channel for tests
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

/*
 *  Test support: the client end of an in-memory MCP conversation. The channel is opened on a COM::CMcpServer like a
 *  transport's channel; what the test hands to Receive() reaches the server as a client message, and what the server
 *  sends is kept as JSON text, in order, for the test to look at: responses by id, notifications by method.
 *  It is single threaded: the server sends on its protocol thread, which in a test is the test thread that drives
 *  the server's pulse generator (see gucef_test_mcp_rig.h).
 *
 *  For test applications that link gucefCOM; the JSON codec plugin (dstorepluginJSONPARSER) must be loaded.
 */

#ifndef GUCEF_TEST_MCP_CHANNEL_H
#define GUCEF_TEST_MCP_CHANNEL_H

#ifdef __cplusplus

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMCPCHANNEL_H
#include "gucefCOM_CMcpChannel.h"
#define GUCEF_COM_CMCPCHANNEL_H
#endif /* GUCEF_COM_CMCPCHANNEL_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TESTFW {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A channel whose transport is the test: it keeps what the server sends */
class CTestMcpChannel : public COM::CMcpChannel
{
    public:

    typedef GUCEF::vector< CORE::CString > TTextVector;
    typedef GUCEF::vector< CORE::CDataNode > TMessageVector;

    CTestMcpChannel( EKind kind, const COM::CMcpPrincipal& principal )
        : COM::CMcpChannel( kind, principal )
        , m_sent()
        , m_errorCodes()
    {
    }

    virtual ~CTestMcpChannel()
    {
    }

    /** A message from the client: JSON text, as a client writes it */
    bool Receive( const CORE::CString& messageText )
    {
        return ProcessIncomingMessage( messageText );
    }

    /** Everything the server sent, in order */
    const TTextVector& GetSent( void ) const
    {
        return m_sent;
    }

    /** The codes of the error responses, in order */
    const GUCEF::vector< Int32 >& GetErrorCodes( void ) const
    {
        return m_errorCodes;
    }

    void ClearSent( void )
    {
        m_sent.clear();
        m_errorCodes.clear();
    }

    /** The response whose id has the given text ("7" and "\"a\"" find the ids 7 and "a"); false when it was not sent */
    bool FindResponse( const CORE::CString& idText, CORE::CDataNode& response ) const
    {
        CORE::CString id = idText;
        if ( id.Length() >= 2 && '"' == id[ 0 ] && '"' == id[ id.Length() - 1 ] )
            id = id.SubstrFromRange( 1, id.Length() - 1 );
        for ( size_t i=0; i<m_sent.size(); ++i )
        {
            CORE::CDataNode node;
            if ( !COM::CJsonRpcEndpoint::ParseJsonText( m_sent[ i ], node ) )
                continue;
            if ( GUCEF_NULL != node.GetAttribute( "method" ) )
                continue;
            const CORE::CDataNode::TKeyValuePair* responseId = node.GetAttribute( "id" );
            if ( GUCEF_NULL != responseId && (*responseId).second.AsString() == id )
            {
                response = node;
                return true;
            }
        }
        return false;
    }

    /** The notifications of the method that were sent, in order; their number */
    UInt32 GetNotifications( const CORE::CString& method, TMessageVector& found ) const
    {
        found.clear();
        for ( size_t i=0; i<m_sent.size(); ++i )
        {
            CORE::CDataNode node;
            if ( !COM::CJsonRpcEndpoint::ParseJsonText( m_sent[ i ], node ) )
                continue;
            const CORE::CDataNode::TKeyValuePair* sentMethod = node.GetAttribute( "method" );
            if ( GUCEF_NULL != sentMethod && (*sentMethod).second.AsString() == method )
                found.push_back( node );
        }
        return (UInt32) found.size();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "GUCEF::TESTFW::CTestMcpChannel";
        return classTypeName;
    }

    protected:

    virtual bool SendMessageText( const CORE::CString& messageText ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_sent.push_back( messageText );
        return true;
    }

    virtual void OnErrorCode( Int32 code ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_errorCodes.push_back( code );
    }

    private:

    CTestMcpChannel( const CTestMcpChannel& src );              /**< not implemented */
    CTestMcpChannel& operator=( const CTestMcpChannel& src );   /**< not implemented */

    TTextVector m_sent;
    GUCEF::vector< Int32 > m_errorCodes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace TESTFW */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* __cplusplus ? */

#endif /* GUCEF_TEST_MCP_CHANNEL_H ? */
