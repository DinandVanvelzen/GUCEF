/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_TESTAPP_TESTDATATABLESCAN_H
#define GUCEF_CORE_TESTAPP_TESTDATATABLESCAN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests CDataTableScan: sync and async scans give the same rows for every column type, ordered delivery with
 *  parallel decoding and few credits, the credit bound, limits, cancellation in every phase, errors of sources and
 *  readers, destruction while tasks run, batches outliving the scan, coalesced events, and scans without heap
 *  allocations once warm.
 */
void
PerformDataTableScanTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTDATATABLESCAN_H ? */
