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

#ifndef GUCEF_CORE_CASYNCURIRETRIEVALTASK_H
#define GUCEF_CORE_CASYNCURIRETRIEVALTASK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CPROGRESSREPORTINGIOACCESS_H
#include "gucefCORE_CProgressReportingIOAccess.h"
#define GUCEF_CORE_CPROGRESSREPORTINGIOACCESS_H
#endif /* GUCEF_CORE_CPROGRESSREPORTINGIOACCESS_H ? */

#ifndef GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H
#include "gucefCORE_CAsyncUriResourceRetriever.h"
#define GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H
#endif /* GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H ? */

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

/**
 *  Notifier through which a retrieval task reports to the CAsyncUriResourceRetriever that started it. Shared between
 *  the two so a task that outlives its retriever can still report safely.
 */
class GUCEF_HIDDEN CAsyncUriRetrievalReporter : public CTSGNotifier
{
    public:

    CAsyncUriRetrievalReporter( void );

    virtual ~CAsyncUriRetrievalReporter() GUCEF_VIRTUAL_OVERRIDE;

    bool Report( const CEvent& eventId, CICloneable* eventData );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

typedef CTSharedPtr< CAsyncUriRetrievalReporter, MT::CMutex > CAsyncUriRetrievalReporterPtr;

/*-------------------------------------------------------------------------*/

/**
 *  Everything a retrieval task needs. The destination is not owned and must outlive the task.
 */
class GUCEF_HIDDEN CAsyncUriRetrievalTaskData : public CICloneable
{
    public:

    UInt32 retrievalId;
    CUri uri;
    bool isPartial;
    UInt64 byteOffset;
    Int64 bytesToGet;
    CIOAccess* destination;
    CDynamicBuffer* destinationBuffer;
    bool queryTotalSize;
    UInt32 reportIntervalInBytes;
    CAsyncUriRetrievalReporterPtr reporter;

    // result
    bool success;
    CString errorText;

    CAsyncUriRetrievalTaskData( void );

    CAsyncUriRetrievalTaskData( const CAsyncUriRetrievalTaskData& src );

    virtual ~CAsyncUriRetrievalTaskData() GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  Task consumer which does the actual retrieval on its own thread, synchronously using the accessor for the URI
 */
class GUCEF_HIDDEN CAsyncUriRetrievalTaskConsumer : public CTaskConsumer      ,
                                                    public CIIOProgressObserver
{
    public:

    static const CString TaskType;

    CAsyncUriRetrievalTaskConsumer( void );

    virtual ~CAsyncUriRetrievalTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskStart( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CTaskPtr task ,
                              bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnIOProgress( UInt64 totalBytesRead    ,
                               UInt64 totalBytesWritten ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAsyncUriRetrievalTaskConsumer( const CAsyncUriRetrievalTaskConsumer& src );            /**< not supported */
    CAsyncUriRetrievalTaskConsumer& operator=( const CAsyncUriRetrievalTaskConsumer& src ); /**< not supported */

    bool IsStopRequested( void ) const;

    void Report( const CEvent& eventId, UInt64 bytesTransferred, Int64 totalBytes, const CString& errorText );

    private:

    CAsyncUriRetrievalTaskData* m_taskData;
    CTask* m_task;
    Int64 m_totalBytes;
    UInt64 m_bytesTransferred;
    UInt64 m_lastReportedBytes;
    bool m_stoppedEarly;
};

typedef CTFactory< CTaskConsumer, CAsyncUriRetrievalTaskConsumer, MT::CMutex > TAsyncUriRetrievalTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CASYNCURIRETRIEVALTASK_H ? */
