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

#ifndef GUCEF_MCPCI_ICIPROGRESS_H
#define GUCEF_MCPCI_ICIPROGRESS_H

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

/**
 *  Where a long call of the service (a submit that sends a large snapshot) reports how far it is, and asks whether
 *  it should stop. Called on the thread that makes the call of the service.
 */
class ICiProgress
{
    public:

    virtual ~ICiProgress();

    /**
     *  @param done     how much is done, in the unit of the step (bytes while sending)
     *  @param total    how much there is, below 0 when not known
     */
    virtual void OnProgress( GUCEF::Float64 done, GUCEF::Float64 total, const GUCEF::CORE::CString& message ) = 0;

    /** Whether the caller gave up: the call ends as soon as it can, with an error */
    virtual bool IsCancelled( void ) const = 0;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_ICIPROGRESS_H ? */
