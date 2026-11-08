/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_TESTAPP_TESTFRAMEWORKSUITES_H
#define GUCEF_CORE_TESTAPP_TESTFRAMEWORKSUITES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  First argument with which a test starts the test app itself to make assertions outside every test case
 *  (RunAssertionsOutsideCases()); the second argument is the path of the results XML it writes.
 */
#define GUCEF_CORE_TESTAPP_ASSERT_OUTSIDE_CASES_ARG     "--assertions-outside-cases"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests suites of the test framework that open inside other suites: every test case is recorded in the suite that
 *  is open where it runs, also the cases that follow an inner suite, and the JUnit XML output stays well formed.
 *  Assertions that fail outside every test case (in a suite, in a nested suite, outside every suite) count as
 *  failures of the run, and a child process started by a test keeps its sanitizer reports in a log.
 */
void
PerformTestFrameworkSuitesTests( void );

/*-------------------------------------------------------------------------*/

/**
 *  What the test app does when started with GUCEF_CORE_TESTAPP_ASSERT_OUTSIDE_CASES_ARG: a run of its own of the test
 *  framework with failing assertions outside every test case and passing test cases, its results written to the
 *  given path. Returns the exit code of that run.
 */
int
RunAssertionsOutsideCases( const char* resultsXmlPath );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTFRAMEWORKSUITES_H ? */
