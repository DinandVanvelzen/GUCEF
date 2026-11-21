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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "AssetConverterLib_CPackIndexSerial.h"
#include "AssetConverterLib_CPackIndexRunResult.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPackIndexRunResult::CPackIndexRunResult( void )
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CPackIndexRunResult::Clear( void )
{GUCEF_TRACE;

    indexDecision = CPackGateDecision();
    structureBuilt = false;
    written = false;
    reused = false;
    locked = false;
    cancelled = false;
    refused = false;
    indexFolder.Clear();
    holder.Clear();
    message.Clear();
    resourcesGrouped = 0;
    recordsKept = 0;
    resourcesAnalysed = 0;
    analysisReused = 0;
    analysisFailed = 0;
    outputsRefreshed = 0;
    shardsWritten = 0;
    shardsReused = 0;
    structureMs = 0;
    totalMs = 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackIndexRunResult::ToSummaryText( void ) const
{GUCEF_TRACE;

    CORE::CString text = "pack index: " + indexDecision.ToString();
    if ( reused )
        text += "; the stored index stands, nothing was done";
    else
    {
        if ( structureBuilt )
            text += "; structure built (" + CORE::UInt32ToString( resourcesGrouped ) + " resources grouped, " + CORE::UInt32ToString( recordsKept ) + " records kept, " + CORE::UInt32ToString( structureMs ) + " ms)";
        if ( resourcesAnalysed > 0 || analysisReused > 0 || analysisFailed > 0 )
            text += "; analysis: " + CORE::UInt32ToString( resourcesAnalysed ) + " done, " + CORE::UInt32ToString( analysisReused ) + " reused, " + CORE::UInt32ToString( analysisFailed ) + " failed";
        if ( outputsRefreshed > 0 )
            text += "; outputs of " + CORE::UInt32ToString( outputsRefreshed ) + " resources";
        text += written ? "; written" : "; not written";
    }
    if ( locked )
        text += "; locked by " + holder;
    if ( !message.IsNULLOrEmpty() )
        text += "; " + message;
    return text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
