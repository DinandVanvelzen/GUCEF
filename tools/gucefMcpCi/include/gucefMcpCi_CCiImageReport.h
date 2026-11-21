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

#ifndef GUCEF_MCPCI_CCIIMAGEREPORT_H
#define GUCEF_MCPCI_CCIIMAGEREPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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
 *  The images a run built: images.json, which the images job of the CI publishes next to the reports of the cells
 *  (package ci-reports, version "<run>-images"), and which the gateway API passes on in the same form:
 *
 *      { "schema": 1, "run": "<id>", "commit": "<sha>", "ref": "<ref>", "scratch": false, "status": "ok|failed",
 *        "images": [ { "name": "...", "registryName": "...", "tag": "sha-<12>", "alsoTags": [ "candidate" ],
 *                      "digest": "sha256:...", "status": "ok|failed", "seconds": 42, "logTail": "..." } ] }
 *
 *  Fields that are missing keep their defaults; fields that are not known are ignored.
 */
class CCiImageReport
{
    public:

    class CImage
    {
        public:

        GUCEF::CORE::CString name;
        GUCEF::CORE::CString registryName;
        GUCEF::CORE::CString tag;
        GUCEF::CORE::CString::StringVector alsoTags;
        GUCEF::CORE::CString digest;            /**< sha256:<64 hexadecimal digits> */
        GUCEF::CORE::CString status;            /**< ok or failed */
        GUCEF::UInt64 seconds;
        GUCEF::CORE::CString logTail;

        CImage( void );

        /** Whether it was built and pushed: status ok and a digest of the right form */
        bool IsOk( void ) const;
    };

    typedef GUCEF::vector< CImage > TImageVector;

    GUCEF::CORE::CString runId;                 /**< the run id of the service ("<CI id>:<run>"), not part of images.json */
    GUCEF::UInt32 schema;
    GUCEF::CORE::CString run;                   /**< the run id of the CI as the job knew it */
    GUCEF::CORE::CString commit;
    GUCEF::CORE::CString ref;
    bool scratch;
    GUCEF::CORE::CString status;
    TImageVector images;

    CCiImageReport( void );

    /** @return false with the reason when the node is not an image report */
    bool LoadFromJson( const GUCEF::CORE::CDataNode& node, GUCEF::CORE::CString& problem );

    void SaveToJson( GUCEF::CORE::CDataNode& node ) const;

    const CImage* FindImage( const GUCEF::CORE::CString& name ) const;

    /** Whether a job of a run is the images job of the CI: "images", or a name that starts with it */
    static bool IsImagesJob( const GUCEF::CORE::CString& jobId );

    /** Whether the text is "sha256:" followed by 64 lower case hexadecimal digits */
    static bool IsDigest( const GUCEF::CORE::CString& text );
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCIIMAGEREPORT_H ? */
