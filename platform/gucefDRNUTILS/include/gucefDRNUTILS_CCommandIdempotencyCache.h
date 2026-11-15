/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CCOMMANDIDEMPOTENCYCACHE_H
#define GUCEF_DRNUTILS_CCOMMANDIDEMPOTENCYCACHE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTCOMMANDIDEMPOTENCYCACHE_H
#include "gucefCORE_CTCommandIdempotencyCache.h"
#define GUCEF_CORE_CTCOMMANDIDEMPOTENCYCACHE_H
#endif /* GUCEF_CORE_CTCOMMANDIDEMPOTENCYCACHE_H ? */

#ifndef GUCEF_CORE_CJOBCONTROLRESULT_H
#include "gucefCORE_CJobControlResult.h"
#define GUCEF_CORE_CJOBCONTROLRESULT_H
#endif /* GUCEF_CORE_CJOBCONTROLRESULT_H ? */

#ifndef GUCEF_DRNUTILS_DRNUTILSTYPES_H
#include "gucefDRNUTILS_DrnUtilsTypes.h"
#define GUCEF_DRNUTILS_DRNUTILSTYPES_H
#endif /* GUCEF_DRNUTILS_DRNUTILSTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Module-private: the outcomes of remote commands by (subject, command id), so that a repeated command returns the
 *  stored outcome instead of being applied twice (design 6.6). In memory only; bounded by a time to live, a maximum
 *  per subject and a maximum in total, oldest out first. Its lock is a leaf. Times are monotonic milliseconds given by
 *  the caller. The bookkeeping is the public command idempotency cache of gucefCORE (CORE::CTCommandIdempotencyCache);
 *  this class keeps the outcome type of the remote commands.
 */
class CCommandIdempotencyCache
{
    public:

    CCommandIdempotencyCache( void );

    /** 0 = no bound for that dimension */
    void SetBounds( UInt64 ttlMs, UInt32 maxPerSubject, UInt32 maxTotal );

    /** True with the stored outcome when the command was applied before and its record did not expire */
    bool Find( const CORE::CString& subject, const CORE::CString& commandId, UInt64 nowMs,
               TDrnUtilsCommandStatus& status, CORE::SJobControlResult& result );

    void Store( const CORE::CString& subject, const CORE::CString& commandId, UInt64 nowMs,
                TDrnUtilsCommandStatus status, const CORE::SJobControlResult& result );

    UInt32 GetCount( void ) const;

    void Clear( void );

    private:

    struct SOutcome
    {
        TDrnUtilsCommandStatus status;
        CORE::SJobControlResult result;

        SOutcome( void );
        SOutcome( TDrnUtilsCommandStatus commandStatus, const CORE::SJobControlResult& commandResult );
    };

    CCommandIdempotencyCache( const CCommandIdempotencyCache& src );                /**< not implemented */
    CCommandIdempotencyCache& operator=( const CCommandIdempotencyCache& src );     /**< not implemented */

    CORE::CTCommandIdempotencyCache< SOutcome > m_cache;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CCOMMANDIDEMPOTENCYCACHE_H ? */
