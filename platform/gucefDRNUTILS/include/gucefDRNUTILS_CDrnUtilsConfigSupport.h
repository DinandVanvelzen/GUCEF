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

#ifndef GUCEF_DRNUTILS_CDRNUTILSCONFIGSUPPORT_H
#define GUCEF_DRNUTILS_CDRNUTILSCONFIGSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_DRNUTILS_DRNUTILSTYPES_H
#include "gucefDRNUTILS_DrnUtilsTypes.h"
#define GUCEF_DRNUTILS_DRNUTILSTYPES_H
#endif /* GUCEF_DRNUTILS_DRNUTILSTYPES_H ? */

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
 *  Module-private: the configuration forms the host settings share: string sets as arrays, the exposure as text
 *  ("NONE", "VIEW", "CONTROL"), the role map as { "roles": { "<generic role>": [ ... ] }, "actionRoles": { "<action>": [ ... ] } }
 */
class CDrnUtilsConfigSupport
{
    public:

    static void SaveStringSet( CORE::CDataNode& config, const CORE::CString& name, const CORE::CString::StringSet& values );
    /** False when the setting is absent (values unchanged) */
    static bool LoadStringSet( const CORE::CDataNode& config, const CORE::CString& name, CORE::CString::StringSet& values );

    static const char* ExposureToString( TDrnUtilsExposure exposure );
    static TDrnUtilsExposure ExposureFromString( const CORE::CString& text, TDrnUtilsExposure defaultIfNeeded );

    static void SaveRoleMap( CORE::CDataNode& config, const CDrnUtilsRoleMap& roleMap );
    static void LoadRoleMap( const CORE::CDataNode& config, CDrnUtilsRoleMap& roleMap );

    /** The action of a name of JobControlActionToString(); JOBCONTROLACTION_UNSPECIFIED when unknown */
    static CORE::TJobControlAction JobControlActionFromString( const CORE::CString& name );

    private:
    CDrnUtilsConfigSupport( void );     /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDRNUTILSCONFIGSUPPORT_H ? */
