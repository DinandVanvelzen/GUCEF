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

#ifndef GUCEF_COMCORE_CRPCSERVICESET_H
#define GUCEF_COMCORE_CRPCSERVICESET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#ifndef GUCEF_COMCORE_CIRPCSERVICEIMPL_H
#include "gucefCOMCORE_CIRpcServiceImpl.h"
#define GUCEF_COMCORE_CIRPCSERVICEIMPL_H
#endif /* GUCEF_COMCORE_CIRPCSERVICEIMPL_H ? */

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

class CRpcServiceSet;
typedef CORE::CTSharedPtr< CRpcServiceSet, MT::CMutex > CRpcServiceSetPtr;

/*-------------------------------------------------------------------------*/

/**
 *  The services one side exports (gucefCOMCORE RPC design 10.1, 10.5): (contract, service) -> implementation and
 *  execution mode. A host shares one set between all its endpoints. Thread safe.
 *  The set does not own the implementations. An implementation may be destroyed once UnregisterService() returned for
 *  every service it was registered for: from then on no handler of it runs and none starts, also not for calls that
 *  arrived before (a call handed to a thread pool or mailed to the notification thread checks that its implementation
 *  is still registered when its handler is about to run, and is answered UNAVAILABLE otherwise).
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcServiceSet : public CORE::CTSharedObjCreator< CRpcServiceSet, MT::CMutex >
{
    public:

    struct GUCEF_COMCORE_EXPORT_CPP SEntry
    {
        CRpcContractPtr contract;
        const CRpcServiceDescriptor* service;
        CIRpcServiceImpl* impl;
        TRpcExecutionMode mode;

        SEntry( void );
    };
    typedef GUCEF::vector< SEntry > TEntryVector;

    static CRpcServiceSetPtr Create( void );

    virtual ~CRpcServiceSet() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  serviceName: full or within the contract. False when the contract has no such service. Another implementation
     *  registered for the service is replaced: as with UnregisterService() its handlers under way finish first
     */
    bool RegisterService( const CRpcContractPtr& contract, const CORE::CString& serviceName,
                          CIRpcServiceImpl* impl, TRpcExecutionMode mode = RPC_EXECUTION_CALLING_THREAD );

    /**
     *  Waits for the handlers of the service that run on other threads: when it returns no handler of the
     *  implementation runs for the service and none starts (a handler that unregisters its own service is not waited for)
     */
    bool UnregisterService( const CRpcContractPtr& contract, const CORE::CString& serviceName );

    /** The implementation of a service of a contract (by hash), false when not exported */
    bool Find( const CRpcContractHash& hash, UInt32 serviceNumber, SEntry& entry ) const;

    /** The contracts with at least one exported service */
    void GetExportedContracts( TRpcContractPtrVector& contracts ) const;

    /** An exported contract by hash, or by name and major (highest minor) */
    CRpcContractPtr FindExported( const CRpcContractHash& hash ) const;
    CRpcContractPtr FindExported( const CORE::CString& name, UInt32 major ) const;
    bool ExportsName( const CORE::CString& name ) const;

    UInt32 GetServiceCount( void ) const;

    private:
    friend class CORE::CTSharedObjCreator< CRpcServiceSet, MT::CMutex >;
    friend class CRpcServerCall;

    /** A handler that runs: of which implementation, for which service, on which thread */
    struct SRunningHandler
    {
        CIRpcServiceImpl* impl;
        const CRpcServiceDescriptor* service;   /**< the descriptor of the entry it runs for */
        TThreadId threadId;
    };
    typedef GUCEF::vector< SRunningHandler > TRunningHandlerVector;

    CRpcServiceSet( void );
    CRpcServiceSet( const CRpcServiceSet& src );                /**< not implemented */
    CRpcServiceSet& operator=( const CRpcServiceSet& src );     /**< not implemented */

    /**
     *  Before a handler runs (CRpcServerCall::RunHandler()): true, and the handler counts as running, when the
     *  implementation is still the one registered for the service of the method; false when the service was
     *  unregistered or registered with another implementation since the call arrived. Every true is followed by EndHandler()
     */
    bool BeginHandler( const CRpcContractHash& hash, const CRpcMethodDescriptor& method, CIRpcServiceImpl* impl );
    void EndHandler( CIRpcServiceImpl* impl );

    /** Waits until no handler of the implementation for the service runs on another thread; without m_lock held */
    void WaitForHandlersOf( CIRpcServiceImpl* impl, const CRpcServiceDescriptor* service );

    mutable MT::CMutex m_lock;
    TEntryVector m_entries;
    TRunningHandlerVector m_running;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCSERVICESET_H ? */
