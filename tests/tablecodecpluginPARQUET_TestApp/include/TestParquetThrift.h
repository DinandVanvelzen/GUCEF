/*
 *  tablecodecpluginPARQUET_TestApp: tests of the parquet table codec plugin
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

#ifndef TABLECODECPLUGIN_PARQUET_TESTAPP_TESTPARQUETTHRIFT_H
#define TABLECODECPLUGIN_PARQUET_TESTAPP_TESTPARQUETTHRIFT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The Thrift compact protocol reader and writer, with golden bytes of the protocol specification, and the metadata structures */
void PerformParquetThriftTests( void );

/*-------------------------------------------------------------------------*/

#endif /* TABLECODECPLUGIN_PARQUET_TESTAPP_TESTPARQUETTHRIFT_H ? */
