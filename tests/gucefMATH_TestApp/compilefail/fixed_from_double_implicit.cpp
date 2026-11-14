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
// A float converts to fixed point only explicitly: copy initialisation from 2.75 would otherwise truncate to 2
int main( void )
{
    GUCEF::MATH::TFixed32 value = 2.75;
    return (int) value.GetRaw();
}
