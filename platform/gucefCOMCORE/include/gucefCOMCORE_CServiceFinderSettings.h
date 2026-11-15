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

#ifndef GUCEF_COMCORE_CSERVICEFINDERSETTINGS_H
#define GUCEF_COMCORE_CSERVICEFINDERSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COMCORE_CSERVICECANDIDATEFILTER_H
#include "gucefCOMCORE_CServiceCandidateFilter.h"
#define GUCEF_COMCORE_CSERVICECANDIDATEFILTER_H
#endif /* GUCEF_COMCORE_CSERVICECANDIDATEFILTER_H ? */

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
 *  Settings of a CServiceFinder.
 *
 *  Config: serviceType, findTimeoutMs, findMinCandidates, threadPoolName, a "Filter" child (CServiceCandidateFilter) and
 *  the discovery clients as for CDiscoveryClientList (attribute "discoveryClients" or "DiscoveryClient" children);
 *  without either the defaultClientTypes are used.
 */
class GUCEF_COMCORE_EXPORT_CPP CServiceFinderSettings : public CORE::CIConfigurable
{
    public:

    CORE::CString serviceType;                      /**< the type to look for, for example "_gucef-assetstore._tcp" */
    CServiceCandidateFilter filter;                 /**< which candidates are kept */
    UInt32 findTimeoutMs;                           /**< how long FindOnce() looks; default 3000 */
    UInt32 findMinCandidates;                       /**< FindOnce() ends early once it has this many candidates (0: looks the whole time); default 1 */
    CORE::CString threadPoolName;                   /**< the pool of FindOnceAsync() without a pool given; default empty (the default pool) */
    CORE::CDataNode clientConfig;                   /**< the "discoveryClients" type list and the "DiscoveryClient" children */
    CORE::CString::StringVector defaultClientTypes; /**< when the config names no clients; default broadcast, multicast */

    CServiceFinderSettings( void );
    CServiceFinderSettings( const CServiceFinderSettings& src );
    virtual ~CServiceFinderSettings();
    CServiceFinderSettings& operator=( const CServiceFinderSettings& src );

    /** Sets the client types as the comma separated list of the config, replacing the "DiscoveryClient" children */
    void SetClientTypes( const CORE::CString& typeList );

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

#endif /* GUCEF_COMCORE_CSERVICEFINDERSETTINGS_H ? */
