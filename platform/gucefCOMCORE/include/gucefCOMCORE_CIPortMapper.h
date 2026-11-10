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

#ifndef GUCEF_COMCORE_CIPORTMAPPER_H
#define GUCEF_COMCORE_CIPORTMAPPER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_COMCORE_CPORTMAPPINGRESULT_H
#include "gucefCOMCORE_CPortMappingResult.h"
#define GUCEF_COMCORE_CPORTMAPPINGRESULT_H
#endif /* GUCEF_COMCORE_CPORTMAPPINGRESULT_H ? */

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
 *  Asks the gateway of the network to forward a port from the outside to this host
 *
 *  The requests are asynchronous: each hands back a future that completes with a CPortMappingResult as its result
 *  data, see CPortMappingResult::TryGetFromFuture(). A mapper keeps the mappings it made alive until they are
 *  removed or the mapper is destroyed.
 */
class GUCEF_COMCORE_EXPORT_CPP CIPortMapper : public CORE::CObservingNotifier ,
                                              public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent MappingCreatedEvent;     /**< CPortMappingResult: a mapping was made */
    static const CORE::CEvent MappingLostEvent;        /**< CPortMappingResult: a mapping could not be kept alive or was removed */
    static const CORE::CEvent PortMapperErrorEvent;    /**< TPortMapperErrorEventData: a description of the problem */

    typedef CORE::TCloneableString                      TPortMapperErrorEventData;
    typedef CPortMappingResult::TProtocol               TProtocol;

    static void RegisterEvents( void );

    CIPortMapper( void );

    virtual ~CIPortMapper();

    /**
     *  Asks for the given port of this host to be reachable from the outside
     *
     *  @param externalPort the wanted external port in host byte order, 0 for any. The gateway may assign another.
     *  @param leaseInSecs how long the mapping is wanted at a time. The mapper renews it while it lives.
     *  @param future receives the future of the request
     *  @return false if the request could not be made at all
     */
    virtual bool RequestMapping( TProtocol protocol            ,
                                 UInt16 internalPort           ,
                                 UInt16 externalPort           ,
                                 UInt32 leaseInSecs            ,
                                 CORE::CFutureResult& future   ) = 0;

    /**
     *  Removes the mapping made for the given internal port
     */
    virtual bool RemoveMapping( TProtocol protocol            ,
                                UInt16 internalPort           ,
                                CORE::CFutureResult& future   ) = 0;

    /**
     *  Asks the gateway for its public address. The result has the external address and no ports.
     */
    virtual bool GetExternalAddress( CORE::CFutureResult& future ) = 0;

    /** False when the method cannot work here, for example when there is no gateway to talk to */
    virtual bool IsAvailable( void ) const = 0;

    /** The name the mapper is registered with in the factory, for example "natpmp" */
    virtual const CORE::CString& GetMapperName( void ) const = 0;

    private:

    CIPortMapper( const CIPortMapper& src );
    CIPortMapper& operator=( const CIPortMapper& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTAbstractFactory< CORE::CString, CIPortMapper, MT::CMutex > CPortMapperFactory;
typedef CPortMapperFactory::TProductPtr                                     CIPortMapperPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CIPORTMAPPER_H ? */
