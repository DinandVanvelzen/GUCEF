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

#ifndef GUCEF_COMCORE_CDATABASETRANSACTION_H
#define GUCEF_COMCORE_CDATABASETRANSACTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CIDATABASECLIENT_H
#include "gucefCOMCORE_CIDatabaseClient.h"
#define GUCEF_COMCORE_CIDATABASECLIENT_H
#endif /* GUCEF_COMCORE_CIDATABASECLIENT_H ? */

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
 *  The transaction of a database client as a scope: the constructor begins it, Commit() applies the changes and the
 *  destructor rolls back a transaction that was neither committed nor rolled back.
 *
 *      COMCORE::CDatabaseTransaction transaction( client );
 *      ... writes through the client and its tables ...
 *      if ( CORE::DATATABLE_STATUS_OK != transaction.Commit() ) ...
 *
 *  What belongs to the transaction is described with CIDatabaseClient. On a backend without
 *  GUCEF_DATABASE_CAPABILITY_TRANSACTIONS (or when a transaction is open already) the begin fails: IsActive() is false,
 *  GetBeginStatus() tells why (UNSUPPORTED, INVALID_ARGUMENT, ...), the changes are applied as they are made, and
 *  Commit() and Rollback() return the status of the failed begin, so the caller can tell nothing was held back.
 *
 *  The scope is used by one thread; it holds a reference on the client when made from a CIDatabaseClientPtr.
 */
class GUCEF_COMCORE_EXPORT_CPP CDatabaseTransaction
{
    public:

    /** Begins a transaction on the client, which the scope keeps alive */
    explicit CDatabaseTransaction( const CIDatabaseClientPtr& client );

    /** Begins a transaction on the client, which must outlive the scope */
    explicit CDatabaseTransaction( CIDatabaseClient& client );

    /** Rolls back the transaction when it is still active */
    ~CDatabaseTransaction();

    /** Begun and not yet committed or rolled back */
    bool IsActive( void ) const;

    /** The outcome of the begin: OK, or why there is no transaction */
    CORE::TDataTableStatus GetBeginStatus( void ) const;

    /** Commits the transaction; afterwards the scope is no longer active, also when the commit failed */
    CORE::TDataTableStatus Commit( void );

    /** Rolls back the transaction; afterwards the scope is no longer active */
    CORE::TDataTableStatus Rollback( void );

    private:

    CDatabaseTransaction( const CDatabaseTransaction& src );                /**< not implemented */
    CDatabaseTransaction& operator=( const CDatabaseTransaction& src );     /**< not implemented */

    void Begin( void );

    CIDatabaseClientPtr m_clientPtr;
    CIDatabaseClient* m_client;
    CORE::TDataTableStatus m_beginStatus;
    bool m_active;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CDATABASETRANSACTION_H ? */
