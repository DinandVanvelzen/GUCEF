/*
 *  gucefCOM: GUCEF module providing communication functionality
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#define GUCEF_COM_CJSONRPCENDPOINT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_COM_JSONRPCTYPES_H
#include "gucefCOM_JsonRpcTypes.h"
#define GUCEF_COM_JSONRPCTYPES_H
#endif /* GUCEF_COM_JSONRPCTYPES_H ? */

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
 *  One side of a JSON-RPC conversation. JSON-RPC is symmetric: both sides can make calls, send notifications
 *  and answer calls, so the same class is the base of the client and of every connection of the server.
 *
 *  The endpoint sends messages through a CIJsonRpcTransport and is given the messages received by that transport
 *  through ProcessIncoming(). JSON is handled by the "json" dstore codec which is provided by a plugin
 *  (dstorepluginJSONPARSER) and found through the codec registry: without it nothing can be sent or received.
 *
 *  Events:
 *    ResponseEvent      TJsonRpcResponseEventData      the answer to a call, also a timeout or lost transport
 *    NotificationEvent  TJsonRpcNotificationEventData  the peer sent a notification
 *
 *  Calls the peer makes are given to the request handler, see CIJsonRpcRequestHandler.
 *  Not thread-safe: use from the thread which receives the messages.
 */
class GUCEF_COM_PUBLIC_CPP CJsonRpcEndpoint : public CORE::CNotifier
{
    public:

    static const CORE::CEvent ResponseEvent;
    static const CORE::CEvent NotificationEvent;

    static void RegisterEvents( void );

    enum EProcessResult
    {
        PROCESS_OK        ,     /**< the message was valid and dealt with */
        PROCESS_INVALID   ,     /**< the message was not valid, the peer was told so where the protocol calls for that */
        PROCESS_DELETED         /**< this endpoint was deleted as a result of dealing with the message: do not touch it anymore */
    };

    /** One call or notification of a batch */
    struct SBatchItem
    {
        CORE::CString method;
        CORE::CDataNode params;
        bool hasParams;
        bool isNotification;                /**< no answer is expected, no id is assigned */
        UInt64 callId;                      /**< set by CallBatch() for calls */

        SBatchItem( void ) : method(), params(), hasParams( false ), isNotification( false ), callId( 0 ) {}
    };
    typedef GUCEF::vector< SBatchItem > TBatchItemVector;

    static const UInt32 DefaultCallTimeoutInMs = 30000;

    public:

    explicit CJsonRpcEndpoint( CIJsonRpcTransport* transport = GUCEF_NULL );

    virtual ~CJsonRpcEndpoint();

    /** The transport must outlive the endpoint or be unset (GUCEF_NULL) before it is destroyed */
    void SetTransport( CIJsonRpcTransport* transport );

    void SetDialect( EJsonRpcDialect dialect );

    EJsonRpcDialect GetDialect( void ) const;

    /**
     *  Batches (an array of messages) are part of JSON-RPC 2.0 and accepted by default.
     *  When refused an incoming batch is answered with a single invalid request error.
     */
    void SetAllowBatches( bool allowBatches );

    bool GetAllowBatches( void ) const;

    /**
     *  A request with "id": null is accepted by default, as the specification discourages it but allows it.
     *  When refused such a request is answered with an invalid request error and not given to the handler.
     */
    void SetAllowNullRequestId( bool allowNullRequestId );

    bool GetAllowNullRequestId( void ) const;

    void SetRequestHandler( CIJsonRpcRequestHandler* handler );

    /**
     *  Adds a field to every call and notification this endpoint sends, next to "method" and "params".
     *  Allows protocols built on JSON-RPC to add to the envelope, for example the sessionId of the Chrome DevTools Protocol.
     *  A scalar value is added as is. Passing an empty name removes all such fields.
     */
    void SetOutgoingCallField( const CORE::CString& name, const CORE::CVariant& value );

    void SetDefaultCallTimeoutInMs( UInt32 timeoutInMs );

