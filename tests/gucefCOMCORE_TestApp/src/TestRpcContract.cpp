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

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#include "gucefCOMCORE_CRpcContractBuilder.h"
#define GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTBUILDER_H ? */

#ifndef GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H
#include "gucefCOMCORE_CRpcBuiltInContracts.h"
#define GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H
#endif /* GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTCOMPATIBILITY_H
#include "gucefCOMCORE_CRpcContractCompatibility.h"
#define GUCEF_COMCORE_CRPCCONTRACTCOMPATIBILITY_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTCOMPATIBILITY_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#include "gucefCOMCORE_CRpcContractRegistry.h"
#define GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H ? */

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#include "gucefCOMCORE_CRpcMessage.h"
#define GUCEF_COMCORE_CRPCMESSAGE_H
#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestRpcSupport.h"

#include "TestRpcContract.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

namespace {

/**
 *  Canonical schema hashes computed independently of this code (a separate encoder written from design 3.8 that
 *  shares nothing with gucefCOMCORE); a change here means the canonical encoding changed
 */
const char* g_expectedRpcHash = "92bec8da1d305eceb69eca3a4a29f7ae3e61dc8be89c943dd1d01bf6eee4b636";
const char* g_expectedNotes12Hash = "63ad439612a9adc29facc6e2ebd985b6064409b3c93d9f3424768b8fdc1f4ff4";
const char* g_expectedNotes10Hash = "4a7718ba9ccbc0a64aa3f89962d0d139e30e0b761ef8fd07c34ad625e1c36c76";

struct SInvalidCase
{
    const char* json;
    const char* expectedPath;
};

const SInvalidCase g_invalidCases[] =
{
    { "{\"contract\":\"T.x\",\"version\":\"1.0\"}", "contract" },
    { "{\"contract\":\"t.x\",\"version\":\"one\"}", "version" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\",\"fields\":[{\"number\":1,\"name\":\"a\",\"type\":\"bool\"},{\"number\":1,\"name\":\"b\",\"type\":\"bool\"}]}]}", "types[0].fields[1].number" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\",\"fields\":[{\"number\":1,\"name\":\"a\",\"type\":\"bool\"},{\"number\":2,\"name\":\"a\",\"type\":\"bool\"}]}]}", "types[0].fields[1].name" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\",\"fields\":[{\"number\":5,\"name\":\"a\",\"type\":\"bool\"}],\"reserved\":{\"numbers\":[5]}}]}", "types[0].fields[0].number" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\",\"fields\":[{\"number\":1,\"name\":\"old\",\"type\":\"bool\"}],\"reserved\":{\"names\":[\"old\"]}}]}", "types[0].fields[0].name" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\",\"fields\":[{\"number\":1,\"name\":\"a\",\"type\":\"Missing\"}]}]}", "types[0].fields[0].type" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\",\"fields\":[{\"number\":1,\"name\":\"a\",\"type\":\"map<string,map<string,bool>>\"}]}]}", "types[0].fields[0].type" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\",\"fields\":[{\"number\":1,\"name\":\"a\",\"type\":\"map<float64,bool>\"}]}]}", "types[0].fields[0].type" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\",\"fields\":[{\"number\":1,\"name\":\"a\",\"type\":\"bool\",\"since\":1}]}]}", "types[0].fields[0].since" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\",\"fields\":[{\"number\":1,\"name\":\"Bad\",\"type\":\"bool\"}]}]}", "types[0].fields[0].name" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\",\"fields\":[{\"number\":1,\"name\":\"a\",\"type\":\"decimal(19)\"}]}]}", "types[0].fields[0].type" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\",\"fields\":[{\"number\":536870912,\"name\":\"a\",\"type\":\"bool\"}]}]}", "types[0].fields[0].number" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\",\"fields\":[{\"number\":1,\"name\":\"a\",\"type\":\"other.pkg.Thing\"}]}]}", "types[0].fields[0].type" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"bad\"}]}", "types[0].name" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"enums\":[{\"name\":\"E\",\"values\":[{\"name\":\"E_A\",\"number\":1},{\"name\":\"E_B\",\"number\":1}]}]}", "enums[0].values[1].number" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"enums\":[{\"name\":\"E\",\"values\":[{\"name\":\"lower\",\"number\":1}]}]}", "enums[0].values[0].name" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\"}],\"services\":[{\"name\":\"S\",\"number\":1,\"methods\":[{\"number\":1,\"name\":\"M\",\"kind\":\"server_stream\",\"request\":\"A\",\"response\":\"A\",\"flags\":[\"mayAnswerAsync\"]}]}]}", "services[0].methods[0].flags" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\"}],\"services\":[{\"name\":\"S\",\"number\":1,\"methods\":[{\"number\":1,\"name\":\"M\",\"kind\":\"unary\",\"request\":\"A\",\"response\":\"A\",\"initialCredit\":5}]}]}", "services[0].methods[0].initialCredit" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\"}],\"services\":[{\"name\":\"S\",\"number\":0,\"methods\":[]}]}", "services[0].number" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\"}],\"services\":[{\"name\":\"S\",\"number\":1,\"methods\":[{\"number\":1,\"name\":\"M\",\"request\":\"Nope\",\"response\":\"A\"}]}]}", "services[0].methods[0].request" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\"}],\"services\":[{\"name\":\"S\",\"number\":1,\"methods\":[{\"number\":7,\"name\":\"M\",\"request\":\"A\",\"response\":\"A\"}],\"reserved\":{\"numbers\":[7]}}]}", "services[0].methods[0].number" },
    { "{\"contract\":\"t.x\",\"version\":\"1.0\",\"types\":[{\"name\":\"A\"}],\"services\":[{\"name\":\"S\",\"number\":1,\"methods\":[{\"number\":1,\"name\":\"M\",\"kind\":\"sideways\",\"request\":\"A\",\"response\":\"A\"}]}]}", "services[0].methods[0].kind" }
};

/** Variants of one small contract for the compatibility table of design 3.9 */
COMCORE::CRpcContractPtr
BuildCompatVariant( UInt32 major, UInt32 minor, int change )
{
    COMCORE::CRpcContractBuilder b( "gucef.test.compat", major, minor );
    b.AddEnum( "Level" );
    b.AddEnumValue( "Level", 0, "LEVEL_LOW" );
    b.AddEnumValue( "Level", 1, 6 == change ? "LEVEL_UPPER" : "LEVEL_HIGH" );
    if ( 7 == change )
        b.AddEnumValue( "Level", 2, "LEVEL_MAX", minor );

    b.AddType( "Item" );
    b.AddField( "Item", 1, "id", 3 == change ? "uint64" : ( 4 == change ? "int32" : "uint32" ),
                5 == change ? COMCORE::RPC_CARDINALITY_OPTIONAL : ( 8 == change ? COMCORE::RPC_CARDINALITY_REPEATED : COMCORE::RPC_CARDINALITY_SINGLE ) );
    if ( 1 != change && 9 != change )
        b.AddField( "Item", 2, 2 == change ? "label" : "name", "string" );
    if ( 9 == change )
        b.AddField( "Item", 3, "name", "string" );          // renumbered: 2 removed, 3 with the old name
    if ( 10 == change )
        b.AddField( "Item", 4, "extra", "string", COMCORE::RPC_CARDINALITY_SINGLE, minor );
    if ( 11 == change )
        b.AddField( "Item", 4, "extra", "string", COMCORE::RPC_CARDINALITY_SINGLE, 0 );
    if ( 12 == change )
        b.AddField( "Item", 4, "extra", "string", COMCORE::RPC_CARDINALITY_SINGLE, minor, GUCEF_RPC_FIELDFLAG_REQUIRED );
    if ( 13 == change )
        b.AddField( "Item", 5, "reused", "string", COMCORE::RPC_CARDINALITY_SINGLE, minor );
    else
        b.AddReservedFieldNumber( "Item", 5 );
    b.AddType( "Other" );
    b.AddField( "Other", 1, "x", "bool" );

    b.AddService( "Store", 1 );
    UInt32 flags = 14 == change ? 0 : GUCEF_RPC_METHODFLAG_IDEMPOTENT;
    b.AddMethod( "Store", 1, "Get", 15 == change ? COMCORE::RPC_METHODKIND_SERVER_STREAM : COMCORE::RPC_METHODKIND_UNARY,
                 16 == change ? "Other" : "Item", "Item", flags, 17 == change ? 999 : 0 );
    if ( 18 != change )
        b.AddMethod( "Store", 2, "Put", COMCORE::RPC_METHODKIND_UNARY, "Item", "Item" );
    if ( 19 == change )
        b.AddMethod( "Store", 3, "List", COMCORE::RPC_METHODKIND_UNARY, "Item", "Item", 0, 0, minor );
    if ( 20 == change )
        b.SetMethodProtection( "Store", "Put", COMCORE::RPC_PROTECTION_PUBLIC, COMCORE::RPC_PROTECTION_CONFIDENTIAL );
    if ( 21 == change )
    {
        b.AddService( "Extra", 2, minor );
        b.AddMethod( "Extra", 1, "Ping", COMCORE::RPC_METHODKIND_UNARY, "Item", "Item", 0, 0, minor );
    }
    if ( 22 == change )
        b.AddType( "Added" );
    CORE::CString error;
    COMCORE::CRpcContractPtr contract = b.Build( error );
    if ( contract.IsNULL() )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcContract: compat variant " << change << " does not build: " << error );
    return contract;
}

bool
CompatIs( const COMCORE::CRpcContractPtr& older, const COMCORE::CRpcContractPtr& newer,
          COMCORE::CRpcContractCompatibility::TResult expected, const char* rule )
{
    if ( older.IsNULL() || newer.IsNULL() )
        return false;
    COMCORE::CRpcCompatibilityReport report;
    COMCORE::CRpcContractCompatibility::TResult result = COMCORE::CRpcContractCompatibility::Check( *older, *newer, report );
    bool ok = result == expected && ( GUCEF_NULL == rule || report.HasRule( rule ) );
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcContract: compatibility " << (Int32) result << " expected " << (Int32) expected <<
                         " rule " << ( GUCEF_NULL != rule ? rule : "-" ) << ":\n" << report.ToString() );
    return ok;
}

