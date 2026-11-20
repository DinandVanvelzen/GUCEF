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

#ifndef GUCEF_MCPCI_TESTAPP_TESTCISERVER_H
#define GUCEF_MCPCI_TESTAPP_TESTCISERVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "TestMcpSupport.h"
#include "TestMcpClientSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace MCPCITEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The gucefMcpCi server as a child process, talked to over its standard input and output. Keeps everything it
 *  answered, so that a test can check that a secret appears nowhere.
 */
class CTestCiServer
{
    public:

    CTestCiServer( void );

    bool Start( const GUCEF::CORE::CProcess::TStringVector& args );

    /** Calls a tool. @return false when no response came */
    bool Call( const GUCEF::CORE::CString& tool, const GUCEF::CORE::CString& argumentsJson, GUCEF::CORE::CString& text, bool& isError,
               GUCEF::UInt32 timeoutInMs = 60000 );

    /** The names of the tools, comma separated, in the order of the list */
    GUCEF::CORE::CString ToolNames( void );

    /** The instructions of the server */
    GUCEF::CORE::CString Instructions( void );

    /** Closes the input and waits for the server to end. @return whether it ended with exit code 0 */
    bool Stop( void );

    /** Everything the server answered and logged so far */
    GUCEF::CORE::CString GetTranscript( void ) const;

    private:

    bool Request( const GUCEF::CORE::CString& method, const GUCEF::CORE::CString& params, GUCEF::CORE::CDataNode& response, GUCEF::UInt32 timeoutInMs );

    CMcpSubprocessClient m_client;
    GUCEF::UInt32 m_nextId;
    bool m_started;
    GUCEF::CORE::CString m_transcript;
};

/*-------------------------------------------------------------------------*/

}; /* namespace MCPCITEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_TESTAPP_TESTCISERVER_H ? */
