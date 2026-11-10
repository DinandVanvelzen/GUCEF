/*
 *  gucefCOMCORE_TestApp: GUCEF test application for the COMCORE module
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

#ifndef GUCEF_COMCORE_TESTAPP_TESTRPCSUPPORT_H
#define GUCEF_COMCORE_TESTAPP_TESTRPCSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Test support for the RPC tests of gucefCOMCORE_TestApp, header only so gucefCOM_TestApp (tcp and MCP transports)
 *  includes it as well: the sample contract of the RPC design (gucef.sample.notes) in its versions, JSON parsing
 *  through the json dstore codec, pumping helpers.
 */

namespace RpcTestSamples {

static const char* g_notes12 =
"{"
"  \"contract\": \"gucef.sample.notes\","
"  \"version\": \"1.2\","
"  \"doc\": \"Notes kept per user, used by the RPC tests and as the authoring example\","
"  \"enums\": ["
"    {"
"      \"name\": \"NoteState\","
"      \"values\": ["
"        { \"name\": \"NOTE_STATE_UNSPECIFIED\", \"number\": 0 },"
"        { \"name\": \"NOTE_STATE_DRAFT\", \"number\": 1 },"
"        { \"name\": \"NOTE_STATE_PUBLISHED\", \"number\": 2 },"
"        { \"name\": \"NOTE_STATE_ARCHIVED\", \"number\": 3, \"since\": 1 }"
"      ],"
"      \"reserved\": { \"numbers\": [ 4 ], \"names\": [ \"NOTE_STATE_DELETED\" ] }"
"    }"
"  ],"
"  \"types\": ["
"    {"
"      \"name\": \"Note\","
"      \"fields\": ["
"        { \"number\": 1, \"name\": \"noteId\", \"type\": \"uint64\" },"
"        { \"number\": 2, \"name\": \"title\", \"type\": \"string\", \"required\": true },"
"        { \"number\": 3, \"name\": \"body\", \"type\": \"string\", \"sensitive\": true },"
"        { \"number\": 4, \"name\": \"state\", \"type\": \"NoteState\" },"
"        { \"number\": 5, \"name\": \"tags\", \"type\": \"string\", \"cardinality\": \"repeated\" },"
"        { \"number\": 6, \"name\": \"createdUtc\", \"type\": \"timestamp\" },"
"        { \"number\": 7, \"name\": \"dueDate\", \"type\": \"date\", \"cardinality\": \"optional\" },"
"        { \"number\": 8, \"name\": \"price\", \"type\": \"decimal(4)\", \"since\": 1 },"
"        { \"number\": 9, \"name\": \"attributes\", \"type\": \"map<string,string>\", \"since\": 2 },"
"        { \"number\": 10, \"name\": \"attachment\", \"type\": \"bytes\", \"since\": 2 }"
"      ],"
"      \"reserved\": { \"numbers\": [ 11 ], \"names\": [ \"author\" ] }"
"    },"
"    { \"name\": \"GetNoteRequest\", \"fields\": [ { \"number\": 1, \"name\": \"noteId\", \"type\": \"uint64\" } ] },"
"    {"
"      \"name\": \"SaveNoteRequest\","
"      \"fields\": ["
"        { \"number\": 1, \"name\": \"note\", \"type\": \"Note\" },"
"        { \"number\": 2, \"name\": \"expectedRevision\", \"type\": \"uint64\" },"
"        { \"number\": 3, \"name\": \"commandId\", \"type\": \"string\" }"
"      ]"
"    },"
"    {"
"      \"name\": \"SaveNoteResponse\","
"      \"fields\": ["
"        { \"number\": 1, \"name\": \"noteId\", \"type\": \"uint64\" },"
"        { \"number\": 2, \"name\": \"revision\", \"type\": \"uint64\" }"
"      ]"
"    },"
"    { \"name\": \"WatchRequest\", \"fields\": [ { \"number\": 1, \"name\": \"tags\", \"type\": \"string\", \"cardinality\": \"repeated\" } ] },"
"    {"
"      \"name\": \"NoteEvent\","
"      \"fields\": ["
"        { \"number\": 1, \"name\": \"note\", \"type\": \"Note\" },"
"        { \"number\": 2, \"name\": \"removed\", \"type\": \"bool\" }"
"      ]"
"    },"
"    { \"name\": \"ImportChunk\", \"fields\": [ { \"number\": 1, \"name\": \"data\", \"type\": \"bytes\" } ] },"
"    {"
"      \"name\": \"ImportSummary\","
"      \"fields\": ["
"        { \"number\": 1, \"name\": \"imported\", \"type\": \"uint32\" },"
"        { \"number\": 2, \"name\": \"rejected\", \"type\": \"uint32\" },"
"        { \"number\": 3, \"name\": \"details\", \"type\": \"datanode\" }"
"      ]"
"    },"
"    {"
"      \"name\": \"EditOp\","
"      \"fields\": ["
"        { \"number\": 1, \"name\": \"noteId\", \"type\": \"uint64\" },"
"        { \"number\": 2, \"name\": \"text\", \"type\": \"string\" },"
"        { \"number\": 3, \"name\": \"cell\", \"type\": \"variant\" }"
"      ]"
"    },"
"    { \"name\": \"ReindexRequest\", \"fields\": [ { \"number\": 1, \"name\": \"full\", \"type\": \"bool\" } ] },"
"    {"
"      \"name\": \"ReindexResult\","
"      \"fields\": ["
"        { \"number\": 1, \"name\": \"indexed\", \"type\": \"uint64\" },"
"        { \"number\": 2, \"name\": \"elapsed\", \"type\": \"duration\" }"
"      ]"
"    }"
"  ],"
"  \"services\": ["
"    {"
"      \"name\": \"Notes\","
"      \"number\": 1,"
"      \"methods\": ["
"        { \"number\": 1, \"name\": \"Get\", \"kind\": \"unary\","
"          \"request\": \"GetNoteRequest\", \"response\": \"Note\","
"          \"flags\": [ \"idempotent\" ], \"deadlineMs\": 5000 },"
"        { \"number\": 2, \"name\": \"Save\", \"kind\": \"unary\","
"          \"request\": \"SaveNoteRequest\", \"response\": \"SaveNoteResponse\","
"          \"flags\": [ \"sensitive\" ], \"deadlineMs\": 10000 },"
"        { \"number\": 3, \"name\": \"Watch\", \"kind\": \"server_stream\","
"          \"request\": \"WatchRequest\", \"response\": \"NoteEvent\","
"          \"flags\": [ \"resumableStream\" ], \"initialCredit\": 64 },"
"        { \"number\": 4, \"name\": \"Import\", \"kind\": \"client_stream\","
"          \"request\": \"ImportChunk\", \"response\": \"ImportSummary\", \"since\": 1 },"
"        { \"number\": 5, \"name\": \"Edit\", \"kind\": \"bidi_stream\","
"          \"request\": \"EditOp\", \"response\": \"EditOp\", \"since\": 1 },"
"        { \"number\": 6, \"name\": \"Reindex\", \"kind\": \"unary\","
"          \"request\": \"ReindexRequest\", \"response\": \"ReindexResult\","
"          \"flags\": [ \"mayAnswerAsync\", \"idempotent\" ], \"since\": 2 }"
"      ],"
"      \"reserved\": { \"numbers\": [ 7 ] }"
"    },"
"    {"
"      \"name\": \"PublicIndex\","
"      \"number\": 2,"
"      \"since\": 2,"
"      \"methods\": ["
"        { \"number\": 1, \"name\": \"ListTags\", \"kind\": \"server_stream\","
"          \"request\": \"WatchRequest\", \"response\": \"NoteEvent\","
"          \"protection\": \"public\", \"requestProtection\": \"confidential\" }"
"      ]"
"    }"
"  ]"
"}";

static const char* g_notes10 =
"{"
"  \"contract\": \"gucef.sample.notes\","
"  \"version\": \"1.0\","
"  \"enums\": ["
"    {"
"      \"name\": \"NoteState\","
"      \"values\": ["
"        { \"name\": \"NOTE_STATE_UNSPECIFIED\", \"number\": 0 },"
"        { \"name\": \"NOTE_STATE_DRAFT\", \"number\": 1 },"
"        { \"name\": \"NOTE_STATE_PUBLISHED\", \"number\": 2 }"
"      ],"
"      \"reserved\": { \"numbers\": [ 4 ], \"names\": [ \"NOTE_STATE_DELETED\" ] }"
"    }"
"  ],"
"  \"types\": ["
"    {"
"      \"name\": \"Note\","
"      \"fields\": ["
"        { \"number\": 1, \"name\": \"noteId\", \"type\": \"uint64\" },"
"        { \"number\": 2, \"name\": \"title\", \"type\": \"string\", \"required\": true },"
"        { \"number\": 3, \"name\": \"body\", \"type\": \"string\", \"sensitive\": true },"
"        { \"number\": 4, \"name\": \"state\", \"type\": \"NoteState\" },"
"        { \"number\": 5, \"name\": \"tags\", \"type\": \"string\", \"cardinality\": \"repeated\" },"
"        { \"number\": 6, \"name\": \"createdUtc\", \"type\": \"timestamp\" },"
"        { \"number\": 7, \"name\": \"dueDate\", \"type\": \"date\", \"cardinality\": \"optional\" }"
"      ],"
"      \"reserved\": { \"numbers\": [ 11 ], \"names\": [ \"author\" ] }"
"    },"
"    { \"name\": \"GetNoteRequest\", \"fields\": [ { \"number\": 1, \"name\": \"noteId\", \"type\": \"uint64\" } ] },"
"    {"
"      \"name\": \"SaveNoteRequest\","
"      \"fields\": ["
"        { \"number\": 1, \"name\": \"note\", \"type\": \"Note\" },"
"        { \"number\": 2, \"name\": \"expectedRevision\", \"type\": \"uint64\" },"
"        { \"number\": 3, \"name\": \"commandId\", \"type\": \"string\" }"
"      ]"
"    },"
"    {"
"      \"name\": \"SaveNoteResponse\","
"      \"fields\": ["
"        { \"number\": 1, \"name\": \"noteId\", \"type\": \"uint64\" },"
"        { \"number\": 2, \"name\": \"revision\", \"type\": \"uint64\" }"
"      ]"
"    },"
"    { \"name\": \"WatchRequest\", \"fields\": [ { \"number\": 1, \"name\": \"tags\", \"type\": \"string\", \"cardinality\": \"repeated\" } ] },"
"    {"
"      \"name\": \"NoteEvent\","
"      \"fields\": ["
"        { \"number\": 1, \"name\": \"note\", \"type\": \"Note\" },"
"        { \"number\": 2, \"name\": \"removed\", \"type\": \"bool\" }"
"      ]"
"    }"
"  ],"
"  \"services\": ["
"    {"
"      \"name\": \"Notes\","
"      \"number\": 1,"
"      \"methods\": ["
"        { \"number\": 1, \"name\": \"Get\", \"kind\": \"unary\","
"          \"request\": \"GetNoteRequest\", \"response\": \"Note\","
"          \"flags\": [ \"idempotent\" ], \"deadlineMs\": 5000 },"
"        { \"number\": 2, \"name\": \"Save\", \"kind\": \"unary\","
"          \"request\": \"SaveNoteRequest\", \"response\": \"SaveNoteResponse\","
"          \"flags\": [ \"sensitive\" ], \"deadlineMs\": 10000 },"
"        { \"number\": 3, \"name\": \"Watch\", \"kind\": \"server_stream\","
"          \"request\": \"WatchRequest\", \"response\": \"NoteEvent\","
"          \"flags\": [ \"resumableStream\" ], \"initialCredit\": 64 }"
"      ],"
"      \"reserved\": { \"numbers\": [ 7 ] }"
"    }"
"  ]"
"}";

static const char* g_notes20 =
"{"
"  \"contract\": \"gucef.sample.notes\","
"  \"version\": \"2.0\","
"  \"types\": ["
"    { \"name\": \"GetNoteRequest\", \"fields\": [ { \"number\": 1, \"name\": \"noteKey\", \"type\": \"string\" } ] },"
"    { \"name\": \"Note\", \"fields\": [ { \"number\": 1, \"name\": \"noteKey\", \"type\": \"string\" } ] }"
"  ],"
"  \"services\": ["
"    {"
"      \"name\": \"Notes\","
"      \"number\": 1,"
"      \"methods\": ["
"        { \"number\": 1, \"name\": \"Get\", \"kind\": \"unary\", \"request\": \"GetNoteRequest\", \"response\": \"Note\" }"
"      ]"
"    }"
"  ]"
"}";

}

