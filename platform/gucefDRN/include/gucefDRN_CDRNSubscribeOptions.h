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

#ifndef GUCEF_DRN_CDRNSUBSCRIBEOPTIONS_H
#define GUCEF_DRN_CDRNSUBSCRIBEOPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  Where an application subscription of a stream starts (CDRNSession::Subscribe). Grids ignore the options.
 *  The options apply to the first subscription of an item object; later resubscriptions of the same object continue
 *  from CDRNDataStream::GetNextExpectedSeq().
 */
class GUCEF_DRN_EXPORT_CPP CDRNSubscribeOptions
{
    public:

    UInt64 fromSeq;         /**< the first entry wanted; 0 = the retained history */
    UInt64 expectedEpoch;   /**< the epoch fromSeq belongs to; 0 = any. Another epoch is reported as DRN_STREAMGAP_EPOCH_CHANGED */

    CDRNSubscribeOptions( void );
    CDRNSubscribeOptions( UInt64 resumeFromSeq, UInt64 epoch );

    bool operator==( const CDRNSubscribeOptions& other ) const;
    bool operator!=( const CDRNSubscribeOptions& other ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNSUBSCRIBEOPTIONS_H ? */
