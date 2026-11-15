/*
 *  gucefIMAGE_TestApp: Test application for the gucefIMAGE module
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

#include <string.h>
#include <math.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#include "TestMipChainGenerator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

const Float64 FOLIAGE_THRESHOLD = 0.6;

/** A small deterministic random generator so the pictures are the same on every run */
class CRandom
{
    public:

    CRandom( UInt32 seed ) : m_state( seed ) {}

    UInt32 Next( void )
    {
        m_state = m_state * 1664525u + 1013904223u;
        return m_state >> 8;
    }

    private:

    UInt32 m_state;
};

/** A normal map of gently tilted normals */
IMAGE::TPixelMapPtr
MakeNormalMap( UInt32 size, IMAGE::TPixelStorageFormat format, GUCEF::TBuildinDataType dataType, bool signedValues )
{
    IMAGE::TPixelMapPtr map( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, size, size, format, dataType ) );
    const UInt32 channelCount = IMAGE::CPixelMap::GetChannelCountForFormat( format );
    const UInt32 valueSize = IMAGE::CPixelMap::GetPixelChannelSize( dataType );
    const bool bgr = IMAGE::PSF_BGR == format || IMAGE::PSF_BGRA == format;
    UInt8* data = static_cast< UInt8* >( map->GetDataPtr() );
    for ( UInt32 y=0; y<size; ++y )
    {
        for ( UInt32 x=0; x<size; ++x )
        {
            Float64 nx = 0.7 * sin( x * 0.9 ) * cos( y * 0.5 ), ny = 0.7 * cos( x * 0.3 + y * 0.8 ), nz = 1.0;
            Float64 length = sqrt( nx * nx + ny * ny + nz * nz );
            nx /= length; ny /= length; nz /= length;
            Float32 components[ 3 ] = { (Float32) nx, (Float32) ny, (Float32) nz };
            for ( UInt32 c=0; c<3; ++c )
            {
                Float32 stored = signedValues ? components[ c ] : components[ c ] * 0.5f + 0.5f;
                UInt32 channel = bgr ? 2 - c : c;
                IMAGE::CPixelMapChannelAccess::WriteValue( stored, dataType, data + ( (size_t) ( y * size + x ) * channelCount + channel ) * valueSize );
            }
            if ( channelCount > 3 )
                IMAGE::CPixelMapChannelAccess::WriteValue( 1.0f, dataType, data + ( (size_t) ( y * size + x ) * channelCount + 3 ) * valueSize );
        }
    }
    return map;
}

/** The largest deviation from unit length of the normals of a normal map */
Float32
LargestNormalError( const IMAGE::CPixelMap& map, bool signedValues )
{
    const UInt32 channelCount = map.GetNumberOfChannelsPerPixel();
    const UInt32 valueSize = IMAGE::CPixelMap::GetPixelChannelSize( map.GetPixelComponentDataType() );
    const bool bgr = IMAGE::PSF_BGR == map.GetPixelStorageFormat() || IMAGE::PSF_BGRA == map.GetPixelStorageFormat();
    const UInt8* data = static_cast< const UInt8* >( map.GetDataPtr() );
    Float32 largest = 0.0f;
    for ( UInt32 i=0; i<map.GetPixelCount(); ++i )
    {
        Float32 v[ 3 ];
        for ( UInt32 c=0; c<3; ++c )
        {
            UInt32 channel = bgr ? 2 - c : c;
            v[ c ] = IMAGE::CPixelMapChannelAccess::ReadValue( data + ( (size_t) i * channelCount + channel ) * valueSize, map.GetPixelComponentDataType() );
            if ( !signedValues )
                v[ c ] = v[ c ] * 2.0f - 1.0f;
        }
        Float32 length = (Float32) sqrt( v[ 0 ] * v[ 0 ] + v[ 1 ] * v[ 1 ] + v[ 2 ] * v[ 2 ] );
        Float32 error = (Float32) fabs( length - 1.0f );
        if ( error > largest )
            largest = error;
    }
    return largest;
}

