/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_MCPCI_CCIJSON_H
#define GUCEF_MCPCI_CCIJSON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefMcpCi_CCiSubmitOptions.h"
#include "gucefMcpCi_CCiSnapshot.h"
#include "gucefMcpCi_CCiDescription.h"
#include "gucefMcpCi_CCiRunStatus.h"
#include "gucefMcpCi_CCiReport.h"
#include "gucefMcpCi_CCiJobDetail.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The objects of the service as the JSON of the gateway API, both ways: what a gateway writes, a proxy reads back
 *  into the same objects, so that the tools are the same in every mode. Fields that are missing keep their
 *  defaults; fields that are not known are ignored.
 */
class CCiJson
{
    public:

    static void Write( const CCiSubmitOptions& options, GUCEF::CORE::CDataNode& node );
    static void Read( const GUCEF::CORE::CDataNode& node, CCiSubmitOptions& options );

    static void Write( const CCiSnapshot& snapshot, GUCEF::CORE::CDataNode& node );
    static void Read( const GUCEF::CORE::CDataNode& node, CCiSnapshot& snapshot );

    static void Write( const CCiDescription& description, GUCEF::CORE::CDataNode& node );
    static void Read( const GUCEF::CORE::CDataNode& node, CCiDescription& description );

    static void Write( const CCiRunStatus& status, GUCEF::CORE::CDataNode& node );
    static void Read( const GUCEF::CORE::CDataNode& node, CCiRunStatus& status );

    static void Write( const CCiReport::CCell& cell, GUCEF::CORE::CDataNode& node );
    static void Read( const GUCEF::CORE::CDataNode& node, CCiReport::CCell& cell );

    static void Write( const CCiReport& report, GUCEF::CORE::CDataNode& node );
    static void Read( const GUCEF::CORE::CDataNode& node, CCiReport& report );

    /** The logs as they are in 'detail': a gateway bounds them before it writes */
    static void Write( const CCiJobDetail& detail, GUCEF::CORE::CDataNode& node );
    static void Read( const GUCEF::CORE::CDataNode& node, CCiJobDetail& detail );

    /** A list of strings as an array child */
    static void WriteStrings( const GUCEF::CORE::CString::StringVector& values, const char* name, GUCEF::CORE::CDataNode& node );

    /** The state of the interface from its name, STATE_UNKNOWN for any other text */
    static CCiRunStatus::EState StateFromString( const GUCEF::CORE::CString& text );

    private:

    CCiJson( void );    /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCIJSON_H ? */
