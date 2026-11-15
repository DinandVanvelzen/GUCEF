/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef TESTASSETSTORESUPPORT_H
#define TESTASSETSTORESUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */
#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */
#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#include "gucefASSETSTORE_CAssetStoreDownloadManager.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSERT_TRUE
  #define ASSERT_TRUE( expr )   GUCEF_TESTFW_ASSERT_TRUE( expr )
#endif
#ifndef ASSERT_FALSE
  #define ASSERT_FALSE( expr )  GUCEF_TESTFW_ASSERT_FALSE( expr )
#endif
#ifndef ERRORHERE
  #define ERRORHERE             GUCEF_TESTFW_ERRORHERE
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Loads the plugins the tests need (JSON, zlib, parquet, vfstables) and registers the linked postgresql backend */
void SetUpTestEnvironment( void );

/** Unloads the plugins of the tests */
void TearDownTestEnvironment( void );

/** A new empty folder in the temp folder of the tests, mounted writeable at the VFS mount point; its file system path */
GUCEF::CORE::CString MountFreshDirectory( const GUCEF::CORE::CString& name, const GUCEF::CORE::CString& mountPoint );

void UnmountDirectory( const GUCEF::CORE::CString& name );

/** The file system path of a test folder */
GUCEF::CORE::CString TestDirectory( const GUCEF::CORE::CString& name );

/** Pulses the global pulse generator for the time */
void PumpFor( GUCEF::UInt32 durationInMs );

/** Pulses the global pulse generator until the job is done, failed, cancelled or paused; true when it got there in time */
bool PumpUntilJobSettled( GUCEF::ASSETSTORE::CAssetStoreDownloadManager& manager, const GUCEF::CORE::CString& jobId, GUCEF::UInt32 timeoutInMs );

/** Pulses until the job is in the state; true when it got there in time */
bool PumpUntilJobState( GUCEF::ASSETSTORE::CAssetStoreDownloadManager& manager, const GUCEF::CORE::CString& jobId,
                        GUCEF::ASSETSTORE::TAssetDownloadJobState state, GUCEF::UInt32 timeoutInMs );

/** Pulses until the job has at least the bytes; true when it got there in time */
bool PumpUntilJobBytes( GUCEF::ASSETSTORE::CAssetStoreDownloadManager& manager, const GUCEF::CORE::CString& jobId, GUCEF::UInt64 bytes, GUCEF::UInt32 timeoutInMs );

bool WriteVfsFile( const GUCEF::CORE::CString& path, const GUCEF::CORE::CDynamicBuffer& content );
bool WriteVfsFile( const GUCEF::CORE::CString& path, const GUCEF::CORE::CString& content );
bool ReadVfsFile( const GUCEF::CORE::CString& path, GUCEF::CORE::CDynamicBuffer& content );

/** Bytes of a pseudo random pattern */
void MakePattern( GUCEF::UInt32 size, GUCEF::UInt32 seed, GUCEF::CORE::CDynamicBuffer& content );

/** An RGB PNG of the size, encoded with the PNG codec of gucefIMAGE; the colour is that of a normal map when asked */
bool MakePng( GUCEF::UInt32 width, GUCEF::UInt32 height, bool normalMapColour, GUCEF::CORE::CDynamicBuffer& content );

/** An uncompressed 32 bit TGA of the size */
void MakeTga( GUCEF::UInt32 width, GUCEF::UInt32 height, GUCEF::CORE::CDynamicBuffer& content );

/** The SHA-256 of the bytes as lower case hexadecimal */
GUCEF::CORE::CString Sha256Of( const GUCEF::CORE::CDynamicBuffer& content );
GUCEF::CORE::CString Sha1Of( const GUCEF::CORE::CDynamicBuffer& content );

/** A connection string of a PostgreSQL test database from GUCEF_TEST_POSTGRESQL, false when it is not set */
bool GetPostgreSqlConnectionString( GUCEF::CORE::CString& connectionString );

/** A schema name for this run */
GUCEF::CORE::CString MakeTestSchemaName( void );

/** Waits until the future is ready, pulsing the global pulse generator; true when it is in time */
bool WaitForFuture( const GUCEF::CORE::CFutureResult& future, GUCEF::UInt32 timeoutInMs );

/** The result data of a future that finished, false when it did not finish or carries no T */
template < class T >
bool
GetFutureResult( const GUCEF::CORE::CFutureResult& future, T& result, GUCEF::UInt32 timeoutInMs = 30000 )
{
    if ( !WaitForFuture( future, timeoutInMs ) )
        return false;
    GUCEF::CORE::CTaskPtr task = future.GetResult( 0 );
    if ( task.IsNULL() || GUCEF::CORE::TASKSTATUS_FINISHED != task->GetTaskStatus() )
        return false;
    const T* data = GUCEF::CORE::CICloneable::CloneableAs< T >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
        return false;
    result = *data;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTASSETSTORESUPPORT_H ? */
