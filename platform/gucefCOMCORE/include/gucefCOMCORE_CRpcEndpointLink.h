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

#ifndef GUCEF_COMCORE_CRPCENDPOINTLINK_H
#define GUCEF_COMCORE_CRPCENDPOINTLINK_H

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
#include "gucefCOMCORE_macros.h"
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

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

class CRpcEndpoint;
class CRpcEndpointLink;
typedef CORE::CTSharedPtr< CRpcEndpointLink, MT::CMutex > CRpcEndpointLinkPtr;

/*-------------------------------------------------------------------------*/

/**
 *  How server calls and client streams reach their endpoint without keeping it alive: a small shared object with the
 *  endpoint's address, cleared when the endpoint is destroyed. A CScope counts as a user for its duration; the link's
 *  own lock is held only to count, never while the endpoint runs, so it adds no lock order to the endpoint's.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcEndpointLink : public CORE::CTSharedObjCreator< CRpcEndpointLink, MT::CMutex >
{
    public:

    /** Holds the link while an endpoint member runs; GetEndpoint() is NULL when the endpoint is gone */
    class GUCEF_COMCORE_EXPORT_CPP CScope
    {
        public:
        explicit CScope( const CRpcEndpointLinkPtr& link );
        ~CScope();
        CRpcEndpoint* GetEndpoint( void ) const;
        private:
        CScope( const CScope& src );
        CScope& operator=( const CScope& src );
        CRpcEndpointLinkPtr m_link;
        CRpcEndpoint* m_endpoint;
    };

    static CRpcEndpointLinkPtr Create( CRpcEndpoint* endpoint );

    virtual ~CRpcEndpointLink() GUCEF_VIRTUAL_OVERRIDE;

    /** Called by the endpoint's destructor: no new scope reaches the endpoint, scopes in progress are waited for */
    void Detach( void );

    private:
    friend class CORE::CTSharedObjCreator< CRpcEndpointLink, MT::CMutex >;

    explicit CRpcEndpointLink( CRpcEndpoint* endpoint );
    CRpcEndpointLink( const CRpcEndpointLink& src );                /**< not implemented */
    CRpcEndpointLink& operator=( const CRpcEndpointLink& src );     /**< not implemented */

    MT::CMutex m_lock;
    CRpcEndpoint* m_endpoint;
    UInt32 m_users;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCENDPOINTLINK_H ? */
