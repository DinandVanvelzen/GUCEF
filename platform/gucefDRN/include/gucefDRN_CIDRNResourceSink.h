/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CIDRNRESOURCESINK_H
#define GUCEF_DRN_CIDRNRESOURCESINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEPARTIALSTATE_H
#include "gucefDRN_CDRNResourcePartialState.h"
#define GUCEF_DRN_CDRNRESOURCEPARTIALSTATE_H
#endif /* GUCEF_DRN_CDRNRESOURCEPARTIALSTATE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Where the bytes of a resource transfer go: a file that can be resumed, memory, a CORE::CIOAccess stream, or a
 *  target of the application (a block cache).
 *
 *  The transfer calls the sink from threads of the pool, one call at a time, never from the node's thread:
 *  PeekPartialState() first (from the thread that starts the transfer), then Begin(), any number of Write() and
 *  Commit() calls, then Finish() or Abort(). Offsets are those of the resource; only bytes inside the window that was
 *  asked for are written. A sink that IsInOrder() gets its writes in offset order without gaps, others in any order.
 */
class GUCEF_DRN_EXPORT_CPP CIDRNResourceSink
{
    public:

    CIDRNResourceSink( void );

    virtual ~CIDRNResourceSink();

    /** A state the sink kept from an earlier transfer of a resource into it (to ask the server for that revision) */
    virtual bool PeekPartialState( CDRNResourcePartialState& state );

    /**
     *  The resource as the server resolved it and the window of it to be written. With 'resume' a resumable sink keeps
     *  what it holds of this very revision and gives its state (chunks held); otherwise, and for another revision, it
     *  starts empty and 'state' is reset for this resource.
     */
    virtual TDRNResourceStatus Begin( const CORE::CString& provider       ,
                                      const CDRNResourceInfo& info        ,
                                      UInt64 windowOffset                 ,
                                      UInt64 windowLength                 ,
                                      bool resume                         ,
                                      CDRNResourcePartialState& state     ) = 0;

    virtual TDRNResourceStatus Write( UInt64 offset, const void* data, UInt32 size ) = 0;

    /** Makes what was written durable and records the state (the chunks held), after the data. Default: nothing to do */
    virtual TDRNResourceStatus Commit( const CDRNResourcePartialState& state );

    /** Reads back bytes that were written (final checks of chunks kept from an earlier transfer). Default: UNSUPPORTED */
    virtual TDRNResourceStatus ReadBack( UInt64 offset, UInt32 size, void* destination );

    /** Everything was written and checked */
    virtual TDRNResourceStatus Finish( void ) = 0;

    /** The transfer ended without completing: keep what is held for a later resume, unless 'discard' */
    virtual void Abort( const CDRNResourcePartialState& state, bool discard ) = 0;

    /** Whether writes must come in offset order without gaps (a stream). Default false */
    virtual bool IsInOrder( void ) const;

    /** Whether the sink keeps partial data for a later transfer. Default false */
    virtual bool IsResumable( void ) const;

    private:

    CIDRNResourceSink( const CIDRNResourceSink& src );              /**< not implemented */
    CIDRNResourceSink& operator=( const CIDRNResourceSink& src );   /**< not implemented */
};

typedef CORE::CTSharedPtr< CIDRNResourceSink, MT::CMutex > CIDRNResourceSinkPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CIDRNRESOURCESINK_H ? */
