/*
 *  AssetConverterLib: a collider of the intermediate model, as an OMI physics shape
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

#include "AssetConverterLib_CAssetCollider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetCollider::CAssetCollider( void )
    : name()
    , shape( COLLIDERSHAPE_BOX )
    , radius( 0.5f )
    , height( 1.0f )
    , transform()
    , trigger( false )
    , meshIndex( -1 )
{GUCEF_TRACE;

    halfExtents[ 0 ] = halfExtents[ 1 ] = halfExtents[ 2 ] = 0.5f;
}

/*-------------------------------------------------------------------------*/

bool
CAssetCollider::IsMeshShape( void ) const
{GUCEF_TRACE;

    return COLLIDERSHAPE_CONVEX == shape || COLLIDERSHAPE_TRIMESH == shape;
}

/*-------------------------------------------------------------------------*/

bool
CAssetCollider::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "name", name );
    node.SetAttribute( "shape", ToString( shape ) );
    CORE::CDataNode::TVariantVector values;
    switch ( shape )
    {
        case COLLIDERSHAPE_BOX:
        {
            for ( UInt32 i=0; i<3; ++i )
                values.push_back( CORE::CVariant( halfExtents[ i ] ) );
            node.SetValuesOfChildByName( "halfExtents", values );
            break;
        }
        case COLLIDERSHAPE_SPHERE:
            node.SetAttribute( "radius", radius );
            break;
        case COLLIDERSHAPE_CAPSULE:
        case COLLIDERSHAPE_CYLINDER:
            node.SetAttribute( "radius", radius );
            node.SetAttribute( "height", height );
            break;
        default:
            node.SetAttribute( "mesh", meshIndex );
            break;
    }
    values.clear();
    values.push_back( CORE::CVariant( transform.position.x ) );
    values.push_back( CORE::CVariant( transform.position.y ) );
    values.push_back( CORE::CVariant( transform.position.z ) );
    node.SetValuesOfChildByName( "translation", values );
    values.clear();
    values.push_back( CORE::CVariant( transform.rotation.x ) );
    values.push_back( CORE::CVariant( transform.rotation.y ) );
    values.push_back( CORE::CVariant( transform.rotation.z ) );
    values.push_back( CORE::CVariant( transform.rotation.w ) );
    node.SetValuesOfChildByName( "rotation", values );
    node.SetAttribute( "trigger", trigger );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
