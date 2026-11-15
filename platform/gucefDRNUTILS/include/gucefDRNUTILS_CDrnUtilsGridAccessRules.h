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

#ifndef GUCEF_DRNUTILS_CDRNUTILSGRIDACCESSRULES_H
#define GUCEF_DRNUTILS_CDRNUTILSGRIDACCESSRULES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  Subscription decisions for the DRNUTILS grids, for a service's DRN::CIDRNAccessPolicy::MaySubscribeAs(): grid name
 *  patterns (for example "svc/" or "gucef.drnutils/node1/" followed by an asterisk) to generic roles. Writes by members to these grids are always refused.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CDrnUtilsGridAccessRules
{
    public:

    CDrnUtilsGridAccessRules( void );

    void SetRoleMap( const CDrnUtilsRoleMap& roles );

    /** Grids matching the pattern ("*" matches the rest of the name) need one of the service roles of the generic role */
    void AddGridPattern( const CORE::CString& gridNamePattern, const CORE::CString& genericRole );

    /** Whether the grid is one of the DRNUTILS grids known here */
    bool IsGoverned( const CORE::CString& gridName ) const;

    /** Whether a member with the given roles may subscribe to the grid; false for grids that are not governed */
    bool MaySubscribe( const CORE::CString::StringSet& memberRoles, const CORE::CString& gridName ) const;

    /** Always false for governed grids */
    bool MayWrite( const CORE::CString::StringSet& memberRoles, const CORE::CString& gridName ) const;

    private:

    typedef GUCEF::map< CORE::CString, CORE::CString > TPatternMap;

    CDrnUtilsRoleMap m_roles;
    TPatternMap m_patterns;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDRNUTILSGRIDACCESSRULES_H ? */
