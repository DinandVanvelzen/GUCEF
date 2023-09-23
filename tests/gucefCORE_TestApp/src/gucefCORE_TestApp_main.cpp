/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTSHAREDPTR_H
#include "TestSharedPtr.h"
#define GUCEF_CORE_TESTAPP_TESTSHAREDPTR_H
#endif /* GUCEF_CORE_TESTAPP_TESTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTSTRING_H
#include "TestString.h"
#define GUCEF_CORE_TESTAPP_TESTSTRING_H
#endif /* GUCEF_CORE_TESTAPP_TESTSTRING_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTVARS_H
#include "TestVars.h"
#define GUCEF_CORE_TESTAPP_TESTVARS_H
#endif /* GUCEF_CORE_TESTAPP_TESTVARS_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTVARIANT_H
#include "TestVariant.h"
#define GUCEF_CORE_TESTAPP_TESTVARIANT_H
#endif /* GUCEF_CORE_TESTAPP_TESTVARIANT_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTURI_H
#include "TestUri.h"
#define GUCEF_CORE_TESTAPP_TESTURI_H
#endif /* GUCEF_CORE_TESTAPP_TESTURI_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTVARIANTBINARYSERIALIZER_H
#include "TestVariantBinarySerializer.h"
#define GUCEF_CORE_TESTAPP_TESTVARIANTBINARYSERIALIZER_H
#endif /* GUCEF_CORE_TESTAPP_TESTVARIANTBINARYSERIALIZER_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTNOTIFIEROBSERVER_H
#include "TestNotifierObserver.h"
#define GUCEF_CORE_TESTAPP_TESTNOTIFIEROBSERVER_H
#endif /* GUCEF_CORE_TESTAPP_TESTNOTIFIEROBSERVER_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTCYCLICDYNAMICBUFFER_H
#include "TestCyclicDynamicBuffer.h"
#define GUCEF_CORE_TESTAPP_TESTCYCLICDYNAMICBUFFER_H
#endif /* GUCEF_CORE_TESTAPP_TESTCYCLICDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDYNAMICBUFFERSWAP_H
#include "TestDynamicBufferSwap.h"
#define GUCEF_CORE_TESTAPP_TESTDYNAMICBUFFERSWAP_H
#endif /* GUCEF_CORE_TESTAPP_TESTDYNAMICBUFFERSWAP_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDYNAMICBUFFERSTRINGSTREAM_H
#include "TestDynamicBufferStringStream.h"
#define GUCEF_CORE_TESTAPP_TESTDYNAMICBUFFERSTRINGSTREAM_H
#endif /* GUCEF_CORE_TESTAPP_TESTDYNAMICBUFFERSTRINGSTREAM_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTINIPARSER_H
#include "TestIniParser.h"
#define GUCEF_CORE_TESTAPP_TESTINIPARSER_H
#endif /* GUCEF_CORE_TESTAPP_TESTINIPARSER_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDATANODEBINARYSERIALIZER_H
#include "TestDataNodeBinarySerializer.h"
#define GUCEF_CORE_TESTAPP_TESTDATANODEBINARYSERIALIZER_H
#endif /* GUCEF_CORE_TESTAPP_TESTDATANODEBINARYSERIALIZER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    try
    {
        GUCEF::CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( logFilename, "gucefCORE_TestApp_Log.txt" );
        GUCEF::CORE::CFileAccess logFileAccess( logFilename, "w" );

        GUCEF::CORE::CStdLogger logger( logFileAccess );
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        GUCEF::CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        //GUCEF::CORE::CGUCEFApplication::Instance()->main( argc, argv, true );
        
        //PerformStringTests();
        //PerformVarsTests();
        //PerformUriTests();
        //PerformVariantTests();
        //PerformVariantBinarySerializerTests();
        //PerformDataNodeBinarySerializerTests();
        //PerformIniParserTests();        
        //PerformSharedPtrTests();
        //PerformDynamicBufferSwapTests();
        PerformDynamicBufferStringStreamTests();
        //PerformCyclicDynamicBufferTests();
        //PerformNotifierObserverTests();

        return 1;
    }
    catch ( ... )
    {
        GUCEF::CORE::ShowErrorMessage( "Unknown exception"                                                                 ,
                                       "Unhandled exception during program execution, the application will now terminate"  );
    }
    return 1;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------*/
