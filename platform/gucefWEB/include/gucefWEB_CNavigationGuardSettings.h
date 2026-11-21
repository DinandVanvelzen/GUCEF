/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#ifndef GUCEF_WEB_CNAVIGATIONGUARDSETTINGS_H
#define GUCEF_WEB_CNAVIGATIONGUARDSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings of a CNavigationGuard. Every member is a configuration key of the same name (attributes or child values
 *  of one data node, lists as arrays or comma separated text). A list that is configured replaces the default list.
 *  Rules are CNavigationRule texts: "host" or "host/path".
 */
class GUCEF_WEB_PUBLIC_CPP CNavigationGuardSettings : public CORE::CIConfigurable
{
    public:

    CORE::CString::StringVector allow;                      /**< allow rules; empty: every http(s) address that no deny rule names */
    CORE::CString::StringVector deny;                       /**< deny rules, they win over allow rules */
    bool guardSubframes;                                    /**< also guard the documents of frames, default true */
    bool refuseNeverRecordUnlessAllowed;                    /**< refuse a never-record address (sign-in, payment) unless an allow rule names it, default true */
    CORE::CString::StringVector neverRecordHosts;           /**< host glob patterns, default those of CSiteDiscoverySettings */
    CORE::CString::StringVector neverRecordPathPatterns;    /**< parts of the lower cased path, default those of CSiteDiscoverySettings */

    CNavigationGuardSettings( void );

    CNavigationGuardSettings( const CNavigationGuardSettings& src );

    virtual ~CNavigationGuardSettings() GUCEF_VIRTUAL_OVERRIDE;

    CNavigationGuardSettings& operator=( const CNavigationGuardSettings& src );

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    /** False, leaving the settings as they were, when a rule does not parse */
    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Do all the rules parse? Names the first that does not */
    bool AreRulesValid( CORE::CString* invalidRule = GUCEF_NULL ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CNAVIGATIONGUARDSETTINGS_H ? */
