/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNNAMESPACESHARESETTINGS_H
#define GUCEF_DRN_CDRNNAMESPACESHARESETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a share of CDRNFileSystemAccessResourceProvider is besides its name, its root and the share rules it is registered
 *  with (who may see what): how it treats what the file system holds, and its own limits.
 */
class GUCEF_DRN_EXPORT_CPP CDRNNamespaceShareSettings
{
    public:

    bool readOnly;                  /**< the share cannot be changed through DRN; always true in this version, the share is advertised read-only whatever this says */
    bool followSymlinks;            /**< entries that are symbolic links are listed (and followed), default false: they are left out of listings */
    bool showHiddenEntries;         /**< entries the file system marks hidden are listed and can be statted, default true */
    bool weakRevisions;             /**< the file system keeps no modification times: advertised as DRN_NS_SHAREFLAG_WEAK_REVISION, default false */
    UInt32 maxStatPaths;            /**< paths in one STAT of this share, 0 = the limit of the serving side */
    UInt32 maxPageEntries;          /**< entries in one page of a listing of this share, 0 = the limit of the serving side */

    CDRNNamespaceShareSettings( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNNAMESPACESHARESETTINGS_H ? */
