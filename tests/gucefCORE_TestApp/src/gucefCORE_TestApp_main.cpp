/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#define GUCEF_TESTFW_IMPL
#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_LOG_CALLBACK_H
#include "gucef_test_log_callback.h"
#define GUCEF_TEST_LOG_CALLBACK_H
#endif /* GUCEF_TEST_LOG_CALLBACK_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

#ifndef GUCEF_CORE_TESTAPP_TESTVERSION_H
#include "TestVersion.h"
#define GUCEF_CORE_TESTAPP_TESTVERSION_H
#endif /* GUCEF_CORE_TESTAPP_TESTVERSION_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTVARIANTBINARYSERIALIZER_H
#include "TestVariantBinarySerializer.h"
#define GUCEF_CORE_TESTAPP_TESTVARIANTBINARYSERIALIZER_H
#endif /* GUCEF_CORE_TESTAPP_TESTVARIANTBINARYSERIALIZER_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTNOTIFIEROBSERVER_H
#include "TestNotifierObserver.h"
#define GUCEF_CORE_TESTAPP_TESTNOTIFIEROBSERVER_H
#endif /* GUCEF_CORE_TESTAPP_TESTNOTIFIEROBSERVER_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTSCHEDULE_H
#include "TestSchedule.h"
#include "TestScheduleTrigger.h"
#include "TestFloatText.h"
#define GUCEF_CORE_TESTAPP_TESTSCHEDULE_H
#endif /* GUCEF_CORE_TESTAPP_TESTSCHEDULE_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTCRONSCHEDULE_H
#include "TestCronSchedule.h"
#define GUCEF_CORE_TESTAPP_TESTCRONSCHEDULE_H
#endif /* GUCEF_CORE_TESTAPP_TESTCRONSCHEDULE_H ? */

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

#ifndef GUCEF_CORE_TESTAPP_TESTDATANODE_H
#include "TestDataNode.h"
#include "TestWeakPtr.h"
#include "TestX11EventDispatcher.h"
#include "TestX11Clipboard.h"
#include "TestSysConsole.h"
#include "TestConfigStore.h"
#include "TestScopedValue.h"
#define GUCEF_CORE_TESTAPP_TESTDATANODE_H
#endif /* GUCEF_CORE_TESTAPP_TESTDATANODE_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDATETIME_H
#include "TestDateTime.h"
#define GUCEF_CORE_TESTAPP_TESTDATETIME_H
#endif /* GUCEF_CORE_TESTAPP_TESTDATETIME_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDATETIMERANGE_H
#include "TestDateTimeRange.h"
#define GUCEF_CORE_TESTAPP_TESTDATETIMERANGE_H
#endif /* GUCEF_CORE_TESTAPP_TESTDATETIMERANGE_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTTOKENIZER_H
#include "TestTokenizer.h"
#define GUCEF_CORE_TESTAPP_TESTTOKENIZER_H
#endif /* GUCEF_CORE_TESTAPP_TESTTOKENIZER_H ? */
                                
