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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_RpcTypes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
RpcStatusCodeToString( TRpcStatusCode code )
{GUCEF_TRACE;

    switch ( code )
    {
        case RPC_STATUS_OK: return "OK";
        case RPC_STATUS_CANCELLED: return "CANCELLED";
        case RPC_STATUS_UNKNOWN: return "UNKNOWN";
        case RPC_STATUS_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case RPC_STATUS_DEADLINE_EXCEEDED: return "DEADLINE_EXCEEDED";
        case RPC_STATUS_NOT_FOUND: return "NOT_FOUND";
        case RPC_STATUS_ALREADY_EXISTS: return "ALREADY_EXISTS";
        case RPC_STATUS_PERMISSION_DENIED: return "PERMISSION_DENIED";
        case RPC_STATUS_RESOURCE_EXHAUSTED: return "RESOURCE_EXHAUSTED";
        case RPC_STATUS_FAILED_PRECONDITION: return "FAILED_PRECONDITION";
        case RPC_STATUS_ABORTED: return "ABORTED";
        case RPC_STATUS_OUT_OF_RANGE: return "OUT_OF_RANGE";
        case RPC_STATUS_UNIMPLEMENTED: return "UNIMPLEMENTED";
        case RPC_STATUS_INTERNAL: return "INTERNAL";
        case RPC_STATUS_UNAVAILABLE: return "UNAVAILABLE";
        case RPC_STATUS_DATA_LOSS: return "DATA_LOSS";
        case RPC_STATUS_UNAUTHENTICATED: return "UNAUTHENTICATED";
        case RPC_STATUS_RESUME_GAP: return "RESUME_GAP";
        default: return "UNKNOWN";
    }
}

/*-------------------------------------------------------------------------*/

namespace {

struct SFieldTypeName
{
    TRpcFieldType type;
    const char* name;
};

const SFieldTypeName g_fieldTypeNames[] =
{
    { RPC_FIELDTYPE_BOOL, "bool" },
    { RPC_FIELDTYPE_INT8, "int8" },
    { RPC_FIELDTYPE_INT16, "int16" },
    { RPC_FIELDTYPE_INT32, "int32" },
    { RPC_FIELDTYPE_INT64, "int64" },
    { RPC_FIELDTYPE_UINT8, "uint8" },
    { RPC_FIELDTYPE_UINT16, "uint16" },
    { RPC_FIELDTYPE_UINT32, "uint32" },
    { RPC_FIELDTYPE_UINT64, "uint64" },
    { RPC_FIELDTYPE_FLOAT32, "float32" },
    { RPC_FIELDTYPE_FLOAT64, "float64" },
    { RPC_FIELDTYPE_STRING, "string" },
    { RPC_FIELDTYPE_BYTES, "bytes" },
    { RPC_FIELDTYPE_DECIMAL, "decimal" },
    { RPC_FIELDTYPE_DATE, "date" },
    { RPC_FIELDTYPE_TIMESTAMP, "timestamp" },
    { RPC_FIELDTYPE_DATETIME, "datetime" },
    { RPC_FIELDTYPE_DURATION, "duration" },
    { RPC_FIELDTYPE_ENUM, "enum" },
    { RPC_FIELDTYPE_MESSAGE, "message" },
    { RPC_FIELDTYPE_VARIANT, "variant" },
    { RPC_FIELDTYPE_DATANODE, "datanode" }
};

}

/*-------------------------------------------------------------------------*/

const char*
RpcFieldTypeToString( TRpcFieldType type )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<sizeof( g_fieldTypeNames ) / sizeof( g_fieldTypeNames[ 0 ] ); ++i )
        if ( g_fieldTypeNames[ i ].type == type )
            return g_fieldTypeNames[ i ].name;
    return "invalid";
}

/*-------------------------------------------------------------------------*/

