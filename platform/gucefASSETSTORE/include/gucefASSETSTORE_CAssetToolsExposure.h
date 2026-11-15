/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLSEXPOSURE_H
#define GUCEF_ASSETSTORE_CASSETTOOLSEXPOSURE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the owner opens of the asset tools to MCP clients (section 12.2 of the asset tools design): the exposure file
 *  "GUCEF.AssetTools.McpExposure", version 1, read strictly. Without a file, or with a file that does not read, the
 *  closed defaults apply: the read-only tools only.
 *
 *  - mcpEnabled: false hides every tool, resource and prompt
 *  - toolGroups: convert, author, store, each off by default; the group "read" is always on
 *  - allowedWriteTools: single write tools beyond the groups; deniedTools wins over both
 *  - exposePurchaseDetails: whether purchaser and order reference of licence records are shown
 *  - maxWritesPerHour (60), maxJobsPerDay (50) per principal; maxRows (100), maxUntrustedTextChars (500)
 *  Hard maxima bound the numbers, so an edited file cannot open the limits further than the code allows.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolsExposure
{
    public:

    static const CORE::CString FormatName;          /**< "GUCEF.AssetTools.McpExposure" */
    static const UInt32 FormatVersion = 1;
    static const UInt32 HardMaxRows = 100;
    static const UInt32 HardMaxUntrustedTextChars = 2000;
    static const UInt32 HardMaxWritesPerHour = 3600;
    static const UInt32 HardMaxJobsPerDay = 10000;

    static const CORE::CString GroupRead;           /**< "read" */
    static const CORE::CString GroupConvert;        /**< "convert" */
    static const CORE::CString GroupAuthor;         /**< "author" */
    static const CORE::CString GroupStore;          /**< "store" */

    bool mcpEnabled;
    bool convertEnabled;
    bool authorEnabled;
    bool storeEnabled;
    CORE::CString::StringVector allowedWriteTools;
    CORE::CString::StringVector deniedTools;
    bool exposePurchaseDetails;
    UInt32 maxWritesPerHour;
    UInt32 maxJobsPerDay;
    UInt32 maxRows;
    UInt32 maxUntrustedTextChars;

    /** Where the state came from: "defaults", "file"; and why the defaults are in force when a file did not read */
    CORE::CString origin;
    CORE::CString problem;

    /** The closed defaults */
    CAssetToolsExposure( void );

    /** Whether the group is on: "read" always, the others as the file says */
    bool IsGroupEnabled( const CORE::CString& group ) const;

    bool IsDenied( const CORE::CString& toolName ) const;

    bool IsAllowedWriteTool( const CORE::CString& toolName ) const;

    /** Reads the exposure object strictly; nothing changes when it is refused */
    bool LoadStrict( const CORE::CDataNode& node, CORE::CString& problemText );

    /** Reads the file (the file system, not the VFS); 'missing' tells a file that is not there from one that is wrong */
    bool LoadFromFile( const CORE::CString& path, CORE::CString& problemText, bool& missing );

    /** The state as an object, for the status resource (no secrets in it) */
    void ToDataNode( CORE::CDataNode& node ) const;

    bool operator==( const CAssetToolsExposure& other ) const;

    bool operator!=( const CAssetToolsExposure& other ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTOOLSEXPOSURE_H ? */
