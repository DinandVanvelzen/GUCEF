/*
 *  gucef common header: header-only support for tests that start child processes
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

/*
 *  Test support: a test that starts a child process built like itself (the test application again, a tool next to it)
 *  and checks the exit code of that child has to see the exit code of the child's own code.
 *
 *  The thread sanitizer replaces the exit code of a process that ran to its end with 66 when it reported anything
 *  during the run (TSAN_OPTIONS exitcode). KeepChildExitCodeUnderSanitizers() adds exitcode=0 to the TSAN_OPTIONS of
 *  the child, on top of the options of this process: the thread sanitizer then leaves the exit code alone. The exit
 *  code then no longer tells that the child had reports, so they always go to a file: to the log_path of the options
 *  given for the child or of this process, and without one to tsan.<pid> files in the directory
 *  gucef_test_child_sanitizer_logs of the temporary directory (TMPDIR), where a sanitizer run has to look as well.
 *  Without the thread sanitizer the variable has no effect.
 *
 *  Errors of the address sanitizer end the process at the error, so the exit code that comes with them is no exit
 *  code of the child's own code to keep: those stay as they are and the test fails, as it should.
 *
 *    CORE::CProcess::TStringMap environment;
 *    GUCEF::TESTFW::KeepChildExitCodeUnderSanitizers( environment );
 *    child.Launch( exePath, args, CORE::CString(), environment );
 */

#ifndef GUCEF_TEST_CHILD_PROCESS_H
#define GUCEF_TEST_CHILD_PROCESS_H

#ifdef __cplusplus

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

namespace GUCEF {
namespace TESTFW {

/*-------------------------------------------------------------------------*/

/**
 *  @return the value of the last option of that name in a list of sanitizer options (separated by spaces, tabs or
 *  colons, as the sanitizers read them), empty when there is none
 */
inline CORE::CString
SanitizerOptionValue( const CORE::CString& options, const char* name )
{
    CORE::CString value;
    const CORE::CString prefix = CORE::CString( name ) + "=";
    Int32 position = options.HasSubstr( prefix, 0, true );
    while ( position >= 0 )
    {
        const char before = position > 0 ? options[ (UInt32) position - 1 ] : ' ';
        if ( ' ' == before || '\t' == before || ':' == before )
        {
            UInt32 end = (UInt32) position + prefix.Length();
            while ( end < options.Length() && ' ' != options[ end ] && '\t' != options[ end ] && ':' != options[ end ] )
                ++end;
            value = options.SubstrFromRange( (UInt32) position + prefix.Length(), end );
        }
        position = options.HasSubstr( prefix, position + 1, true );
    }
    return value;
}

/*-------------------------------------------------------------------------*/

/** Adds to the environment overrides of a child what keeps the exit code of the child its own under the sanitizers */
inline void
KeepChildExitCodeUnderSanitizers( CORE::CProcess::TStringMap& envOverrides )
{
    // a later option of the same name wins, so the options given for the child or of this process carry over
    CORE::CProcess::TStringMap::iterator given = envOverrides.find( "TSAN_OPTIONS" );
    const CORE::CString environmentOptions = CORE::GetEnv( "TSAN_OPTIONS" );
    CORE::CString tsanOptions = given != envOverrides.end() ? (*given).second : environmentOptions;

    // With exitcode=0 only a log tells that the child had reports: without a log_path they would go to its stderr,
    // which the test may not keep. Then the log_path of this process, else one in the temporary directory
    if ( SanitizerOptionValue( tsanOptions, "log_path" ).IsNULLOrEmpty() )
    {
        CORE::CString logPath = SanitizerOptionValue( environmentOptions, "log_path" );
        if ( logPath.IsNULLOrEmpty() )
        {
            const CORE::CString logDir = CORE::CombinePath( CORE::TempDir(), "gucef_test_child_sanitizer_logs" );
            CORE::CreateDirs( logDir );
            logPath = CORE::CombinePath( logDir, "tsan" );
        }
        tsanOptions = tsanOptions.IsNULLOrEmpty() ? "log_path=" + logPath : tsanOptions + " log_path=" + logPath;
    }
    envOverrides[ "TSAN_OPTIONS" ] = tsanOptions + " exitcode=0";
}

/*-------------------------------------------------------------------------*/

/** The environment overrides for a child whose exit code a test checks, see KeepChildExitCodeUnderSanitizers() */
inline CORE::CProcess::TStringMap
ChildEnvironmentWithOwnExitCode( void )
{
    CORE::CProcess::TStringMap envOverrides;
    KeepChildExitCodeUnderSanitizers( envOverrides );
    return envOverrides;
}

/*-------------------------------------------------------------------------*/

}; /* namespace TESTFW */
}; /* namespace GUCEF */

#endif /* __cplusplus ? */

#endif /* GUCEF_TEST_CHILD_PROCESS_H ? */