/*-------------------------------------------------------------------------*/

inline const char*
GetSampleNotesContractJson( GUCEF::UInt32 major, GUCEF::UInt32 minor )
{GUCEF_TRACE;

    if ( 2 == major )
        return RpcTestSamples::g_notes20;
    return 0 == minor ? RpcTestSamples::g_notes10 : RpcTestSamples::g_notes12;
}

/*-------------------------------------------------------------------------*/

inline bool
RpcTestJsonCodecAvailable( void )
{GUCEF_TRACE;

    GUCEF::CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( GUCEF::CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) && !codec.IsNULL() )
        return true;

    #ifdef GUCEF_DEBUG_MODE
    GUCEF::CORE::CString firstPluginPath = "$MODULEDIR$/dstorepluginJSONPARSER_d";
    GUCEF::CORE::CString secondPluginPath = "$MODULEDIR$/dstorepluginJSONPARSER";
    #else
    GUCEF::CORE::CString firstPluginPath = "$MODULEDIR$/dstorepluginJSONPARSER";
    GUCEF::CORE::CString secondPluginPath = "$MODULEDIR$/dstorepluginJSONPARSER_d";
    #endif

    GUCEF::CORE::CPluginControl& pluginControl = GUCEF::CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
    return GUCEF::CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) && !codec.IsNULL();
}

