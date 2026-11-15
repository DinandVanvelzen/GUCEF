/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

#include <stdlib.h>

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_TestAssetFormSupport.h"

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

const UInt8*
PixelAt( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 y )
{
    return static_cast< const UInt8* >( pixels->GetDataPtr() ) + ( (size_t) y * pixels->GetWidthInPixels() + x ) * 4;
}

/** Same colour, give or take a little for the rounding of blending on a GPU */
bool
Near( const UInt8* a, const UInt8* b, Int32 tolerance = 2 )
{
    return abs( (Int32) a[ 0 ] - (Int32) b[ 0 ] ) <= tolerance && abs( (Int32) a[ 1 ] - (Int32) b[ 1 ] ) <= tolerance &&
           abs( (Int32) a[ 2 ] - (Int32) b[ 2 ] ) <= tolerance;
}

/** Builds the scene in a context and gives its frame; false when something is missing */
bool
DrawScene( GUI::TGuiContextPtr& contextPtr, GUITEST::TFormSceneBuilder builder, GUITEST::CFormScene*& scene, IMAGE::TPixelMapPtr& pixels )
{
    GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
    if ( GUCEF_NULL == context )
        return false;
    GUITEST::PrepareFormContext( context );
    scene = builder( context );
    if ( GUCEF_NULL == scene )
        return false;
    for ( UInt32 i=0; i<3; ++i )
        context->RenderFrame( 16 );
    return context->CaptureFrame( pixels ) && !pixels.IsNULL() && 640 == pixels->GetWidthInPixels() && 480 == pixels->GetHeightInPixels();
}

void
CompareWithReference( const char* driverName, const char* formName, GUI::TGuiContextPtr ( *createContext )( void ), GUITEST::TFormSceneBuilder builder )
{
    CORE::CString title = CORE::CString( "Render (" ) + driverName + "): The " + formName + " has the same pixels as the software reference";
    GUCEF_TESTFW_TESTCASE( title.C_String() )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    if ( !IsInBrowser() || CORE::CString( "ImGui" ) == driverName )
        GUCEF_TESTFW_SKIP( "the driver has no window here" );
    #endif
    try
    {
        GUI::TGuiContextPtr driverPtr = createContext();
        if ( driverPtr.IsNULL() )
            GUCEF_TESTFW_SKIP( "the driver or its window is not there" );
        GUI::TGuiContextPtr referencePtr = CreateNullContext();
        ASSERT_TRUE( !referencePtr.IsNULL() );

        GUITEST::CFormScene* referenceScene = GUCEF_NULL;
        GUITEST::CFormScene* driverScene = GUCEF_NULL;
        IMAGE::TPixelMapPtr expected, actual;
        const bool haveReference = DrawScene( referencePtr, builder, referenceScene, expected );
        const bool haveDriver = DrawScene( driverPtr, builder, driverScene, actual );
        ASSERT_TRUE( haveReference );
        ASSERT_TRUE( haveDriver );

        // GUCEF_TEST_FORM_DUMP names a folder that the pictures of the reference are written to, to look at
        const char* dumpEnv = getenv( "GUCEF_TEST_FORM_DUMP" );
        const CORE::CString dumpDir = GUCEF_NULL != dumpEnv ? dumpEnv : "";
        if ( haveReference && !dumpDir.IsNULLOrEmpty() )
        {
            CORE::CString name = CORE::CString( formName ).ReplaceChar( ' ', '_' );
            IMAGE::CImage dump( expected );
            dump.Save( CORE::CombinePath( dumpDir, name + ".png" ), "png" );
        }

        // there is something to compare: the picture is not one colour
        UInt32 differentFromCorner = 0;
        for ( Int32 y=0; y<480 && haveReference; y+=3 )
            for ( Int32 x=0; x<640; x+=3 )
                if ( !Near( PixelAt( expected, x, y ), PixelAt( expected, 0, 0 ), 1 ) )
                    ++differentFromCorner;
        ASSERT_TRUE( differentFromCorner > 200 );

        UInt32 different = 0;
        Int32 firstX = -1, firstY = -1;
        for ( Int32 y=0; y<480 && haveReference && haveDriver; ++y )
        {
            for ( Int32 x=0; x<640; ++x )
            {
                if ( !Near( PixelAt( expected, x, y ), PixelAt( actual, x, y ) ) )
                {
                    if ( 0 == different )
                    {
                        firstX = x;
                        firstY = y;
                    }
                    ++different;
                }
            }
        }
        if ( 0 != different )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, driverName << " differs from the reference in " << different << " pixels for the " << formName << ", the first at " << firstX << "," << firstY );
        ASSERT_TRUE( 0 == different );

        // the scenes go before their contexts
        delete driverScene;
        delete referenceScene;
    }
    catch ( const TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
RunFormRenderTests( const char* formName, GUITEST::TFormSceneBuilder builder )
{GUCEF_TRACE;

    CompareWithReference( "RmlUi", formName, &CreateRmlUiContext, builder );
    CompareWithReference( "ImGui", formName, &CreateImGuiContext, builder );
}

/*-------------------------------------------------------------------------*/
