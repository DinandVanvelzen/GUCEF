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

#ifndef GUCEF_CORE_TESTAPP_JSONDATANODEFIDELITY_H
#define GUCEF_CORE_TESTAPP_JSONDATANODEFIDELITY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Pins down how JSON maps onto CDataNode through the json dstore codec (dstorepluginJSONPARSER):
 *  the variant types of numbers, booleans and null, empty objects and arrays, member names with '/' and '.',
 *  escapes, deep nesting and arrays of objects in a round trip
 */
void
PerformJsonDataNodeFidelityTests( void );

/*-------------------------------------------------------------------------*/

/** True when the json codec is registered, loading dstorepluginJSONPARSER when it is not */
bool
JsonTestCodecAvailable( void );

/*-------------------------------------------------------------------------*/

/** Parses JSON text into a tree with the json codec */
bool
ParseJsonTestText( const GUCEF::CORE::CString& text, GUCEF::CORE::CDataNode& tree );

/*-------------------------------------------------------------------------*/

/** Writes a tree as JSON text with the json codec */
bool
WriteJsonTestText( const GUCEF::CORE::CDataNode& tree, GUCEF::CORE::CString& text );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_JSONDATANODEFIDELITY_H ? */
