/*
 *  gucefMATH_TestApp: negative compile test, see README
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

#include "gucefMATH_ScalarFunctions.h"
#include "gucefMATH_CTNumericTraitsFraction.h"

// Expected to fail: the compiler error must name Sqrt_requires_a_numeric_kind_with_sqrt
// Exact fractions have no square root
int main( void )
{
    GUCEF::CORE::TInt64Fraction value( 1, 4 );
    GUCEF::MATH::Sqrt( value );
    return 0;
}
