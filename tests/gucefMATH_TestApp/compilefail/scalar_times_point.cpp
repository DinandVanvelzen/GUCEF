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

#include "gucefMATH_points.h"

// Expected to fail: the compiler error must name Scaling_a_point_is_undefined_scale_a_vector
// A position cannot be scaled; scale the vector from a reference point instead
int main( void )
{
    GUCEF::MATH::TFloat32Point3D a( 1.0f, 2.0f, 3.0f );
    GUCEF::MATH::TFloat32Point3D b = 2.0f * a;
    return (int) b.x;
}
