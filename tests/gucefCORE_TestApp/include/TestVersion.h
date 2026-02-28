/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_TESTAPP_TESTVERSION_H
#define GUCEF_CORE_TESTAPP_TESTVERSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests CVersion class behaviour including:
 *  - Construction (default, parameterized, copy, from TVersion)
 *  - Getters and setters for version components
 *  - Comparison operators (<, >, ==)
 *  - Assignment operators
 *  - String conversion (FromString, ToString, operator CString)
 *  - IsAllZero and Clear
 *  Sets a user break when a test fails.
 */
void
PerformVersionTests( void );

/**
 *  Tests CVersionRange class behaviour including:
 *  - Construction (default, parameterized, copy)
 *  - Getters for min/max and inclusion flags
 *  - Assignment operators
 *  - Equality and ordering operators
 *  - C-style struct conversion (GetCStyle, SetCStyle)
 *  - FromString for supported single-section formats
 *  Sets a user break when a test fails.
 */
void
PerformVersionRangeTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTVERSION_H ? */
