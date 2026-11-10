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

#ifndef GUCEF_CORE_DVMD5UTILS_H
#include "dvmd5utils.h"
#define GUCEF_CORE_DVMD5UTILS_H
#endif /* GUCEF_CORE_DVMD5UTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#include "gucefPATCHER_CPatchSetFileEngine.h"

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

CPatchSetFileEngine::CPatchSetFileEngine( void )
    : CObservingNotifier()               ,
      CPatchSetFileEngineEvents()        ,
      m_isActive( false )                ,
      m_stopSignalGiven( false )         ,
      m_localRoot()                      ,
      m_tempStorageRoot()                ,
      m_retriever()                      ,
      m_fileAccess( NULL )               ,
      m_fileList()                       ,
      m_curFileLocIndex( 0 )             ,
      m_curFileIndex( 0 )                ,
      m_totalBytesProcessed( 0 )         ,
      m_currentFileReceivedBytes( 0 )    ,
      m_stopOnReplacementFailure( true )
{GUCEF_TRACE;

    SubscribeTo( &m_retriever );
}

/*-------------------------------------------------------------------------*/

CPatchSetFileEngine::CPatchSetFileEngine( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CObservingNotifier()               ,
      CPatchSetFileEngineEvents()        ,
      m_isActive( false )                ,
      m_stopSignalGiven( false )         ,
      m_localRoot()                      ,
      m_tempStorageRoot()                ,
      m_retriever( pulseGenerator )      ,
      m_fileAccess( NULL )               ,
      m_fileList()                       ,
      m_curFileLocIndex( 0 )             ,
      m_curFileIndex( 0 )                ,
      m_totalBytesProcessed( 0 )         ,
      m_currentFileReceivedBytes( 0 )    ,
      m_stopOnReplacementFailure( true )
{GUCEF_TRACE;

    SubscribeTo( &m_retriever );
}

/*-------------------------------------------------------------------------*/

CPatchSetFileEngine::~CPatchSetFileEngine()
{GUCEF_TRACE;

    // Whatever is retrieving into our file has to stop before the file goes away
    m_retriever.Cancel( true );
    delete m_fileAccess;
    m_fileAccess = NULL;
}

/*-------------------------------------------------------------------------*/

CPatchSetFileEngine::TPatchSetFileEngineEventData*
CPatchSetFileEngine::CreateEventStatusObj( void ) const
{GUCEF_TRACE;
    
    TPatchSetFileEngineEventDataStorage storage;
    if ( m_fileList.size() > m_curFileIndex )
    {        
        storage.currentFileEntry = m_fileList[ m_curFileIndex ];
        storage.currentRemoteLocationIndex = m_curFileLocIndex;
        storage.localRoot = m_localRoot;
        storage.tempStorageRoot = m_tempStorageRoot;
        storage.totalBytesProcessed = m_totalBytesProcessed;
    }
    
    return new TPatchSetFileEngineEventData( storage );
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetFileEngine::Start( const TFileList& fileList            ,
                            const CORE::CString& localRoot       ,
                            const CORE::CString& tempStorageRoot ,
                            const bool stopOnReplacementFailure  )

{GUCEF_TRACE;

    // The user should explicitly stop first if we are already busy
    if ( !IsActive() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Starting inactive engine" );
        
        // parameter sanity check
        if ( localRoot.Length() > 0 )
        {
            // Check if we even need to do anything
            if ( fileList.size() > 0 )
            {
                m_isActive = true;
                m_stopSignalGiven = false;
                m_totalBytesProcessed = 0;
                m_currentFileReceivedBytes = 0;
                m_stopOnReplacementFailure = stopOnReplacementFailure;
                m_fileList = fileList;
                m_localRoot = localRoot;
                m_tempStorageRoot = tempStorageRoot;
                m_curFileIndex = 0;
                m_curFileLocIndex = 0;
                
                NotifyObservers( FileListProcessingStartedEvent, CreateEventStatusObj() );
                
                // Set things in motion
                return ProcessCurrentFile();
            }
            else
            {
                // There are no files to process,.. so we are done
                NotifyObservers( FileListProcessingStartedEvent, CreateEventStatusObj() );
                NotifyObservers( FileListProcessingCompleteEvent, CreateEventStatusObj() );
                return true;
            }
        }
    }
    
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetFileEngine::ProcessCurrentFile( void )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Processing the current file" );    
    
    if ( !m_stopSignalGiven                 && 
         m_isActive                         && 
         m_fileList.size() > m_curFileIndex )
    {
        // Get our current file
        const TFileEntry* curFile = &m_fileList[ m_curFileIndex ];
        
        // Create a path string to the file
        CORE::CString filePath = m_localRoot;
        CORE::AppendToPath( filePath, curFile->name );
        
        // Check if the file exists
        if ( CORE::FileExists( filePath ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): There is already a file with the same name as the new file in the destination folder" );
            
            // The file exists, let's see if it's up to date
            UInt64 fileSize = CORE::FileSize( filePath );
            if ( fileSize == curFile->sizeInBytes )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): The old file is the same size as the new file so we will have to do a hash check" );
                
                // The file matches in byte size, it may be up-to-date
                // We will try a hash comparison to make sure
                UInt8 md5Digest[ 16 ];
                CORE::CFileAccess fileAccess( filePath, "rb" );
                fileAccess.Open();
                if ( fileAccess.Opened() && fileAccess.IsValid() )
                {
                    if ( 0 != CORE::md5fromfile( fileAccess.CStyleAccess() , 
                                                 md5Digest                 ) )
                    {
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Finished old file hash generation" );
                        
                        // This may have been a lengthy operation so we re-check the stop signal
                        if ( m_stopSignalGiven || !m_isActive )
                        {
                            m_stopSignalGiven = false;
                            m_isActive = false;
                            return false;
                        }                        
                        
                        // We have now generated an md5 hash from the file
                        // We will now compare it to the one an up-to-date file should have
                        CORE::CString md5String( CORE::MD5ToString( md5Digest ) );
                        if ( curFile->hash == md5String )
                        {
                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): The old file has the same hash as the new file so it is already up-to-date" );
                            
                            // Woohoo, this file is already up-to-date
                            m_totalBytesProcessed += curFile->sizeInBytes;
                            NotifyObservers( LocalFileIsOKEvent, CreateEventStatusObj() );
                            
                            // We can now proceed to the next file if there is any
                            if ( m_curFileIndex+1 < m_fileList.size() )
                            {
                                ++m_curFileIndex;
                                return ProcessCurrentFile();
                            }
                            else
                            {
                                // We are finished,.. engine stopping
                                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Depleted list of files to process, we are finished" );
                                NotifyObservers( FileListProcessingCompleteEvent, CreateEventStatusObj() );
                                m_isActive = false;
                            }
                            return true;
                        }
                        else
                        {
                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): The hash of the remote file does not match that of the local file" );
                            NotifyObservers( LocalFileHashMismatchEvent, CreateEventStatusObj() );
                        }                        
                    }
                    else
                    {
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Failed to generate hash from the old file so we treat this as a hash mismatch" );
                        NotifyObservers( LocalFileHashMismatchEvent, CreateEventStatusObj() );
                    }
                }  
            }
            else
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): The local file has a different size then the remote file" );
                NotifyObservers( LocalFileSizeMismatchEvent, CreateEventStatusObj() );
            }
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): The remote file has no local equavalant" );
            NotifyObservers( LocalFileNotFoundEvent, CreateEventStatusObj() );
        }    
        
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): The local file did not match the remote file or is missing thus it will be retrieved and updated" );
        
        // If we get here then the file was found but it is not up-to-date or
        // the file does not even exist locally, either way,.. we have to update it
        CORE::CString tmpFilePath = m_tempStorageRoot;
        CORE::AppendToPath( tmpFilePath, curFile->name );
        
        // make sure the path exists
        CORE::CreateDirs( CORE::StripFilename( tmpFilePath ) );
        
        // Try to create the file
        m_fileAccess = new CORE::CFileAccess( tmpFilePath, "wb" );
        m_fileAccess->Open();
        if ( m_fileAccess->Opened() && m_fileAccess->IsValid() )
        {
            // We now have write access to our temp new file.
            // We will now tell our retriever to get the file for us from the first location that works
            while ( m_curFileLocIndex < curFile->fileLocations.size() )
            {
                m_currentFileReceivedBytes = 0;
                if ( m_retriever.Retrieve( CORE::CUri( curFile->fileLocations[ m_curFileLocIndex ].URL ), *m_fileAccess ).HasAFuture() )
                {
                    // The data retrieval process has now commenced
                    // This is an a-sync operation, we wait for notification
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Started retrieving the file" );
                    NotifyObservers( FileRetrievalStartedEvent, CreateEventStatusObj() );
                    return true;
                }
                else
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Failed to retrieve the file" );
                    NotifyObservers( FileRetrievalErrorEvent, CreateEventStatusObj() );
                    
                    // if we get here then we where unable to start retrieving from the given URL
                    // We will try the next one,.. if there is a next one
                    if ( m_curFileLocIndex+1 >= curFile->fileLocations.size() )
                    {
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): File processing has been aborted, no alternative download locations remaining" );
                        
                        // we ran out of options,.. the end
                        delete m_fileAccess;
                        m_fileAccess = NULL;
                        NotifyObservers( FileListProcessingAbortedEvent, CreateEventStatusObj() );
                        m_stopSignalGiven = false;
                        m_isActive = false;
                        return false;
                    }
                }
                ++m_curFileLocIndex;
            }
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Unable to obtain write access to the temporary storage location,.. the end" );
            
            // We failed to obtain writable access to our temp file storage
            // we ran out of options,.. the end
            delete m_fileAccess;
            m_fileAccess = NULL;
            NotifyObservers( FileStorageErrorEvent, CreateEventStatusObj() );
            m_stopSignalGiven = false;
            m_isActive = false;                
            return false;
        }       
    }
    else
    {
        if ( m_fileList.size() < m_curFileIndex )
        {
            NotifyObservers( FileListProcessingAbortedEvent, CreateEventStatusObj() );
            m_stopSignalGiven = false;
            m_isActive = false;
        }
        else
        {
            // We are finished,.. engine stopping
            NotifyObservers( FileListProcessingCompleteEvent, CreateEventStatusObj() );
            m_isActive = false;
            m_stopSignalGiven = false;        
        }
    }
    
    return false;
}