    /**
     *  Names the pulse generator which drives the transport of this endpoint, by default the application's
     *  (global) pulse generator. CallAndWait() pumps it while it waits.
     */
    virtual void SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator );

    CORE::PulseGeneratorPtr GetPulseGenerator( void ) const;

    /**
     *  Makes a call. The answer is delivered through ResponseEvent, which also reports a timeout or
     *  the loss of the transport as a local error.
     *  @param params       the parameters, an array or an object node. GUCEF_NULL for none
     *  @param callId       receives the id of the call, which the response carries
     *  @param timeoutInMs  0 uses the default
     */
    bool Call( const CORE::CString& method       ,
               const CORE::CDataNode* params     ,
               UInt64& callId                    ,
               UInt32 timeoutInMs = 0            );

    /**
     *  Makes a call and waits for the answer by pumping the pulse generator of the endpoint, see SetPulseGenerator().
     *  The answer is returned and not delivered as an event.
     *  The wait runs on the calling thread, which must be the thread that drives that pulse generator: a pulse generator
     *  is never pumped from a thread other than its own. Called from any other thread, or while it is not known which
     *  thread drives the pulse generator, the call is refused without being made. Use Call() and ResponseEvent there.
     *  @return true if an answer, which can be an error, came back. false if the call was refused or could not be made
     */
    bool CallAndWait( const CORE::CString& method   ,
                      const CORE::CDataNode* params ,
                      SJsonRpcResponse& response    ,
                      UInt32 timeoutInMs = 0        );

    /** Sends a message which expects no answer */
    bool Notify( const CORE::CString& method, const CORE::CDataNode* params );

    /**
     *  Sends several calls and notifications as a single batch message. The ids of the calls are set in the items.
     *  Their answers are delivered as individual responses.
     */
    bool CallBatch( TBatchItemVector& items, UInt32 timeoutInMs = 0 );

    /** Answers a call, to be used for calls whose handler deferred the answer */
    bool SendResponse( const CORE::CVariant& callId, const CORE::CDataNode& result );

    bool SendErrorResponse( const CORE::CVariant& callId       ,
                            Int32 errorCode                    ,
                            const CORE::CString& errorMessage  ,
                            const CORE::CDataNode* errorData = GUCEF_NULL );

    /**
     *  Helpers for the JSON conventions used by the endpoint: parsing goes through the "json" dstore codec,
     *  writing is done here as the codec's writer turns an object with exactly one member into a bare value.
     *  A JSON object is a node with its scalar members as attributes and its object and array members as children,
     *  an array is a node of the array type with its items as children and null is a node of the nil type.
     */
    static bool ParseJsonText( const CORE::CString& jsonText, CORE::CDataNode& root );

    static bool ToJsonText( const CORE::CDataNode& root, CORE::CString& jsonText );

    /**
     *  The members of a JSON object in those conventions. A member is a direct attribute or child of the object:
     *  these never search deeper, so a member that is missing is not found further down in the tree.
     *  Member names are matched as a whole, also names with '/' and '.' such as the _meta keys of MCP.
     */
    static bool HasJsonMember( const CORE::CDataNode& object, const CORE::CString& name );

    /** The child node of an object, array or null member; GUCEF_NULL for a scalar member or a missing one */
    static const CORE::CDataNode* FindJsonMemberNode( const CORE::CDataNode& object, const CORE::CString& name );

    /** Copies a member as a node: for a scalar the value is in the node, which then has the type of the value */
    static bool GetJsonMember( const CORE::CDataNode& object, const CORE::CString& name, CORE::CDataNode& value );

    /** Adds a member: an object or array node as a child, a nil node as a null child, any other node as a scalar attribute */
    static void AddJsonMember( CORE::CDataNode& object, const CORE::CString& name, const CORE::CDataNode& value );

    /** To be called by the owner of the transport for every complete message that was received */
    EProcessResult ProcessIncoming( const CORE::CString& messageText );

    /**
     *  Delivers a timeout response for every call which was not answered in time. To be called regularly,
     *  CJsonRpcClient does this on a timer.
     *  @return false if this endpoint was deleted as a result of delivering a response
     */
    bool ExpireTimedOutCalls( UInt32* expiredCount = GUCEF_NULL );

    /**
     *  Delivers an error response for every call still waiting, to be used when the transport is lost
     *  @return false if this endpoint was deleted as a result of delivering a response
     */
    bool FailAllPendingCalls( Int32 errorCode, const CORE::CString& errorMessage );

    UInt32 GetPendingCallCount( void ) const;

    private:

    CJsonRpcEndpoint( const CJsonRpcEndpoint& src );
    CJsonRpcEndpoint& operator=( const CJsonRpcEndpoint& src );

    class CWaitScope;
    friend class CWaitScope;

    struct SPendingCall
    {
        CORE::CVariant callId;
        UInt64 deadlineInMs;
    };
    typedef GUCEF::map< CORE::CString, SPendingCall > TPendingCallMap;

    void InitMessage( CORE::CDataNode& message ) const;

    void AddCallFields( CORE::CDataNode& message ) const;

    bool BuildCall( const CORE::CString& method, const CORE::CDataNode* params, bool isNotification, UInt64& callId, UInt32 timeoutInMs, CORE::CDataNode& message );

    bool SendNode( const CORE::CDataNode& message );

    EProcessResult ProcessMessageNode( const CORE::CDataNode& node, CORE::CDataNode* batchResponses );

    EProcessResult HandleResponseNode( const CORE::CDataNode& node );

    bool DeliverResponse( SJsonRpcResponse& response );

    private:

    CIJsonRpcTransport* m_transport;
    CIJsonRpcRequestHandler* m_requestHandler;
    EJsonRpcDialect m_dialect;
    bool m_allowBatches;
    bool m_allowNullRequestId;
    UInt32 m_defaultCallTimeoutInMs;
    UInt64 m_nextCallId;
    TPendingCallMap m_pendingCalls;
    GUCEF::map< CORE::CString, CORE::CVariant > m_outgoingCallFields;
    CORE::PulseGeneratorPtr m_pulseGenerator;

    // state of CallAndWait()
    bool m_isWaiting;
    bool* m_waitDestroyedFlag;          /**< set by the destructor, lives in the CallAndWait() that is in progress */
    bool m_waitDone;
    CORE::CString m_waitKey;
    SJsonRpcResponse m_waitResponse;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */
