/*
 *  AssetConverterLib: a named set of output settings of the cooking
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

#include "AssetConverterLib_CCookProfile.h"

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

CCookProfile::CCookProfile( void )
    : name( "desktop" )
    , maxTextureSize( 0 )
    , generateMips( true )
    , lodBias( 0.0f )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<TEXTUREUSAGE_COUNT; ++i )
        textureFormats[ i ] = "png";
}

/*-------------------------------------------------------------------------*/

CCookProfile::CCookProfile( const CORE::CString& profileName )
    : name( profileName )
    , maxTextureSize( 0 )
    , generateMips( true )
    , lodBias( 0.0f )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<TEXTUREUSAGE_COUNT; ++i )
        textureFormats[ i ] = "png";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CCookProfile::GetTextureFormat( TTextureUsage usage ) const
{GUCEF_TRACE;

    return textureFormats[ usage < TEXTUREUSAGE_COUNT ? usage : TEXTUREUSAGE_COLOUR ];
}

/*-------------------------------------------------------------------------*/

bool
CCookProfile::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "name", name );
    CORE::CDataNode* formats = node.AddChild( "textureFormats", GUCEF_DATATYPE_OBJECT );
    for ( UInt32 i=0; i<TEXTUREUSAGE_COUNT; ++i )
        formats->SetAttribute( ToString( (TTextureUsage) i ), textureFormats[ i ] );
    node.SetAttribute( "maxTextureSize", maxTextureSize );
    node.SetAttribute( "generateMips", generateMips );
    node.SetAttribute( "lodBias", lodBias );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCookProfile::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CCookProfile profile( node.GetAttributeValueOrChildValueByName( "name" ).AsString() );
    if ( profile.name.IsNULLOrEmpty() )
        return false;
    const CORE::CDataNode* formats = node.FindChild( "textureFormats" );
    if ( GUCEF_NULL != formats )
    {
        for ( UInt32 i=0; i<TEXTUREUSAGE_COUNT; ++i )
            profile.textureFormats[ i ] = formats->GetAttributeValueOrChildValueByName( ToString( (TTextureUsage) i ) ).AsString( profile.textureFormats[ i ] );
    }
    profile.maxTextureSize = node.GetAttributeValueOrChildValueByName( "maxTextureSize" ).AsUInt32( 0 );
    profile.generateMips = node.GetAttributeValueOrChildValueByName( "generateMips" ).AsBool( true );
    profile.lodBias = node.GetAttributeValueOrChildValueByName( "lodBias" ).AsFloat32( 0.0f );
    *this = profile;
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