Int32
Difference( UInt8 a, UInt8 b )
{
    return a > b ? (Int32) ( a - b ) : (Int32) ( b - a );
}

/** The fraction of texels whose 8 bit alpha is above the test reference of 0.5 */
Float64
AlphaCoverage( const IMAGE::CPixelMap& map )
{
    const UInt8* data = static_cast< const UInt8* >( map.GetDataPtr() );
    UInt32 count = 0;
    for ( UInt32 i=0; i<map.GetPixelCount(); ++i )
        if ( data[ i * 4 + 3 ] >= 128 )
            ++count;
    return (Float64) count / (Float64) map.GetPixelCount();
}

/** Foliage: leaf like alpha with soft edges and fine detail, about 30 percent of the texels above the alpha test reference of 0.5 */
IMAGE::TPixelMapPtr
MakeFoliage( UInt32 size )
{
    IMAGE::TPixelMapPtr map( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, size, size, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8* data = static_cast< UInt8* >( map->GetDataPtr() );
    CRandom random( 4242 );
    for ( UInt32 y=0; y<size; ++y )
    {
        for ( UInt32 x=0; x<size; ++x )
        {
            Float64 smooth = 0.5 + 0.25 * sin( x * 0.35 + 1.0 ) * cos( y * 0.27 ) + 0.2 * sin( x * 0.11 + y * 0.17 + 2.0 ) + 0.12 * sin( x * 0.8 ) * sin( y * 0.7 );
            Float64 detail = (Float64) ( random.Next() & 0xFFFF ) / 65535.0;
            Float64 alpha = ( 0.6 * smooth + 0.4 * detail - FOLIAGE_THRESHOLD ) * 6.0 + 0.5;
            alpha = alpha < 0.0 ? 0.0 : ( alpha > 1.0 ? 1.0 : alpha );
            UInt8* pixel = data + ( (size_t) y * size + x ) * 4;
            pixel[ 0 ] = 40; pixel[ 1 ] = 160; pixel[ 2 ] = 40;
            pixel[ 3 ] = (UInt8) ( alpha * 255.0 + 0.5 );
        }
    }
    return map;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMipChainGeneratorTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefIMAGE.MipChainGenerator" );

    GUCEF_TESTFW_TESTCASE( "MipChain: Level count and sizes, square, non-square and odd" )
    try
    {
        ASSERT_TRUE( 9 == IMAGE::CMipChainGenerator::GetLevelCount( 256, 256, 1 ) );
        ASSERT_TRUE( 7 == IMAGE::CMipChainGenerator::GetLevelCount( 256, 256, 4 ) );
        ASSERT_TRUE( 11 == IMAGE::CMipChainGenerator::GetLevelCount( 1024, 16, 1 ) );
        ASSERT_TRUE( 7 == IMAGE::CMipChainGenerator::GetLevelCount( 1024, 16, 16 ) );
        ASSERT_TRUE( 3 == IMAGE::CMipChainGenerator::GetLevelCount( 5, 3, 1 ) );
        ASSERT_TRUE( 3 == IMAGE::CMipChainGenerator::GetLevelCount( 7, 7, 1 ) );
        ASSERT_TRUE( 1 == IMAGE::CMipChainGenerator::GetLevelCount( 1, 1, 1 ) );
        ASSERT_TRUE( 1 == IMAGE::CMipChainGenerator::GetLevelCount( 8, 8, 64 ) );
        ASSERT_TRUE( 0 == IMAGE::CMipChainGenerator::GetLevelCount( 0, 8, 1 ) );

        // Sizes halve per level, rounded down, never below 1
        IMAGE::TPixelMapPtr source( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 37, 20, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
        IMAGE::CMipChainSettings settings;
        IMAGE::CMipChainGenerator::TPixelMapPtrVector levels;
        ASSERT_TRUE( IMAGE::CMipChainGenerator::Generate( *source, settings, levels ) );
        const UInt32 expectedSizes[ 6 ][ 2 ] = { { 37, 20 }, { 18, 10 }, { 9, 5 }, { 4, 2 }, { 2, 1 }, { 1, 1 } };
        ASSERT_TRUE( 6 == levels.size() );
        for ( UInt32 i=0; i<levels.size() && i<6; ++i )
            ASSERT_TRUE( expectedSizes[ i ][ 0 ] == levels[ i ]->GetWidthInPixels() && expectedSizes[ i ][ 1 ] == levels[ i ]->GetHeightInPixels() &&
                         IMAGE::PSF_RGBA == levels[ i ]->GetPixelStorageFormat() && GUCEF::DATATYPE_UINT8 == levels[ i ]->GetPixelComponentDataType() );

        // A minimum size stops the chain
        settings.minimumSize = 4;
        ASSERT_TRUE( IMAGE::CMipChainGenerator::Generate( *source, settings, levels ) );
        ASSERT_TRUE( 4 == levels.size() && 4 == levels[ 3 ]->GetWidthInPixels() );

        // Level 0 is a copy that is equal to the source, not the source itself
        IMAGE::TPixelMapPtr patterned( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 8, 8, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 ) );
        UInt8* data = static_cast< UInt8* >( patterned->GetDataPtr() );
        for ( UInt32 i=0; i<8 * 8 * 3; ++i )
            data[ i ] = (UInt8) ( i * 5 );
        settings.minimumSize = 1;
        ASSERT_TRUE( IMAGE::CMipChainGenerator::Generate( *patterned, settings, levels ) );
        ASSERT_TRUE( 4 == levels.size() && &(*levels[ 0 ]) != &(*patterned) && 0 == memcmp( levels[ 0 ]->GetDataPtr(), patterned->GetDataPtr(), 8 * 8 * 3 ) );

        // Palette pictures are refused
        IMAGE::CPixelMap palettised( GUCEF_NULL, 8, 8, IMAGE::PSF_PALETTE_INDICES, GUCEF::DATATYPE_UINT8 );
        ASSERT_FALSE( IMAGE::CMipChainGenerator::Generate( palettised, settings, levels ) );
        ASSERT_TRUE( levels.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MipChain: Colour is filtered in linear light, each level from level 0" )
    try
    {
        // A black and white checkerboard: every level is the mean in linear light, which is about 188 as sRGB
        IMAGE::TPixelMapPtr checker( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 16, 16, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
        UInt8* data = static_cast< UInt8* >( checker->GetDataPtr() );
        for ( UInt32 y=0; y<16; ++y )
            for ( UInt32 x=0; x<16; ++x )
            {
                UInt8 value = ( x + y ) % 2 ? 255 : 0;
                UInt8* pixel = data + ( y * 16 + x ) * 4;
                pixel[ 0 ] = value; pixel[ 1 ] = value; pixel[ 2 ] = value; pixel[ 3 ] = 255;
            }

        IMAGE::CMipChainSettings settings;
        settings.filter = IMAGE::RESAMPLEFILTER_BOX;
        IMAGE::CMipChainGenerator::TPixelMapPtrVector srgbLevels, linearLevels;
        ASSERT_TRUE( IMAGE::CMipChainGenerator::Generate( *checker, settings, srgbLevels ) );
        settings.colourSpace = IMAGE::COLOURSPACE_LINEAR;
        ASSERT_TRUE( IMAGE::CMipChainGenerator::Generate( *checker, settings, linearLevels ) );
        ASSERT_TRUE( 5 == srgbLevels.size() && 5 == linearLevels.size() );
        for ( UInt32 i=1; i<srgbLevels.size() && i<linearLevels.size(); ++i )
        {
            const UInt8* s = static_cast< const UInt8* >( srgbLevels[ i ]->GetDataPtr() );
            const UInt8* l = static_cast< const UInt8* >( linearLevels[ i ]->GetDataPtr() );
            ASSERT_TRUE( s[ 0 ] >= 187 && s[ 0 ] <= 188 && 255 == s[ 3 ] );
            ASSERT_TRUE( l[ 0 ] >= 127 && l[ 0 ] <= 128 );
        }

        // Every level comes from level 0: the last level of a constant stays exactly constant
        ASSERT_TRUE( 1 == srgbLevels.back()->GetWidthInPixels() );

        // Data content is never treated as colour even when the colour space says sRGB
        settings.colourSpace = IMAGE::COLOURSPACE_SRGB;
        settings.content = IMAGE::IMAGECONTENT_DATA;
        IMAGE::CMipChainGenerator::TPixelMapPtrVector dataLevels;
        ASSERT_TRUE( IMAGE::CMipChainGenerator::Generate( *checker, settings, dataLevels ) );
        ASSERT_TRUE( dataLevels.size() > 1 && static_cast< const UInt8* >( dataLevels[ 1 ]->GetDataPtr() )[ 0 ] >= 127 && static_cast< const UInt8* >( dataLevels[ 1 ]->GetDataPtr() )[ 0 ] <= 128 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MipChain: Normal map levels are unit length, for the formats and data types of normal maps" )
    try
    {
        IMAGE::CMipChainSettings settings;
        settings.content = IMAGE::IMAGECONTENT_NORMALMAP;
        settings.filter = IMAGE::RESAMPLEFILTER_TRIANGLE;
        settings.wrapMode = IMAGE::WRAPMODE_CLAMP;

        const IMAGE::TPixelStorageFormat formats[] = { IMAGE::PSF_RGB, IMAGE::PSF_RGBA, IMAGE::PSF_BGR, IMAGE::PSF_BGRA };
        for ( UInt32 f=0; f<4; ++f )
        {
            IMAGE::TPixelMapPtr normals = MakeNormalMap( 32, formats[ f ], GUCEF::DATATYPE_UINT8, false );
            IMAGE::CMipChainGenerator::TPixelMapPtrVector levels;
            settings.normalMapSigned = false;
            ASSERT_TRUE( IMAGE::CMipChainGenerator::Generate( *normals, settings, levels ) );
            ASSERT_TRUE( 6 == levels.size() );
            for ( UInt32 i=1; i<levels.size(); ++i )
                ASSERT_TRUE( LargestNormalError( *levels[ i ], false ) < 0.02f );
        }

        // Without the renormalisation the averaged normals are shorter, so the effect is real
        IMAGE::TPixelMapPtr normals = MakeNormalMap( 32, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8, false );
        IMAGE::CMipChainGenerator::TPixelMapPtrVector plainLevels, normalLevels;
        IMAGE::CMipChainSettings plain = settings;
        plain.content = IMAGE::IMAGECONTENT_DATA;
        ASSERT_TRUE( IMAGE::CMipChainGenerator::Generate( *normals, plain, plainLevels ) );
        ASSERT_TRUE( IMAGE::CMipChainGenerator::Generate( *normals, settings, normalLevels ) );
        ASSERT_TRUE( plainLevels.size() > 3 && LargestNormalError( *plainLevels[ 3 ], false ) > 0.03f );

        // Signed components, 8 bit integers and floating point
        settings.normalMapSigned = true;
        IMAGE::TPixelMapPtr signedNormals = MakeNormalMap( 32, IMAGE::PSF_RGB, GUCEF::DATATYPE_INT8, true ), floatNormals = MakeNormalMap( 32, IMAGE::PSF_RGBA, GUCEF::DATATYPE_LE_FLOAT32, true );
        IMAGE::CMipChainGenerator::TPixelMapPtrVector signedLevels, floatLevels;
        ASSERT_TRUE( IMAGE::CMipChainGenerator::Generate( *signedNormals, settings, signedLevels ) );
        ASSERT_TRUE( IMAGE::CMipChainGenerator::Generate( *floatNormals, settings, floatLevels ) );
        for ( UInt32 i=1; i<signedLevels.size() && i<floatLevels.size(); ++i )
        {
            ASSERT_TRUE( LargestNormalError( *signedLevels[ i ], true ) < 0.02f );
            ASSERT_TRUE( LargestNormalError( *floatLevels[ i ], true ) < 0.0001f );
        }

        // A picture without red, green and blue is no normal map
        IMAGE::CPixelMap single( GUCEF_NULL, 8, 8, IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_UINT8 );
        IMAGE::CMipChainGenerator::TPixelMapPtrVector none;
        ASSERT_FALSE( IMAGE::CMipChainGenerator::Generate( single, settings, none ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MipChain: Alpha coverage stays within 1 percent of level 0 at every level, against a measurable loss without" )
    try
    {
        IMAGE::TPixelMapPtr foliage = MakeFoliage( 128 );
        const Float64 level0Coverage = AlphaCoverage( *foliage );
        ASSERT_TRUE( level0Coverage > 0.2 && level0Coverage < 0.4 );

        IMAGE::CMipChainSettings settings;
        settings.filter = IMAGE::RESAMPLEFILTER_BOX;
        settings.wrapMode = IMAGE::WRAPMODE_CLAMP;
        IMAGE::CMipChainGenerator::TPixelMapPtrVector withoutLevels, withLevels;
        ASSERT_TRUE( IMAGE::CMipChainGenerator::Generate( *foliage, settings, withoutLevels ) );
        settings.alphaCoverageCutoff = 0.5f;
        ASSERT_TRUE( IMAGE::CMipChainGenerator::Generate( *foliage, settings, withLevels ) );
        ASSERT_TRUE( 8 == withoutLevels.size() && 8 == withLevels.size() );

        bool lossWithout = false;
        for ( UInt32 i=1; i<withLevels.size() && i<withoutLevels.size(); ++i )
        {
            const UInt32 texels = withLevels[ i ]->GetPixelCount();
            // the number of texels limits how close a level can get: 1 percent, or one texel
            Float64 allowed = 0.01 > 1.0 / texels ? 0.01 : 1.0 / texels;
            ASSERT_TRUE( fabs( AlphaCoverage( *withLevels[ i ] ) - level0Coverage ) <= allowed );
            if ( texels >= 100 && fabs( AlphaCoverage( *withoutLevels[ i ] ) - level0Coverage ) > 0.05 )
                lossWithout = true;
        }
        ASSERT_TRUE( lossWithout );

        // The colour is not touched by the alpha scaling
        ASSERT_TRUE( 40 == static_cast< const UInt8* >( withLevels[ 1 ]->GetDataPtr() )[ 0 ] );

        // A format without alpha takes the setting without effect
        IMAGE::TPixelMapPtr rgb( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 8, 8, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 ) );
        ASSERT_TRUE( IMAGE::CMipChainGenerator::Generate( *rgb, settings, withLevels ) && 4 == withLevels.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MipChain: The CImage overload fills every frame" )
    try
    {
        IMAGE::CImage::TFrameList frames;
        for ( UInt32 f=0; f<3; ++f )
        {
            IMAGE::CImage::TMipMapList frame;
            IMAGE::TPixelMapPtr picture( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 16, 8, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
            memset( picture->GetDataPtr(), 10 + (int) f * 50, picture->GetTotalSizeInBytes() );
            frame.push_back( picture );
            frames.push_back( frame );
        }
        IMAGE::CImage image( frames );
        image.SetFrameDurationInMs( 0, 40 );
        image.SetFrameDurationInMs( 1, 50 );
        image.SetFrameDurationInMs( 2, 60 );

        IMAGE::CMipChainSettings settings;
        ASSERT_TRUE( IMAGE::CMipChainGenerator::Generate( image, settings ) );
        ASSERT_TRUE( 3 == image.GetFrameCount() );
        for ( UInt32 f=0; f<3; ++f )
        {
            ASSERT_TRUE( 5 == image.GetMipmapLevels( f ) );
            IMAGE::TPixelMapPtr last = image.GetPixelMap( f, 4 );
            ASSERT_TRUE( 1 == last->GetWidthInPixels() && 1 == last->GetHeightInPixels() );
            // each frame has its own picture: a constant stays constant
            UInt8 expected = (UInt8) ( 10 + f * 50 );
            ASSERT_TRUE( Difference( expected, *static_cast< const UInt8* >( image.GetPixelMap( f, 2 )->GetDataPtr() ) ) <= 1 );
        }
        ASSERT_TRUE( 40 == image.GetFrameDurationInMs( 0 ) && 60 == image.GetFrameDurationInMs( 2 ) );

        // Doing it again replaces the levels instead of adding to them
        ASSERT_TRUE( IMAGE::CMipChainGenerator::Generate( image, settings ) );
        ASSERT_TRUE( 5 == image.GetMipmapLevels( 1 ) );

        // An image without frames and an image with a picture that cannot be used are refused and left alone
        IMAGE::CImage empty;
        ASSERT_FALSE( IMAGE::CMipChainGenerator::Generate( empty, settings ) );
        IMAGE::CImage::TFrameList mixed;
        IMAGE::CImage::TMipMapList good, bad;
        good.push_back( IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 4, 4, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 ) ) );
        bad.push_back( IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 4, 4, IMAGE::PSF_PALETTE_INDICES, GUCEF::DATATYPE_UINT8 ) ) );
        mixed.push_back( good );
        mixed.push_back( bad );
        IMAGE::CImage mixedImage( mixed );
        ASSERT_FALSE( IMAGE::CMipChainGenerator::Generate( mixedImage, settings ) );
        ASSERT_TRUE( 1 == mixedImage.GetMipmapLevels( 0 ) && 1 == mixedImage.GetMipmapLevels( 1 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MipChain: The settings round trip through a CDataNode and the type names through strings" )
    try
    {
        IMAGE::CMipChainSettings defaults;
        ASSERT_TRUE( IMAGE::RESAMPLEFILTER_KAISER == defaults.filter && IMAGE::COLOURSPACE_SRGB == defaults.colourSpace && IMAGE::IMAGECONTENT_COLOUR == defaults.content &&
                     IMAGE::WRAPMODE_REPEAT == defaults.wrapMode && defaults.alphaCoverageCutoff < 0.0f && 1 == defaults.minimumSize && !defaults.normalMapSigned );

        IMAGE::CMipChainSettings settings;
        settings.filter = IMAGE::RESAMPLEFILTER_LANCZOS3;
        settings.colourSpace = IMAGE::COLOURSPACE_LINEAR;
        settings.content = IMAGE::IMAGECONTENT_NORMALMAP;
        settings.wrapMode = IMAGE::WRAPMODE_MIRROR;
        settings.alphaCoverageCutoff = 0.375f;
        settings.minimumSize = 8;
        settings.normalMapSigned = true;

        CORE::CDataNode tree;
        ASSERT_TRUE( settings.SaveConfig( tree ) );
        ASSERT_TRUE( GUCEF_NULL != tree.FindChild( IMAGE::CMipChainSettings::ConfigSectionName ) );

        IMAGE::CMipChainSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( tree ) );
        ASSERT_TRUE( settings.filter == loaded.filter && settings.colourSpace == loaded.colourSpace && settings.content == loaded.content && settings.wrapMode == loaded.wrapMode &&
                     settings.alphaCoverageCutoff == loaded.alphaCoverageCutoff && settings.minimumSize == loaded.minimumSize && settings.normalMapSigned == loaded.normalMapSigned );

        // the section itself loads as well, and what it lacks stays as it is
        CORE::CDataNode partial( IMAGE::CMipChainSettings::ConfigSectionName );
        partial.SetAttribute( "filter", "box" );
        IMAGE::CMipChainSettings partialSettings;
        partialSettings.minimumSize = 4;
        ASSERT_TRUE( partialSettings.LoadConfig( partial ) );
        ASSERT_TRUE( IMAGE::RESAMPLEFILTER_BOX == partialSettings.filter && 4 == partialSettings.minimumSize && IMAGE::COLOURSPACE_SRGB == partialSettings.colourSpace );

        // a node without the section is no settings
        CORE::CDataNode other( "Other" );
        ASSERT_FALSE( loaded.LoadConfig( other ) );

        // the type names
        const IMAGE::TResampleFilter filters[] = { IMAGE::RESAMPLEFILTER_BOX, IMAGE::RESAMPLEFILTER_TRIANGLE, IMAGE::RESAMPLEFILTER_KAISER, IMAGE::RESAMPLEFILTER_LANCZOS3 };
        for ( UInt32 i=0; i<4; ++i )
        {
            IMAGE::TResampleFilter parsed = IMAGE::RESAMPLEFILTER_BOX;
            ASSERT_TRUE( IMAGE::FromString( IMAGE::ToString( filters[ i ] ), parsed ) && filters[ i ] == parsed );
        }
        IMAGE::TColourSpace space = IMAGE::COLOURSPACE_LINEAR;
        ASSERT_TRUE( IMAGE::FromString( "sRGB", space ) && IMAGE::COLOURSPACE_SRGB == space && "linear" == IMAGE::ToString( IMAGE::COLOURSPACE_LINEAR ) );
        IMAGE::TWrapMode wrap = IMAGE::WRAPMODE_CLAMP;
        ASSERT_TRUE( IMAGE::FromString( "mirror", wrap ) && IMAGE::WRAPMODE_MIRROR == wrap && "repeat" == IMAGE::ToString( IMAGE::WRAPMODE_REPEAT ) );
        IMAGE::TImageContent content = IMAGE::IMAGECONTENT_COLOUR;
        ASSERT_TRUE( IMAGE::FromString( "normalmap", content ) && IMAGE::IMAGECONTENT_NORMALMAP == content && "data" == IMAGE::ToString( IMAGE::IMAGECONTENT_DATA ) );
        IMAGE::TResampleFilter unchanged = IMAGE::RESAMPLEFILTER_TRIANGLE;
        ASSERT_FALSE( IMAGE::FromString( "bicubic", unchanged ) );
        ASSERT_TRUE( IMAGE::RESAMPLEFILTER_TRIANGLE == unchanged );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MipChain: Benchmark of a 2048x2048 RGBA8 mip chain with the Kaiser filter" )
    try
    {
        IMAGE::TPixelMapPtr big( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 2048, 2048, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
        UInt8* data = static_cast< UInt8* >( big->GetDataPtr() );
        CRandom random( 7 );
        for ( UInt32 i=0; i<2048 * 2048; ++i )
        {
            UInt32 r = random.Next();
            data[ i * 4 ] = (UInt8) ( r ); data[ i * 4 + 1 ] = (UInt8) ( r >> 8 ); data[ i * 4 + 2 ] = (UInt8) ( r >> 16 ); data[ i * 4 + 3 ] = 255;
        }

        IMAGE::CMipChainSettings settings;
        settings.filter = IMAGE::RESAMPLEFILTER_KAISER;
        IMAGE::CMipChainGenerator::TPixelMapPtrVector levels;
        UInt64 start = MT::PrecisionTickCount();
        bool generated = IMAGE::CMipChainGenerator::Generate( *big, settings, levels );
        UInt64 ticks = MT::PrecisionTickCount() - start;
        ASSERT_TRUE( generated && 12 == levels.size() );

        Float64 milliseconds = (Float64) ticks * 1000.0 / (Float64) MT::PrecisionTimerResolution();
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "MipChain benchmark: a 2048x2048 RGBA8 mip chain (" << levels.size() << " levels, Kaiser, sRGB) took " << milliseconds << " ms" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
