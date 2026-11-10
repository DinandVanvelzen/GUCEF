/*
 *  comcorepluginUPNP: Generic GUCEF COMCORE plugin
 *  providing port mapping with UPnP IGD through miniupnpc.
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#ifndef COMCOREPLUGIN_UPNP_CUPNPPORTMAPPER_H
#define COMCOREPLUGIN_UPNP_CUPNPPORTMAPPER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_COMCORE_CIPORTMAPPER_H
#include "gucefCOMCORE_CIPortMapper.h"
#define GUCEF_COMCORE_CIPORTMAPPER_H
#endif /* GUCEF_COMCORE_CIPORTMAPPER_H ? */

#ifndef COMCOREPLUGIN_UPNP_MACROS_H
#include "comcorepluginUPNP_macros.h"
#define COMCOREPLUGIN_UPNP_MACROS_H
#endif /* COMCOREPLUGIN_UPNP_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCOREPLUGIN {
namespace UPNP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CUpnpGateway;
typedef CORE::CTSharedPtr< CUpnpGateway, MT::CMutex > CUpnpGatewayPtr;

/*-------------------------------------------------------------------------*/

/**
 *  Port mapping with a UPnP Internet Gateway Device, through miniupnpc.
 *
 *  miniupnpc blocks while it talks to the gateway, so every request runs as a task in a thread pool and its
 *  future completes from there. The events come on the pulse generator of the mapper, once it sees a request
 *  is done. The gateway is found with SSDP on the first request and remembered.
 *
 *  UPnP mappings have no lease here: they last until removed. The mapper removes the mappings it made when it
 *  is destroyed.
 *
 *  Settings (LoadConfig): discoveryTimeoutInMs, multicastInterface, description, threadPool
 */
class COMCOREPLUGIN_UPNP_PLUGIN_PRIVATE_CPP CUpnpPortMapper : public COMCORE::CIPortMapper
{
    public:

    static const CORE::CString MapperName;      /**< "upnp" */

    CUpnpPortMapper( void );

    virtual ~CUpnpPortMapper();

    virtual bool RequestMapping( TProtocol protocol            ,
                                 UInt16 internalPort           ,
                                 UInt16 externalPort           ,
                                 UInt32 leaseInSecs            ,
                                 CORE::CFutureResult& future   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RemoveMapping( TProtocol protocol            ,
                                UInt16 internalPort           ,
                                CORE::CFutureResult& future   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetExternalAddress( CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    /** False once looking for a gateway found none */
    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetMapperName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    enum ERequestKind
    {
        REQUESTKIND_MAP              ,
        REQUESTKIND_UNMAP            ,
        REQUESTKIND_EXTERNAL_ADDRESS
    };

    struct SPendingRequest
    {
        ERequestKind kind;
        TProtocol protocol;
        UInt16 internalPort;
        CORE::CFutureResult future;

        SPendingRequest( void );
    };
    typedef struct SPendingRequest TPendingRequest;
    typedef GUCEF::vector< TPendingRequest > TPendingRequestVector;

    struct SMapping
    {
        TProtocol protocol;
        UInt16 internalPort;
        UInt16 externalPort;
    };
    typedef struct SMapping TMapping;
    typedef GUCEF::vector< TMapping > TMappingVector;

    CUpnpPortMapper( const CUpnpPortMapper& src );
    CUpnpPortMapper& operator=( const CUpnpPortMapper& src );

    bool QueueRequest( ERequestKind kind, TProtocol protocol, UInt16 internalPort, UInt16 externalPort, CORE::CFutureResult& future );
    void CheckPendingRequests( void );

    private:

    CUpnpGatewayPtr m_gateway;
    CORE::CTimer m_timer;
    TPendingRequestVector m_pending;
    TMappingVector m_mappings;
    UInt32 m_discoveryTimeoutInMs;
    CORE::CString m_multicastInterface;
    CORE::CString m_description;
    CORE::CString m_threadPoolName;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< COMCORE::CIPortMapper, CUpnpPortMapper, MT::CMutex > TUpnpPortMapperFactory;

/*-------------------------------------------------------------------------*/

/**
 *  The task that carries out one request of a CUpnpPortMapper in a thread pool
 */
class COMCOREPLUGIN_UPNP_PLUGIN_PRIVATE_CPP CUpnpPortMapperTask : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CUpnpPortMapperTask( void );

    virtual ~CUpnpPortMapperTask() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CUpnpPortMapperTask( const CUpnpPortMapperTask& src );
    CUpnpPortMapperTask& operator=( const CUpnpPortMapperTask& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CORE::CTaskConsumer, CUpnpPortMapperTask, MT::CMutex > TUpnpPortMapperTaskFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UPNP */
}; /* namespace COMCOREPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* COMCOREPLUGIN_UPNP_CUPNPPORTMAPPER_H ? */
