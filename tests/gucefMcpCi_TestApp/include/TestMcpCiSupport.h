/*
 *  gucefMcpCi_TestApp: tests of the gucefMcpCi MCP server
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

#ifndef GUCEF_MCPCI_TESTAPP_TESTMCPCISUPPORT_H
#define GUCEF_MCPCI_TESTAPP_TESTMCPCISUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace MCPCITEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The gucefMcpCi executable next to the test application, empty when it is not there */
GUCEF::CORE::CString FindServer( void );

/** The environment of a server child: the log goes to its standard error, its exit code is its own */
GUCEF::CORE::CProcess::TStringMap ChildEnvironment( void );

/** A JSON string literal */
GUCEF::CORE::CString Json( const GUCEF::CORE::CString& text );

/** The text without white space, line ends included, at both ends */
GUCEF::CORE::CString Trimmed( const GUCEF::CORE::CString& text );

bool Contains( const GUCEF::CORE::CString& text, const GUCEF::CORE::CString& part );

bool WriteFile( const GUCEF::CORE::CString& path, const GUCEF::CORE::CString& content );

bool ReadFile( const GUCEF::CORE::CString& path, GUCEF::CORE::CString& content );

/** A file for a secret: readable by the owner only, as the server demands */
bool WriteOwnerOnlyFile( const GUCEF::CORE::CString& path, const GUCEF::CORE::CString& content );

/**
 *  Runs a program to the end and collects its standard output and standard error.
 *  @return its exit code, -1 when it did not start or did not end in time
 */
GUCEF::Int32 RunCapture( const GUCEF::CORE::CString& exe                         ,
                         const GUCEF::CORE::CProcess::TStringVector& args        ,
                         GUCEF::CORE::CString& output                            ,
                         const GUCEF::CORE::CString& workingDir = GUCEF::CORE::CString::Empty ,
                         GUCEF::UInt32 timeoutInMs = 60000                       );

/** git -C dir with up to six arguments. @return its exit code */
GUCEF::Int32 Git( const GUCEF::CORE::CString& dir, const char* a, const char* b = GUCEF_NULL, const char* c = GUCEF_NULL,
                  const char* d = GUCEF_NULL, const char* e = GUCEF_NULL, const char* f = GUCEF_NULL );

/** The output of git -C dir with up to six arguments, without white space at its ends; empty when git failed */
GUCEF::CORE::CString GitOut( const GUCEF::CORE::CString& dir, const char* a, const char* b = GUCEF_NULL, const char* c = GUCEF_NULL,
                             const char* d = GUCEF_NULL, const char* e = GUCEF_NULL, const char* f = GUCEF_NULL );

/** A commit of everything that is staged, with an identity of its own */
bool GitCommit( const GUCEF::CORE::CString& dir, const GUCEF::CORE::CString& message );

bool GitAvailable( void );

/** Compresses with the gzip codec (codecspluginZLIB, loaded when needed). @return false when there is no such codec */
bool Gzip( const GUCEF::CORE::CString& text, GUCEF::CORE::CDynamicBuffer& compressed );

/** One MCP POST over HTTP(S) as a 2025-11-25 client sends it: @return the status, 0 when no response came */
GUCEF::Int32 McpHttpPost( const GUCEF::CORE::CString& url, const GUCEF::CORE::CString& token, const GUCEF::CORE::CString& body,
                          const GUCEF::CORE::CString& caBundle, GUCEF::CORE::CString& responseText );

/** Percent decoding of a URL path */
GUCEF::CORE::CString PercentDecode( const GUCEF::CORE::CString& text );

/*-------------------------------------------------------------------------*/

}; /* namespace MCPCITEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_TESTAPP_TESTMCPCISUPPORT_H ? */
