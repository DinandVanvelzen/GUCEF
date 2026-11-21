/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMOUNTRESULT_H
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMOUNTRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_MACROS_H
#include "vfspluginDRN_macros.h"
#define GUCEF_VFSPLUGIN_DRN_MACROS_H
#endif /* GUCEF_VFSPLUGIN_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The outcome of mounting a share (CDrnBridgeMounter::Mount()): whether it was mounted and, when it was not, why not in
 *  words for a log or a person.
 */
class GUCEF_VFSPLUGIN_DRN_EXPORT_CPP CDrnBridgeMountResult
{
    public:

    bool mounted;                       /**< the share is mounted */
    CORE::CString reason;               /**< why it is not mounted, empty when it is */
    CORE::CString mountPoint;           /**< where it is mounted (the VFS form: a leading and a trailing separator), empty when it is not */
    CORE::CString archiveName;          /**< the name the archive was mounted under, which is what the VFS unmounts by, empty when it is not */

    CDrnBridgeMountResult( void );

    /** A refusal with its reason */
    static CDrnBridgeMountResult Refused( const CORE::CString& why );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMOUNTRESULT_H ? */
