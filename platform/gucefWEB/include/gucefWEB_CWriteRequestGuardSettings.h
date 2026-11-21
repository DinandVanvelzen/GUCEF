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

#ifndef GUCEF_WEB_CWRITEREQUESTGUARDSETTINGS_H
#define GUCEF_WEB_CWRITEREQUESTGUARDSETTINGS_H

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
 *  The settings of a CWriteRequestGuard. Every member is a configuration key of the same name (attributes or child
 *  values of one data node, lists as arrays or comma separated text). Rules are CWriteRequestRule texts:
 *  "METHOD [http://]host[:port]/path[?query|?*]". GET, HEAD and OPTIONS requests always pass; there is no setting that
 *  switches the guard off.
 */
class GUCEF_WEB_PUBLIC_CPP CWriteRequestGuardSettings : public CORE::CIConfigurable
{
    public:

    CORE::CString::StringVector allow;      /**< allow rules; empty: every request other than GET, HEAD and OPTIONS is refused */

    CWriteRequestGuardSettings( void );

    CWriteRequestGuardSettings( const CWriteRequestGuardSettings& src );

    virtual ~CWriteRequestGuardSettings() GUCEF_VIRTUAL_OVERRIDE;

    CWriteRequestGuardSettings& operator=( const CWriteRequestGuardSettings& src );

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    /** False, leaving the settings as they were, when a rule does not parse */
    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Do all the rules parse? Names the first that does not, and why */
    bool AreRulesValid( CORE::CString* invalidRule = GUCEF_NULL, CORE::CString* problem = GUCEF_NULL ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWRITEREQUESTGUARDSETTINGS_H ? */
