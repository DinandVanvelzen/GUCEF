/*
 *  AssetConverterLib: a punctual light of the intermediate model
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

#include "AssetConverterLib_CAssetLight.h"

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

CAssetLight::CAssetLight( void )
    : name()
    , type( LIGHTTYPE_POINT )
    , intensity( 1.0f )
    , range( 0.0f )
    , innerConeAngle( 0.0f )
    , outerConeAngle( 0.785398163f )
{GUCEF_TRACE;

    colour[ 0 ] = colour[ 1 ] = colour[ 2 ] = 1.0f;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLight::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "name", name );
    node.SetAttribute( "type", ToString( type ) );
    CORE::CDataNode::TVariantVector values;
    for ( UInt32 i=0; i<3; ++i )
        values.push_back( CORE::CVariant( colour[ i ] ) );
    node.SetValuesOfChildByName( "colour", values );
    node.SetAttribute( "intensity", intensity );
    node.SetAttribute( "range", range );
    if ( LIGHTTYPE_SPOT == type )
    {
        node.SetAttribute( "innerConeAngle", innerConeAngle );
        node.SetAttribute( "outerConeAngle", outerConeAngle );
    }
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
