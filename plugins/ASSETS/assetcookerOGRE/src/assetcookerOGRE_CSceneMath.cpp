/*
 *  assetcookerOGRE: GUCEF asset converter plugin: the cooker of the intermediate into Ogre 1.x runtime files
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#include "assetcookerOGRE_CSceneMath.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const Ogre::Real Tolerance = (Ogre::Real) 1e-5;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSceneMath::STrs::STrs( void )
    : position( Ogre::Vector3::ZERO )
    , rotation( Ogre::Quaternion::IDENTITY )
    , scale( Ogre::Vector3::UNIT_SCALE )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSceneMath::STrs
CSceneMath::ToTrs( const MATH::TFloat32Transform3D& transform )
{GUCEF_TRACE;

    STrs trs;
    trs.position = Ogre::Vector3( (Ogre::Real) transform.position.x, (Ogre::Real) transform.position.y, (Ogre::Real) transform.position.z );
    trs.rotation = Ogre::Quaternion( transform.rotation.w, transform.rotation.x, transform.rotation.y, transform.rotation.z );
    trs.scale = Ogre::Vector3( transform.scale.x, transform.scale.y, transform.scale.z );
    return trs;
}

/*-------------------------------------------------------------------------*/

Ogre::Affine3
CSceneMath::ToAffine( const STrs& trs )
{GUCEF_TRACE;

    return Ogre::Affine3( trs.position, trs.rotation, trs.scale );
}

/*-------------------------------------------------------------------------*/

Ogre::Affine3
CSceneMath::ToAffine( const MATH::TFloat32Transform3D& transform )
{GUCEF_TRACE;

    return ToAffine( ToTrs( transform ) );
}

/*-------------------------------------------------------------------------*/

Ogre::Affine3
CSceneMath::ToAffine( const MATH::TFloat32Matrix4D& matrix )
{GUCEF_TRACE;

    return Ogre::Affine3( matrix( 0, 0 ), matrix( 0, 1 ), matrix( 0, 2 ), matrix( 0, 3 ),
                          matrix( 1, 0 ), matrix( 1, 1 ), matrix( 1, 2 ), matrix( 1, 3 ),
                          matrix( 2, 0 ), matrix( 2, 1 ), matrix( 2, 2 ), matrix( 2, 3 ) );
}

/*-------------------------------------------------------------------------*/

CSceneMath::STrs
CSceneMath::Decompose( const Ogre::Affine3& matrix )
{GUCEF_TRACE;

    STrs trs;
    matrix.decomposition( trs.position, trs.scale, trs.rotation );
    trs.rotation.normalise();
    return trs;
}

/*-------------------------------------------------------------------------*/

Ogre::Affine3
CSceneMath::GetWorld( const ASSETCONVERTER::CAssetScene& scene, Int32 node )
{GUCEF_TRACE;

    Ogre::Affine3 world = Ogre::Affine3::IDENTITY;
    UInt32 guard = 0;
    while ( node >= 0 && (size_t) node < scene.nodes.size() && guard++ < scene.nodes.size() )
    {
        world = ToAffine( scene.nodes[ node ].transform ) * world;
        node = scene.nodes[ node ].parent;
    }
    return world;
}

/*-------------------------------------------------------------------------*/

bool
CSceneMath::IsNear( const STrs& a, const STrs& b )
{GUCEF_TRACE;

    return a.position.positionEquals( b.position, Tolerance ) &&
           a.scale.positionEquals( b.scale, Tolerance ) &&
           fabs( fabs( a.rotation.Dot( b.rotation ) ) - 1.0 ) <= Tolerance;
}

/*-------------------------------------------------------------------------*/

bool
CSceneMath::IsIdentity( const STrs& trs )
{GUCEF_TRACE;

    return IsNear( trs, STrs() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
