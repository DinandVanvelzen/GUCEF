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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

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

#ifndef GUCEF_PATCHER_CPATCHSETENGINE_H
#include "gucefPATCHER_CPatchSetEngine.h"
#define GUCEF_PATCHER_CPATCHSETENGINE_H
#endif /* GUCEF_PATCHER_CPATCHSETENGINE_H ? */

#include "gucefPATCHER_CPatchListEngine.h"	/* definition of this class */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PATCHER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CPatchListEngine::CPatchListEngine( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CForwardingNotifier()                               ,
      CPatchSetDirEngineEvents()                                ,
      CPatchSetFileEngineEvents()                               ,
      CPatchSetEngineEvents()                                   ,
      CPatchListEngineEvents()                                  ,
      m_patchSetEngine( new CPatchSetEngine( pulseGenerator ) ) ,
      m_setDataBuffer()                                         ,
      m_retriever( pulseGenerator )                             ,
      m_patchList()                                             ,
      m_setIndex( 0 )                                           ,
      m_setLocIndex( 0 )                                        ,
      m_isActive( false )                                       ,
      m_stopSignalGiven( false )                                ,
      m_localRoot()                                             ,
      m_tempStorageRoot()                                       ,
      m_totalDataSizeInBytes( 0 )                               ,
      m_processedDataSizeInBytes( 0 )                           ,
      m_processedCurrentSetDataSizeInBytes( 0 )                 ,
      m_stopOnFileReplacementFailure( false )

