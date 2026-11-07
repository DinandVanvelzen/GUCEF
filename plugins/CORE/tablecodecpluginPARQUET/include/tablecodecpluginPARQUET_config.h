/*
 *  tablecodecpluginPARQUET: GUCEF table codec plugin for Apache Parquet files
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

#ifndef TABLECODECPLUGIN_PARQUET_CONFIG_H
#define TABLECODECPLUGIN_PARQUET_CONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      BUILD CONFIG                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The classes of the plugin are exported as well, so the tests of the plugin can link it and test the
 *  Thrift, encoding and metadata classes directly.
 */
#define TABLECODECPLUGIN_PARQUET_PUBLIC_CPP_CODE
#define TABLECODECPLUGIN_PARQUET_PUBLIC_C_CODE

/*-------------------------------------------------------------------------*/

#endif /* TABLECODECPLUGIN_PARQUET_CONFIG_H ? */
