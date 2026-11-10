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

#ifndef GUCEF_COM_CMCPINPUTREQUIRED_H
#define GUCEF_COM_CMCPINPUTREQUIRED_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COM_MCPTYPES_H
#include "gucefCOM_McpTypes.h"
#define GUCEF_COM_MCPTYPES_H
#endif /* GUCEF_COM_MCPTYPES_H ? */

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
 *  The output of a handler that needs input from the client before it can finish (multi round-trip requests).
 *  The server answers with resultType "input_required", the input requests and the state, which it seals: the
 *  client echoes the sealed state on its retry and the handler gets it back verified, see
 *  CMcpRequestContext::GetRequestState(). At least one input request or a state is required.
 *
 *  The state is integrity protected, and with an AEAD codec also hidden, but a handler must not rely on the latter:
 *  it must not put secrets in it.
 */
class GUCEF_COM_PUBLIC_CPP CMcpInputRequired : public CORE::CICloneable
{
    public:

    struct GUCEF_COM_PUBLIC_CPP SInputRequest
    {
        CORE::CString method;           /**< "elicitation/create", "sampling/createMessage" or "roots/list" */
        CORE::CDataNode params;         /**< an object node */
        bool hasParams;

        SInputRequest( void );
    };
    typedef GUCEF::map< CORE::CString, SInputRequest > TInputRequestMap;

    CMcpInputRequired( void );

    CMcpInputRequired( const CMcpInputRequired& src );

    virtual ~CMcpInputRequired() GUCEF_VIRTUAL_OVERRIDE;

    CMcpInputRequired& operator=( const CMcpInputRequired& src );

    /**
     *  Asks the user for structured input through the client: an elicitation/create request in form mode.
     *  The requested schema is a flat object schema of primitive properties, as the elicitation page prescribes.
     *  The key is chosen by the handler and must be unique within the result; adding a key again replaces it.
     */
    CMcpInputRequired& AddFormElicitation( const CORE::CString& key, const CORE::CString& message, const CORE::CDataNode& requestedSchema );

    /** Adds any input request as a raw node, for sampling and roots, which 2026-07-28 deprecates */
    CMcpInputRequired& AddInputRequest( const CORE::CString& key, const CORE::CString& method, const CORE::CDataNode& params );

    /** The state of the handler for the retry; sealed by the server */
    CMcpInputRequired& SetState( const CORE::CDataNode& handlerState );

    /**
     *  Puts a random nonce into the sealed state, which the handler reads back on the retry from
     *  CMcpRequestContext::GetStateNonce(): for a handler that keeps its own used-set to make a state single use.
     *  A seal does not stop a replay within its lifetime by itself.
     */
    CMcpInputRequired& SetStateNonce( void );

    void Clear( void );

    const TInputRequestMap& GetInputRequests( void ) const;

    bool HasInputRequests( void ) const;

    bool HasState( void ) const;

    const CORE::CDataNode& GetState( void ) const;

    bool GetWantsStateNonce( void ) const;

    /**
     *  The client capabilities the input requests need, as a ClientCapabilities object: { "elicitation": {} } for
     *  form mode, { "elicitation": { "url": {} } } for URL mode, { "sampling": {} }, { "roots": {} }.
     *  The server compares them with the capabilities of the request before anything is sent.
     */
    void GetRequiredCapabilities( CORE::CDataNode& capabilities ) const;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    private:

    TInputRequestMap m_inputRequests;
    CORE::CDataNode m_state;
    bool m_hasState;
    bool m_wantsStateNonce;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPINPUTREQUIRED_H ? */