{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

CPatchListEngine::CPatchListEngine( void )
    : CORE::CForwardingNotifier()               ,
      CPatchSetDirEngineEvents()                ,
      CPatchSetFileEngineEvents()               ,
      CPatchSetEngineEvents()                   ,
      CPatchListEngineEvents()                  ,
      m_patchSetEngine( new CPatchSetEngine() ) ,
      m_setDataBuffer()                         ,
      m_retriever()                             ,
      m_patchList()                             ,
      m_setIndex( 0 )                           ,
      m_setLocIndex( 0 )                        ,
      m_isActive( false )                       ,
      m_stopSignalGiven( false )                ,
      m_localRoot()                             ,
      m_tempStorageRoot()                       ,
      m_totalDataSizeInBytes( 0 )               ,
      m_processedDataSizeInBytes( 0 )           ,
      m_processedCurrentSetDataSizeInBytes( 0 ) ,
      m_stopOnFileReplacementFailure( false )

{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

void
CPatchListEngine::Initialize( void )
{GUCEF_TRACE;

    assert( m_patchSetEngine != NULL );

    // Forward events from the set engine
    AddForwardingForEvent( PatchSetProcessingStartedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchSetProcessingProgressEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchSetProcessingCompletedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchSetProcessingAbortedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( PatchSetProcessingFailedEvent, EVENTORIGINFILTER_UNMODIFIED );

    // Forward events from the dir engines
    AddForwardingForEvent( DirProcessingStartedEvent, EVENTORIGINFILTER_UNMODIFIED );
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

    // Subscribe to all events from the patch set engine
    // This should allow us to receive all events that we wish to forward
    SubscribeTo( m_patchSetEngine );
}

/*-------------------------------------------------------------------------*/

CPatchListEngine::~CPatchListEngine()
{GUCEF_TRACE;

    Stop();

    delete m_patchSetEngine;
    m_patchSetEngine = NULL;
}

/*-------------------------------------------------------------------------*/

CPatchListEngine::TPatchListEngineEventData*
CPatchListEngine::CreateEventStatusObj( void ) const
{GUCEF_TRACE;

    TPatchListEngineEventDataStorage storage;
    storage.totalDataSizeInBytes = m_totalDataSizeInBytes;
    storage.processedDataSizeInBytes = m_processedDataSizeInBytes;
    return new TPatchListEngineEventData( storage );
}

/*-------------------------------------------------------------------------*/

bool
CPatchListEngine::GetCurrentPatchSetLocation( TPatchSetLocation** location )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchListEngine: Obtaining the current patch set location" );

    // Get the current set
    TPatchList::iterator i = m_patchList.begin();
    for ( UInt32 n=0; n<m_setIndex; ++n ) { ++i; }

    // Get the current set location
    TPatchSetLocations& patchSetLocations = (*i).second;
    if ( m_setLocIndex < patchSetLocations.size() )
    {
        // Assign the location
        *location = &( patchSetLocations[ m_setLocIndex ] );
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPatchListEngine::ObtainCurrentPatchSet( void )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchListEngine: Obtaining the current patch set" );

    TPatchSetLocation* setLocation = NULL;
    if ( GetCurrentPatchSetLocation( &setLocation ) )
    {
        // Make sure the buffer is cleared
        m_setDataBuffer.Clear();

        // Now we try and obtain the patch set from the URL of the current set location
        NotifyObservers( PatchSetRetrievalStartedEvent, CreateEventStatusObj() );
        if ( !m_retriever.Retrieve( CORE::CUri( setLocation->URL ), m_setDataBuffer ).HasAFuture() )
        {
            NotifyObservers( PatchSetRetrievalFailedEvent, CreateEventStatusObj() );
            return false;
        }

        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPatchListEngine::Start( const TPatchList& patchList             ,
                         const CORE::CString& localRoot          ,
                         const CORE::CString& tempStorageRoot    ,
                         const bool stopOnFileReplacementFailure )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchListEngine: Starting,.." );

    assert( &patchList != NULL );

    // The user should explicitly stop first if we are already busy
    if ( !IsActive() )
    {
        // parameter sanity check
        if ( ( patchList.size() > 0 )   &&
             ( localRoot.Length() > 0 )  )
        {
            m_isActive = true;
            m_stopSignalGiven = false;
            m_totalDataSizeInBytes = 0;
            m_processedDataSizeInBytes = 0;
            m_processedCurrentSetDataSizeInBytes = 0;
            m_stopOnFileReplacementFailure = stopOnFileReplacementFailure;

            m_patchList = patchList;

            m_localRoot = CORE::RelativePath( localRoot );
            m_tempStorageRoot = CORE::RelativePath( tempStorageRoot );
            m_setIndex = 0;
            m_setLocIndex = 0;

            NotifyObservers( PatchListProcessingStartedEvent, CreateEventStatusObj() );

            // We must obtain the patch set before we can use it
            if ( !ObtainCurrentPatchSet() )
            {
                NotifyObservers( PatchListProcessingFailedEvent, CreateEventStatusObj() );
                return false;
            }

            // Now we wait
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

void
CPatchListEngine::Stop( void )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchListEngine: Stopping,.." );

    if ( !m_stopSignalGiven && m_isActive )
    {
        m_stopSignalGiven = true;

        m_retriever.Cancel( true );
        m_patchSetEngine->Stop();
    }
}

/*-------------------------------------------------------------------------*/

bool
CPatchListEngine::IsActive( void ) const
{GUCEF_TRACE;

    return m_isActive;
}

/*-------------------------------------------------------------------------*/

bool
CPatchListEngine::ProcessRecievedPatchSet( void )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchListEngine: Processing the received patch set" );

    if ( m_setDataBuffer.GetDataSize() > 0 )
    {
        // Now we must process the raw patch set data to turn it into something we can use
        TPatchSetLocation* setLocation = NULL;
        if ( GetCurrentPatchSetLocation( &setLocation ) )
        {
            // Get the required codec for the current raw patch set data type
            CORE::CDStoreCodecRegistry::TDStoreCodecPtr codecPtr = CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().Lookup( setLocation->codec );
            if ( codecPtr )
            {
                // Prepare vars for the decoding process
                CORE::CDataNode rootNode;
                CORE::CMFileAccess dataAccess( m_setDataBuffer.GetConstBufferPtr() ,
                                               m_setDataBuffer.GetDataSize()       );

                // decode the data in our buffer into a data tree
                if ( codecPtr->BuildDataTree( &rootNode   ,
                                              &dataAccess ) )
                {
                    // Make sure the buffer is cleared
                    m_setDataBuffer.Clear();

                    // Now we have to parse the data tree into something more familiar
                    TPatchSet patchSet;
                    CPatchSetParser setParser;
                    if ( setParser.ParsePatchSet( rootNode ,
                                                  patchSet ) )
                    {
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchListEngine: Parsed the patch set, adding it to the patch set cache" );

                        // Add the newly received patch set to out patch set cache
                        m_patchSets.push_back( patchSet );

                        // Check if we have more patch set documents we have to retrieve
                        if ( m_setIndex+1 < m_patchList.size() )
                        {
                            // Move to the next set entry and obtain the document for it
                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchListEngine: Moving on to the retrieval of the next patch set document" );
                            ++m_setIndex;
                            m_setLocIndex = 0;
                            return ObtainCurrentPatchSet();
                        }
                        else
                        {
                            // Now that all patch sets have been retrieved and turned into a real patch list we can commence
                            // with the patching process for the first patch set. First we will calculate the total size of all data in all patch sets
                            // this will allow us to give accurate progress reports.
                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchListEngine: Retrieved all patch sets and processed the documents" );
                            m_totalDataSizeInBytes = 0;
                            TPatchSetVector::iterator i = m_patchSets.begin();
                            while ( i != m_patchSets.end() )
                            {
                                // Add the size of all root directories for this patch set entry
                                // That size includes the size of everything below it.
                                TPatchSet& patchSetEntry = (*i);
                                TPatchSet::iterator n = patchSetEntry.begin();
                                while ( n != patchSetEntry.end() )
                                {
                                    m_totalDataSizeInBytes += (*n).sizeInBytes;
                                    ++n;
                                }
                                ++i;
                            }
                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchListEngine: The total size of all data in the patch list is: " + CORE::UInt64ToString( m_totalDataSizeInBytes ) + " Bytes" );

                            m_setIndex = 0;
                            return m_patchSetEngine->Start( patchSet                       ,
                                                            m_localRoot                    ,
                                                            m_tempStorageRoot              ,
                                                            m_stopOnFileReplacementFailure );
                        }
                    }
                }
            }
        }
    }

    // If we get here then we failed to decode the raw data into a patch set
    NotifyObservers( PatchSetDecodingFailedEvent, CreateEventStatusObj() );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPatchListEngine::OnNotify( CORE::CNotifier* notifier                 ,
                            const CORE::CEvent& eventid               ,
                            CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( !m_stopSignalGiven && m_isActive )
    {
        // Call base-class version
        CORE::CForwardingNotifier::OnNotify( notifier  ,
                                             eventid   ,
                                             eventdata );

        if ( notifier == m_patchSetEngine )
        {
            if ( eventid == PatchSetProcessingProgressEvent )
            {
                const TPatchSetEngineEventData* eData = static_cast< TPatchSetEngineEventData* >( eventdata );
                const TPatchSetEngineEventDataStorage& storage = eData->GetData();

                // Update the total bytes processed counter
                m_processedDataSizeInBytes = ( m_processedDataSizeInBytes - m_processedCurrentSetDataSizeInBytes ) + storage.processedDataSizeInBytes;
                m_processedCurrentSetDataSizeInBytes = storage.processedDataSizeInBytes;

                NotifyObservers( PatchListProcessingProgressEvent, CreateEventStatusObj() );
            }
            else
            if ( eventid == PatchSetProcessingCompletedEvent )
            {
                const TPatchSetEngineEventData* eData = static_cast< TPatchSetEngineEventData* >( eventdata );
                const TPatchSetEngineEventDataStorage& storage = eData->GetData();

                // Update the total bytes processed counter
                m_processedDataSizeInBytes = ( m_processedDataSizeInBytes - m_processedCurrentSetDataSizeInBytes ) + storage.processedDataSizeInBytes;
                m_processedCurrentSetDataSizeInBytes = 0;

                // W00t,.. we finished processing an entire patch set
                if ( m_setIndex+1 < m_patchList.size() )
                {
                    // Make sure we propergate the progress we calculated above since we are not done yet
                    NotifyObservers( PatchListProcessingProgressEvent, CreateEventStatusObj() );

                    ++m_setIndex;
                    m_setLocIndex = 0;
                    ObtainCurrentPatchSet();
                }
                else
                {
                    // YIHA,.. We are finished
                    m_stopSignalGiven = false;
                    m_isActive = false;
                    NotifyObservers( PatchListProcessingCompletedEvent, CreateEventStatusObj() );
                }
            }
            else
            if ( eventid == PatchSetProcessingFailedEvent )
            {
                // Cascade failure
                m_stopSignalGiven = false;
                m_isActive = false;
                NotifyObservers( PatchListProcessingFailedEvent, CreateEventStatusObj() );
            }
        }
        else
        if ( notifier == &m_retriever )
        {
            if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalProgressEvent )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchListEngine: Patch set data received" );

                // The data itself is already in our buffer
                NotifyObservers( PatchSetDataRecievedEvent, CreateEventStatusObj() );
            }
            else
            if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalCompletedEvent )
            {
                // The retrieval process of the patch set has been completed
                NotifyObservers( PatchSetRetrievalCompletedEvent, CreateEventStatusObj() );

                // Now we must process the received patch set
                ProcessRecievedPatchSet();
            }
            else
            if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalCancelledEvent )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchListEngine: Aborting patch list processing" );

                // Someone has called Stop() while we where busy with our data retrieval
                m_stopSignalGiven = false;
                m_isActive = false;
                NotifyObservers( PatchListProcessingAbortedEvent, CreateEventStatusObj() );
            }
            else
            if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalFailedEvent )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchListEngine: Patch set data retrieval error" );

                // We failed to obtain the patch set data using the current location URL
                // Perhaps we can use another location
                ++m_setLocIndex;
                if ( !ObtainCurrentPatchSet() )
                {
                    // switching to the next location (if there is one) didn't help.
                    m_stopSignalGiven = false;
                    m_isActive = false;
                    NotifyObservers( PatchSetRetrievalFailedEvent, CreateEventStatusObj() );
                    NotifyObservers( PatchListProcessingFailedEvent, CreateEventStatusObj() );
                }
            }
        }
    }
    else
    {
        if ( m_isActive )
        {
            m_stopSignalGiven = false;
            m_isActive = false;
            NotifyObservers( PatchListProcessingAbortedEvent, CreateEventStatusObj() );
        }
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CPatchListEngine::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CString typeName = "GUCEF::PATCHER::CPatchListEngine";
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
