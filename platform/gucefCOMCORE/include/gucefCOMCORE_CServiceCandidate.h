/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CSERVICECANDIDATE_H
#define GUCEF_COMCORE_CSERVICECANDIDATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_COMCORE_CSERVICEANNOUNCEMENT_H
#include "gucefCOMCORE_CServiceAnnouncement.h"
#define GUCEF_COMCORE_CSERVICEANNOUNCEMENT_H
#endif /* GUCEF_COMCORE_CSERVICEANNOUNCEMENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A service a finder heard of: the announcement plus where it came from. A candidate is something to offer a person or
 *  to try, never something to trust: whoever answers at its endpoints still has to prove who it is.
 */
class GUCEF_COMCORE_EXPORT_CPP CServiceCandidate
{
    public:

    CServiceAnnouncement announcement;
    CHostAddress host;                          /**< the address the announcement came from */
    CORE::CDateTime lastSeen;                   /**< UTC */
    CORE::CString::StringSet sourceClientNames; /**< the discovery clients that know the candidate now */

    CServiceCandidate( void );
    CServiceCandidate( const CServiceCandidate& src );
    ~CServiceCandidate();
    CServiceCandidate& operator=( const CServiceCandidate& src );

    /** Reads the candidate from a discovery entry; false when the entry is not an announcement (see CServiceAnnouncement::FromEntry()) */
    bool FromEntry( const CIDiscoveryEntry& entry );

    /** The identity: the service type plus the instance id */
    CORE::CString GetKey( void ) const;

    /** The first endpoint, empty when there is none */
    CORE::CString GetFirstEndpoint( void ) const;
};

typedef CORE::CTCloneableObj< CServiceCandidate > TServiceCandidateEventData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CSERVICECANDIDATE_H ? */
