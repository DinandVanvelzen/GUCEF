/*
 *  gucefMULTIPLAY_MailProvider_TestApp: Application for testing the mail match provider of gucefMULTIPLAY
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

#ifndef GUCEF_MULTIPLAY_MAILTESTAPP_TESTMAILPUMP_H
#define GUCEF_MULTIPLAY_MAILTESTAPP_TESTMAILPUMP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <exception>
#include <sstream>
#include <cstdio>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_COM_CEMAILCLIENTBASE_H
#include "gucefCOM_CEMailClientBase.h"
#define GUCEF_COM_CEMAILCLIENTBASE_H
#endif /* GUCEF_COM_CEMAILCLIENTBASE_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The assertions of these tests end the test case at the first failure: what follows a failed check (an element of a list that is
 *  not there) would be undefined. The failure is recorded as the framework records it; the catch of the test case does not
 *  count the ending as a second one.
 */
struct SEMailCaseAborted {};

/** A line for the log of the tests, for what explains a failure */
#define MAIL_TEST_NOTE( stream ) GUCEF_LOG( ::GUCEF::CORE::LOGLEVEL_NORMAL, "NOTE: " << stream )

#define MAIL_ASSERT_TRUE( expr )                                                                    \
    do { if ( !(expr) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #expr, __FILE__, __LINE__ ); throw SEMailCaseAborted(); } } while ( 0 )

#define MAIL_ASSERT_FALSE( expr )                                                                   \
    do { if ( (expr) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( "!(" #expr ")", __FILE__, __LINE__ ); throw SEMailCaseAborted(); } } while ( 0 )

#define MAIL_ERRORHERE                                                                              \
    do { try { throw; } catch ( const SEMailCaseAborted& ) { } \
         catch ( const std::exception& error ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( error.what(), __FILE__, __LINE__ ); } \
         catch ( ... ) { GUCEF_TESTFW_ERRORHERE; } } while ( 0 )

/*
 *  The sockets, timers and notifiers run on the global pulse generator. Tests that wait for something pulse it from
 *  the test thread, so everything runs on one thread, and every wait has a deadline.
 */

/** What is done at every step of the pump: a table of players moves its clock with it when the polling of the mailboxes is on the clock */
struct SPumpHook
{
    void (*function)( void* context );
    void* context;
};

inline SPumpHook&
GetPumpHook( void )
{
    static SPumpHook hook = { 0, 0 };
    return hook;
}

/** For the work on one test case: the environment variable MAILTEST_ONLY names a part of the title of the cases to run, none set runs all */
inline bool
MailTestWanted( const char* title )
{
    const char* only = getenv( "MAILTEST_ONLY" );
    return GUCEF_NULL == only || '\0' == only[ 0 ] || GUCEF_NULL != strstr( title, only );
}

/** One pulse of the global pulse generator and a millisecond of waiting */
inline void
PumpOnce( void )
{GUCEF_TRACE;

    if ( 0 != GetPumpHook().function )
        GetPumpHook().function( GetPumpHook().context );
    GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
    GUCEF::MT::ThreadDelay( 1 );
}

/*-------------------------------------------------------------------------*/

/** The time in milliseconds on a clock that runs: the waits of the tests are in real time */
inline GUCEF::UInt64
PumpNowMs( void )
{
    return GUCEF::COM::CEMailClientBase::GetTimeInMs();
}

/*-------------------------------------------------------------------------*/

inline void
PumpFor( GUCEF::UInt32 durationInMs )
{GUCEF_TRACE;

    const GUCEF::UInt64 start = PumpNowMs();
    while ( PumpNowMs() - start < durationInMs )
        PumpOnce();
}

/*-------------------------------------------------------------------------*/

/** Pulses until the condition holds or the time is up; okVariable tells which */
#define PUMP_WAIT( okVariable, condition, timeoutInMs )                                         \
    {                                                                                           \
        (okVariable) = false;                                                                   \
        const GUCEF::UInt64 pumpStart = PumpNowMs();                                            \
        while ( PumpNowMs() - pumpStart < (GUCEF::UInt64)(timeoutInMs) )                        \
        {                                                                                       \
            if ( (condition) ) { (okVariable) = true; break; }                                  \
            PumpOnce();                                                                         \
        }                                                                                       \
        if ( !(okVariable) )                                                                    \
            (okVariable) = (condition);                                                         \
    }

/*-------------------------------------------------------------------------*/

/** Loads a plugin from the directory of the module, debug or release build */
inline bool
LoadTestPlugin( const GUCEF::CORE::CString& pluginName )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    GUCEF::CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    GUCEF::CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    GUCEF::CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    GUCEF::CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif

    GUCEF::CORE::CPluginControl& pluginControl = GUCEF::CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
}

/*-------------------------------------------------------------------------*/

/** Whether the "json" data store codec is there, loading its plugin when it is not */
inline bool
JsonCodecAvailable( void )
{GUCEF_TRACE;

    GUCEF::CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !GUCEF::CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) )
        LoadTestPlugin( "dstorepluginJSONPARSER" );
    return GUCEF::CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec );
}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_MAILTESTAPP_TESTMAILPUMP_H ? */