TRpcFieldType
RpcFieldTypeFromString( const CORE::CString& name )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<sizeof( g_fieldTypeNames ) / sizeof( g_fieldTypeNames[ 0 ] ); ++i )
        if ( name == g_fieldTypeNames[ i ].name )
            return g_fieldTypeNames[ i ].type;
    return RPC_FIELDTYPE_INVALID;
}

/*-------------------------------------------------------------------------*/

const char*
RpcMethodKindToString( TRpcMethodKind kind )
{GUCEF_TRACE;

    switch ( kind )
    {
        case RPC_METHODKIND_UNARY: return "unary";
        case RPC_METHODKIND_SERVER_STREAM: return "server_stream";
        case RPC_METHODKIND_CLIENT_STREAM: return "client_stream";
        case RPC_METHODKIND_BIDI_STREAM: return "bidi_stream";
        default: return "unary";
    }
}

/*-------------------------------------------------------------------------*/

bool
RpcMethodKindFromString( const CORE::CString& name, TRpcMethodKind& kind )
{GUCEF_TRACE;

    if ( name == "unary" ) { kind = RPC_METHODKIND_UNARY; return true; }
    if ( name == "server_stream" ) { kind = RPC_METHODKIND_SERVER_STREAM; return true; }
    if ( name == "client_stream" ) { kind = RPC_METHODKIND_CLIENT_STREAM; return true; }
    if ( name == "bidi_stream" ) { kind = RPC_METHODKIND_BIDI_STREAM; return true; }
    return false;
}

/*-------------------------------------------------------------------------*/

const char*
RpcSessionStateToString( TRpcSessionState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case RPC_SESSIONSTATE_IDLE: return "IDLE";
        case RPC_SESSIONSTATE_CONNECTING: return "CONNECTING";
        case RPC_SESSIONSTATE_ESTABLISHED: return "ESTABLISHED";
        case RPC_SESSIONSTATE_SUSPENDED: return "SUSPENDED";
        case RPC_SESSIONSTATE_CLOSING: return "CLOSING";
        case RPC_SESSIONSTATE_CLOSED: return "CLOSED";
        default: return "UNKNOWN";
    }
}

/*-------------------------------------------------------------------------*/

UInt32
RpcWireTypeOfFieldType( TRpcFieldType type, bool fixedScaleDecimal )
{GUCEF_TRACE;

    switch ( type )
    {
        case RPC_FIELDTYPE_BOOL:
        case RPC_FIELDTYPE_INT8:
        case RPC_FIELDTYPE_INT16:
        case RPC_FIELDTYPE_INT32:
        case RPC_FIELDTYPE_INT64:
        case RPC_FIELDTYPE_UINT8:
        case RPC_FIELDTYPE_UINT16:
        case RPC_FIELDTYPE_UINT32:
        case RPC_FIELDTYPE_UINT64:
        case RPC_FIELDTYPE_DATE:
        case RPC_FIELDTYPE_TIMESTAMP:
        case RPC_FIELDTYPE_DURATION:
        case RPC_FIELDTYPE_ENUM:
            return GUCEF_RPC_WIRETYPE_VARINT;
        case RPC_FIELDTYPE_FLOAT32:
            return GUCEF_RPC_WIRETYPE_FIXED32;
        case RPC_FIELDTYPE_FLOAT64:
            return GUCEF_RPC_WIRETYPE_FIXED64;
        case RPC_FIELDTYPE_DECIMAL:
            return fixedScaleDecimal ? GUCEF_RPC_WIRETYPE_VARINT : GUCEF_RPC_WIRETYPE_LEN;
        default:
            return GUCEF_RPC_WIRETYPE_LEN;
    }
}

/*-------------------------------------------------------------------------*/

bool
RpcFieldTypeIsPackable( TRpcFieldType type, bool fixedScaleDecimal )
{GUCEF_TRACE;

    return GUCEF_RPC_WIRETYPE_LEN != RpcWireTypeOfFieldType( type, fixedScaleDecimal );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
