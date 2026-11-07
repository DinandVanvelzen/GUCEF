/*
 *  dbpluginVFSTABLES_TestApp: tests of the vfstables database backend
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

#ifndef DBPLUGIN_VFSTABLES_TESTAPP_TESTVFSTABLESSUPPORT_H
#define DBPLUGIN_VFSTABLES_TESTAPP_TESTVFSTABLESSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_CORE_CDATATABLESCAN_H
#include "gucefCORE_CDataTableScan.h"
#define GUCEF_CORE_CDATATABLESCAN_H
#endif /* GUCEF_CORE_CDATATABLESCAN_H ? */

#ifndef GUCEF_COMCORE_CIDATABASECLIENT_H
#include "gucefCOMCORE_CIDatabaseClient.h"
#define GUCEF_COMCORE_CIDATABASECLIENT_H
#endif /* GUCEF_COMCORE_CIDATABASECLIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace VFSTABLESTEST {

typedef GUCEF::vector< GUCEF::CORE::CVariant > TRow;
typedef GUCEF::vector< TRow > TRowVector;

/** Loads the plugins the tests need at run time (gzip, JSON), registers the linked parquet codec and the backend */
void SetUpTestEnvironment( void );

/** A new empty directory below the temporary directory, mounted writeable (or not) at the VFS path; the native path */
GUCEF::CORE::CString MountFreshDirectory( const GUCEF::CORE::CString& name, const GUCEF::CORE::CString& mountPoint, bool writeable );

/** Unmounts what MountFreshDirectory mounted */
void UnmountDirectory( const GUCEF::CORE::CString& name );

/** Settings of a vfstables database at the VFS path */
GUCEF::COMCORE::CDatabaseClientSettings MakeSettings( const GUCEF::CORE::CString& database, bool readOnly );

/** A vfstables client opened with the settings, NULL when it would not open */
GUCEF::COMCORE::CIDatabaseClientPtr OpenDatabase( const GUCEF::COMCORE::CDatabaseClientSettings& settings );

/** A full batch of the rows, one value per schema column */
GUCEF::CORE::CDataTableBatchPtr MakeBatch( const GUCEF::CORE::CDataTableSchemaPtr& schema, const TRowVector& rows );

/** The rows of a scan of the table with the query as text, sorted; the status that ended the scan and its statistics */
GUCEF::CORE::TDataTableStatus ScanRows( GUCEF::CORE::CIDataTable& table, const GUCEF::CORE::CDataTableQuery& query,
                                        GUCEF::CORE::CString::StringVector& rows, GUCEF::CORE::CDataTableScanStats* stats = GUCEF_NULL,
                                        bool async = false );

/** The rows that match every predicate of the query, by brute force, as text in the projection of the query, sorted */
void BruteForceRows( const GUCEF::CORE::CDataTableSchemaPtr& schema, const TRowVector& rows, const GUCEF::CORE::CDataTableQuery& query,
                     GUCEF::CORE::CString::StringVector& expected );

/** Whether every file below the native directory is inside it (no file escaped through a path) and how many there are */
GUCEF::UInt32 CountFilesBelow( const GUCEF::CORE::CString& nativeDir );

/** A date variant */
GUCEF::CORE::CVariant Day( GUCEF::Int32 year, GUCEF::UInt32 month, GUCEF::UInt32 day );

/** Writes bytes to a native file, replacing it */
bool WriteNativeFile( const GUCEF::CORE::CString& path, const void* data, GUCEF::UInt32 size );

}; /* namespace VFSTABLESTEST */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_VFSTABLES_TESTAPP_TESTVFSTABLESSUPPORT_H ? */
