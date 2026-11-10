/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H
#define GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAsyncUriRetrievalReporter;

/*-------------------------------------------------------------------------*/

/**
 *  Data of the events of CAsyncUriResourceRetriever. Which fields are filled depends on the event
 */
class GUCEF_CORE_PUBLIC_CPP CAsyncUriRetrievalEventData : public CICloneable
{
    public:

    /** identifies the retrieval, same as the id returned by CAsyncUriResourceRetriever::GetCurrentRetrievalId() */
    UInt32 retrievalId;

    CUri uri;

    /** bytes delivered to the destination so far. Progress, Completed, Failed and Cancelled events */
    UInt64 bytesTransferred;

    /** total size of the resource in bytes if known, -1 if not. Progress and Completed events */
    Int64 totalBytes;

    /** why the retrieval failed. Failed event */
    CString errorText;

    CAsyncUriRetrievalEventData( void );

    CAsyncUriRetrievalEventData( const CAsyncUriRetrievalEventData& src );

    virtual ~CAsyncUriRetrievalEventData() GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  Retrieves the data of a resource identified by a URI (file, data, vfs, http, https, etc, whatever there is a
 *  CUriResourceAccessor for) without blocking the calling thread. The retrieval is carried out as a task on a thread
 *  pool using the CUriResourceAccessor for the scheme of the URI, and progress is reported via events on the thread of
 *  the owner of this object, like any CTSGNotifier.
 *
 *  Only one retrieval at a time per object. The destination given to Retrieve() must stay valid until the
 *  retrieval has ended, cancelling and destroying the retriever wait for that.
 */
class GUCEF_CORE_PUBLIC_CPP CAsyncUriResourceRetriever : public CTSGNotifier
{
    public:

    /** Retrieval has started on its thread. Event data: CAsyncUriRetrievalEventData */
    static const CEvent RetrievalStartedEvent;

    /** Data was delivered to the destination. Event data: CAsyncUriRetrievalEventData */
    static const CEvent RetrievalProgressEvent;

    /** All data was delivered to the destination. Event data: CAsyncUriRetrievalEventData */
    static const CEvent RetrievalCompletedEvent;

    /** The retrieval failed. Event data: CAsyncUriRetrievalEventData */
    static const CEvent RetrievalFailedEvent;

    /** The retrieval was cancelled before it completed. Event data: CAsyncUriRetrievalEventData */
    static const CEvent RetrievalCancelledEvent;

    static void RegisterEvents( void );

    typedef CAsyncUriRetrievalEventData TRetrievalEventData;

    public:

    CAsyncUriResourceRetriever( void );

    /** @param pulseGenerator   pulse generator of the thread on which the events are to be received */
    CAsyncUriResourceRetriever( PulseGeneratorPtr pulseGenerator );

    virtual ~CAsyncUriResourceRetriever() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Starts retrieving the entire resource and writing it to the destination
     *  @return the future of the task doing the retrieval. If a retrieval is already busy or the task cannot be queued
     *          it is a future without a future and the failure is logged and no events are sent.
     */
    CFutureResult Retrieve( const CUri& uri       ,
                            CIOAccess& destination );

    CFutureResult Retrieve( const CUri& uri           ,
                            CDynamicBuffer& destination );

    /**
     *  Same as Retrieve() but only for a part of the resource
     *  @param bytesToGet   number of bytes to get, -1 for everything starting at the offset
     */
    CFutureResult GetPartial( const CUri& uri        ,
                              UInt64 byteOffset      ,
                              Int64 bytesToGet       ,
                              CIOAccess& destination );

    CFutureResult GetPartial( const CUri& uri           ,
                              UInt64 byteOffset         ,
                              Int64 bytesToGet          ,
                              CDynamicBuffer& destination );

    /**
     *  Cancels the busy retrieval, if any. A RetrievalCancelledEvent follows.
     *  @param waitForEnd   whether to wait for the retrieval to have stopped, after which the destination is safe
     *                      to use again
     */
    void Cancel( bool waitForEnd = true );

    /**
     *  True from the moment a retrieval was started until the owner of this object received its final event
     *  (Completed, Failed or Cancelled), or the retrieval ended and it can be known no event will be pumped anymore
     */
    bool IsBusy( void ) const;

    UInt32 GetCurrentRetrievalId( void ) const;

    /**
     *  Whether to ask for the size of the resource before retrieving it, so that progress can tell the total.
     *  Costs an additional request for some schemes, off by default.
     */
    void SetQueryTotalSize( bool queryTotalSize );

    bool GetQueryTotalSize( void ) const;

    /** Minimal number of bytes between progress events, default 64KiB */
    void SetProgressReportIntervalInBytes( UInt32 intervalInBytes );

    UInt32 GetProgressReportIntervalInBytes( void ) const;

    /** Pool on which the retrievals run, by default the default pool of the task manager */
    void SetThreadPool( ThreadPoolPtr threadPool );

    ThreadPoolPtr GetThreadPool( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPumpedNotify( CNotifier* notifier                 ,
                                 const CEvent& eventid               ,
                                 CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAsyncUriResourceRetriever( const CAsyncUriResourceRetriever& src );            /**< not supported */
    CAsyncUriResourceRetriever& operator=( const CAsyncUriResourceRetriever& src ); /**< not supported */

    CFutureResult QueueRetrieval( const CUri& uri            ,
                                  bool isPartial             ,
                                  UInt64 byteOffset          ,
                                  Int64 bytesToGet           ,
                                  CIOAccess* destination     ,
                                  CDynamicBuffer* buffer     );

    void Initialize( void );

    private:

    typedef CTSharedPtr< CAsyncUriRetrievalReporter, MT::CMutex > TReporterPtr;

    TReporterPtr m_reporter;
    ThreadPoolPtr m_threadPool;
    ThreadPoolPtr m_activeThreadPool;
    CFutureResult* m_currentRetrieval;
    UInt32 m_currentRetrievalId;
    UInt32 m_lastRetrievalId;
    bool m_busy;
    bool m_queryTotalSize;
    UInt32 m_progressReportIntervalInBytes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H ? */
