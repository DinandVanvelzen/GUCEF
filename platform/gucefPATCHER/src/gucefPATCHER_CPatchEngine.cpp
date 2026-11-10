/*
 *  gucefPATCHER: GUCEF RAD module providing a patch delivery system
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CMFILEACCESS_H
#include "CMFileAccess.h"
#define GUCEF_CORE_CMFILEACCESS_H
#endif /* GUCEF_CORE_CMFILEACCESS_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_PATCHER_CPATCHLISTENGINE_H
#include "gucefPATCHER_CPatchListEngine.h"
#define GUCEF_PATCHER_CPATCHLISTENGINE_H
#endif /* GUCEF_PATCHER_CPATCHLISTENGINE_H ? */

#include "gucefPATCHER_CPatchEngine.h"	/* definition of this class */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PATCHER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CPatchEngine::PatchProcessStartedEvent = "GUCEF::PATCHER::CPatchEngine::PatchProcessStartedEvent";
const CORE::CEvent CPatchEngine::PatchProcessCompletedEvent = "GUCEF::PATCHER::CPatchEngine::PatchProcessCompletedEvent";
const CORE::CEvent CPatchEngine::PatchProcessAbortedEvent = "GUCEF::PATCHER::CPatchEngine::PatchProcessAbortedEvent";
const CORE::CEvent CPatchEngine::PatchProcessFailedEvent = "GUCEF::PATCHER::CPatchEngine::PatchProcessFailedEvent";
const CORE::CEvent CPatchEngine::PatchListDataReceivedEvent = "GUCEF::PATCHER::CPatchEngine::PatchListDataReceivedEvent";
const CORE::CEvent CPatchEngine::PatchListRetrievalStartedEvent = "GUCEF::PATCHER::CPatchEngine::PatchListRetrievalStartedEvent";
const CORE::CEvent CPatchEngine::PatchListRetrievalCompletedEvent = "GUCEF::PATCHER::CPatchEngine::PatchListRetrievalCompletedEvent";
const CORE::CEvent CPatchEngine::PatchListRetrievalFailedEvent = "GUCEF::PATCHER::CPatchEngine::PatchListRetrievalFailedEvent";
const CORE::CEvent CPatchEngine::PatchListRetrievalAbortedEvent = "GUCEF::PATCHER::CPatchEngine::PatchListRetrievalAbortedEvent";
const CORE::CEvent CPatchEngine::PatchListDecodingFailedEvent = "GUCEF::PATCHER::CPatchEngine::PatchListDecodingFailedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CPatchEngine::CPatchEngine( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CForwardingNotifier()                                 ,
      CPatchSetDirEngineEvents()                                  ,
      CPatchSetFileEngineEvents()                                 ,
      CPatchSetEngineEvents()                                     ,
      CPatchListEngineEvents()                                    ,
      m_patchListEngine( new CPatchListEngine( pulseGenerator ) ) ,
      m_listDataBuffer()                                          ,
      m_retriever( pulseGenerator )                               ,
      m_isActive( false )                                         ,
      m_stopSignalGiven( false )                                  ,
      m_config()
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

CPatchEngine::CPatchEngine( void )
    : CORE::CForwardingNotifier()                 ,
      CPatchSetDirEngineEvents()                  ,
      CPatchSetFileEngineEvents()                 ,
      CPatchSetEngineEvents()                     ,
      CPatchListEngineEvents()                    ,
      m_patchListEngine( new CPatchListEngine() ) ,
      m_listDataBuffer()                          ,
      m_retriever()                               ,
      m_isActive( false )                         ,
      m_stopSignalGiven( false )                  ,
      m_config()

