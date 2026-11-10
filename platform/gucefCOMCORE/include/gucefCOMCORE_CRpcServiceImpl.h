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

#ifndef GUCEF_COMCORE_CRPCSERVICEIMPL_H
#define GUCEF_COMCORE_CRPCSERVICEIMPL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COMCORE_CIRPCSERVICEIMPL_H
#include "gucefCOMCORE_CIRpcServiceImpl.h"
#define GUCEF_COMCORE_CIRPCSERVICEIMPL_H
#endif /* GUCEF_COMCORE_CIRPCSERVICEIMPL_H ? */

#ifndef GUCEF_COMCORE_CIRPCMETHODHANDLER_H
#include "gucefCOMCORE_CIRpcMethodHandler.h"
#define GUCEF_COMCORE_CIRPCMETHODHANDLER_H
#endif /* GUCEF_COMCORE_CIRPCMETHODHANDLER_H ? */

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
 *  Dispatches the calls of a service by method name to bound handlers (gucefCOMCORE RPC design 10.6); methods
 *  without a handler are answered UNIMPLEMENTED. Execution modes may be set per method.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcServiceImpl : public CIRpcServiceImpl
{
    public:

    CRpcServiceImpl( void );
    virtual ~CRpcServiceImpl() GUCEF_VIRTUAL_OVERRIDE;

    bool BindMethod( const CORE::CString& methodName, CIRpcMethodHandler* handler, bool takeOwnership = true );
    void UnbindMethod( const CORE::CString& methodName );

    bool GetExecutionMode( const CORE::CString& methodName, TRpcExecutionMode& mode ) const;
    void SetExecutionMode( const CORE::CString& methodName, TRpcExecutionMode mode );

    virtual TRpcHandlerResult OnCall( CRpcServerCall& call ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetMethodExecutionMode( const CRpcMethodDescriptor& method, TRpcExecutionMode& mode ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool HasHandler( const CRpcMethodDescriptor& method ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SHandler
    {
        CIRpcMethodHandler* handler;
        bool owned;
    };
    typedef GUCEF::map< CORE::CString, SHandler > THandlerMap;
    typedef GUCEF::map< CORE::CString, TRpcExecutionMode > TModeMap;

    CRpcServiceImpl( const CRpcServiceImpl& src );                  /**< not implemented */
    CRpcServiceImpl& operator=( const CRpcServiceImpl& src );       /**< not implemented */

    mutable MT::CMutex m_lock;
    THandlerMap m_handlers;
    TModeMap m_modes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCSERVICEIMPL_H ? */