#ifndef GUCEF_CORE_TESTAPP_TESTFILEUTILS_H
#include "TestFileUtils.h"
#define GUCEF_CORE_TESTAPP_TESTFILEUTILS_H
#endif /* GUCEF_CORE_TESTAPP_TESTFILEUTILS_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTFRACTION_H
#include "TestFraction.h"
#define GUCEF_CORE_TESTAPP_TESTFRACTION_H
#endif /* GUCEF_CORE_TESTAPP_TESTFRACTION_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTUINT128_H
#include "TestUInt128.h"
#define GUCEF_CORE_TESTAPP_TESTUINT128_H
#endif /* GUCEF_CORE_TESTAPP_TESTUINT128_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDECIMAL_H
#include "TestDecimal.h"
#define GUCEF_CORE_TESTAPP_TESTDECIMAL_H
#endif /* GUCEF_CORE_TESTAPP_TESTDECIMAL_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDECIMALVARIANT_H
#include "TestDecimalVariant.h"
#define GUCEF_CORE_TESTAPP_TESTDECIMALVARIANT_H
#endif /* GUCEF_CORE_TESTAPP_TESTDECIMALVARIANT_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDATATABLEVARIANT_H
#include "TestDataTableVariant.h"
#define GUCEF_CORE_TESTAPP_TESTDATATABLEVARIANT_H
#endif /* GUCEF_CORE_TESTAPP_TESTDATATABLEVARIANT_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDATATABLESCHEMA_H
#include "TestDataTableSchema.h"
#define GUCEF_CORE_TESTAPP_TESTDATATABLESCHEMA_H
#endif /* GUCEF_CORE_TESTAPP_TESTDATATABLESCHEMA_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDATATABLEBUFFERPOOL_H
#include "TestDataTableBufferPool.h"
#define GUCEF_CORE_TESTAPP_TESTDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_TESTAPP_TESTDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDATATABLECOLUMNBATCH_H
#include "TestDataTableColumnBatch.h"
#define GUCEF_CORE_TESTAPP_TESTDATATABLECOLUMNBATCH_H
#endif /* GUCEF_CORE_TESTAPP_TESTDATATABLECOLUMNBATCH_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDATATABLEBATCHBUILDER_H
#include "TestDataTableBatchBuilder.h"
#define GUCEF_CORE_TESTAPP_TESTDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_TESTAPP_TESTDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDATATABLEPREDICATE_H
#include "TestDataTablePredicate.h"
#define GUCEF_CORE_TESTAPP_TESTDATATABLEPREDICATE_H
#endif /* GUCEF_CORE_TESTAPP_TESTDATATABLEPREDICATE_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDATATABLESCAN_H
#include "TestDataTableScan.h"
#define GUCEF_CORE_TESTAPP_TESTDATATABLESCAN_H
#endif /* GUCEF_CORE_TESTAPP_TESTDATATABLESCAN_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTMEMORYDATATABLE_H
#include "TestMemoryDataTable.h"
#define GUCEF_CORE_TESTAPP_TESTMEMORYDATATABLE_H
#endif /* GUCEF_CORE_TESTAPP_TESTMEMORYDATATABLE_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTFILEDATATABLE_H
#include "TestFileDataTable.h"
#define GUCEF_CORE_TESTAPP_TESTFILEDATATABLE_H
#endif /* GUCEF_CORE_TESTAPP_TESTFILEDATATABLE_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDATATABLESCANBENCHMARK_H
#include "TestDataTableScanBenchmark.h"
#define GUCEF_CORE_TESTAPP_TESTDATATABLESCANBENCHMARK_H
#endif /* GUCEF_CORE_TESTAPP_TESTDATATABLESCANBENCHMARK_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTTABLECODECREGISTRY_H
#include "TestTableCodecRegistry.h"
#define GUCEF_CORE_TESTAPP_TESTTABLECODECREGISTRY_H
#endif /* GUCEF_CORE_TESTAPP_TESTTABLECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTCSVTABLECODECROUNDTRIP_H
#include "TestCsvTableCodecRoundTrip.h"
#define GUCEF_CORE_TESTAPP_TESTCSVTABLECODECROUNDTRIP_H
#endif /* GUCEF_CORE_TESTAPP_TESTCSVTABLECODECROUNDTRIP_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTCSVTABLECODECINFERENCE_H
#include "TestCsvTableCodecInference.h"
#define GUCEF_CORE_TESTAPP_TESTCSVTABLECODECINFERENCE_H
#endif /* GUCEF_CORE_TESTAPP_TESTCSVTABLECODECINFERENCE_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTCSVTABLECODECWRITER_H
#include "TestCsvTableCodecWriter.h"
#define GUCEF_CORE_TESTAPP_TESTCSVTABLECODECWRITER_H
#endif /* GUCEF_CORE_TESTAPP_TESTCSVTABLECODECWRITER_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTCSVTABLECODEC_H
#include "TestCsvTableCodec.h"
#define GUCEF_CORE_TESTAPP_TESTCSVTABLECODEC_H
#endif /* GUCEF_CORE_TESTAPP_TESTCSVTABLECODEC_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDATADRIVENDSTORECODECS_H
#include "TestDataDrivenDStoreCodecs.h"
#define GUCEF_CORE_TESTAPP_TESTDATADRIVENDSTORECODECS_H
#endif /* GUCEF_CORE_TESTAPP_TESTDATADRIVENDSTORECODECS_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTABSTRACTFACTORY_H
#include "TestAbstractFactory.h"
#define GUCEF_CORE_TESTAPP_TESTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_TESTAPP_TESTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTASYNC_H
#include "TestASync.h"
#define GUCEF_CORE_TESTAPP_TESTASYNC_H
#endif /* GUCEF_CORE_TESTAPP_TESTASYNC_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTTHREADPOOLCANCELLATION_H
#include "TestThreadPoolCancellation.h"
#include "TestTaskMemoryRetention.h"
#include "TestFutureResultSource.h"
#include "TestFileSystemAccess.h"
#include "TestVariantMapAdapter.h"
#include "TestPluginUnload.h"
#include "TestSfinaeUtils.h"
#include "TestExternalThreads.h"
#include "TestDedicatedTasks.h"
#include "TestDynamicBuffer.h"
#include "TestAsyncUriResourceRetriever.h"
#include "TestDesktopTerminalConsoleLogger.h"
#define GUCEF_CORE_TESTAPP_TESTTHREADPOOLCANCELLATION_H
#endif /* GUCEF_CORE_TESTAPP_TESTTHREADPOOLCANCELLATION_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTFREELIST_H
#include "TestFreeList.h"
#define GUCEF_CORE_TESTAPP_TESTFREELIST_H
#endif /* GUCEF_CORE_TESTAPP_TESTFREELIST_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTVARIANTSTREAM_H
#include "TestVariantStream.h"
#define GUCEF_CORE_TESTAPP_TESTVARIANTSTREAM_H
#endif /* GUCEF_CORE_TESTAPP_TESTVARIANTSTREAM_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTTIMESTAMP_H
#include "TestTimestamp.h"
#define GUCEF_CORE_TESTAPP_TESTTIMESTAMP_H
#endif /* GUCEF_CORE_TESTAPP_TESTTIMESTAMP_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTSTOPWATCH_H
#include "TestStopwatch.h"
#define GUCEF_CORE_TESTAPP_TESTSTOPWATCH_H
#endif /* GUCEF_CORE_TESTAPP_TESTSTOPWATCH_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTLOGGING_H
#include "TestLogging.h"
#define GUCEF_CORE_TESTAPP_TESTLOGGING_H
#endif /* GUCEF_CORE_TESTAPP_TESTLOGGING_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTPROCESS_H
#include "TestProcess.h"
#define GUCEF_CORE_TESTAPP_TESTPROCESS_H
#endif /* GUCEF_CORE_TESTAPP_TESTPROCESS_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTXMLDSTORECODEC_H
#include "TestXmlDStoreCodec.h"
#define GUCEF_CORE_TESTAPP_TESTXMLDSTORECODEC_H
#endif /* GUCEF_CORE_TESTAPP_TESTXMLDSTORECODEC_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDSTORESTREAMING_H
#include "TestDStoreStreaming.h"
#define GUCEF_CORE_TESTAPP_TESTDSTORESTREAMING_H
#endif /* GUCEF_CORE_TESTAPP_TESTDSTORESTREAMING_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTCODECPLUGINS_H
#include "TestCodecPlugins.h"
#include "TestCryptoCodecs.h"
#define GUCEF_CORE_TESTAPP_TESTCODECPLUGINS_H
#endif /* GUCEF_CORE_TESTAPP_TESTCODECPLUGINS_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTRANDOM_H
#include "TestRandom.h"
#define GUCEF_CORE_TESTAPP_TESTRANDOM_H
#endif /* GUCEF_CORE_TESTAPP_TESTRANDOM_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTBASE64URL_H
#include "TestBase64Url.h"
#define GUCEF_CORE_TESTAPP_TESTBASE64URL_H
#endif /* GUCEF_CORE_TESTAPP_TESTBASE64URL_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTREDACTOR_H
#include "TestRedactor.h"
#define GUCEF_CORE_TESTAPP_TESTREDACTOR_H
#endif /* GUCEF_CORE_TESTAPP_TESTREDACTOR_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTBIGUNSIGNED_H
#include "TestBigUnsigned.h"
#define GUCEF_CORE_TESTAPP_TESTBIGUNSIGNED_H
#endif /* GUCEF_CORE_TESTAPP_TESTBIGUNSIGNED_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTBROWSERLAUNCHER_H
#include "TestBrowserLauncher.h"
#define GUCEF_CORE_TESTAPP_TESTBROWSERLAUNCHER_H
#endif /* GUCEF_CORE_TESTAPP_TESTBROWSERLAUNCHER_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTUSERNAMES_H
#include "TestUserNames.h"
#define GUCEF_CORE_TESTAPP_TESTUSERNAMES_H
#endif /* GUCEF_CORE_TESTAPP_TESTUSERNAMES_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTRATELIMITING_H
#include "TestRateLimiting.h"
#define GUCEF_CORE_TESTAPP_TESTRATELIMITING_H
#endif /* GUCEF_CORE_TESTAPP_TESTRATELIMITING_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTSECUREBUFFER_H
#include "TestSecureBuffer.h"
#define GUCEF_CORE_TESTAPP_TESTSECUREBUFFER_H
#endif /* GUCEF_CORE_TESTAPP_TESTSECUREBUFFER_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTFILEACCESS_H
#include "TestFileAccess.h"
#define GUCEF_CORE_TESTAPP_TESTFILEACCESS_H
#endif /* GUCEF_CORE_TESTAPP_TESTFILEACCESS_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTFRAMEWORKSKIP_H
#include "TestFrameworkSkip.h"
#define GUCEF_CORE_TESTAPP_TESTFRAMEWORKSKIP_H
#endif /* GUCEF_CORE_TESTAPP_TESTFRAMEWORKSKIP_H ? */

