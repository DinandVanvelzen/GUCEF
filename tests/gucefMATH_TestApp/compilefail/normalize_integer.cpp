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

// Expected to fail: the compiler error must name Normalization_requires_a_numeric_kind_with_sqrt
// Normalising needs 1 / sqrt( length^2 ), which an integer kind does not have (vectors use the same assertion name pattern)
int main( void )
{
    GUCEF::Int32 lengthSquared = 25;
    return (int) GUCEF::MATH::InverseSqrt( lengthSquared );
}
