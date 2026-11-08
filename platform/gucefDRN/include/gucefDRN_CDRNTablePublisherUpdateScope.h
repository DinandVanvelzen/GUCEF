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

#ifndef GUCEF_DRN_CDRNTABLEPUBLISHERUPDATESCOPE_H
#define GUCEF_DRN_CDRNTABLEPUBLISHERUPDATESCOPE_H

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

class CDRNTablePublisher;

/** One update of a CDRNTablePublisher: BeginUpdate() on construction, EndUpdate() on destruction */
class GUCEF_DRN_EXPORT_CPP CDRNTablePublisherUpdateScope
{
    public:

    explicit CDRNTablePublisherUpdateScope( CDRNTablePublisher& publisher );

    ~CDRNTablePublisherUpdateScope();

    /** Whether the update could begin */
    bool IsOpen( void ) const;

    /** Ends the update before the scope ends, returns what EndUpdate() returned */
    bool EarlyEnd( void );

    private:

    CDRNTablePublisherUpdateScope( const CDRNTablePublisherUpdateScope& src );
    CDRNTablePublisherUpdateScope& operator=( const CDRNTablePublisherUpdateScope& src );

    CDRNTablePublisher* m_publisher;
    bool m_open;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNTABLEPUBLISHERUPDATESCOPE_H ? */
