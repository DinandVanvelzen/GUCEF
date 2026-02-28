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

#ifndef GUCEF_CORE_TESTAPP_TESTFRACTION_H
#define GUCEF_CORE_TESTAPP_TESTFRACTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests CTFraction template class behaviour including:
 *  - Construction (default, parameterized, from C-style structs)
 *  - Normalization via GCD reduction
 *  - Zero-denominator exception
 *  - Arithmetic operators (+=, -=, *=, /=, binary forms)
 *  - Scalar arithmetic operators
 *  - Comparison operators (==, !=, <, <=, >, >=)
 *  - Float32 / Float64 conversion
 *  - Mixed fraction conversion
 *  - String conversion (ToString, FromString)
 *  - Multiple typedef instantiations
 *  Sets a user break when a test fails.
 */
void
PerformFractionTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTFRACTION_H ? */
