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

#ifndef GUCEF_DRN_CDRNSTREAMGAPINFO_H
#define GUCEF_DRN_CDRNSTREAMGAPINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

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
 *  Entries a stream replica will never get: the range and why (CDRNDataStream::GapEvent).
 *  toSeq is inclusive; it is 0 for DRN_STREAMGAP_EPOCH_CHANGED, where the numbering restarted.
 */
class GUCEF_DRN_EXPORT_CPP CDRNStreamGapInfo
{
    public:

    UInt64 fromSeq;
    UInt64 toSeq;
    TDRNStreamGapReason reason;

    CDRNStreamGapInfo( void );
    CDRNStreamGapInfo( UInt64 gapFromSeq, UInt64 gapToSeq, TDRNStreamGapReason gapReason );

    /** The number of entries missed, 0 when not known (epoch change) */
    UInt64 GetEntryCount( void ) const;

    static const char* ReasonToString( TDRNStreamGapReason reason );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTCloneableObj< CDRNStreamGapInfo > CDRNStreamGapEventData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNSTREAMGAPINFO_H ? */
