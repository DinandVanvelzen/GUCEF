/*
 *  gucefGEOMETRY_TestApp: GUCEF test application for the Geometry module
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

#ifndef GUCEF_GEOMETRY_TESTAPP_TESTEPA_H
#define GUCEF_GEOMETRY_TESTAPP_TESTEPA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests EPA penetration depths, normals and witness points against analytic sphere depths and SAT box overlaps, the contact query of the convex pairs, degenerate polytopes, the caps, the fixed-point run with its pinned hash and the absence of allocations in GJK and EPA
 */
void
PerformEpaTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_TESTAPP_TESTEPA_H ? */