/*-------------------------------------------------------------------------*/
    
void
CPatchSetFileEngine::Stop( void )
{GUCEF_TRACE;

    if ( m_isActive )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Stop called while active" );
        
        m_stopSignalGiven = true;
        
        if ( m_retriever.IsBusy() )
        {
            m_retriever.Cancel( true );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetFileEngine::IsActive( void ) const
{GUCEF_TRACE;

    return m_isActive;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetFileEngine::TryNextFileLocation( void )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Attempting to retrieve file from an alternate location" );
    
    m_fileAccess->Close();

    const TFileEntry& curFile = m_fileList[ m_curFileIndex ];
    do
    {
        // if we get here then we where unable to retrieve the file using the given URL
        // We will try the next one,.. if there is a next one
        if ( m_curFileLocIndex+1 >= curFile.fileLocations.size() )
        {
            // we ran out of options,.. the end
            NotifyObservers( FileListProcessingAbortedEvent, CreateEventStatusObj() );
            m_stopSignalGiven = false;
            m_isActive = false;
            return false;
        }

        ++m_curFileLocIndex;

        // The progress of the failed attempt does not count and the new attempt starts with an empty file
        m_totalBytesProcessed -= m_currentFileReceivedBytes;
        m_currentFileReceivedBytes = 0;
        m_fileAccess->Open();
        if ( m_retriever.Retrieve( CORE::CUri( curFile.fileLocations[ m_curFileLocIndex ].URL ), *m_fileAccess ).HasAFuture() )
        {
            // The data retrieval process has now commenced
            // This is an a-sync operation, we wait for notification
            return true;
        }
    }
    while ( m_curFileLocIndex < curFile.fileLocations.size() );
    
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetFileEngine::ProceedToNextFile( void )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Proceeding to the next file" );
    
    // Close the last file if it is still opened
    if ( m_fileAccess != NULL )
    {
        m_fileAccess->Close();
        delete m_fileAccess;
        m_fileAccess = NULL;
    }
    
    m_curFileLocIndex = 0;
    ++m_curFileIndex;
    
    return ProcessCurrentFile();
}

/*-------------------------------------------------------------------------*/

void
CPatchSetFileEngine::OnNotify( CORE::CNotifier* notifier                 ,
                               const CORE::CEvent& eventid               ,
                               CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( !m_stopSignalGiven && m_isActive )
    {
        // Call base-class version
        CORE::CObservingNotifier::OnNotify( notifier  ,
                                            eventid   ,
                                            eventdata );
                                            
        if ( notifier == &m_retriever )
        {
            if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalStartedEvent )
            {
                m_currentFileReceivedBytes = 0;
                return;
            }
            else
            if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalProgressEvent )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Data segment received" );
                const CORE::CAsyncUriResourceRetriever::TRetrievalEventData* progress = static_cast< CORE::CAsyncUriResourceRetriever::TRetrievalEventData* >( eventdata );
                m_totalBytesProcessed = ( m_totalBytesProcessed - m_currentFileReceivedBytes ) + progress->bytesTransferred;
                m_currentFileReceivedBytes = progress->bytesTransferred;
                NotifyObservers( FileRetrievalProgressEvent, CreateEventStatusObj() );
                return;
            }
            else
            if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalCompletedEvent )
            {
                const CORE::CAsyncUriResourceRetriever::TRetrievalEventData* completed = static_cast< CORE::CAsyncUriResourceRetriever::TRetrievalEventData* >( eventdata );
                m_totalBytesProcessed = ( m_totalBytesProcessed - m_currentFileReceivedBytes ) + completed->bytesTransferred;
                m_currentFileReceivedBytes = completed->bytesTransferred;

                // The file has to be written completely and released before it can be moved
                if ( m_fileAccess != NULL )
                    m_fileAccess->Close();

                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): All data for the file has been received" );
                
                // Check if we stored the file in an alternate location
                if ( m_tempStorageRoot != m_localRoot )
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): The new file is stored in a temporary location" );
                    
                    // safety check,.. just in case
                    assert( m_fileAccess != NULL );
                    if ( m_fileAccess != NULL )
                    {
                        // Create the destination path
                        CORE::CString destPath( m_localRoot );
                        CORE::AppendToPath( destPath, CORE::ExtractFilename( m_fileAccess->GetFilename() ) );
                        
                        // If a file already exists at the target location attempt to delete it
                        if ( CORE::FileExists( destPath ) )
                        {
                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): A file with the same name already exists at the destination location of the new file" );
                            
                            if ( !CORE::DeleteFile( destPath ) )
                            {
                                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Failed to delete an old file" );
                                
                                // We failed to delete the old file,..
                                NotifyObservers( LocalFileReplacementFailure, CreateEventStatusObj() );
                                
                                // This is a critical feature for implementing self-replacement of products that use the
                                // patching libraries which will be in use when we patch.
                                if ( m_stopOnReplacementFailure )
                                {
                                    m_stopSignalGiven = false;
                                    m_isActive = false;
                                    return;
                                }                            
                            }
                            else
                            {
                                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Deleted old file" );
                            }
                        }
                        
                        // Move the new file to the desired final location
                        if ( !CORE::MoveFile( m_fileAccess->GetFilename(), destPath, true ) )
                        {
                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Failed to move the new file to the location of the old file!" );
                            
                            // Moving the file failed,.. the end
                            NotifyObservers( FileStorageErrorEvent, CreateEventStatusObj() );
                            m_stopSignalGiven = false;
                            m_isActive = false;
                            return;                             
                        }
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Moved the new file to the location of the old file thus replacing it" );
                        
                        // We successfully replaced/patched a local file
                        NotifyObservers( LocalFileReplacedEvent, CreateEventStatusObj() );
                    }
                }
                
                ProceedToNextFile();
            }
            else
            if ( eventid == CORE::CAsyncUriResourceRetriever::RetrievalFailedEvent )
            {   
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): Unable to retrieve the file from the current remote file location" );
                TryNextFileLocation();
            }
        }
    }
    else
    {
        if ( m_stopSignalGiven )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetFileEngine(" + CORE::PointerToString( this ) + "): File processing has been aborted" );
            
            NotifyObservers( FileListProcessingAbortedEvent, CreateEventStatusObj() );        
            m_stopSignalGiven = false;
            m_isActive = false;
        }
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CPatchSetFileEngine::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CString typeName = "GUCEF::PATCHER::CPatchSetFileEngine";
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