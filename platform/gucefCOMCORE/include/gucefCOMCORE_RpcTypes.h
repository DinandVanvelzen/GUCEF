/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_RPCTYPES_H
#define GUCEF_COMCORE_RPCTYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The field types of an RPC contract (gucefCOMCORE RPC design 3.2). The numbers are part of the canonical schema
 *  encoding, so they never change
 */
enum ERpcFieldType
{
    RPC_FIELDTYPE_INVALID   = 0  ,
    RPC_FIELDTYPE_BOOL      = 1  ,
    RPC_FIELDTYPE_INT8      = 2  ,
    RPC_FIELDTYPE_INT16     = 3  ,
    RPC_FIELDTYPE_INT32     = 4  ,
    RPC_FIELDTYPE_INT64     = 5  ,
    RPC_FIELDTYPE_UINT8     = 6  ,
    RPC_FIELDTYPE_UINT16    = 7  ,
    RPC_FIELDTYPE_UINT32    = 8  ,
    RPC_FIELDTYPE_UINT64    = 9  ,
    RPC_FIELDTYPE_FLOAT32   = 10 ,
    RPC_FIELDTYPE_FLOAT64   = 11 ,
    RPC_FIELDTYPE_STRING    = 12 ,
    RPC_FIELDTYPE_BYTES     = 13 ,
    RPC_FIELDTYPE_DECIMAL   = 14 ,  /**< free scale: LEN, zig-zag unscaled, varint scale; fixed scale: VARINT */
    RPC_FIELDTYPE_DATE      = 15 ,
    RPC_FIELDTYPE_TIMESTAMP = 16 ,
    RPC_FIELDTYPE_DATETIME  = 17 ,
    RPC_FIELDTYPE_DURATION  = 18 ,
    RPC_FIELDTYPE_ENUM      = 19 ,
    RPC_FIELDTYPE_MESSAGE   = 20 ,
    RPC_FIELDTYPE_VARIANT   = 21 ,
    RPC_FIELDTYPE_DATANODE  = 22
};
typedef enum ERpcFieldType TRpcFieldType;

/** Field cardinality (3.2) */
enum ERpcCardinality
{
    RPC_CARDINALITY_SINGLE   = 0 ,
    RPC_CARDINALITY_OPTIONAL = 1 ,
    RPC_CARDINALITY_REPEATED = 2 ,
    RPC_CARDINALITY_MAP      = 3
};
typedef enum ERpcCardinality TRpcCardinality;

/** The four call kinds (3.3) */
enum ERpcMethodKind
{
    RPC_METHODKIND_UNARY         = 0 ,
    RPC_METHODKIND_SERVER_STREAM = 1 ,
    RPC_METHODKIND_CLIENT_STREAM = 2 ,
    RPC_METHODKIND_BIDI_STREAM   = 3
};
typedef enum ERpcMethodKind TRpcMethodKind;

/** Protection class of the frames of a method (3.3, 14.3) */
enum ERpcProtectionClass
{
    RPC_PROTECTION_CONFIDENTIAL = 0 ,
    RPC_PROTECTION_PUBLIC       = 1
};
typedef enum ERpcProtectionClass TRpcProtectionClass;

/** Status codes: the values of gRPC plus RESUME_GAP (9) */
enum ERpcStatusCode
{
    RPC_STATUS_OK                  = 0   ,
    RPC_STATUS_CANCELLED           = 1   ,
    RPC_STATUS_UNKNOWN             = 2   ,
    RPC_STATUS_INVALID_ARGUMENT    = 3   ,
    RPC_STATUS_DEADLINE_EXCEEDED   = 4   ,
    RPC_STATUS_NOT_FOUND           = 5   ,
    RPC_STATUS_ALREADY_EXISTS      = 6   ,
    RPC_STATUS_PERMISSION_DENIED   = 7   ,
    RPC_STATUS_RESOURCE_EXHAUSTED  = 8   ,
    RPC_STATUS_FAILED_PRECONDITION = 9   ,
    RPC_STATUS_ABORTED             = 10  ,
    RPC_STATUS_OUT_OF_RANGE        = 11  ,
    RPC_STATUS_UNIMPLEMENTED       = 12  ,
    RPC_STATUS_INTERNAL            = 13  ,
    RPC_STATUS_UNAVAILABLE         = 14  ,
    RPC_STATUS_DATA_LOSS           = 15  ,
    RPC_STATUS_UNAUTHENTICATED     = 16  ,
    RPC_STATUS_RESUME_GAP          = 100
};
typedef enum ERpcStatusCode TRpcStatusCode;

/** Where a server-side handler runs (10.6) */
enum ERpcExecutionMode
{
    RPC_EXECUTION_CALLING_THREAD      = 0 ,     /**< on the protocol thread; the handler must not block */
    RPC_EXECUTION_NOTIFICATION_THREAD = 1 ,     /**< mailed to the notification pulse generator of the endpoint */
    RPC_EXECUTION_THREAD_POOL         = 2       /**< a task on the endpoint's thread pool */
};
typedef enum ERpcExecutionMode TRpcExecutionMode;

