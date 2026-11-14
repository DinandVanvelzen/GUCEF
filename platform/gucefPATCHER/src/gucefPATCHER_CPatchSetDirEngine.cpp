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

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_PATCHER_CPATCHSETFILEENGINE_H
#include "gucefPATCHER_CPatchSetFileEngine.h"
#define GUCEF_PATCHER_CPATCHSETFILEENGINE_H
#endif /* GUCEF_PATCHER_CPATCHSETFILEENGINE_H ? */

#include "gucefPATCHER_CPatchSetDirEngine.h"

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

CPatchSetDirEngine::CPatchSetDirEngine( void )
    : CORE::CForwardingNotifier()            ,
      CPatchSetFileEngineEvents()            ,
      m_curSubDirIndex( 0 )                  ,
      m_subDirPatchEngine( GUCEF_NULL )            ,
      m_filePatchEngine( GUCEF_NULL )              ,
      m_dir()                                ,
      m_isActive( false )                    ,
      m_stopSignalGiven( false )             ,
      m_localRoot()                          ,
      m_tempStorageRoot()                    ,
      m_pulseGenerator()                     ,
      m_processedDataSizeInBytes( 0 )        ,
      m_stopOnFileReplacementFailure( true )
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

CPatchSetDirEngine::CPatchSetDirEngine( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CForwardingNotifier()             ,
      CPatchSetFileEngineEvents()             ,
      m_curSubDirIndex( 0 )                   ,
      m_subDirPatchEngine( GUCEF_NULL )             ,
      m_filePatchEngine( GUCEF_NULL )               , 
      m_dir()                                 ,
      m_isActive( false )                     ,
      m_stopSignalGiven( false )              ,
      m_localRoot()                           ,
      m_tempStorageRoot()                     ,
      m_pulseGenerator( pulseGenerator )      ,
      m_processedDataSizeInBytes( 0 )         ,
      m_stopOnFileReplacementFailure( false )
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

void
CPatchSetDirEngine::Initialize( void )
{
    // Forward events from child sub-dir engines
    AddForwardingForEvent( DirProcessingStartedEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( DirProcessingProgressEvent, EVENTORIGINFILTER_UNMODIFIED );
    AddForwardingForEvent( DirProcessingStartedEvent, EVENTORIGINFILTER_UNMODIFIED );
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
}

/*-------------------------------------------------------------------------*/

CPatchSetDirEngine::~CPatchSetDirEngine()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPatchSetDirEngine::TPatchSetDirEngineEventData*
CPatchSetDirEngine::CreateEventStatusObj( void ) const
{GUCEF_TRACE;
    
    TPatchSetDirEngineEventDataStorage storage;
    storage.dirHash = m_dir.hash;
    storage.dirName = m_dir.name;
    storage.dirSizeInBytes = m_dir.sizeInBytes;
    storage.processedDataSizeInBytes = m_processedDataSizeInBytes;
    return new TPatchSetDirEngineEventData( storage );
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetDirEngine::ProcessFilesInDir( void )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetDirEngine(" + CORE::PointerToString( this ) + "): Processing files in the directory" );
    
    if ( m_dir.files.size() > 0 )
    {
        // We will be needing an engine for our file processing
        if ( m_filePatchEngine == GUCEF_NULL )
        {
            if ( !m_pulseGenerator.IsNULL() )
            {
                m_filePatchEngine = new CPatchSetFileEngine( m_pulseGenerator );
            }
            else
            {
                m_filePatchEngine = new CPatchSetFileEngine();
            }
            SubscribeTo( m_filePatchEngine );
        }
        
        // process the files in this directory
        return m_filePatchEngine->Start( m_dir.files                    ,
                                         m_localRoot                    ,
                                         m_tempStorageRoot              ,
                                         m_stopOnFileReplacementFailure );
    }
    
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetDirEngine::ProcessCurSubDir( void )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetDirEngine(" + CORE::PointerToString( this ) + "): Processing current sub-directory" );
    
    if ( m_curSubDirIndex < m_dir.subDirs.size() )
    {
        // reset the sub-dir progress counter
        m_processedSubDirDataSizeInBytes = 0;
        
        NotifyObservers( SubDirProcessingStartedEvent );
        
        // We will be needing an engine for our sub-dir processing
        if ( m_subDirPatchEngine == GUCEF_NULL )
        {
            if ( !m_pulseGenerator.IsNULL() )
            {
                m_subDirPatchEngine = new CPatchSetDirEngine( m_pulseGenerator );
            }
            else
            {
                m_subDirPatchEngine = new CPatchSetDirEngine();
            }
            SubscribeTo( m_subDirPatchEngine ); 
        }
        
        // Create the correct path strings for this sub-dir
        const TDirEntry* subDir = &m_dir.subDirs[ m_curSubDirIndex ];
        CORE::CString subDirPath = m_localRoot;
        CORE::AppendToPath( subDirPath, subDir->name );
        CORE::CString subTmpDirPath = m_tempStorageRoot;
        CORE::AppendToPath( subTmpDirPath, subDir->name );
        
        // make sure the sub-dir exists locally
        CORE::CreateDirs( subDirPath );
        
        // process the sub-dir
        return m_subDirPatchEngine->Start( *subDir                        ,
                                           subDirPath                     ,
                                           subTmpDirPath                  ,
                                           m_stopOnFileReplacementFailure );
    }
    
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetDirEngine::Start( const TDirEntry& startingDir            ,
                           const CORE::CString& localRoot          ,
                           const CORE::CString& tempStorageRoot    ,
                           const bool stopOnFileReplacementFailure )
{GUCEF_TRACE;

    assert( &startingDir != GUCEF_NULL );
    
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetDirEngine(" + CORE::PointerToString( this ) + "): Started" );
    
    // The user should explicitly stop first if we are already busy
    if ( !IsActive() )
    {
        // parameter sanity check
        if ( localRoot.Length() > 0 )
        {
            m_isActive = true;
            m_stopSignalGiven = false;
            
            // Copy and link params for later use
            m_dir = startingDir;
            m_localRoot = localRoot;
            m_tempStorageRoot = tempStorageRoot;
            m_curSubDirIndex = 0;
            m_processedDataSizeInBytes = 0;
            m_processedSubDirDataSizeInBytes = 0;
            m_stopOnFileReplacementFailure = stopOnFileReplacementFailure;
            
            NotifyObservers( DirProcessingStartedEvent, CreateEventStatusObj() );
            
            if ( m_dir.files.size() > 0 )
            {
                return ProcessFilesInDir();
            }
            else
            {
                NotifyObservers( SubDirProcessingStartedEvent, CreateEventStatusObj() );
                return ProcessCurSubDir();
            }
        }
    }
    
    return false;
}

/*-------------------------------------------------------------------------*/

void
CPatchSetDirEngine::Stop( void )
{GUCEF_TRACE;

    if ( m_isActive )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetDirEngine(" + CORE::PointerToString( this ) + "): Stop called while active" );
        
        m_stopSignalGiven = true;
        
        if ( m_filePatchEngine != GUCEF_NULL )
        {
            if ( m_filePatchEngine->IsActive() )
            {
                m_filePatchEngine->Stop();
            }
        }
        if ( m_subDirPatchEngine != GUCEF_NULL )
        {
            if ( m_subDirPatchEngine->IsActive() )
            {
                m_subDirPatchEngine->Stop();
            }
        }
    }    
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetDirEngine::IsActive( void ) const
{GUCEF_TRACE;

    return m_isActive;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetDirEngine::ProcessNextSubDir( void )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetDirEngine(" + CORE::PointerToString( this ) + "): Processing the next sub-directory" );
    
    // Move on to the next sub-dir (if any exists)
    if ( m_curSubDirIndex+1 < m_dir.subDirs.size() )
    {
        ++m_curSubDirIndex;
        return ProcessCurSubDir();
    }
    
    // If we get here then we have worked trough our list of sub-dirs
    // Since this operation comes after file handling we can now tell
    // everyone we are finished
    m_stopSignalGiven = false;
    m_isActive = false;
    NotifyObservers( DirProcessingCompletedEvent, CreateEventStatusObj() );
    
    return true;
}

/*-------------------------------------------------------------------------*/

void
CPatchSetDirEngine::OnNotify( CORE::CNotifier* notifier                 ,
                              const CORE::CEvent& eventid               ,
                              CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( !m_stopSignalGiven && m_isActive )
    {
        // Call base-class version
        CORE::CForwardingNotifier::OnNotify( notifier  ,
                                             eventid   ,
                                             eventdata );
        
        if ( ( m_filePatchEngine != GUCEF_NULL ) && ( notifier == m_filePatchEngine ) )
        {
            if ( ( eventid == CPatchSetFileEngineEvents::FileRetrievalProgressEvent ) ||
                 ( eventid == CPatchSetFileEngineEvents::LocalFileIsOKEvent )         ||
                 ( eventid == CPatchSetFileEngineEvents::LocalFileReplacedEvent )     )
            {
                // Data has been processed by the file engine and we should update the progress counter
                const TPatchSetFileEngineEventData* eData = static_cast< TPatchSetFileEngineEventData* >( eventdata );
                const TPatchSetFileEngineEventDataStorage& storage = eData->GetData();
                
                // Files get processed first so we can simply assign the value without worrying about sub-dirs
                m_processedDataSizeInBytes = storage.totalBytesProcessed;
                NotifyObservers( DirProcessingProgressEvent, CreateEventStatusObj() );
            }
            else
            if ( eventid == CPatchSetFileEngine::FileListProcessingCompleteEvent )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetDirEngine(" + CORE::PointerToString( this ) + "): Finished processing all files in the directory" );
                
                if ( m_curSubDirIndex < m_dir.subDirs.size() )
                {
                    // The files in this dir have been processed, we can move on to the
                    // sub-directories. The first sub-dir is the current sub-dir.
                    ProcessCurSubDir();
                }
                else
                {
                    // If we get here then we have worked trough our list of sub-dirs
                    // Since this operation comes after file handling we can now tell
                    // everyone we are finished
                    m_stopSignalGiven = false;
                    m_isActive = false;
                    NotifyObservers( DirProcessingCompletedEvent, CreateEventStatusObj() );
                }
                return;
            }
        }
        else
        if ( ( m_subDirPatchEngine != GUCEF_NULL ) && ( notifier == m_subDirPatchEngine ) )
        {
            if ( eventid == DirProcessingProgressEvent )
            {
                // Data has been processed by the dir engine and we should update the progress counter
                const TPatchSetDirEngineEventData* eData = static_cast< TPatchSetDirEngineEventData* >( eventdata );
                const TPatchSetDirEngineEventDataStorage& storage = eData->GetData();
                
                m_processedDataSizeInBytes = ( m_processedDataSizeInBytes - m_processedSubDirDataSizeInBytes ) + storage.processedDataSizeInBytes;
                m_processedSubDirDataSizeInBytes = storage.processedDataSizeInBytes;
                
                NotifyObservers( DirProcessingProgressEvent, CreateEventStatusObj() );
                return;
            }
            else
            if ( eventid == CPatchSetDirEngineEvents::DirProcessingCompletedEvent )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetDirEngine(" + CORE::PointerToString( this ) + "): Finished processing the current sub-directory" );
                NotifyObservers( SubDirProcessingCompletedEvent, CreateEventStatusObj() );
                
                // Move on to the next sub-dir (if any exists)
                ProcessNextSubDir();
                return;
            }
        }
    }
    else
    {
        m_stopSignalGiven = false;
        m_isActive = false;
        NotifyObservers( DirProcessingAbortedEvent, CreateEventStatusObj() );
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CPatchSetDirEngine::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CString typeName = "GUCEF::PATCHER::CPatchSetDirEngine";
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