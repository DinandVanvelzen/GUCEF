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

#include "gucefMATH_fractiontypes.h"
#include "gucefMATH_matrices.h"

// Expected to fail: the compiler error must name Rotation_requires_transcendentals
// Exact fraction matrices support products and inverses, but rotation builders need sine and cosine
int main( void )
{
    typedef GUCEF::MATH::CTMatrix3D< GUCEF::CORE::TInt64Fraction > TFractionMatrix3D;
    TFractionMatrix3D m = TFractionMatrix3D::RotationX( GUCEF::MATH::CTRadian< GUCEF::CORE::TInt64Fraction >() );
    return m.IsIdentity() ? 0 : 1;
}
