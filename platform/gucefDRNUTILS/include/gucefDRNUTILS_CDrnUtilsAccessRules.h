/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CDRNUTILSACCESSRULES_H
#define GUCEF_DRNUTILS_CDRNUTILSACCESSRULES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H
#include "gucefCOMCORE_CRpcRoleAccessPolicy.h"
#define GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H
#endif /* GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSROLEMAP_H
#include "gucefDRNUTILS_CDrnUtilsRoleMap.h"
#define GUCEF_DRNUTILS_CDRNUTILSROLEMAP_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSROLEMAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The RPC access rules of the two contracts, for a service that uses CRpcRoleAccessPolicy: per method group the
 *  service roles of the role map (view methods: scheduler.view / tasks.view; Control: scheduler.control /
 *  tasks.control; SubmitJob: scheduler.submit). A service with an access policy of its own adds equivalent rules.
 *  The hosts check roles again per action and target (defense in depth).
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CDrnUtilsAccessRules
{
    public:

    static void AddSchedulerRules( COMCORE::CRpcRoleAccessPolicy& policy, const CDrnUtilsRoleMap& roles );

    static void AddTaskManagerRules( COMCORE::CRpcRoleAccessPolicy& policy, const CDrnUtilsRoleMap& roles );

    /** Both */
    static void AddTo( COMCORE::CRpcRoleAccessPolicy& policy, const CDrnUtilsRoleMap& roles );

    private:

    CDrnUtilsAccessRules( void );       /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDRNUTILSACCESSRULES_H ? */
