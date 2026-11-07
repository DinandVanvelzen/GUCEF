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

#ifndef GUCEF_COM_CMCPREQUESTMETA_H
#define GUCEF_COM_CMCPREQUESTMETA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMCPIMPLEMENTATIONINFO_H
#include "gucefCOM_CMcpImplementationInfo.h"
#define GUCEF_COM_CMCPIMPLEMENTATIONINFO_H
#endif /* GUCEF_COM_CMCPIMPLEMENTATIONINFO_H ? */

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
 *  The per request metadata of a modern request, params._meta: the protocol version, the client capabilities of
 *  this request only, the client identity (display, logging and audit only) and the optional progress token and
 *  trace context. Parsed and validated once per request. The reserved keys are whole member names: they are never
 *  looked up with a path search.
 *
 *  For a legacy request the version and capabilities come from the session instead, see SetForLegacy().
 */
class GUCEF_COM_PUBLIC_CPP CMcpRequestMeta
{
    public:

    enum EParseResult
    {
        META_OK                 ,
        META_NO_PROTOCOL_VERSION,   /**< no _meta, or no protocolVersion in it: not a modern request */
        META_MALFORMED              /**< a reserved key is missing or of the wrong JSON type: -32602 */
    };

    CMcpRequestMeta( void );

    /**
     *  Parses params._meta of a modern request
     *  @param meta     the _meta member, GUCEF_NULL when the request has none
     *  @param error    for META_MALFORMED: what is wrong, for the error message
     */
    EParseResult Parse( const CORE::CDataNode* meta, CORE::CString& error );

    /** The metadata of a legacy request: what the session negotiated */
    void SetForLegacy( const CORE::CString& protocolVersion, const CORE::CDataNode& clientCapabilities, const CMcpImplementationInfo* clientInfo );

    /** The metadata of an in-process call: the newest modern version and no client capabilities */
    void SetForInProcess( void );

    /** Sets the progress token, used for legacy requests which carry it in _meta too */
    void SetProgressToken( const CORE::CVariant& token );

    const CORE::CString& GetProtocolVersion( void ) const;

    /** The ClientCapabilities object, an object node */
    const CORE::CDataNode& GetClientCapabilities( void ) const;

    bool HasClientInfo( void ) const;

    const CMcpImplementationInfo& GetClientInfo( void ) const;

    bool HasProgressToken( void ) const;

    /** A string or an integer */
    const CORE::CVariant& GetProgressToken( void ) const;

    const CORE::CString& GetLogLevel( void ) const;

    const CORE::CString& GetTraceParent( void ) const;

    const CORE::CString& GetTraceState( void ) const;

    const CORE::CString& GetBaggage( void ) const;

    /** See HasCapability( capabilities, path ) */
    bool HasCapability( const CORE::CString& capabilityPath ) const;

    /**
     *  Whether a ClientCapabilities object declares a capability, named as a path of member names separated by '/':
     *  "elicitation", "elicitation/form", "sampling/tools", "roots". An elicitation object without form and url
     *  declares form mode, as the elicitation page specifies for backwards compatibility.
     */
    static bool HasCapability( const CORE::CDataNode& capabilities, const CORE::CString& capabilityPath );

    /** Whether 'capabilities' declares every capability that 'required' names, both ClientCapabilities objects */
    static bool HasAllCapabilities( const CORE::CDataNode& capabilities, const CORE::CDataNode& required );

    private:

    CORE::CString m_protocolVersion;
    CORE::CDataNode m_clientCapabilities;
    bool m_hasClientInfo;
    CMcpImplementationInfo m_clientInfo;
    bool m_hasProgressToken;
    CORE::CVariant m_progressToken;
    CORE::CString m_logLevel;
    CORE::CString m_traceParent;
    CORE::CString m_traceState;
    CORE::CString m_baggage;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPREQUESTMETA_H ? */
