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

#ifndef GUCEF_COMCORE_CSERVICEADVERTISERSETTINGS_H
#define GUCEF_COMCORE_CSERVICEADVERTISERSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

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
 *  Settings of a CServiceAdvertiser. What the code puts in an announcement wins over these; these fill what it leaves
 *  empty, so a service can be configured without code changes.
 *
 *  Config: serviceType, instanceId, displayName, ttlInSecs, and the discovery clients as for CDiscoveryClientList
 *  (attribute "discoveryClients", for example "broadcast,multicast", or "DiscoveryClient" children with their own
 *  settings). Without either the defaultClientTypes are used.
 */
class GUCEF_COMCORE_EXPORT_CPP CServiceAdvertiserSettings : public CORE::CIConfigurable
{
    public:

    CORE::CString serviceType;                      /**< for example "_gucef-assetstore._tcp"; default empty (the announcement gives it) */
    CORE::CString instanceId;                       /**< default empty: the announcement gives it, else a new one is made at Advertise() */
    CORE::CString displayName;                      /**< default empty */
    UInt32 ttlInSecs;                               /**< default 30 */
    CORE::CDataNode clientConfig;                   /**< the "discoveryClients" type list and the "DiscoveryClient" children */
    CORE::CString::StringVector defaultClientTypes; /**< when the config names no clients; default broadcast, multicast */

    CServiceAdvertiserSettings( void );
    CServiceAdvertiserSettings( const CServiceAdvertiserSettings& src );
    virtual ~CServiceAdvertiserSettings();
    CServiceAdvertiserSettings& operator=( const CServiceAdvertiserSettings& src );

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

#endif /* GUCEF_COMCORE_CSERVICEADVERTISERSETTINGS_H ? */
