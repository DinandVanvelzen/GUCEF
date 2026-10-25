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

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"      /* often used gucefCOMCORE macros */
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

#ifndef GUCEF_COMCORE_CDNSCACHE_H
#include "gucefCOMCORE_CDnsCache.h"
#define GUCEF_COMCORE_CDNSCACHE_H
#endif /* GUCEF_COMCORE_CDNSCACHE_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H
#include "gucefCOMCORE_CDiscoveryClientFactory.h"
#define GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H
#endif /* GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H ? */

#ifndef GUCEF_COMCORE_CIRTTPROBE_H
#include "gucefCOMCORE_CIRttProbe.h"
#define GUCEF_COMCORE_CIRTTPROBE_H
#endif /* GUCEF_COMCORE_CIRTTPROBE_H ? */

#ifndef GUCEF_COMCORE_CIPORTMAPPER_H
#include "gucefCOMCORE_CIPortMapper.h"
#define GUCEF_COMCORE_CIPORTMAPPER_H
#endif /* GUCEF_COMCORE_CIPORTMAPPER_H ? */

#ifndef GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H
#include "gucefCOMCORE_CTlsProviderRegistry.h"
#define GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H
#endif /* GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H ? */

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

class CCom;
class CDnsCache;
class CDiscoveryManager;
class CGeoLocationLookupService;

/*-------------------------------------------------------------------------*/

class GUCEF_COMCORE_EXPORT_CPP CComCoreGlobal
{
    public:

    static CComCoreGlobal* Instance( void );

    CCom& GetCom( void );

    CDnsCache& GetDnsCache( void );
    
    CDiscoveryManager& GetDiscoveryManager( void );

    /** Creates discovery clients by type name, for example "mdns" */
    CDiscoveryClientFactory& GetDiscoveryClientFactory( void );

    /** Creates round trip time probes by method name, for example "udp-echo" */
    CRttProbeFactory& GetRttProbeFactory( void );

    /** Creates port mappers by method name, for example "natpmp" */
    CPortMapperFactory& GetPortMapperFactory( void );

    CGeoLocationLookupService& GetGeoLocationLookupService( void );

    /**
     *  The providers of TLS (the OpenSSL of the system on Linux, others from plugins and later the TLS of the OS).
     *  The system provider is registered but loads nothing until TLS is first used, see CTlsProviderRegistry
     */
    CTlsProviderRegistry& GetTlsProviderRegistry( void );

    private:
    friend class CModule;

    static void Deinstance( void );

    void Initialize( void );

    void Shutdown( void );

    CComCoreGlobal( void );
    CComCoreGlobal( const CComCoreGlobal& src );
    ~CComCoreGlobal();
    CComCoreGlobal& operator=( const CComCoreGlobal& src );

    private:

    CCom* m_com;
    CDnsCachePtr m_dnsCache;
    CDiscoveryManager* m_discoveryManager;
    CDiscoveryClientFactory* m_discoveryClientFactory;
    CRttProbeFactory* m_rttProbeFactory;
    CPortMapperFactory* m_portMapperFactory;
    CGeoLocationLookupService* m_geoLocationLookupService;
    CTlsProviderRegistry* m_tlsProviderRegistry;

    static MT::CMutex g_dataLock;
    static CComCoreGlobal* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 12-02-2005 :
        - Initial implementation

-----------------------------------------------------------------------------*/
