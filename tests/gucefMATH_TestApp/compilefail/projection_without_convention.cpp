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

#include "gucefMATH_matrices.h"

// Expected to fail: the compiler error must name SProjectionConvention
// Projection builders have no default convention: the depth range, handedness and Y direction are always stated
int main( void )
{
    GUCEF::MATH::TFloat32Matrix4D p = GUCEF::MATH::TFloat32Matrix4D::Perspective( GUCEF::MATH::TFloat32Radian( 1.0f ), 1.5f, 0.1f, 100.0f );
    return (int) p( 0, 0 );
}