/** What a handler did with the call */
enum ERpcHandlerResult
{
    RPC_HANDLER_DONE     = 0 ,      /**< the call was answered (or will be failed with UNKNOWN if it was not) */
    RPC_HANDLER_DEFERRED = 1        /**< the handler kept the call (CRpcServerCall::GetSharedPtr()) and completes it later */
};
typedef enum ERpcHandlerResult TRpcHandlerResult;

/** Session states (7.1) */
enum ERpcSessionState
{
    RPC_SESSIONSTATE_IDLE        = 0 ,     /**< not started */
    RPC_SESSIONSTATE_CONNECTING  = 1 ,     /**< HELLO sent or awaited */
    RPC_SESSIONSTATE_ESTABLISHED = 2 ,
    RPC_SESSIONSTATE_SUSPENDED   = 3 ,     /**< no transport, within the grace period */
    RPC_SESSIONSTATE_CLOSING     = 4 ,     /**< GOODBYE sent or received */
    RPC_SESSIONSTATE_CLOSED      = 5
};
typedef enum ERpcSessionState TRpcSessionState;

/** State of accepted work, as STATUS reports it (6.3) */
enum ERpcRequestState
{
    RPC_REQUESTSTATE_UNKNOWN   = 0 ,
    RPC_REQUESTSTATE_QUEUED    = 1 ,
    RPC_REQUESTSTATE_RUNNING   = 2 ,
    RPC_REQUESTSTATE_DONE      = 3 ,
    RPC_REQUESTSTATE_CANCELLED = 4 ,
    RPC_REQUESTSTATE_FAILED    = 5
};
typedef enum ERpcRequestState TRpcRequestState;

/** The outcome of CIRpcAccessPolicy::AuthorizeCall() */
enum ERpcAccessDecision
{
    RPC_ACCESS_ALLOW           = 0 ,
    RPC_ACCESS_DENY            = 1 ,     /**< PERMISSION_DENIED */
    RPC_ACCESS_UNAUTHENTICATED = 2       /**< UNAUTHENTICATED: the policy needs an authenticated principal */
};
typedef enum ERpcAccessDecision TRpcAccessDecision;

/** What happens when the retention bounds are reached while a session is suspended (7.3) */
enum ERpcSuspendedOverflow
{
    RPC_SUSPENDEDOVERFLOW_REFUSE_NEW  = 0 ,
    RPC_SUSPENDEDOVERFLOW_DROP_OLDEST = 1
};
typedef enum ERpcSuspendedOverflow TRpcSuspendedOverflow;

/** Method flag bits (3.3); the bit values are part of the canonical schema encoding */
#define GUCEF_RPC_METHODFLAG_IDEMPOTENT         0x01
#define GUCEF_RPC_METHODFLAG_MAYANSWERASYNC     0x02
#define GUCEF_RPC_METHODFLAG_RESUMABLESTREAM    0x04
#define GUCEF_RPC_METHODFLAG_SENSITIVE          0x08

/** Field flag bits (3.8) */
#define GUCEF_RPC_FIELDFLAG_REQUIRED            0x01
#define GUCEF_RPC_FIELDFLAG_SENSITIVE           0x02
#define GUCEF_RPC_FIELDFLAG_DEPRECATED          0x04

/** Protobuf wire types (5.1) */
#define GUCEF_RPC_WIRETYPE_VARINT               0
#define GUCEF_RPC_WIRETYPE_FIXED64              1
#define GUCEF_RPC_WIRETYPE_LEN                  2
#define GUCEF_RPC_WIRETYPE_FIXED32              5

/** Largest field number: the protobuf tag limit, 2^29 - 1 */
#define GUCEF_RPC_MAX_FIELD_NUMBER              536870911UL

/** Nesting limit of messages (3.2) */
#define GUCEF_RPC_MAX_MESSAGE_DEPTH             32

/*-------------------------------------------------------------------------*/

GUCEF_COMCORE_EXPORT_CPP const char* RpcStatusCodeToString( TRpcStatusCode code );
GUCEF_COMCORE_EXPORT_CPP const char* RpcFieldTypeToString( TRpcFieldType type );
GUCEF_COMCORE_EXPORT_CPP TRpcFieldType RpcFieldTypeFromString( const CORE::CString& name );
GUCEF_COMCORE_EXPORT_CPP const char* RpcMethodKindToString( TRpcMethodKind kind );
GUCEF_COMCORE_EXPORT_CPP bool RpcMethodKindFromString( const CORE::CString& name, TRpcMethodKind& kind );
GUCEF_COMCORE_EXPORT_CPP const char* RpcSessionStateToString( TRpcSessionState state );

/** The protobuf wire type that a field of the given type uses (5.1); a fixed-scale decimal is a VARINT */
GUCEF_COMCORE_EXPORT_CPP UInt32 RpcWireTypeOfFieldType( TRpcFieldType type, bool fixedScaleDecimal );

/** True for the types that may be packed when repeated: every VARINT and FIXED kind */
GUCEF_COMCORE_EXPORT_CPP bool RpcFieldTypeIsPackable( TRpcFieldType type, bool fixedScaleDecimal );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_RPCTYPES_H ? */
