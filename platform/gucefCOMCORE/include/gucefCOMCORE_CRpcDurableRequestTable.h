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

#ifndef GUCEF_COMCORE_CRPCDURABLEREQUESTTABLE_H
#define GUCEF_COMCORE_CRPCDURABLEREQUESTTABLE_H

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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COMCORE_CRPCREQUESTID_H
#include "gucefCOMCORE_CRpcRequestId.h"
#define GUCEF_COMCORE_CRPCREQUESTID_H
#endif /* GUCEF_COMCORE_CRPCREQUESTID_H ? */

#ifndef GUCEF_COMCORE_CRPCSTATUS_H
#include "gucefCOMCORE_CRpcStatus.h"
#define GUCEF_COMCORE_CRPCSTATUS_H
#endif /* GUCEF_COMCORE_CRPCSTATUS_H ? */

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

class CRpcDurableRequestTable;
typedef CORE::CTSharedPtr< CRpcDurableRequestTable, MT::CMutex > CRpcDurableRequestTablePtr;

/*-------------------------------------------------------------------------*/

/**
 *  The provider's record of ACCEPTED work with a DURABLE request id (gucefCOMCORE RPC design 6.5.3, 6.7): state,
 *  progress and the outcome, kept for durableResultRetentionMs, so a STATUS_QUERY or CANCEL from a later session is
 *  answered. Ids are bound to the subject that created them. A host shares one table between its endpoints.
 *  In memory only (a table persisted to disk is a later addition). Thread safe.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcDurableRequestTable : public CORE::CTSharedObjCreator< CRpcDurableRequestTable, MT::CMutex >
{
    public:

    struct GUCEF_COMCORE_EXPORT_CPP SEntry
    {
        CRpcRequestId id;
        CORE::CString ownerSubject;
        TRpcRequestState state;
        UInt32 progressPermille;
        UInt32 etaMs;
        CORE::CString note;
        bool hasResult;
        CRpcStatus resultStatus;
        CORE::CDynamicBuffer resultPayload;     /**< the encoded response message */
        bool cancelRequested;
        UInt64 expiresAtMs;                     /**< monotonic; 0 while running */

        SEntry( void );
    };

    static CRpcDurableRequestTablePtr Create( UInt32 retentionMs, UInt32 maxPerSubject );

    virtual ~CRpcDurableRequestTable() GUCEF_VIRTUAL_OVERRIDE;

    /** False when the subject has reached its maximum of live ids (0 = no maximum) */
    bool Add( const CRpcRequestId& id, const CORE::CString& ownerSubject );
    bool Get( const CRpcRequestId& id, SEntry& entry ) const;
    bool SetProgress( const CRpcRequestId& id, TRpcRequestState state, UInt32 progressPermille, UInt32 etaMs, const CORE::CString& note );
    bool Complete( const CRpcRequestId& id, const CRpcStatus& status, const CORE::CDynamicBuffer* payload );
    bool RequestCancel( const CRpcRequestId& id );
    bool IsCancelRequested( const CRpcRequestId& id ) const;

    /** Drops finished entries whose retention passed */
    void Expire( void );

    UInt32 GetCount( void ) const;

    static UInt64 NowMs( void );

    private:
    friend class CORE::CTSharedObjCreator< CRpcDurableRequestTable, MT::CMutex >;

    CRpcDurableRequestTable( UInt32 retentionMs, UInt32 maxPerSubject );
    CRpcDurableRequestTable( const CRpcDurableRequestTable& src );              /**< not implemented */
    CRpcDurableRequestTable& operator=( const CRpcDurableRequestTable& src );   /**< not implemented */

    typedef GUCEF::map< CRpcRequestId, SEntry > TEntryMap;

    mutable MT::CMutex m_lock;
    TEntryMap m_entries;
    UInt32 m_retentionMs;
    UInt32 m_maxPerSubject;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCDURABLEREQUESTTABLE_H ? */
