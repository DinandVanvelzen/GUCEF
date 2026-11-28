/*
 *  gucefIMAGE_TestApp: Test application for the gucefIMAGE module
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

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

#include "TestPixelMapConversion.h"

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPixelMapConversionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefIMAGE.PixelMapConversion" );

    GUCEF_TESTFW_TESTCASE( "PixelMap: A picture without alpha becomes fully opaque when alpha is added" )
    try
    {
        const IMAGE::TPixelStorageFormat sources[] = { IMAGE::PSF_RGB, IMAGE::PSF_BGR };
        const IMAGE::TPixelStorageFormat targets[] = { IMAGE::PSF_RGBA, IMAGE::PSF_BGRA };
        for ( UInt32 s=0; s<2; ++s )
        {
            for ( UInt32 t=0; t<2; ++t )
            {
                IMAGE::CPixelMap pixels( GUCEF_NULL, 2, 1, sources[ s ], GUCEF::DATATYPE_UINT8 );
                UInt8* data = static_cast< UInt8* >( pixels.GetDataPtr() );
                data[ 0 ] = 10; data[ 1 ] = 20; data[ 2 ] = 30;
                data[ 3 ] = 40; data[ 4 ] = 50; data[ 5 ] = 60;
                ASSERT_TRUE( pixels.ConvertFormatTo( targets[ t ], GUCEF::DATATYPE_UINT8 ) );
                ASSERT_TRUE( targets[ t ] == pixels.GetPixelStorageFormat() && 8 == pixels.GetTotalSizeInBytes() );
                const UInt8* out = static_cast< const UInt8* >( pixels.GetDataPtr() );
                ASSERT_TRUE( 255 == out[ 3 ] && 255 == out[ 7 ] );
                // the colour channels stay: in the same order when the order matches, swapped when not
                bool sameOrder = ( IMAGE::PSF_RGB == sources[ s ] ) == ( IMAGE::PSF_RGBA == targets[ t ] );
                ASSERT_TRUE( 20 == out[ 1 ] && ( sameOrder ? 10 == out[ 0 ] && 30 == out[ 2 ] : 30 == out[ 0 ] && 10 == out[ 2 ] ) );
            }
        }

        // with floating point components opaque is 1
        IMAGE::CPixelMap floats( GUCEF_NULL, 1, 1, IMAGE::PSF_RGB, GUCEF::DATATYPE_LE_FLOAT32 );
        memset( floats.GetDataPtr(), 0, floats.GetTotalSizeInBytes() );
        ASSERT_TRUE( floats.ConvertFormatTo( IMAGE::PSF_RGBA, GUCEF::DATATYPE_LE_FLOAT32 ) );
        ASSERT_TRUE( 1.0f == static_cast< const Float32* >( floats.GetDataPtr() )[ 3 ] );

        // grey with alpha added
        IMAGE::CPixelMap grey( GUCEF_NULL, 1, 1, IMAGE::PSF_SINGLE_CHANNEL_STD_LUMINANCE, GUCEF::DATATYPE_UINT8 );
        static_cast< UInt8* >( grey.GetDataPtr() )[ 0 ] = 77;
        if ( grey.ConvertFormatTo( IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) )
        {
            const UInt8* out = static_cast< const UInt8* >( grey.GetDataPtr() );
            ASSERT_TRUE( 77 == out[ 0 ] && 77 == out[ 2 ] && 255 == out[ 3 ] );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PixelMap: converting only the component type keeps the picture, scaled by the range of the types" )
        try
        {
            // 16 bit to 8 bit: full stays full, a multiple of 257 maps to the 8 bit value it was made from
            IMAGE::CPixelMap wide( GUCEF_NULL, 4, 1, IMAGE::PSF_RGB, GUCEF::DATATYPE_LE_UINT16 );
            UInt16* wideData = static_cast< UInt16* >( wide.GetDataPtr() );
            const UInt16 wideValues[ 12 ] = { 0, 257, 65535, 128 * 257, 1, 2, 3, 4, 5, 6, 7, 8 };
            for ( int i=0; i<12; ++i )
                wideData[ i ] = wideValues[ i ];
            IMAGE::TPixelMapPtr narrow;
            ASSERT_TRUE( wide.ConvertFormatTo( IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8, narrow ) && !narrow.IsNULL() );
            if ( !narrow.IsNULL() )
            {
                const UInt8* out = static_cast< const UInt8* >( narrow->GetDataPtr() );
                ASSERT_TRUE( IMAGE::PSF_RGB == narrow->GetPixelStorageFormat() && GUCEF::DATATYPE_UINT8 == narrow->GetPixelComponentDataType() );
                ASSERT_TRUE( 0 == out[ 0 ] && 1 == out[ 1 ] && 255 == out[ 2 ] && 128 == out[ 3 ] );
                ASSERT_TRUE( 0 == out[ 4 ] && 0 == out[ 5 ] && 0 == out[ 6 ] && 0 == out[ 7 ] );
            }

            // 8 bit to 16 bit and back is exact
            IMAGE::CPixelMap bytes( GUCEF_NULL, 3, 1, IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_UINT8 );
            static_cast< UInt8* >( bytes.GetDataPtr() )[ 0 ] = 0;
            static_cast< UInt8* >( bytes.GetDataPtr() )[ 1 ] = 77;
            static_cast< UInt8* >( bytes.GetDataPtr() )[ 2 ] = 255;
            IMAGE::TPixelMapPtr widened;
            ASSERT_TRUE( bytes.ConvertFormatTo( IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_LE_UINT16, widened ) && !widened.IsNULL() );
            if ( !widened.IsNULL() )
            {
                const UInt16* out = static_cast< const UInt16* >( widened->GetDataPtr() );
                ASSERT_TRUE( 0 == out[ 0 ] && 77 * 257 == out[ 1 ] && 65535 == out[ 2 ] );
                IMAGE::TPixelMapPtr back;
                ASSERT_TRUE( widened->ConvertFormatTo( IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_UINT8, back ) && !back.IsNULL() );
                if ( !back.IsNULL() )
                    ASSERT_TRUE( 0 == memcmp( back->GetDataPtr(), bytes.GetDataPtr(), 3 ) );
            }

            // a signed type covers its positive half
            IMAGE::CPixelMap signedBytes( GUCEF_NULL, 3, 1, IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_INT8 );
            static_cast< Int8* >( signedBytes.GetDataPtr() )[ 0 ] = -5;
            static_cast< Int8* >( signedBytes.GetDataPtr() )[ 1 ] = 127;
            static_cast< Int8* >( signedBytes.GetDataPtr() )[ 2 ] = 0;
            IMAGE::TPixelMapPtr unsignedBytes;
            ASSERT_TRUE( signedBytes.ConvertFormatTo( IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_UINT8, unsignedBytes ) && !unsignedBytes.IsNULL() );
            if ( !unsignedBytes.IsNULL() )
            {
                const UInt8* out = static_cast< const UInt8* >( unsignedBytes->GetDataPtr() );
                ASSERT_TRUE( 0 == out[ 0 ] && 255 == out[ 1 ] && 0 == out[ 2 ] );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PixelMap: floating point channels cover 0.0 to 1.0 when they meet integers" )
        try
        {
            IMAGE::CPixelMap bytes( GUCEF_NULL, 2, 1, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 );
            UInt8* byteData = static_cast< UInt8* >( bytes.GetDataPtr() );
            const UInt8 byteValues[ 6 ] = { 0, 255, 51, 255, 0, 0 };
            memcpy( byteData, byteValues, 6 );

            IMAGE::TPixelMapPtr floats;
            ASSERT_TRUE( bytes.ConvertFormatTo( IMAGE::PSF_RGB, GUCEF::DATATYPE_LE_FLOAT32, floats ) && !floats.IsNULL() );
            if ( !floats.IsNULL() )
            {
                const Float32* out = static_cast< const Float32* >( floats->GetDataPtr() );
                ASSERT_TRUE( 0.0f == out[ 0 ] && 1.0f == out[ 1 ] && out[ 2 ] > 0.1999f && out[ 2 ] < 0.2001f && 1.0f == out[ 3 ] );
                ASSERT_TRUE( floats->IsNormalizedAsPercentage() );

                // and back: exact
                IMAGE::TPixelMapPtr back;
                ASSERT_TRUE( floats->ConvertFormatTo( IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8, back ) && !back.IsNULL() );
                if ( !back.IsNULL() )
                    ASSERT_TRUE( 0 == memcmp( back->GetDataPtr(), bytes.GetDataPtr(), 6 ) );
            }

            // values outside of the range clamp when they go to integers, and are kept between floating point types
            IMAGE::CPixelMap hdr( GUCEF_NULL, 3, 1, IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_LE_FLOAT32 );
            Float32* hdrData = static_cast< Float32* >( hdr.GetDataPtr() );
            hdrData[ 0 ] = -0.5f;
            hdrData[ 1 ] = 0.5f;
            hdrData[ 2 ] = 7.0f;
            IMAGE::TPixelMapPtr clamped;
            ASSERT_TRUE( hdr.ConvertFormatTo( IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_UINT8, clamped ) && !clamped.IsNULL() );
            if ( !clamped.IsNULL() )
            {
                const UInt8* out = static_cast< const UInt8* >( clamped->GetDataPtr() );
                ASSERT_TRUE( 0 == out[ 0 ] && 128 == out[ 1 ] && 255 == out[ 2 ] );
            }
            IMAGE::TPixelMapPtr doubles;
            ASSERT_TRUE( hdr.ConvertFormatTo( IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_LE_FLOAT64, doubles ) && !doubles.IsNULL() );
            if ( !doubles.IsNULL() )
            {
                const Float64* out = static_cast< const Float64* >( doubles->GetDataPtr() );
                ASSERT_TRUE( -0.5 == out[ 0 ] && 0.5 == out[ 1 ] && 7.0 == out[ 2 ] );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PixelMap: the picture turns over from top to bottom and mirrors from left to right, whole pixels at a time" )
        try
        {
            // 3 x 2 RGB: pixel (x, y) is { x, y, 9 }
            IMAGE::CPixelMap pixels( GUCEF_NULL, 3, 2, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 );
            UInt8* data = static_cast< UInt8* >( pixels.GetDataPtr() );
            for ( UInt32 y=0; y<2; ++y )
                for ( UInt32 x=0; x<3; ++x )
                {
                    data[ ( y * 3 + x ) * 3 + 0 ] = (UInt8) x;
                    data[ ( y * 3 + x ) * 3 + 1 ] = (UInt8) y;
                    data[ ( y * 3 + x ) * 3 + 2 ] = 9;
                }

            ASSERT_TRUE( pixels.FlipVertical() );
            data = static_cast< UInt8* >( pixels.GetDataPtr() );
            for ( UInt32 y=0; y<2; ++y )
                for ( UInt32 x=0; x<3; ++x )
                    ASSERT_TRUE( x == data[ ( y * 3 + x ) * 3 + 0 ] && 1 - y == data[ ( y * 3 + x ) * 3 + 1 ] && 9 == data[ ( y * 3 + x ) * 3 + 2 ] );

            ASSERT_TRUE( pixels.FlipVertical() );
            ASSERT_TRUE( pixels.FlipHorizontal() );
            data = static_cast< UInt8* >( pixels.GetDataPtr() );
            for ( UInt32 y=0; y<2; ++y )
                for ( UInt32 x=0; x<3; ++x )
                    ASSERT_TRUE( 2 - x == data[ ( y * 3 + x ) * 3 + 0 ] && y == data[ ( y * 3 + x ) * 3 + 1 ] && 9 == data[ ( y * 3 + x ) * 3 + 2 ] );

            // a row of one pixel and a column of one pixel have nothing to swap, an odd number of rows keeps the middle one
            IMAGE::CPixelMap column( GUCEF_NULL, 1, 3, IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_UINT8 );
            UInt8* columnData = static_cast< UInt8* >( column.GetDataPtr() );
            columnData[ 0 ] = 1; columnData[ 1 ] = 2; columnData[ 2 ] = 3;
            ASSERT_TRUE( column.FlipHorizontal() && 1 == columnData[ 0 ] && 2 == columnData[ 1 ] && 3 == columnData[ 2 ] );
            ASSERT_TRUE( column.FlipVertical() && 3 == columnData[ 0 ] && 2 == columnData[ 1 ] && 1 == columnData[ 2 ] );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
