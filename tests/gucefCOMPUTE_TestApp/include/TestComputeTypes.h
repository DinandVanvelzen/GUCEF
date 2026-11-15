/*
 *  gucefCOMPUTE_TestApp: GUCEF test application for the Compute module
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

#ifndef GUCEF_COMPUTE_TESTAPP_TESTCOMPUTETYPES_H
#define GUCEF_COMPUTE_TESTAPP_TESTCOMPUTETYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The value types of gucefCOMPUTE: results and their names, capabilities and requirements, device info, signatures,
 *  dispatch sizes, arguments, artefacts, tolerances (with an independent Float64 reference), aligned memory and the
 *  settings with their config round trip
 */
void
PerformComputeTypesTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_TESTAPP_TESTCOMPUTETYPES_H ? */
