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

#ifndef GUCEF_COMCORE_CSERVICECANDIDATEFILTER_H
#define GUCEF_COMCORE_CSERVICECANDIDATEFILTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CSERVICECANDIDATE_H
#include "gucefCOMCORE_CServiceCandidate.h"
#define GUCEF_COMCORE_CSERVICECANDIDATE_H
#endif /* GUCEF_COMCORE_CSERVICECANDIDATE_H ? */

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
 *  Which candidates a service finder keeps. Every condition that is set must hold; an empty filter keeps every candidate
 *  that has an endpoint.
 *
 *  Config (LoadConfig/SaveConfig): instanceId, protocolMajor, keyFingerprint, requireEndpoint and a "requiredProperties"
 *  child with the properties as attributes.
 */
class GUCEF_COMCORE_EXPORT_CPP CServiceCandidateFilter : public CORE::CIConfigurable
{
    public:

    CORE::CString instanceId;               /**< only this instance; default any */
    CORE::CString protocolMajor;            /**< only candidates whose protocol version has this major part; default any */
    CORE::CString keyFingerprint;           /**< only the service with this key fingerprint, for a client that pinned a key; default any */
    CORE::CValueList requiredProperties;    /**< properties that must be there with these values */
    bool requireEndpoint;                   /**< a candidate without an endpoint is of no use; default true */

    CServiceCandidateFilter( void );
    CServiceCandidateFilter( const CServiceCandidateFilter& src );
    virtual ~CServiceCandidateFilter();
    CServiceCandidateFilter& operator=( const CServiceCandidateFilter& src );

    bool IsMatch( const CServiceCandidate& candidate ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CSERVICECANDIDATEFILTER_H ? */