/*-------------------------------------------------------------------------*/

inline bool
RpcTestParseJson( const char* text, GUCEF::CORE::CDataNode& tree )
{GUCEF_TRACE;

    GUCEF::CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !RpcTestJsonCodecAvailable() ||
         !GUCEF::CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) || codec.IsNULL() )
        return false;
    tree.Clear();
    GUCEF::CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( (GUCEF::UInt32) strlen( text ), text );
    GUCEF::CORE::CDynamicBufferAccess access( buffer );
    return codec->BuildDataTree( &tree, &access );
}

/*-------------------------------------------------------------------------*/

inline GUCEF::COMCORE::CRpcContractPtr
LoadSampleNotesContract( GUCEF::UInt32 major, GUCEF::UInt32 minor )
{GUCEF_TRACE;

    GUCEF::CORE::CDataNode tree;
    if ( !RpcTestParseJson( GetSampleNotesContractJson( major, minor ), tree ) )
    {
        GUCEF_ERROR_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "RpcTest: the sample contract JSON does not parse" );
        return GUCEF::COMCORE::CRpcContractPtr();
    }
    GUCEF::CORE::CString error;
    GUCEF::COMCORE::CRpcContractPtr contract = GUCEF::COMCORE::CRpcContract::LoadFromDataNode( tree, error );
    if ( contract.IsNULL() )
        GUCEF_ERROR_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "RpcTest: the sample contract does not load: " << error );
    return contract;
}