CORE::CString
TestCacheDir( const char* name )
{
    CORE::CString dir = CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( dir, name );
    CORE::DeleteDir( dir, true );
    return dir;
}

}

/*-------------------------------------------------------------------------*/

void
PerformRpcContractTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "rpc_contract" );

    GUCEF_TESTFW_TESTCASE( "rpc_contract: gucef.rpc 1.0 is built in code with the reflection service and meta-types" )
    try
    {
        COMCORE::CRpcContractPtr rpc = COMCORE::CRpcBuiltInContracts::GetRpcContract();
        ASSERT_FALSE( rpc.IsNULL() );
        if ( !rpc.IsNULL() )
        {
            ASSERT_TRUE( "gucef.rpc" == rpc->GetName() && 1 == rpc->GetMajor() && 0 == rpc->GetMinor() );
            ASSERT_TRUE( GUCEF_NULL != rpc->FindMethod( "Reflection/ListContracts" ) );
            ASSERT_TRUE( GUCEF_NULL != rpc->FindMethod( "gucef.rpc.Reflection/GetContract" ) );
            ASSERT_TRUE( GUCEF_NULL != rpc->FindType( "gucef.rpc.ContractSchema" ) );
            const CORE::CDynamicBuffer& schema = rpc->GetSchema();
            ASSERT_TRUE( schema.GetDataSize() > 5 && 0 == memcmp( schema.GetConstBufferPtr(), "GUCR\x01", 5 ) );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcContract: gucef.rpc 1.0 hash " << rpc->GetHash().ToHexString() << " schema " <<
                       RpcTestHex( schema.GetConstBufferPtr(), schema.GetDataSize() ) );
            ASSERT_TRUE( rpc->GetHash().ToHexString() == g_expectedRpcHash );
            // the same object every time
            ASSERT_TRUE( COMCORE::CRpcBuiltInContracts::GetRpcContract().GetPointerAlways() == rpc.GetPointerAlways() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_contract: the design example loads from JSON with resolved references" )
    try
    {
        ASSERT_TRUE( RpcTestJsonCodecAvailable() );
        COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
        ASSERT_FALSE( notes.IsNULL() );
        if ( !notes.IsNULL() )
        {
            ASSERT_TRUE( "gucef.sample.notes" == notes->GetName() && 1 == notes->GetMajor() && 2 == notes->GetMinor() );
            const COMCORE::CRpcTypeDescriptor* note = notes->FindType( "Note" );
            ASSERT_TRUE( GUCEF_NULL != note && 10 == note->GetFieldCount() );
            if ( GUCEF_NULL != note )
            {
                const COMCORE::CRpcFieldDescriptor* state = note->FindFieldByName( "state" );
                ASSERT_TRUE( GUCEF_NULL != state && COMCORE::RPC_FIELDTYPE_ENUM == state->GetType() && GUCEF_NULL != state->GetEnumType() );
                const COMCORE::CRpcFieldDescriptor* price = note->FindFieldByNumber( 8 );
                ASSERT_TRUE( GUCEF_NULL != price && price->IsFixedScaleDecimal() && 4 == price->GetDecimalScale() && 1 == price->GetSince() );
                const COMCORE::CRpcFieldDescriptor* attributes = note->FindFieldByName( "attributes" );
                ASSERT_TRUE( GUCEF_NULL != attributes && attributes->IsMap() && COMCORE::RPC_FIELDTYPE_STRING == attributes->GetMapKeyType() );
                ASSERT_TRUE( note->FindFieldByName( "body" )->IsSensitive() && note->FindFieldByName( "title" )->IsRequired() );
                ASSERT_TRUE( note->GetReservedNumbers().count( 11 ) == 1 );
            }
            const COMCORE::CRpcMethodDescriptor* save = notes->FindMethod( "gucef.sample.notes.Notes/Save" );
            ASSERT_TRUE( GUCEF_NULL != save && 2 == save->GetNumber() && save->IsSensitive() && 10000 == save->GetDefaultDeadlineMs() );
            ASSERT_TRUE( GUCEF_NULL != save && "gucef.sample.notes.Notes/Save" == save->GetPath() );
            const COMCORE::CRpcMethodDescriptor* listTags = notes->FindMethod( "PublicIndex/ListTags" );
            ASSERT_TRUE( GUCEF_NULL != listTags && COMCORE::RPC_PROTECTION_PUBLIC == listTags->GetProtection() &&
                         COMCORE::RPC_PROTECTION_CONFIDENTIAL == listTags->GetRequestProtection() );
            const COMCORE::CRpcMethodDescriptor* watch = notes->FindMethod( "Notes/Watch" );
            ASSERT_TRUE( GUCEF_NULL != watch && watch->IsResumableStream() && 64 == watch->GetInitialCredit() );
            ASSERT_TRUE( notes->HasDocs() );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcContract: gucef.sample.notes 1.2 hash " << notes->GetHash().ToHexString() );
            ASSERT_TRUE( notes->GetHash().ToHexString() == g_expectedNotes12Hash );
        }
        COMCORE::CRpcContractPtr notes10 = LoadSampleNotesContract( 1, 0 );
        ASSERT_FALSE( notes10.IsNULL() );
        if ( !notes10.IsNULL() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcContract: gucef.sample.notes 1.0 hash " << notes10->GetHash().ToHexString() );
            ASSERT_TRUE( notes10->GetHash().ToHexString() == g_expectedNotes10Hash );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_contract: save and load round trip; the hash ignores formatting and documentation" )
    try
    {
        COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
        ASSERT_FALSE( notes.IsNULL() );
        if ( !notes.IsNULL() )
        {
            CORE::CDataNode saved;
            ASSERT_TRUE( notes->SaveToDataNode( saved ) );
            CORE::CString error;
            COMCORE::CRpcContractPtr again = COMCORE::CRpcContract::LoadFromDataNode( saved, error );
            ASSERT_FALSE( again.IsNULL() );
            ASSERT_TRUE( !again.IsNULL() && again->GetHash() == notes->GetHash() );
            ASSERT_TRUE( !again.IsNULL() && again->GetDoc() == notes->GetDoc() );

            // documentation is not hashed
            saved.SetAttribute( "doc", "other words" );
            COMCORE::CRpcContractPtr otherDoc = COMCORE::CRpcContract::LoadFromDataNode( saved, error );
            ASSERT_TRUE( !otherDoc.IsNULL() && otherDoc->GetHash() == notes->GetHash() );

            // the schema decodes into the same contract, with its documentation
            const CORE::CDynamicBuffer& schema = notes->GetSchema();
            const CORE::CDynamicBuffer& docs = notes->GetDocs();
            COMCORE::CRpcContractPtr decoded = COMCORE::CRpcContract::FromSchema( schema.GetConstBufferPtr(), schema.GetDataSize(), error,
                                                                                docs.GetConstBufferPtr(), docs.GetDataSize() );
            ASSERT_FALSE( decoded.IsNULL() );
            if ( !decoded.IsNULL() )
            {
                ASSERT_TRUE( decoded->GetHash() == notes->GetHash() );
                ASSERT_TRUE( decoded->GetDoc() == notes->GetDoc() );
                ASSERT_TRUE( GUCEF_NULL != decoded->FindMethod( "Notes/Reindex" ) && decoded->FindMethod( "Notes/Reindex" )->MayAnswerAsync() );
                CORE::CDataNode decodedSaved;
                decoded->SaveToDataNode( decodedSaved );
                COMCORE::CRpcContractPtr third = COMCORE::CRpcContract::LoadFromDataNode( decodedSaved, error );
                ASSERT_TRUE( !third.IsNULL() && third->GetHash() == notes->GetHash() );
            }

            // garbage is refused
            const UInt8 garbage[] = { 'G', 'U', 'C', 'R', 1, 0xFF, 0xFF };
            ASSERT_TRUE( COMCORE::CRpcContract::FromSchema( garbage, sizeof( garbage ), error ).IsNULL() );
            const UInt8 badMagic[] = { 'X', 'U', 'C', 'R', 1 };
            ASSERT_TRUE( COMCORE::CRpcContract::FromSchema( badMagic, sizeof( badMagic ), error ).IsNULL() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_contract: every validation rule refuses with the JSON path" )
    try
    {
        for ( UInt32 i=0; i<sizeof( g_invalidCases ) / sizeof( g_invalidCases[ 0 ] ); ++i )
        {
            CORE::CDataNode tree;
            if ( !RpcTestParseJson( g_invalidCases[ i ].json, tree ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcContract: case " << i << " is not valid JSON" );
                ERRORHERE;
                continue;
            }
            CORE::CString error;
            COMCORE::CRpcContractPtr contract = COMCORE::CRpcContract::LoadFromDataNode( tree, error );
            bool refused = contract.IsNULL();
            bool pathOk = error.HasSubstr( g_invalidCases[ i ].expectedPath ) == 0;
            if ( !refused || !pathOk )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RpcContract: case " << i << " expected a refusal at " << g_invalidCases[ i ].expectedPath << ", got: " << error );
            ASSERT_TRUE( refused );
            ASSERT_TRUE( pathOk );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_contract: the compatibility checker applies every rule of design 3.9" )
    try
    {
        typedef COMCORE::CRpcContractCompatibility C;
        COMCORE::CRpcContractPtr base = BuildCompatVariant( 1, 0, 0 );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 0, 0 ), C::RESULT_COMPATIBLE, GUCEF_NULL ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 10 ), C::RESULT_COMPATIBLE, "ADDED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 11 ), C::RESULT_BREAKING, "SINCE_MISMATCH" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 1 ), C::RESULT_BREAKING, "REMOVED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 2 ), C::RESULT_BREAKING, "RENAMED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 9 ), C::RESULT_BREAKING, "RENUMBERED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 3 ), C::RESULT_COMPATIBLE_WITH_WARNINGS, "WIDENED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 4 ), C::RESULT_BREAKING, "TYPE_CHANGED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 5 ), C::RESULT_COMPATIBLE_WITH_WARNINGS, "OPTIONALITY_CHANGED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 8 ), C::RESULT_BREAKING, "CARDINALITY_CHANGED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 12 ), C::RESULT_BREAKING, "REQUIRED_ADDED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 13 ), C::RESULT_BREAKING, "RESERVED_REUSED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 6 ), C::RESULT_BREAKING, "RENAMED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 7 ), C::RESULT_COMPATIBLE, "ADDED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 14 ), C::RESULT_BREAKING, "FLAG_CHANGED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 15 ), C::RESULT_BREAKING, "KIND_CHANGED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 16 ), C::RESULT_BREAKING, "TYPE_CHANGED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 17 ), C::RESULT_COMPATIBLE_WITH_WARNINGS, "DEADLINE_CHANGED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 18 ), C::RESULT_BREAKING, "REMOVED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 19 ), C::RESULT_COMPATIBLE, "ADDED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 20 ), C::RESULT_BREAKING, "PROTECTION_WEAKENED" ) );
        ASSERT_TRUE( CompatIs( BuildCompatVariant( 1, 0, 20 ), BuildCompatVariant( 1, 1, 0 ), C::RESULT_COMPATIBLE_WITH_WARNINGS, "PROTECTION_CHANGED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 21 ), C::RESULT_COMPATIBLE, "ADDED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 1, 1, 22 ), C::RESULT_COMPATIBLE, "ADDED" ) );
        ASSERT_TRUE( CompatIs( BuildCompatVariant( 1, 1, 0 ), base, C::RESULT_BREAKING, "MINOR_LOWERED" ) );
        // a new major may change anything: reported, not breaking
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 2, 0, 1 ), C::RESULT_COMPATIBLE_WITH_WARNINGS, "MAJOR_CHANGED" ) );
        ASSERT_TRUE( CompatIs( base, BuildCompatVariant( 2, 0, 15 ), C::RESULT_COMPATIBLE_WITH_WARNINGS, "KIND_CHANGED" ) );

        // the sample: 1.0 -> 1.2 is a compatible minor step
        COMCORE::CRpcContractPtr n10 = LoadSampleNotesContract( 1, 0 );
        COMCORE::CRpcContractPtr n12 = LoadSampleNotesContract( 1, 2 );
        COMCORE::CRpcCompatibilityReport report;
        if ( !n10.IsNULL() && !n12.IsNULL() )
        {
            // the sample adds elements of minors 1 and 2: a step over two minors is compatible
            C::TResult r = C::Check( *n10, *n12, report );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcContract: notes 1.0 -> 1.2:\n" << report.ToString() );
            ASSERT_TRUE( C::RESULT_COMPATIBLE_WITH_WARNINGS == r || C::RESULT_COMPATIBLE == r );
            ASSERT_TRUE( 0 == report.GetBreakingCount() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_contract: the registry finds contracts by hash and by name; a peer contract is not picked by name" )
    try
    {
        COMCORE::CRpcContractRegistry registry;
        registry.SetCacheDirectory( TestCacheDir( "rpc_registry_noncache" ) );
        COMCORE::CRpcContractPtr n12 = LoadSampleNotesContract( 1, 2 );
        COMCORE::CRpcContractPtr n10 = LoadSampleNotesContract( 1, 0 );
        ASSERT_TRUE( !n12.IsNULL() && !n10.IsNULL() );
        if ( !n12.IsNULL() && !n10.IsNULL() )
        {
            ASSERT_TRUE( registry.Register( n10, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE ) );
            ASSERT_TRUE( registry.Register( n12, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_PEER ) );
            ASSERT_TRUE( registry.FindByHash( n12->GetHash() ).GetPointerAlways() == n12.GetPointerAlways() );
            COMCORE::CRpcContractPtr latest = registry.FindLatest( "gucef.sample.notes", 1 );
            ASSERT_TRUE( !latest.IsNULL() && 0 == latest->GetMinor() );
            ASSERT_TRUE( registry.FindAtLeast( "gucef.sample.notes", 1, 1 ).IsNULL() );
            ASSERT_TRUE( !registry.FindLatest( "gucef.rpc", 1 ).IsNULL() );
            ASSERT_TRUE( registry.FindLatest( "gucef.sample.notes", 2 ).IsNULL() );

            // registering the same contract as a file makes it eligible by name
            ASSERT_TRUE( registry.Register( n12, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE ) );
            latest = registry.FindLatest( "gucef.sample.notes", 1 );
            ASSERT_TRUE( !latest.IsNULL() && 2 == latest->GetMinor() );
        }

        // the default cache directory resolves $CACHEDIR$
        COMCORE::CRpcContractRegistry defaults;
        ASSERT_TRUE( defaults.GetCacheDirectory().HasSubstr( "$CACHEDIR$" ) < 0 );
        ASSERT_TRUE( defaults.GetCacheDirectory().HasSubstr( "rpc-contracts" ) > 0 );
        ASSERT_TRUE( COMCORE::CRpcContractRegistry::DefaultCacheMaxBytes == defaults.GetCacheMaxBytes() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "rpc_contract: the disk cache keeps peer contracts, verifies them on load, deletes corrupt files and prunes" )
    try
    {
        CORE::CString dir = TestCacheDir( "rpc_registry_cache" );
        COMCORE::CRpcContractPtr n12 = LoadSampleNotesContract( 1, 2 );
        COMCORE::CRpcContractPtr n10 = LoadSampleNotesContract( 1, 0 );
        ASSERT_TRUE( !n12.IsNULL() && !n10.IsNULL() );
        if ( !n12.IsNULL() && !n10.IsNULL() )
        {
            CORE::CString path;
            {
                COMCORE::CRpcContractRegistry first;
                first.SetCacheDirectory( dir );
                ASSERT_TRUE( first.Register( n12, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_PEER ) );
                path = first.GetCacheFilePath( n12 );
                ASSERT_TRUE( CORE::FileExists( path ) );
                ASSERT_TRUE( path.HasSubstr( "gucef.sample.notes" ) > 0 && path.HasSubstr( "1.2" ) > 0 );
                ASSERT_TRUE( CORE::FileExists( path.CutChars( 4, false ) + ".docs.bin" ) );
                // built-in contracts are not cached
                ASSERT_FALSE( CORE::FileExists( first.GetCacheFilePath( COMCORE::CRpcBuiltInContracts::GetRpcContract() ) ) );
            }
            {
                // a fresh registry (a new process) finds it in the cache, with its documentation
                COMCORE::CRpcContractRegistry second;
                second.SetCacheDirectory( dir );
                COMCORE::CRpcContractPtr found = second.FindByHash( n12->GetHash() );
                ASSERT_TRUE( !found.IsNULL() && found->GetHash() == n12->GetHash() && found->GetDoc() == n12->GetDoc() );
                COMCORE::CRpcContractRegistry::TContractSource source;
                ASSERT_TRUE( second.GetSource( n12->GetHash(), source ) && COMCORE::CRpcContractRegistry::CONTRACTSOURCE_CACHE == source );
                ASSERT_TRUE( second.FindLatest( "gucef.sample.notes", 1 ).IsNULL() );
            }
            {
                // a corrupted file is deleted and not used
                CORE::CDynamicBuffer content;
                content.LoadContentFromFile( path );
                ASSERT_TRUE( content.GetDataSize() > 10 );
                static_cast< UInt8* >( content.GetBufferPtr() )[ 8 ] ^= 0x01;
                CORE::WriteFileAtomically( path, content );
                COMCORE::CRpcContractRegistry third;
                third.SetCacheDirectory( dir );
                ASSERT_TRUE( third.FindByHash( n12->GetHash() ).IsNULL() );
                ASSERT_FALSE( CORE::FileExists( path ) );
            }
            {
                // pruning, oldest first
                COMCORE::CRpcContractRegistry fourth;
                fourth.SetCacheDirectory( dir );
                fourth.SetCacheMaxBytes( 1 );
                ASSERT_TRUE( fourth.StoreInCache( n10, GUCEF_NULL ) );
                ASSERT_FALSE( CORE::FileExists( fourth.GetCacheFilePath( n10 ) ) );
                fourth.SetCacheMaxBytes( 0 );
                ASSERT_TRUE( fourth.StoreInCache( n10, GUCEF_NULL ) && CORE::FileExists( fourth.GetCacheFilePath( n10 ) ) );
                MT::PrecisionDelay( 1100 );
                fourth.SetCacheMaxBytes( n10->GetSchema().GetDataSize() + n12->GetSchema().GetDataSize() - 1 );
                ASSERT_TRUE( fourth.StoreInCache( n12, GUCEF_NULL ) );
                ASSERT_FALSE( CORE::FileExists( fourth.GetCacheFilePath( n10 ) ) );
                ASSERT_TRUE( CORE::FileExists( fourth.GetCacheFilePath( n12 ) ) );
            }
        }
        CORE::DeleteDir( dir, true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