{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

void
CPatchEngine::Initialize( void )
{GUCEF_TRACE;

    assert( m_patchListEngine != NULL );

    RegisterEvents();

    // Forward events from the list engine
    AddForwardingForEvent( PatchListProcessingStartedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchListProcessingProgressEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchListProcessingCompletedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchListProcessingAbortedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchListProcessingFailedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchSetDataRecievedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchSetRetrievalStartedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchSetRetrievalCompletedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchSetRetrievalFailedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchSetRetrievalAbortedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchSetDecodingFailedEvent, EVENTORIGINFILTER_UNMODIFIED );

    // Forward events from the set engine
    AddForwardingForEvent( PatchSetProcessingStartedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchSetProcessingProgressEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchSetProcessingCompletedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchSetProcessingAbortedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchSetProcessingFailedEvent, EVENTORIGINFILTER_UNMODIFIED );

    // Forward events from the dir engines
    AddForwardingForEvent( DirProcessingStartedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( DirProcessingProgressEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( DirProcessingCompletedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( SubDirProcessingStartedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( SubDirProcessingCompletedEvent, EVENTORIGINFILTER_UNMODIFIED );

    // Forward file engine events
    AddForwardingForEvent( FileListProcessingStartedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( LocalFileIsOKEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( LocalFileSizeMismatchEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( LocalFileHashMismatchEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( LocalFileNotFoundEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( LocalFileReplacedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( FileRetrievalStartedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( FileRetrievalProgressEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( FileRetrievalCompleteEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( FileRetrievalErrorEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( FileStorageErrorEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( FileListProcessingCompleteEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( FileListProcessingAbortedEvent, EVENTORIGINFILTER_UNMODIFIED );

    SubscribeTo( &m_retriever );

    // Subscribe to all events from the patch list engine
    // This should allow us to receive all events that we wish to forward
    SubscribeTo( m_patchListEngine );
}

/*-------------------------------------------------------------------------*/

CPatchEngine::~CPatchEngine()
{GUCEF_TRACE;

    Stop();

    delete m_patchListEngine;
    m_patchListEngine = NULL;
}

/*-------------------------------------------------------------------------*/

void
CPatchEngine::RegisterEvents( void )
{GUCEF_TRACE;

    PatchProcessStartedEvent.Initialize();
    PatchProcessCompletedEvent.Initialize();
    PatchProcessAbortedEvent.Initialize();
    PatchProcessFailedEvent.Initialize();
    PatchListDataReceivedEvent.Initialize();
    PatchListRetrievalStartedEvent.Initialize();
    PatchListRetrievalCompletedEvent.Initialize();
    PatchListRetrievalFailedEvent.Initialize();
    PatchListRetrievalAbortedEvent.Initialize();
    PatchListDecodingFailedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CPatchConfig&
CPatchEngine::GetConfig( void )
{GUCEF_TRACE;

    return m_config;
}

/*-------------------------------------------------------------------------*/

const CPatchConfig&
CPatchEngine::GetConfig( void ) const
{GUCEF_TRACE;

    return m_config;
}



/*-------------------------------------------------------------------------*/

bool
CPatchEngine::Start( void )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchEngine: Starting" );

    // The user should explicitly stop first if we are already busy
    if ( !m_isActive )
    {
        // configuration sanity check
        if ( ( m_config.GetPatchListURL().Length() > 0 )   &&
             ( m_config.GetPatchListCodec().Length() > 0 ) &&
             ( m_config.GetLocalRootDir().Length() > 0 )    )
        {
            m_isActive = true;
            m_stopSignalGiven = false;
            m_listDataBuffer.Clear();

            NotifyObservers( PatchProcessStartedEvent );

            // We must obtain the patch list before we can use it,..
            // The list location is given as a URL, which is retrieved straight into our buffer
            NotifyObservers( PatchListRetrievalStartedEvent );
            if ( !m_retriever.Retrieve( CORE::CUri( m_config.GetPatchListURL() ), m_listDataBuffer ).HasAFuture() )
            {
                m_isActive = false;
                NotifyObservers( PatchListRetrievalFailedEvent );
                NotifyObservers( PatchProcessFailedEvent );
                return false;
            }

            // Now we wait
            return true;
        }
    }

    NotifyObservers( PatchProcessFailedEvent );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPatchEngine::Stop( void )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchEngine: Stopping" );

    if ( !m_stopSignalGiven && m_isActive )
    {
        m_stopSignalGiven = true;

        m_retriever.Cancel( true );
        m_patchListEngine->Stop();
    }
}

/*-------------------------------------------------------------------------*/

bool
CPatchEngine::IsActive( void ) const
{GUCEF_TRACE;

    return m_isActive;
}

/*-------------------------------------------------------------------------*/

bool
CPatchEngine::ProcessRecievedPatchList( void )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchEngine: Processing patch list" );

    // Now we must process the raw patch set data to turn it into something we can use
    // Get the required codec for the current raw patch set data type
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codecPtr = CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().Lookup( m_config.GetPatchListCodec() );
    if ( codecPtr )
    {
        // Prepare vars for the decoding process
        CORE::CDataNode rootNode;
        CORE::CMFileAccess dataAccess( m_listDataBuffer.GetConstBufferPtr() ,
                                       m_listDataBuffer.GetDataSize()       );

        // decode the data in our buffer into a data tree
        if ( codecPtr->BuildDataTree( &rootNode   ,
                                      &dataAccess ) )
        {
            // Now we have to parse the data tree into something more familiar
            CPatchListParser::TPatchList patchList;
            CPatchListParser listParser;
            listParser.ParsePatchList( rootNode  ,
                                       patchList );

            // Now that the raw data has been processed into a real patch list we can commence
            // with the patching process for this patch list
            return m_patchListEngine->Start( patchList                                  ,
                                             m_config.GetLocalRootDir()                 ,
                                             m_config.GetLocalTempStorageRootDir()      ,
                                             m_config.GetStopOnFileReplacementFailure() );
        }
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchEngine: Error while processing patch list" );

    // If we get here then we failed to decode the raw data into a patch set
    NotifyObservers( PatchListDecodingFailedEvent );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPatchEngine::OnNotify( CORE::CNotifier* notifier                 ,
                        const CORE::CEvent& eventid               ,
                        CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( !m_stopSignalGiven && m_isActive )
    {
        // Call base-class version
        CORE::CForwardingNotifier::OnNotify( notifier  ,
                                             eventid   ,
                                             eventdata );

        if ( notifier == m_patchListEngine )
        {
            if ( eventid == PatchListProcessingCompletedEvent )
            {
                // YIHA,.. We are finished
                m_stopSignalGiven = false;
                m_isActive = false;
                NotifyObservers( PatchProcessCompletedEvent );
            }
            else
            if ( eventid == PatchListProcessingFailedEvent )
            {
                // Cascade failure
                m_stopSignalGiven = false;
                m_isActive = false;
                NotifyObservers( PatchProcessFailedEvent );
            }
        }
        else
        if ( notifier == &m_retriever )
        {
            if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalProgressEvent )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchEngine: Patch list data received" );

                // The data itself is already in our buffer
                NotifyObservers( PatchListDataReceivedEvent );
            }
            else
            if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalCompletedEvent )
            {
                // The retrieval process of the patch list has been completed
                NotifyObservers( PatchListRetrievalCompletedEvent );

                // Now we must process the received patch list
                if ( !ProcessRecievedPatchList() )
                {
                    NotifyObservers( PatchProcessFailedEvent );
                }
            }
            else
            if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalCancelledEvent )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchEngine: Aborting patch list retrieval" );

                // Someone has called Stop() while we where busy with our data retrieval
                m_stopSignalGiven = false;
                m_isActive = false;
                NotifyObservers( PatchProcessAbortedEvent );
            }
            else
            if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalFailedEvent )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchEngine: Patch list data retrieval error" );

                // We failed to obtain the patch list data using the URL
                 m_stopSignalGiven = false;
                 m_isActive = false;
                 NotifyObservers( PatchListRetrievalFailedEvent );
                 NotifyObservers( PatchProcessFailedEvent );
            }
        }
    }
    else
    {
        if ( m_stopSignalGiven && m_isActive )
        {
            m_stopSignalGiven = false;
            m_isActive = false;
            NotifyObservers( PatchProcessAbortedEvent );
        }
        else
        if ( ( eventid != CORE::CNotifier::DestructionEvent ) &&
             ( eventid != CORE::CNotifier::UnsubscribeEvent ) &&
             ( eventid != CORE::CNotifier::SubscribeEvent   )  )
        {
            if ( m_config.HasEngineStartTriggerEvent( eventid ) )
            {
                Start();
            }
            else
            if ( m_config.HasEngineStopTriggerEvent( eventid ) )
            {
                Stop();
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CPatchEngine::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CString typeName = "GUCEF::PATCHER::CPatchEngine";
    return typeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PATCHER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
