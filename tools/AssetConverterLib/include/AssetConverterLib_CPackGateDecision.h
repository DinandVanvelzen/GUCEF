/*
 *  AssetConverterLib: the answer of the work gate: whether a stored result may be used and why not
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

#ifndef ASSETCONVERTERLIB_CPACKGATEDECISION_H
#define ASSETCONVERTERLIB_CPACKGATEDECISION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

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

/** One answer of CPackWorkGate: the outcome, the reason in the vocabulary of the conversion cache and the detail that names what differs */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackGateDecision
{
    public:

    enum EOutcome
    {
        OUTCOME_FRESH = 0   ,   /**< the stored result is valid: nothing runs */
        OUTCOME_STALE       ,   /**< the stored result exists and an input changed */
        OUTCOME_MISSING     ,   /**< there is no stored result */
        OUTCOME_FORCED      ,   /**< the person asked for the step to be done again */
        OUTCOME_REFUSED         /**< the step is not allowed (the licence), or the stored document is of a newer build and is not touched */
    };

    EOutcome outcome;
    TActivityReason reason;
    CORE::CString detail;

    CPackGateDecision( void );
    CPackGateDecision( EOutcome decided, TActivityReason why, const CORE::CString& why2 = CORE::CString::Empty );

    /** Whether the step has to run: stale, missing or forced */
    bool NeedsWork( void ) const;
    bool IsFresh( void ) const;

    /** The outcome as the word the documents use */
    CORE::CString GetOutcomeName( void ) const;

    /** "stale: source changed (the file count changed from 12 to 13)" */
    CORE::CString ToString( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKGATEDECISION_H ? */

