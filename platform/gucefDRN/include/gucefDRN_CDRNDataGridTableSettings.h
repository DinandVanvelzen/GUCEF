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


#ifndef GUCEF_DRN_CDRNDATAGRIDTABLESETTINGS_H
#define GUCEF_DRN_CDRNDATAGRIDTABLESETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DATATABLETYPES_H
#include "gucefCORE_DataTableTypes.h"
#define GUCEF_CORE_DATATABLETYPES_H
#endif /* GUCEF_CORE_DATATABLETYPES_H ? */

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
 *  How a CDRNDataGridTable presents its grid. These are the options of the table only: the grid never sees them,
 *  and a grid that no table is opened on is not affected by any of them.
 */
class GUCEF_DRN_EXPORT_CPP CDRNDataGridTableSettings
{
    public:

    /** Keys copied per hold of the grid lock by a scan */
    static const UInt32 DefaultScanChunkKeyCount = 4096;

    /** Milliseconds an asynchronous write waits for the answer of the publisher */
    static const UInt32 DefaultWriteTimeoutMs = 10000;

    /**
     *  The type of the key and value columns. DATATABLE_TYPE_UNSUPPORTED takes the type declared in the
     *  CDRNDataGridSettings of the grid; a grid that declares none, or of which only one is declared, is shown as
     *  text and is read only. A replica does not learn the declared types of the publisher (they do not travel),
     *  the application that opens the table names them here.
     */
    CORE::TDataTableLogicalType keyType;
    CORE::TDataTableLogicalType valueType;

    bool readOnly;                      /**< the table refuses every change, whatever the grid allows; default false */
    bool includeRemoved;                /**< keys that were removed are rows too, with the column "removed" set; default false */
    UInt32 scanChunkKeyCount;           /**< keys a scan copies while it holds the grid lock; 0 copies the whole grid in one go; default DefaultScanChunkKeyCount */
    UInt32 snapshotCacheMs;             /**< scans of the whole grid may use a copy this many milliseconds old; 0 takes a new copy for every scan; default 0 */
    UInt32 writeTimeoutMs;              /**< the wait of an asynchronous write for the answer of the publisher; 0 waits without end; default DefaultWriteTimeoutMs */

    CDRNDataGridTableSettings( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNDATAGRIDTABLESETTINGS_H ? */
