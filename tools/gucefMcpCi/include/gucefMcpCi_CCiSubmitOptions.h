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

#ifndef GUCEF_MCPCI_CCISUBMITOPTIONS_H
#define GUCEF_MCPCI_CCISUBMITOPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

/** What a run builds: empty lists mean everything, the full matrix */
class CCiSubmitOptions
{
    public:

    GUCEF::CORE::CString::StringVector toolchains;
    GUCEF::CORE::CString::StringVector cppStandards;
    GUCEF::CORE::CString::StringVector products;
    GUCEF::CORE::CString mode;          /**< build or test */
    bool includeUncommitted;            /**< modified, deleted and untracked files too, not only the commits */
    bool dryRun;                        /**< only the summary of the snapshot, nothing is sent */
    GUCEF::CORE::CString::StringVector images;  /**< the images the run also builds: names, or "all"; empty for none */
    GUCEF::CORE::CString commit;        /**< a published commit to build instead of a snapshot of a working tree */

    CCiSubmitOptions( void );
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCISUBMITOPTIONS_H ? */
