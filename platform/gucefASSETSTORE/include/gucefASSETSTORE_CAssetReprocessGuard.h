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

#ifndef GUCEF_ASSETSTORE_CASSETREPROCESSGUARD_H
#define GUCEF_ASSETSTORE_CASSETREPROCESSGUARD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETREPROCESSSETTINGS_H
#include "gucefASSETSTORE_CAssetReprocessSettings.h"
#define GUCEF_ASSETSTORE_CASSETREPROCESSSETTINGS_H
#endif /* GUCEF_ASSETSTORE_CASSETREPROCESSSETTINGS_H ? */
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
 *  The admission of reprocessing jobs (CAssetReprocessRequest) by the limits of CAssetReprocessSettings: Admit() before a job is
 *  queued (busy when too many wait, the hourly limits of machine callers per pack and together), AcquireSlot() and ReleaseSlot()
 *  around the run (the jobs at a time), AdmitRedownload() before a download again by a machine (the daily limit). The ledger of
 *  the starts is kept in memory. Thread safe.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetReprocessGuard
{
    public:

    CAssetReprocessGuard( void );
    ~CAssetReprocessGuard();

    void Configure( const CAssetReprocessSettings& settings );
    CAssetReprocessSettings GetSettings( void ) const;

    /**
     *  Whether a job of the pack may be queued now; on true the start is counted. 'queued' is the number of reprocessing jobs that
     *  wait or run. On false 'status' is ASSETSTORE_STATUS_RATE_LIMITED (an hourly limit) or ASSETSTORE_STATUS_ALREADY_EXISTS (busy)
     *  and 'reason' says which limit and when the next start is allowed.
     */
    bool Admit( const CORE::CString& packId, bool machine, bool owner, UInt32 queued, Int64 nowMs, TAssetStoreStatus& status, CORE::CString& reason );

    /** Whether a machine may download an original again now; on true it is counted */
    bool AdmitRedownload( bool machine, Int64 nowMs, CORE::CString& reason );

    /** What Admit() would say, without counting: for the plan */
    bool WouldAdmit( const CORE::CString& packId, bool machine, bool owner, UInt32 queued, Int64 nowMs, CORE::CString& reason ) const;

    /** Takes a slot of the jobs that run at a time; false when all are taken (the job waits and asks again) */
    bool AcquireSlot( void );
    void ReleaseSlot( void );
    UInt32 GetRunning( void ) const;

    private:

    struct SStart
    {
        CORE::CString packId;
        Int64 atMs;
        bool machine;
        bool redownload;
    };
    typedef GUCEF::vector< SStart > TStartVector;

    bool Check( const CORE::CString& packId, bool machine, bool owner, UInt32 queued, Int64 nowMs, TAssetStoreStatus& status, CORE::CString& reason ) const;
    void Trim( Int64 nowMs );
    static CORE::CString DescribeWait( Int64 waitMs );

    mutable MT::CMutex m_lock;
    CAssetReprocessSettings m_settings;
    TStartVector m_starts;
    UInt32 m_running;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETREPROCESSGUARD_H ? */
