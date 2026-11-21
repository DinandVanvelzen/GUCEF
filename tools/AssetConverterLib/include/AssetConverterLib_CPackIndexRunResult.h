/*
 *  AssetConverterLib: what a pack index update did and why: the counters the tests and the tools read
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXRUNRESULT_H
#define ASSETCONVERTERLIB_CPACKINDEXRUNRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CPACKGATEDECISION_H
#include "AssetConverterLib_CPackGateDecision.h"
#define ASSETCONVERTERLIB_CPACKGATEDECISION_H
#endif /* ASSETCONVERTERLIB_CPACKGATEDECISION_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The outcome of CPackIndexUpdater::EnsureIndex(): the gate's answer and the work that was done, as counters (proof of "done once", not a wall clock) */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexRunResult
{
    public:

    CPackGateDecision indexDecision;
    bool structureBuilt;            /**< the structure level was built (the gate said work) */
    bool written;                   /**< the document was written to the disk */
    bool reused;                    /**< the stored document stood: nothing was built, analysed or written */
    bool locked;                    /**< another writer holds the lock of the index: nothing was written, the stored document (when any) is returned */
    bool cancelled;
    bool refused;                   /**< the stored document is of a newer version or not an index: it was not touched */
    CORE::CString indexFolder;      /**< the folder the index of the pack is in (the user cache folder of the pack key when none was given) */
    CORE::CString holder;           /**< who holds the lock */
    CORE::CString message;
    UInt32 resourcesGrouped;        /**< records the group rules were applied to */
    UInt32 recordsKept;             /**< records taken over from the previous document */
    UInt32 resourcesAnalysed;
    UInt32 analysisReused;
    UInt32 analysisFailed;
    UInt32 outputsRefreshed;        /**< records that got outputs from a cache entry */
    UInt32 shardsWritten;
    UInt32 shardsReused;
    UInt32 structureMs;
    UInt32 totalMs;

    CPackIndexRunResult( void );

    void Clear( void );
    CORE::CString ToSummaryText( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKINDEXRUNRESULT_H ? */

