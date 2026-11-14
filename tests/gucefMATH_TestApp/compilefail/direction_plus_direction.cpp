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

#include "gucefMATH_directions.h"

// Expected to fail: the compiler error must name Adding_directions_is_undefined_use_AsVector
// The sum of two unit directions is not a unit direction; add their vectors ( AsVector() ) explicitly
int main( void )
{
    GUCEF::MATH::TFloat32DirectionVector3D a = GUCEF::MATH::TFloat32DirectionVector3D::UnitX();
    GUCEF::MATH::TFloat32DirectionVector3D b = GUCEF::MATH::TFloat32DirectionVector3D::UnitY();
    GUCEF::MATH::TFloat32DirectionVector3D c = a + b;
    return (int) c.GetX();
}
