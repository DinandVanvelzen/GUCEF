/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COMCORE_CIDISCOVERYENTRY_H
#define GUCEF_COMCORE_CIDISCOVERYENTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CITYPENAMED_H
#include "CITypeNamed.h"
#define GUCEF_CORE_CITYPENAMED_H
#endif /* GUCEF_CORE_CITYPENAMED_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_COMCORE_CIDISCOVERYCAPABILITY_H
#include "gucefCOMCORE_CIDiscoveryCapability.h"
#define GUCEF_COMCORE_CIDISCOVERYCAPABILITY_H
#endif /* GUCEF_COMCORE_CIDISCOVERYCAPABILITY_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"      /* often used gucefCOMCORE macros */
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
 *  Interface class which serves as the base class for all discovery clients
 */
class GUCEF_COMCORE_EXPORT_CPP CIDiscoveryEntry
{
    public:

    typedef GUCEF::set< CORE::CString > TStringSet;

    CIDiscoveryEntry( void );

    CIDiscoveryEntry( const CIDiscoveryEntry& src );

    virtual ~CIDiscoveryEntry();

    CIDiscoveryEntry& operator=( const CIDiscoveryEntry& src );

    /**
     *  Identifier of the application providing the relevant capabilities
     */
    virtual bool SetApplicationId( const CORE::CString& appId ) = 0;

    virtual bool GetApplicationId( CORE::CString& appId ) const = 0;

    /**
     *  Identifier of the application providing the relevant capabilities
     */
    virtual bool SetApplicationInstanceId( const CORE::CString& appId ) = 0;

    virtual bool GetApplicationInstanceId( CORE::CString& appId ) const = 0;

    /**
     *  Identifier of the host where the relevant capabilities are running
     */
    virtual bool SetHostInstanceId( const CORE::CString& appId ) = 0;

    virtual bool GetHostInstanceId( CORE::CString& appId ) const = 0;

    /**
     *  Identifier of the host where the relevant capabilities are running
     */
    virtual bool SetHost( const COMCORE::CHostAddress& host ) = 0;

    virtual bool GetHost( COMCORE::CHostAddress& host ) const = 0;

    /**
     *  Identifier of the application providing the relevant capabilities
     */
    virtual bool SetCapabilities( const CORE::CString& capability ) = 0;

    virtual bool GetCapabilities( CORE::CString& capability ) const = 0;

    /**
     *  The type of the advertised service, for example _gucef-game._udp
     */
    virtual bool SetServiceType( const CORE::CString& serviceType ) = 0;

    virtual const CORE::CString& GetServiceType( void ) const = 0;

    /**
     *  The human readable name of the advertised instance
     */
    virtual bool SetInstanceName( const CORE::CString& instanceName ) = 0;

    virtual const CORE::CString& GetInstanceName( void ) const = 0;

    /**
     *  The advertised data. Keep it small: discovery over UDP has to fit it in one datagram (under 1 KB)
     */
    virtual bool SetAttributes( const CORE::CValueList& attributes ) = 0;

    virtual const CORE::CValueList& GetAttributes( void ) const = 0;

    /**
     *  Where the service can be reached, each as scheme://host:port
     *  A host of 0.0.0.0 is replaced by the address the advertisement came from when it is received
     */
    virtual bool SetEndpoints( const CORE::CString::StringVector& endpoints ) = 0;

    virtual const CORE::CString::StringVector& GetEndpoints( void ) const = 0;

    /**
     *  How long the entry stays valid without being heard again
     */
    virtual bool SetTtlInSecs( UInt32 ttlInSecs ) = 0;

    virtual UInt32 GetTtlInSecs( void ) const = 0;

    /**
     *  When the entry was last heard, UTC
     */
    virtual bool SetLastSeen( const CORE::CDateTime& lastSeen ) = 0;

    virtual const CORE::CDateTime& GetLastSeen( void ) const = 0;

    /**
     *  The name of the discovery client that found the entry
     */
    virtual bool SetSourceClientName( const CORE::CString& clientName ) = 0;

    virtual const CORE::CString& GetSourceClientName( void ) const = 0;

    /**
     *  The identity of the entry across clients and updates: the service type plus the application instance id,
     *  or plus the instance name when there is no application instance id
     */
    CORE::CString GetEntryKey( void ) const;

    static CORE::CString MakeEntryKey( const CORE::CString& serviceType, const CORE::CString& instanceId );
};

typedef CORE::CTSharedPtr< CIDiscoveryEntry, MT::CMutex > CIDiscoveryEntryPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CIDISCOVERYENTRY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 12-02-2005 :
        - Initial implementation

---------------------------------------------------------------------------*/