#ifndef GUCEF_CORE_TESTAPP_JSONDATANODEFIDELITY_H
#include "TestJsonDataNodeFidelity.h"
#define GUCEF_CORE_TESTAPP_JSONDATANODEFIDELITY_H
#endif /* GUCEF_CORE_TESTAPP_JSONDATANODEFIDELITY_H ? */

#ifndef GUCEF_CORE_TESTAPP_JSONSCHEMA_H
#include "TestJsonSchema.h"
#define GUCEF_CORE_TESTAPP_JSONSCHEMA_H
#endif /* GUCEF_CORE_TESTAPP_JSONSCHEMA_H ? */

#ifndef GUCEF_CORE_TESTAPP_JSONSYNTAXVALIDATOR_H
#include "TestJsonSyntaxValidator.h"
#define GUCEF_CORE_TESTAPP_JSONSYNTAXVALIDATOR_H
#endif /* GUCEF_CORE_TESTAPP_JSONSYNTAXVALIDATOR_H ? */

#ifndef GUCEF_CORE_TESTAPP_STANDARDSTREAMS_H
#include "TestStandardStreams.h"
#define GUCEF_CORE_TESTAPP_STANDARDSTREAMS_H
#endif /* GUCEF_CORE_TESTAPP_STANDARDSTREAMS_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTTIMEZONERULES_H
#include "TestTimeZoneRules.h"
#define GUCEF_CORE_TESTAPP_TESTTIMEZONERULES_H
#endif /* GUCEF_CORE_TESTAPP_TESTTIMEZONERULES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    try
    {
        // started by a test as the application whose exit it looks at
        if ( argc > 1 && 0 == strcmp( argv[ 1 ], GUCEF_CORE_TESTAPP_EXIT_WITH_CONSOLE_LOG_WINDOW_ARG ) )
            return RunExitWithConsoleLogWindow();

        GUCEF::CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( logFilename, "gucefCORE_TestApp_Log.txt" );
        GUCEF::CORE::CFileAccess logFileAccess( logFilename, "w" );

        GUCEF::CORE::CStdLogger logger( logFileAccess );
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        GUCEF::CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        GUCEF_TESTFW_INIT( "gucefCORE_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( GUCEF::TESTFW::TestFrameworkLogCallback, GUCEF_NULL );

        //GUCEF::CORE::CGUCEFApplication::Instance()->main( argc, argv, true );
        
        PerformStringTests();
        PerformVarsTests();
        PerformFileUtilsTests();
        PerformFractionTests();
        PerformUInt128Tests();
        PerformDecimalTests();
        PerformAbstractFactoryTests();
        PerformTimestampTests();
        PerformStopwatchTests();
        PerformDateTimeTests();
        PerformDateTimeRangeTests();
        PerformUriTests();
        PerformVersionTests();
        PerformVersionRangeTests();
        PerformCronScheduleTests();
        PerformScheduleTests();
        PerformScheduleTriggerTests();
        PerformFloatTextTests();
        PerformVariantTests();
        PerformVariantViewTests();
        PerformVariantBinarySerializerTests();
        PerformDataNodeBinarySerializerTests();
        PerformDataNodeTests();
        PerformWeakPtrTests();
        PerformX11EventDispatcherTests();
        PerformX11ClipboardTests();
        PerformSysConsoleTests();
        PerformConfigStoreTests();
        PerformScopedValueTests();
        PerformIniParserTests();        
        PerformSharedPtrTests();
        PerformTokenizerTests();
        PerformDynamicBufferTests();
        PerformDynamicBufferSwapTests();
        PerformDynamicBufferViewTests();
        PerformDynamicBufferStringStreamTests();
        PerformCyclicDynamicBufferTests();
        PerformNotifierObserverTests();
        PerformVariantStreamTests();
        PerformLoggingTests();
        PerformDataDrivenDStoreCodecsTests();
        PerformFreeListTests();
        PerformASyncTests();
        PerformThreadPoolCancellationTests();
        PerformTaskMemoryRetentionTests();
        PerformFutureResultSourceTests();
        PerformFileSystemAccessTests();
        PerformVariantMapAdapterTests();
        PerformExternalThreadTests();
        PerformDedicatedTaskTests();
        PerformAsyncUriResourceRetrieverTests();
        PerformDesktopTerminalConsoleLoggerTests();
        PerformProcessTests();
        PerformXmlDStoreCodecTests();
        PerformDStoreStreamingTests();
        PerformDecimalVariantTests();
        PerformDataTableVariantTests();
        PerformDataTableSchemaTests();
        PerformDataTableBufferPoolTests();
        PerformDataTableColumnBatchTests();
        PerformDataTableBatchBuilderTests();
        PerformDataTablePredicateTests();
        PerformDataTableScanTests();
        PerformMemoryDataTableTests();
        PerformFileDataTableTests();
        PerformDataTableScanBenchmarkTests();
        PerformTableCodecRegistryTests();
        PerformCsvTableCodecRoundTripTests();
        PerformCsvTableCodecInferenceTests();
        PerformCsvTableCodecWriterTests();
        PerformCsvTableCodecTests();
        PerformCodecPluginTests();
        PerformCryptoCodecTests();
        PerformBase64UrlTests();
        PerformRedactorTests();
        PerformBigUnsignedTests();
        PerformBrowserLauncherTests();
        PerformUserNameTests();
        PerformPluginUnloadTests();
        PerformSfinaeUtilsTests();
        PerformRandomTests();
        PerformRateLimitingTests();
        PerformSecureBufferTests();
        PerformFileAccessTests();
        PerformTestFrameworkSkipTests();
        PerformJsonDataNodeFidelityTests();
        PerformJsonSchemaTests();
        PerformJsonSyntaxValidatorTests();
        PerformStandardStreamsTests();
        PerformTimeZoneRulesTests();

        GUCEF::CORE::CString xmlPath = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( xmlPath, "gucefCORE_TestApp_Results.xml" );
        GUCEF_TESTFW_WRITE_RESULTS( xmlPath.C_String() );
        return GUCEF_TESTFW_EXITCODE;
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
