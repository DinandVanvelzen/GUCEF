/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETSHAREDECISION_H
#define GUCEF_ASSETSTORE_CASSETSHAREDECISION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_DRNUTILS_CIREMOTESHAREDECISION_H
#include "gucefDRNUTILS_CIRemoteShareDecision.h"
#define GUCEF_DRNUTILS_CIREMOTESHAREDECISION_H
#endif /* GUCEF_DRNUTILS_CIREMOTESHAREDECISION_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETFILESERVINGGATE_H
#include "gucefASSETSTORE_CAssetFileServingGate.h"
#define GUCEF_ASSETSTORE_CASSETFILESERVINGGATE_H
#endif /* GUCEF_ASSETSTORE_CASSETFILESERVINGGATE_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the host of the asset store service decides about the bytes of its files when they are read through a share of the DRN resource
 *  facility instead of the blocks of the Files contract: the same CAssetFileServingGate (nothing of its rules is repeated here), asked
 *  for the device of the member as the device registry has it at that call, with the VFS path of the service that the name of the share
 *  stands for. Names and sizes are visible to every role of the share (browsing is not reading, and no entry is hidden: the folders of
 *  packs that a learning role may not read stay listed, only their bytes are refused); the refusal of the gate goes to the member as it is.
 *
 *  The hooks run on the thread of the DRN node, where a query of the catalogue must not happen. The gate asks the service for the
 *  restrictions of the pack that holds a path, and the service keeps that answer per pack folder; this class asks the service whether it
 *  has the answer (CIAssetStoreService::TryGetRestrictionsOfPathKept()) before it lets the gate look, and when it has not it refuses
 *  the read in plain words (the member may ask again), and has the answer made on a thread of the pool (the restrictions of the pack are
 *  warmed there). The providers of the share (CAssetShareProvider) warm the answer of every file they describe, so a device that browses
 *  before it reads finds it kept. Callers for which the gate needs no look at a pack (the owner without a learning role, the machine role)
 *  are decided at once.
 *
 *  Not owned: the gate and the service must outlive this object. Shutdown() waits for the work queued on the pool.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetShareDecision : public DRNUTILS::CIRemoteShareDecision
{
    public:

    CAssetShareDecision( const TAssetStoreServicePtr& service, CAssetFileServingGate& gate, const CORE::CString::StringSet& learningRoles );

    virtual ~CAssetShareDecision() GUCEF_VIRTUAL_OVERRIDE;

    /** The VFS path of the service that the root of a share stands for; the share is not known before */
    void SetShareRoot( const CORE::CString& share, const CORE::CString& vfsRoot );
    void RemoveShareRoot( const CORE::CString& share );
    CORE::CString GetShareRoot( const CORE::CString& share ) const;

    /** The VFS path of the service of a name of a share; empty for a share that is not known */
    CORE::CString MakeServicePath( const CORE::CString& share, const CORE::CString& name ) const;

    /** The caller as the gate sees it, from the roles of the device (the one place that makes it, for the Files contract and the share) */
    static CAssetFileServingGate::SCaller MakeCaller( const CORE::CString& deviceId, const CORE::CString::StringSet& roles, const CORE::CString::StringSet& learningRoles );

    virtual bool MayRead( const DRNUTILS::CDeviceRecord& device, const CORE::CString& share, const CORE::CString& name, CORE::CString& refusal ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The same decision for a caller that can be told to ask again: a gated read of a pack whose restrictions are not kept yet is answered with
     *  'retryLater' (the restrictions are being looked up on the pool, the thread of the node does not wait for them); every other refusal is final
     */
    virtual bool CheckRead( const DRNUTILS::CDeviceRecord& device, const CORE::CString& share, const CORE::CString& name, CORE::CString& refusal, bool& retryLater ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Has the restrictions of the pack of a VFS path of the service kept (asks the catalogue when it has to): for a thread that may wait, the pool
     *  or a provider of the share. Does nothing for a path of a share that is gone
     */
    void Warm( const CORE::CString& servicePath );

    /** Queues the warming of a path on the pool; at most a bounded number of different paths wait; false when it was not queued */
    bool QueueWarm( const CORE::CString& servicePath );

    /** Waits for the work queued on the pool (at most the time), and takes no more */
    void Shutdown( UInt32 maxWaitMs );

    UInt64 GetRefusalCount( void ) const;           /**< reads the gate refused */
    UInt64 GetNotKeptCount( void ) const;           /**< reads refused because the restrictions of the pack were not kept yet */
    UInt64 GetWarmCount( void ) const;              /**< warmings done by the pool */

    private:

    friend class CAssetShareWarmJob;

    void WarmDone( const CORE::CString& servicePath );

    CAssetShareDecision( const CAssetShareDecision& src );                  /**< not implemented */
    CAssetShareDecision& operator=( const CAssetShareDecision& src );       /**< not implemented */

    TAssetStoreServicePtr m_service;
    CAssetFileServingGate& m_gate;
    CORE::CString::StringSet m_learningRoles;
    mutable MT::CMutex m_lock;
    GUCEF::map< CORE::CString, CORE::CString > m_roots;
    CORE::CString::StringSet m_queued;
    bool m_shutdown;
    MT::CTAtomic< UInt64 > m_refusals;
    MT::CTAtomic< UInt64 > m_notKept;
    MT::CTAtomic< UInt64 > m_warms;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSHAREDECISION_H ? */
