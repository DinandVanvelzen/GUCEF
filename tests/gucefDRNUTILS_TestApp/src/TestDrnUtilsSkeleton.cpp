/*
 *  gucefDRNUTILS_TestApp: GUCEF test application for the gucefDRNUTILS module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#ifndef GUCEF_DRNUTILS_H
#include "gucefDRNUTILS.h"
#define GUCEF_DRNUTILS_H
#endif /* GUCEF_DRNUTILS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_COMCORE_TESTAPP_TESTRPCSUPPORT_H
#include "TestRpcSupport.h"
#define GUCEF_COMCORE_TESTAPP_TESTRPCSUPPORT_H
#endif /* GUCEF_COMCORE_TESTAPP_TESTRPCSUPPORT_H ? */

#include "TestDrnUtilsSkeleton.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)

/*-------------------------------------------------------------------------*/

template < class TSchema >
static void
CheckTableSchema( void )
{GUCEF_TRACE;

    CORE::CDataTableSchema schema;
    TSchema::GetSchema( schema );
    ASSERT_TRUE( schema.GetColumnCount() == (UInt32) TSchema::COLUMN_COUNT );
    ASSERT_TRUE( 0 == schema.FindColumn( TSchema::KeyColumnName ) );
    CORE::CString error;
    bool valid = schema.Validate( error );
    if ( !valid )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DrnUtilsSkeleton: table schema " << TSchema::TableName << " is not valid: " << error );
    ASSERT_TRUE( valid );
    CORE::CString::StringVector names;
    TSchema::GetColumnNames( names );
    ASSERT_TRUE( names.size() == (size_t) TSchema::COLUMN_COUNT );
}

/*-------------------------------------------------------------------------*/

static bool
LoadContractSource( const CORE::CString& path, CORE::CDataNode& tree )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) || codec.IsNULL() )
        return false;
    CORE::CFileAccess file( path, "rb" );
    if ( !file.IsValid() )
        return false;
    tree.Clear();
    return codec->BuildDataTree( &tree, &file );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDrnUtilsSkeletonTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsSkeleton" );

    GUCEF_TESTFW_TESTCASE( "ModuleGlobal" )
    {
        ASSERT_TRUE( GUCEF_NULL != DRNUTILS::CDrnUtilsGlobal::Instance() );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "TableSchemas" )
    {
        CheckTableSchema< DRNUTILS::CJobsTableSchema >();
        CheckTableSchema< DRNUTILS::CLanesTableSchema >();
        CheckTableSchema< DRNUTILS::CRunsTableSchema >();
        CheckTableSchema< DRNUTILS::CPoolsTableSchema >();
        CheckTableSchema< DRNUTILS::CThreadsTableSchema >();
        CheckTableSchema< DRNUTILS::CTasksTableSchema >();
        CheckTableSchema< DRNUTILS::CScheduledJobsTableSchema >();
        ASSERT_TRUE( CORE::CString( "executionTarget" ) == DRNUTILS::CJobsTableSchema::GetColumnName( DRNUTILS::CJobsTableSchema::COLUMN_EXECUTION_TARGET ) );
        ASSERT_TRUE( CORE::CString( "threadPool" ) == DRNUTILS::CRunsTableSchema::GetColumnName( DRNUTILS::CRunsTableSchema::COLUMN_THREAD_POOL ) );
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "ContractSources" )
    {
        GUCEF_TESTFW_REQUIRE_ENV( rpcDirEnv, "GUCEF_DRNUTILS_RPC_DIR" );
        CORE::CString rpcDir = rpcDirEnv.c_str();
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );

        const char* files[ 2 ] = { "gucef.drnutils.scheduler.json", "gucef.drnutils.tasks.json" };
        const char* names[ 2 ] = { DRNUTILS::CSchedulerContracts::ContractName, DRNUTILS::CTaskManagerContracts::ContractName };
        for ( UInt32 i=0; i<2; ++i )
        {
            CORE::CString path = CORE::CombinePath( rpcDir, files[ i ] );
            CORE::CDataNode tree;
            bool parsed = LoadContractSource( path, tree );
            ASSERT_TRUE( parsed );
            if ( !parsed )
                continue;
            CORE::CString error;
            COMCORE::CRpcContractPtr contract = COMCORE::CRpcContract::LoadFromDataNode( tree, error );
            if ( contract.IsNULL() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DrnUtilsSkeleton: contract " << path << " does not load: " << error );
            ASSERT_TRUE( !contract.IsNULL() );
            if ( !contract.IsNULL() )
                ASSERT_TRUE( contract->GetName() == names[ i ] );
        }
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
