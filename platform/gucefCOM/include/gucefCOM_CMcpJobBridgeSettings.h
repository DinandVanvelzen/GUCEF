/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMCPJOBBRIDGESETTINGS_H
#define GUCEF_COM_CMCPJOBBRIDGESETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COM_MCPTYPES_H
#include "gucefCOM_McpTypes.h"
#define GUCEF_COM_MCPTYPES_H
#endif /* GUCEF_COM_MCPTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings of a CMcpJobBridge (section "McpJobBridge"): how long a job tool waits by default and at most, how
 *  often a waiting call looks at its run, how long handles of finished jobs and request keys are kept, how many jobs a
 *  principal may have going, and the scope that lets an operator see every principal's jobs.
 */
class GUCEF_COM_PUBLIC_CPP CMcpJobBridgeSettings
{
    public:

    static const CORE::CString SectionName;     /**< "McpJobBridge" */

    UInt32 defaultWaitMs;               /**< 20000: the wait of a job tool that names none */
    UInt32 maxWaitMs;                   /**< 60000: a longer wait is cut to this */
    UInt32 pollIntervalMs;              /**< 100: how often a waiting call looks at its run */
    UInt64 resultTtlMs;                 /**< 604800000 (168 h): handles of finished jobs stay valid this long */
    UInt64 requestKeyTtlMs;             /**< 86400000 (24 h): request keys are remembered this long */
    UInt32 maxRequestKeysPerPrincipal;  /**< 256 */
    UInt32 maxRequestKeysTotal;         /**< 4096 */
    UInt32 maxActivePerPrincipal;       /**< 8: jobs that are not final; more is busy */
    UInt32 maxJobs;                     /**< 10000: handles kept in all; the oldest finished go first */
    CORE::CString operatorScope;        /**< "jobs:all": a principal with this scope sees and controls every job */

    CMcpJobBridgeSettings( void );

    bool SaveConfig( CORE::CDataNode& config ) const;

    /** Reads the members that are there; values out of range are refused and change nothing */
    bool LoadConfig( const CORE::CDataNode& config );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPJOBBRIDGESETTINGS_H ? */
