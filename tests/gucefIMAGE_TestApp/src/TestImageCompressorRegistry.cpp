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

#include "TestImageCompressorRegistry.h"

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

/** A compressor that offers the formats it is made with and "compresses" to a block of its name */
class CTestCompressor : public IMAGE::CImageCompressor
{
    public:

    CTestCompressor( const CORE::CString& name, const CORE::CString& format1, const CORE::CString& format2 )
        : m_name( name )
        , m_formats()
    {
        m_formats.push_back( format1 );
        m_formats.push_back( format2 );
    }

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_name; }

    virtual TFormatVector GetFormats( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_formats; }

    virtual bool Compress( const IMAGE::CPixelMap& level           ,
                           const CORE::CString& format             ,
                           Float32 quality                         ,
                           IMAGE::TColourSpace colourSpace         ,
                           CORE::CDynamicBuffer& blocks            ,
                           IMAGE::CIUpscaleProgress* progress      ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL != progress && !progress->OnUpscaleProgress( 1.0f ) )
            return false;
        blocks.Clear();
        blocks.CopyFrom( m_name.Length(), m_name.C_String() );
        return true;
    }

    private:

    CORE::CString m_name;
    TFormatVector m_formats;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformImageCompressorRegistryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefIMAGE.ImageCompressorRegistry" );

    GUCEF_TESTFW_TESTCASE( "CompressorRegistry: A compressor is registered and found by name and by format" )
    try
    {
        IMAGE::CImageCompressorRegistry& registry = IMAGE::CImageGlobal::Instance()->GetImageCompressorRegistry();
        ASSERT_TRUE( registry.GetNames().empty() );

        CTestCompressor first( "first", "BC7", "BC1" ), second( "second", "ASTC_4x4", "ETC2_RGB" );
        ASSERT_TRUE( registry.Register( &first ) );
        ASSERT_TRUE( registry.Register( &second ) );
        ASSERT_FALSE( registry.Register( GUCEF_NULL ) );

        IMAGE::CImageCompressorRegistry::TNameVector names = registry.GetNames();
        ASSERT_TRUE( 2 == names.size() && "first" == names[ 0 ] && "second" == names[ 1 ] );
        ASSERT_TRUE( &first == registry.Get( "first" ) && &second == registry.Get( "second" ) && GUCEF_NULL == registry.Get( "third" ) );
        ASSERT_TRUE( &first == registry.FindByFormat( "BC1" ) && &first == registry.FindByFormat( "BC7" ) );
        ASSERT_TRUE( &second == registry.FindByFormat( "ASTC_4x4" ) && &second == registry.FindByFormat( "ETC2_RGB" ) );
        ASSERT_TRUE( GUCEF_NULL == registry.FindByFormat( "BC6H" ) && GUCEF_NULL == registry.FindByFormat( "" ) );

        // a compressor found by format does its work
        IMAGE::CPixelMap level( GUCEF_NULL, 4, 4, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 );
        CORE::CDynamicBuffer blocks;
        IMAGE::CImageCompressor* found = registry.FindByFormat( "ASTC_4x4" );
        ASSERT_TRUE( GUCEF_NULL != found && found->Compress( level, "ASTC_4x4", 0.5f, IMAGE::COLOURSPACE_SRGB, blocks, GUCEF_NULL ) );
        ASSERT_TRUE( 6 == blocks.GetDataSize() && 0 == memcmp( blocks.GetConstBufferPtr(), "second", 6 ) );

        ASSERT_TRUE( registry.Unregister( "first" ) );
        ASSERT_TRUE( registry.Unregister( "second" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CompressorRegistry: Unregistering and the same name twice" )
    try
    {
        IMAGE::CImageCompressorRegistry& registry = IMAGE::CImageGlobal::Instance()->GetImageCompressorRegistry();
        CTestCompressor first( "first", "BC7", "BC1" ), duplicate( "first", "BC4", "BC5" );
        ASSERT_TRUE( registry.Register( &first ) );
        ASSERT_FALSE( registry.Register( &duplicate ) );
        ASSERT_TRUE( 1 == registry.GetNames().size() && GUCEF_NULL == registry.FindByFormat( "BC4" ) );

        ASSERT_TRUE( registry.Unregister( "first" ) );
        ASSERT_FALSE( registry.Unregister( "first" ) );
        ASSERT_TRUE( GUCEF_NULL == registry.Get( "first" ) && GUCEF_NULL == registry.FindByFormat( "BC7" ) && registry.GetNames().empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CompressorRegistry: The first registered compressor that offers a format is chosen" )
    try
    {
        IMAGE::CImageCompressorRegistry& registry = IMAGE::CImageGlobal::Instance()->GetImageCompressorRegistry();
        CTestCompressor a( "a", "BC7", "BC1" ), b( "b", "BC7", "ASTC_4x4" ), c( "c", "BC7", "ETC2_RGB" );
        ASSERT_TRUE( registry.Register( &b ) );
        ASSERT_TRUE( registry.Register( &a ) );
        ASSERT_TRUE( registry.Register( &c ) );
        ASSERT_TRUE( &b == registry.FindByFormat( "BC7" ) );
        ASSERT_TRUE( &a == registry.FindByFormat( "BC1" ) && &b == registry.FindByFormat( "ASTC_4x4" ) && &c == registry.FindByFormat( "ETC2_RGB" ) );

        // the choice follows the order of registration, also after one is removed and registered again
        ASSERT_TRUE( registry.Unregister( "b" ) );
        ASSERT_TRUE( &a == registry.FindByFormat( "BC7" ) && GUCEF_NULL == registry.FindByFormat( "ASTC_4x4" ) );
        ASSERT_TRUE( registry.Register( &b ) );
        ASSERT_TRUE( &a == registry.FindByFormat( "BC7" ) );
        IMAGE::CImageCompressorRegistry::TNameVector names = registry.GetNames();
        ASSERT_TRUE( 3 == names.size() && "a" == names[ 0 ] && "c" == names[ 1 ] && "b" == names[ 2 ] );

        ASSERT_TRUE( registry.Unregister( "a" ) && registry.Unregister( "b" ) && registry.Unregister( "c" ) );
        ASSERT_TRUE( registry.GetNames().empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
