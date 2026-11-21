/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef ASSETSTORECONVERTERHOOK_CCOOKPHASELOCK_H
#define ASSETSTORECONVERTERHOOK_CCOOKPHASELOCK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

#ifndef ASSETSTORECONVERTERHOOK_MACROS_H
#include "AssetStoreConverterHook_macros.h"
#define ASSETSTORECONVERTERHOOK_MACROS_H
#endif /* ASSETSTORECONVERTERHOOK_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Lets one run of the converter pipeline at a time in the process cook: the cookers use Ogre on the thread of the run, and
 *  Ogre is not thread safe there, while the runs of different packs (the reprocessing jobs of the asset store service, an
 *  ingest) convert in parallel. The pipeline announces its phases to its observer on the thread of the run; the observers of
 *  the hook give them to their CCookPhaseLock: the cook phase takes the process wide lock, the next phase (or the end of the
 *  run, when the lock object goes) gives it back. The other phases are not held up.
 */
class ASSETSTORECONVERTERHOOK_PRIVATE_CPP CCookPhaseLock
{
    public:

    CCookPhaseLock( void );

    /** Gives the lock back when the run ended in the cook phase */
    ~CCookPhaseLock();

    /** Called with every phase the pipeline announces, on the thread of the run */
    void OnPhase( ASSETCONVERTER::TPipelinePhase phase );

    /** Whether this run holds the lock */
    bool IsHeld( void ) const;

    private:

    void Release( void );

    CCookPhaseLock( const CCookPhaseLock& src );              /**< not implemented */
    CCookPhaseLock& operator=( const CCookPhaseLock& src );   /**< not implemented */

    bool m_held;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CCOOKPHASELOCK_H ? */
