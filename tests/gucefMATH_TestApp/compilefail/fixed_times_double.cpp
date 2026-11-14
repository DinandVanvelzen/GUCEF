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

#include "gucefMATH_fixedpoint.h"

// Expected to fail: the compiler error must name CTFixedPoint_implicit_construction_requires_an_integer_of_at_most_32_bits
// x * 0.5 would otherwise convert 0.5 to the integer 0 and give 0
int main( void )
{
    GUCEF::MATH::TFixed32 value( 3 );
    return (int) ( value * 0.5 ).GetRaw();
}