/*-------------------------------------------------------------------------*/

inline void
RpcTestPump( GUCEF::UInt32 milliseconds )
{GUCEF_TRACE;

    GUCEF::CORE::PulseGeneratorPtr pulseGenerator = GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    GUCEF::UInt64 start = GUCEF::MT::PrecisionTickCount();
    GUCEF::UInt64 ticksPerMs = GUCEF::MT::PrecisionTimerResolution() / 1000;
    do
    {
        pulseGenerator->ForceDirectPulse();
        GUCEF::MT::PrecisionDelay( 1 );
    }
    while ( ( GUCEF::MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) < milliseconds );
}

/*-------------------------------------------------------------------------*/

inline bool
RpcTestPumpUntil( const bool& flag, GUCEF::UInt32 timeoutMs )
{GUCEF_TRACE;

    GUCEF::CORE::PulseGeneratorPtr pulseGenerator = GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    GUCEF::UInt64 start = GUCEF::MT::PrecisionTickCount();
    GUCEF::UInt64 ticksPerMs = GUCEF::MT::PrecisionTimerResolution() / 1000;
    while ( !flag )
    {
        pulseGenerator->ForceDirectPulse();
        if ( flag )
            break;
        if ( ( GUCEF::MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) >= timeoutMs )
            return false;
        GUCEF::MT::PrecisionDelay( 1 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

inline GUCEF::CORE::CString
RpcTestHex( const void* data, GUCEF::UInt32 size )
{GUCEF_TRACE;

    static const char digits[] = "0123456789abcdef";
    GUCEF::CORE::CString text;
    const GUCEF::UInt8* bytes = static_cast< const GUCEF::UInt8* >( data );
    for ( GUCEF::UInt32 i=0; i<size; ++i )
    {
        char pair[ 3 ] = { digits[ bytes[ i ] >> 4 ], digits[ bytes[ i ] & 0x0F ], '\0' };
        text += pair;
    }
    return text;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTAPP_TESTRPCSUPPORT_H ? */
