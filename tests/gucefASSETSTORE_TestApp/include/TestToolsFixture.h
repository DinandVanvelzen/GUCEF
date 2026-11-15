/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef TESTTOOLSFIXTURE_H
#define TESTTOOLSFIXTURE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TEST_MCP_RIG_H
#include "gucef_test_mcp_rig.h"
#define GUCEF_TEST_MCP_RIG_H
#endif /* GUCEF_TEST_MCP_RIG_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLSET_H
#include "gucefASSETSTORE_CAssetToolSet.h"
#define GUCEF_ASSETSTORE_CASSETTOOLSET_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLSET_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An asset tool set on a service, registered into the MCP server of a TESTFW::CTestMcpRig, for the end-to-end tests
 *  of the tools: calls go over the in-memory channel as a client sends them (modern requests), the test thread is
 *  the protocol thread, and Pump() also pulses the global pulse generator, which delivers the events of an in-process
 *  service. Every message the server sent is kept (GetTranscript()), for the sweep of the redaction tests.
 */
class CToolsFixture
{
    public:

    GUCEF::TESTFW::CTestMcpRig rig;
    GUCEF::ASSETSTORE::TAssetStoreServicePtr service;
    GUCEF::ASSETSTORE::CAssetToolSet* toolSet;
    bool ok;
    GUCEF::CORE::CString error;

    CToolsFixture( const GUCEF::ASSETSTORE::TAssetStoreServicePtr& service, const GUCEF::ASSETSTORE::CAssetToolSetSettings& settings,
                   const char* subject = "local:tester" );

    ~CToolsFixture();

    /** Puts an exposure in force and lets the tools be registered again */
    void SetExposure( const GUCEF::ASSETSTORE::CAssetToolsExposure& exposure );

    /** Every group on */
    void EnableAll( void );

    /** tools/call; 'argumentsJson' a JSON object text */
    bool Call( const GUCEF::CORE::CString& tool, const GUCEF::CORE::CString& argumentsJson, GUCEF::CORE::CDataNode& response,
               GUCEF::UInt32 timeoutMs = 15000, const GUCEF::CORE::CString& progressToken = GUCEF::CORE::CString::Empty );

    /** A request with the client capabilities and the extra params given (JSON texts) */
    bool RequestWith( const GUCEF::CORE::CString& method, const GUCEF::CORE::CString& params, const GUCEF::CORE::CString& capabilities,
                      GUCEF::CORE::CDataNode& response, GUCEF::UInt32 timeoutMs = 15000 );

    bool Request( const GUCEF::CORE::CString& method, const GUCEF::CORE::CString& params, GUCEF::CORE::CDataNode& response, GUCEF::UInt32 timeoutMs = 15000 );

    void Pump( GUCEF::UInt32 durationInMs );

    /** The names of the tools of tools/list */
    bool ListTools( GUCEF::CORE::CString::StringVector& names, GUCEF::CORE::CDataNode* list = GUCEF_NULL );

    /** Polls job_status until the state is reached or the time is up */
    bool WaitForJobState( const GUCEF::CORE::CString& jobId, const GUCEF::CORE::CString& state, GUCEF::UInt32 timeoutMs = 20000 );

    /** Everything the server sent, as one text */
    GUCEF::CORE::CString GetTranscript( void ) const;

    static GUCEF::CORE::CString Attr( const GUCEF::CORE::CDataNode* node, const char* name );
    static const GUCEF::CORE::CDataNode* Structured( const GUCEF::CORE::CDataNode& response );
    static GUCEF::CORE::CString StructuredAttr( const GUCEF::CORE::CDataNode& response, const char* name );
    static GUCEF::CORE::CString ErrorCode( const GUCEF::CORE::CDataNode& response );
    static bool IsToolError( const GUCEF::CORE::CDataNode& response );
    static GUCEF::Int32 ProtocolError( const GUCEF::CORE::CDataNode& response );
    static GUCEF::UInt32 CountContent( const GUCEF::CORE::CDataNode& response, const char* type );
    static GUCEF::UInt32 CountItems( const GUCEF::CORE::CDataNode& response, const char* listName = "items" );

    private:

    CToolsFixture( const CToolsFixture& src );
    CToolsFixture& operator=( const CToolsFixture& src );

    GUCEF::UInt32 m_nextId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTTOOLSFIXTURE_H ? */
