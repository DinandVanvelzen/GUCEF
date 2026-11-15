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

#include <assert.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_PATCHER_CPATCHSETDIRENGINE_H
#include "gucefPATCHER_CPatchSetDirEngine.h"
#define GUCEF_PATCHER_CPATCHSETDIRENGINE_H
#endif /* GUCEF_PATCHER_CPATCHSETDIRENGINE_H ? */

#include "gucefPATCHER_CPatchSetEngine.h"	/* definition of this class */

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

CPatchSetEngine::CPatchSetEngine( void )
    : CORE::CForwardingNotifier()                     ,
      CPatchSetDirEngineEvents()                      ,
      CPatchSetFileEngineEvents()                     ,
      CPatchSetEngineEvents()                         ,
      m_patchSetDirEngine( new CPatchSetDirEngine() ) ,
      m_patchSet()                                        ,
      m_dirIndex( 0 )                                 ,
      m_isActive( false )                             ,
      m_stopSignalGiven( false )                      ,
      m_localRoot()                                   ,
      m_processedDataSizeInBytes( 0 )                 ,
      m_totalDataSizeInBytes( 0 )                     ,
      m_stopOnFileReplacementFailure( false )

{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

CPatchSetEngine::CPatchSetEngine( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CForwardingNotifier()                                     ,
      CPatchSetDirEngineEvents()                                      ,
      CPatchSetFileEngineEvents()                                     ,
      CPatchSetEngineEvents()                                         ,
      m_patchSetDirEngine( new CPatchSetDirEngine( pulseGenerator ) ) ,
      m_patchSet()                                                        ,
      m_dirIndex( 0 )                                                 ,
      m_isActive( false )                                             ,
      m_stopSignalGiven( false )                                      ,
      m_localRoot()                                                   ,
      m_processedDataSizeInBytes( 0 )                                 ,
      m_totalDataSizeInBytes( 0 )                                     ,
      m_stopOnFileReplacementFailure( false )

{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

void
CPatchSetEngine::Initialize( void )
{GUCEF_TRACE;

    assert( m_patchSetDirEngine != GUCEF_NULL );

    // Forward events from the dir engines
    AddForwardingForEvent( DirProcessingStartedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( DirProcessingProgressEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( DirProcessingCompletedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( SubDirProcessingStartedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( SubDirProcessingCompletedEvent, EVENTORIGINFILTER_UNMODIFIED );

    // Forward file engine events
    AddForwardingForEvent( FileListProcessingStartedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( LocalFileSizeMismatchEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( LocalFileIsOKEvent, EVENTORIGINFILTER_UNMODIFIED );
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

    SubscribeTo( m_patchSetDirEngine );
}

/*-------------------------------------------------------------------------*/

CPatchSetEngine::~CPatchSetEngine()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

CPatchSetEngine::TPatchSetEngineEventData*
CPatchSetEngine::CreateEventStatusObj( void ) const
{GUCEF_TRACE;

    TPatchSetEngineEventDataStorage storage;
    storage.processedDataSizeInBytes = m_processedDataSizeInBytes;
    storage.totalDataSizeInBytes = m_totalDataSizeInBytes;
    return new TPatchSetEngineEventData( storage );
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetEngine::Start( const TPatchSet& patchSet               ,
                        const CORE::CString& localRoot          ,
                        const CORE::CString& tempStorageRoot    ,
                        const bool stopOnFileReplacementFailure )
{GUCEF_TRACE;

    assert( &patchSet != GUCEF_NULL );

    // The user should explicitly stop first if we are already busy
    if ( !IsActive() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetEngine(" + CORE::PointerToString( this ) + "): Start called while inactive,.. starting" );

        // parameter sanity check
        if ( ( patchSet.size() > 0 )     &&
             ( localRoot.Length() > 0 )  )
        {
            // Determine the total patchset size, this is easy since we only have to
            // add the size of all root dirs since they already have the size of everything in it
            m_totalDataSizeInBytes = 0;
            TPatchSet::const_iterator i = patchSet.begin();
            while ( i != patchSet.end() )
            {
                m_totalDataSizeInBytes += (*i).sizeInBytes;
                ++i;
            }

            m_isActive = true;
            m_stopSignalGiven = false;
            m_processedDataSizeInBytes = 0;
            m_currentSubDirProcessedDataSizeInBytes = 0;
            m_stopOnFileReplacementFailure = stopOnFileReplacementFailure;

            m_patchSet = patchSet;

            m_localRoot = localRoot;
            m_tempStorageRoot = tempStorageRoot;
            m_dirIndex = 0;

            NotifyObservers( PatchSetProcessingStartedEvent, CreateEventStatusObj() );
            m_patchSetDirEngine->Start( m_patchSet[ m_dirIndex ]     ,
                                        m_localRoot                  ,
                                        m_tempStorageRoot            ,
                                        stopOnFileReplacementFailure );

            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

void
CPatchSetEngine::Stop( void )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetEngine(" + CORE::PointerToString( this ) + "): Stop called" );

    m_stopSignalGiven = true;
    m_patchSetDirEngine->Stop();
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetEngine::IsActive( void ) const
{GUCEF_TRACE;

    return m_isActive;
}

/*-------------------------------------------------------------------------*/

void
CPatchSetEngine::OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( !m_stopSignalGiven && m_isActive )
    {
        // Call base-class version
        CORE::CForwardingNotifier::OnNotify( notifier  ,
                                             eventid   ,
                                             eventdata );

        if ( notifier == m_patchSetDirEngine )
        {
            if ( eventid == CPatchSetDirEngineEvents::DirProcessingProgressEvent )
            {
                const TPatchSetDirEngineEventData* eData = static_cast< TPatchSetDirEngineEventData* >( eventdata );
                const TPatchSetDirEngineEventDataStorage& storage = eData->GetData();

                // Update the progress byte counter with the byte delta
                m_processedDataSizeInBytes = ( m_processedDataSizeInBytes - m_currentSubDirProcessedDataSizeInBytes ) + storage.processedDataSizeInBytes;
                m_currentSubDirProcessedDataSizeInBytes = storage.processedDataSizeInBytes;

                NotifyObservers( PatchSetProcessingProgressEvent, CreateEventStatusObj() );
                return;
            }
            else
            if ( eventid == CPatchSetDirEngineEvents::DirProcessingCompletedEvent )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetEngine(" + CORE::PointerToString( this ) + "): Completed processing the current directory" );

                const TPatchSetDirEngineEventData* eData = static_cast< TPatchSetDirEngineEventData* >( eventdata );
                const TPatchSetDirEngineEventDataStorage& storage = eData->GetData();

                // Update the progress byte counter with the byte delta
                m_processedDataSizeInBytes = ( m_processedDataSizeInBytes - m_currentSubDirProcessedDataSizeInBytes ) + storage.processedDataSizeInBytes;
                m_currentSubDirProcessedDataSizeInBytes = 0;

                // Move on to the next sub-dir (if any exists)
                if ( m_dirIndex+1 < m_patchSet.size() )
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetEngine(" + CORE::PointerToString( this ) + "): Moving on to the next directory in the patch set" );

                    // Since we are not finished yet make sure we propegate the data counter progress we calculated above to our observers
                    NotifyObservers( PatchSetProcessingProgressEvent, CreateEventStatusObj() );

                    ++m_dirIndex;
                    m_patchSetDirEngine->Start( m_patchSet[ m_dirIndex ]       ,
                                                m_localRoot                    ,
                                                m_tempStorageRoot              ,
                                                m_stopOnFileReplacementFailure );
                }
                else
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetEngine(" + CORE::PointerToString( this ) + "): Completed processing the entire patch set" );
                    NotifyObservers( PatchSetProcessingCompletedEvent, CreateEventStatusObj() );
                }
                return;
            }
            else
            if ( eventid == DirProcessingAbortedEvent )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetEngine(" + CORE::PointerToString( this ) + "): Directory processing has been aborted, aborting patch set processing" );
                NotifyObservers( PatchSetProcessingAbortedEvent, CreateEventStatusObj() );
                return;
            }
        }
    }
    else
    {
        m_stopSignalGiven = false;
        m_isActive = false;
        NotifyObservers( PatchSetProcessingAbortedEvent, CreateEventStatusObj() );
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CPatchSetEngine::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CString typeName = "GUCEF::PATCHER::CPatchSetEngine";
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
