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
}

/*-------------------------------------------------------------------------*/
