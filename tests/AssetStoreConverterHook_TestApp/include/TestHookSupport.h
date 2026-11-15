/*
 *  AssetStoreConverterHook_TestApp: Test application for the converter hook of the primary asset store (AssetStoreConverterHook)
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

#ifndef TESTHOOKSUPPORT_H
#define TESTHOOKSUPPORT_H

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

namespace HOOKTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Loads the plugins the tests need (JSON, zlib, parquet, vfstables, FreeImage and the glTF plugin of the converter) and registers the linked postgresql backend; false when one the tests cannot do without is missing */
bool SetUpTestEnvironment( void );

/** Unloads the plugins of the tests */
void TearDownTestEnvironment( void );

/** A new empty folder in the temp folder of the tests, mounted writeable at the VFS mount point; its file system path */
GUCEF::CORE::CString MountFreshDirectory( const GUCEF::CORE::CString& name, const GUCEF::CORE::CString& mountPoint );
void UnmountDirectory( const GUCEF::CORE::CString& name );

/** Pulses the global pulse generator for the time */
void PumpFor( GUCEF::UInt32 durationInMs );

bool WriteVfsFile( const GUCEF::CORE::CString& path, const GUCEF::CORE::CDynamicBuffer& content );
bool WriteVfsFile( const GUCEF::CORE::CString& path, const GUCEF::CORE::CString& content );
bool ReadVfsFile( const GUCEF::CORE::CString& path, GUCEF::CORE::CDynamicBuffer& content );

/** The text of a file of the VFS, empty when it cannot be read */
GUCEF::CORE::CString ReadVfsText( const GUCEF::CORE::CString& path );

/** An RGB PNG of the size, encoded with the PNG codec of gucefIMAGE */
bool MakePng( GUCEF::UInt32 width, GUCEF::UInt32 height, GUCEF::CORE::CDynamicBuffer& content );

/**
 *  The text of a glTF 2.0 file with one node of the name, a triangle with normals and texture coordinates in a data URI
 *  buffer, and a material whose base colour texture is the picture of the file name (an external file), or no material
 *  without a picture name; the text of 'generator' goes into the asset block, which makes the files of packs differ. Synthetic: made here, no content of anybody
 */
GUCEF::CORE::CString MakeTriangleGltf( const GUCEF::CORE::CString& nodeName, const GUCEF::CORE::CString& pictureName, const GUCEF::CORE::CString& generator = GUCEF::CORE::CString::Empty );

/** A source pack of 'modelCount' glTF models and the picture they share, with a pack.json that has the licence JSON; 'folder' is a VFS path */
bool WriteGltfPack( const GUCEF::CORE::CString& folder, const GUCEF::CORE::CString& title, const GUCEF::CORE::CString& licenceJson, GUCEF::UInt32 modelCount );

/** Waits until the future is ready, pulsing the global pulse generator; true when it is in time */
bool WaitForFuture( const GUCEF::CORE::CFutureResult& future, GUCEF::UInt32 timeoutInMs );

/** A connection string of a PostgreSQL test database from GUCEF_TEST_POSTGRESQL, false when it is not set */
bool GetPostgreSqlConnectionString( GUCEF::CORE::CString& connectionString );

/** A schema name for this run */
GUCEF::CORE::CString MakeTestSchemaName( void );

/** The result data of a future that finished, false when it did not finish or carries no T */
template < class T >
bool
GetFutureResult( const GUCEF::CORE::CFutureResult& future, T& result, GUCEF::UInt32 timeoutInMs = 60000 )
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

}; /* namespace HOOKTEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTHOOKSUPPORT_H ? */
