/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_ESSENTIALS_H
#endif /* GUCEF_CORE_ESSENTIALS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#include "gucefCORE_CFileSystemDirectoryWatcher.h"

#if GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN

    #include <windows.h>
    #include <shlwapi.h>

    #pragma comment(lib, "shlwapi.lib")

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    #include <errno.h>
    #include <sys/types.h>
    #include <sys/inotify.h>
    #include <linux/limits.h>

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#if GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN

/*-------------------------------------------------------------------------*/

class GUCEF_HIDDEN OSSpecificDirectoryWatcher : public CObserver
{
    public:

    typedef CTEventHandlerFunctor< OSSpecificDirectoryWatcher > TEventCallback;
    typedef CIDirectoryWatcher::CDirWatchOptions CDirWatchOptions;


    class GUCEF_HIDDEN OverlappedIOCallbackObj
    {
        public:

        OSSpecificDirectoryWatcher* watcherObj;
        OVERLAPPED overlappedIO;
        CDynamicBuffer notifyBuffer;
        HANDLE dirHandle;
        std::wstring dirPathW;
        CString dirPath;
        DWORD notifyFilter;
        CDirWatchOptions watchOptions;

        OverlappedIOCallbackObj( void )
            : watcherObj( GUCEF_NULL )
            , overlappedIO()
            , notifyBuffer()
            , dirHandle( GUCEF_NULL )
            , dirPathW()
            , dirPath()
            , notifyFilter( 0 )
            , watchOptions()
        {GUCEF_TRACE;

            notifyBuffer.SetBufferSize( 64 * 1024 );
            ::ZeroMemory( &overlappedIO, sizeof(overlappedIO) );
            overlappedIO.hEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL );
        }

        ~OverlappedIOCallbackObj()
        {GUCEF_TRACE;

            if ( NULL != overlappedIO.hEvent )
            {
                ::CloseHandle( overlappedIO.hEvent );
                overlappedIO.hEvent = NULL;
            }
        }
    };

    typedef GUCEF::map< std::wstring, OverlappedIOCallbackObj > OverlappedIOCallbackObjMap;

    CTimer m_osPollingTimer;
    CFileSystemDirectoryWatcher* m_wrapper;
    OverlappedIOCallbackObjMap m_dirsToWatch;

    DWORD OptionsToNotifyFilter( const CDirWatchOptions& options ) const
    {GUCEF_TRACE;

        DWORD notifyFilter = 0;
        if ( options.watchForFileRenames || options.watchForFileCreation || options.watchForFileDeletion )
            notifyFilter |= FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_CREATION;
        if ( options.watchForFileModifications )
            notifyFilter |= FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_SECURITY;
        if ( options.watchForDirRenames || options.watchForDirCreation || options.watchForDirDeletion )
            notifyFilter |= FILE_NOTIFY_CHANGE_DIR_NAME;
        return notifyFilter;
    }

    bool RequestDirChangeNotifications( OverlappedIOCallbackObj& watchObj )
    {GUCEF_TRACE;

        // This call needs to be reissued after every APC.
        DWORD dwBytes=0;
        BOOL success = ::ReadDirectoryChangesW( watchObj.dirHandle,                                // handle to directory
                                                watchObj.notifyBuffer.GetBufferPtr(),              // read results buffer
                                                watchObj.notifyBuffer.GetBufferSize(),             // length of buffer
                                                watchObj.watchOptions.watchSubTree ? TRUE : FALSE, // monitoring option
                                                watchObj.notifyFilter,                             // filter conditions
                                                &dwBytes,                                          // bytes returned if not using overlapped I/O. In this case its a dummy.
                                                &watchObj.overlappedIO,                            // overlapped buffer
                                                NULL );                                            // completion routine, we use a waitable event instead

        if ( success != TRUE )
        {
            DWORD error = GetLastError();
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "OSSpecificDirectoryWatcher:RequestDirChangeNotifications: Failed to set change notification watch for dir \"" + watchObj.dirPath + "\". ErrorCode=" + CORE::UInt32ToString( error ) );
        }
        return success == TRUE;
    }

    bool AddDirToWatch( const CString& dirToWatch       ,
                        const CDirWatchOptions& options )
    {GUCEF_TRACE;

        if ( !DirExists( dirToWatch ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "OSSpecificDirectoryWatcher:AddDirToWatch: Unable to watch dir because it cannot be accessed or doesnt exist. Dir: " + dirToWatch );
            return false;
        }

        std::wstring dirToWatchW;
        if ( !Utf8toUtf16( dirToWatch, dirToWatchW ) )
            return false;

        HANDLE dirHandle = ::CreateFileW( dirToWatchW.c_str(),
                                          FILE_LIST_DIRECTORY,
                                          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                          NULL,
                                          OPEN_EXISTING,
                                          FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                                          NULL                      );
        if ( INVALID_HANDLE_VALUE == dirHandle )
        {
            DWORD error = GetLastError();
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "OSSpecificDirectoryWatcher:AddDirToWatch: Failed to obtain watch handle for dir \"" + dirToWatch + "\". ErrorCode=" + CORE::UInt32ToString( error ) );
            return false;
        }

        OverlappedIOCallbackObj& watchObj = m_dirsToWatch[ dirToWatchW ];
        watchObj.dirHandle = dirHandle;
        watchObj.dirPathW = dirToWatchW;
        watchObj.dirPath = dirToWatch;
        watchObj.notifyFilter = OptionsToNotifyFilter( options );
        watchObj.watchOptions = options;
        watchObj.watcherObj = this;

        if ( RequestDirChangeNotifications( watchObj ) )
        {
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "OSSpecificDirectoryWatcher:AddDirToWatch: Started watching dir: " + dirToWatch );
            m_osPollingTimer.SetEnabled( true );
            return true;
        }

        m_dirsToWatch.erase( dirToWatchW );
        ::CloseHandle( dirHandle );
        return false;
    }

    bool RemoveDirToWatch( const CString& dirToWatch )
    {GUCEF_TRACE;

        std::wstring dirToWatchW;
        if ( !Utf8toUtf16( dirToWatch, dirToWatchW ) )
            return false;

        OverlappedIOCallbackObjMap::iterator i = m_dirsToWatch.find( dirToWatchW );
        if ( i != m_dirsToWatch.end() )
        {
            OverlappedIOCallbackObj& watchObj = (*i).second;
            ::CloseHandle( watchObj.dirHandle );

            std::string dir;
            Utf16toUtf8( (*i).first, dir );
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "OSSpecificDirectoryWatcher:RemoveDirToWatch: Stopped watching dir: " + dir );

            m_dirsToWatch.erase( i );
            return true;
        }
        return false;
    }

    bool RemoveAllWatches( void )
    {GUCEF_TRACE;

        while ( !m_dirsToWatch.empty() )
        {
            OverlappedIOCallbackObjMap::iterator i = m_dirsToWatch.begin();
            OverlappedIOCallbackObj& watchObj = (*i).second;
            ::CloseHandle( watchObj.dirHandle );

            std::string dir;
            Utf16toUtf8( (*i).first, dir );
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "OSSpecificDirectoryWatcher:RemoveAllWatches: Stopped watching dir: " + dir );

            m_dirsToWatch.erase( i );
        }
        return true;
    }

    bool GetAllWatchedDirs( CString::StringSet& dirs ) const
    {GUCEF_TRACE;

        OverlappedIOCallbackObjMap::const_iterator i = m_dirsToWatch.begin();
        while ( i != m_dirsToWatch.end() )
        {
            const std::wstring& watchedDir = (*i).first;
            dirs.insert( ToString( watchedDir ) );

            ++i;
        }
        return true;
    }

    void ProcessNotifications( OverlappedIOCallbackObj& watchObj )
    {GUCEF_TRACE;

        BYTE* pBase = (BYTE*) watchObj.notifyBuffer.GetBufferPtr();

        std::string utf8OldFilenameBeforeRename;
        for (;;)
        {
            FILE_NOTIFY_INFORMATION& fni = (FILE_NOTIFY_INFORMATION&)*pBase;

            std::wstring filename( fni.FileName, fni.FileNameLength/sizeof(wchar_t) );

            // Handle a trailing backslash, such as for a root directory.
            if ( watchObj.dirPathW.size() > 1 && watchObj.dirPath[ (UInt32)(watchObj.dirPathW.size()-1) ] != L'\\' )
                filename = watchObj.dirPathW + L'\\' + filename;
            else
                filename = watchObj.dirPathW + filename;

            // If it could be a short filename, expand it.
            LPCWSTR wszFilename = PathFindFileNameW( filename.c_str() );
            int len = lstrlenW( wszFilename );

            // The maximum length of an 8.3 filename is twelve, including the dot.
            if ( len <= 12 && wcschr( filename.c_str(), L'~') )
            {
                // Convert to the long filename form. Unfortunately, this
                // does not work for deletions, so it's an imperfect fix.
                wchar_t wbuf[ MAX_PATH ];
                if ( ::GetLongPathNameW( filename.c_str(), wbuf, _countof(wbuf) ) > 0 )
                    filename = wbuf;
            }

            std::string utf8Filename;
            if ( Utf16toUtf8( filename, utf8Filename ) )
            {
                switch ( fni.Action )
                {
                    case FILE_ACTION_ADDED :
                    {
                        GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled FILE_ACTION_ADDED for file: " + utf8Filename );
                        if ( watchObj.watchOptions.watchForFileCreation )
                        {
                            CIDirectoryWatcher::TFileCreatedEventData eData( utf8Filename );
                            if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::FileCreatedEvent, &eData ) ) return;
                        }
                        break;
                    }
                    case FILE_ACTION_REMOVED :
                    {
                        GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled FILE_ACTION_REMOVED for file: " + utf8Filename );
                        if ( watchObj.watchOptions.watchForFileDeletion )
                        {
                            CIDirectoryWatcher::TFileDeletedEventData eData( utf8Filename );
                            if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::FileDeletedEvent, &eData ) ) return;
                        }
                        break;
                    }
                    case FILE_ACTION_MODIFIED :
                    {
                        GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled FILE_ACTION_MODIFIED for file: " + utf8Filename );
                        if ( watchObj.watchOptions.watchForFileModifications )
                        {
                            CIDirectoryWatcher::TFileModifiedEventData eData( utf8Filename );
                            if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::FileModifiedEvent, &eData ) ) return;
                        }
                        break;
                    }
                    case FILE_ACTION_RENAMED_OLD_NAME :
                    {
                        GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled FILE_ACTION_RENAMED_OLD_NAME for file: " + utf8Filename );
                        if ( watchObj.watchOptions.watchForFileRenames )
                        {
                            utf8OldFilenameBeforeRename = utf8Filename;
                        }
                        break;
                    }
                    case FILE_ACTION_RENAMED_NEW_NAME :
                    {
                        GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled FILE_ACTION_RENAMED_NEW_NAME for file: " + utf8Filename );
                        if ( watchObj.watchOptions.watchForFileRenames )
                        {
                            struct CIDirectoryWatcher::SFileRenamedEventInfo info = { utf8OldFilenameBeforeRename, utf8Filename };
                            CIDirectoryWatcher::TFileRenamedEventData eData( info );
                            if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::FileRenamedEvent, &eData ) ) return;
                            utf8OldFilenameBeforeRename.clear();
                        }
                        break;
                    }
                }
            }

            if ( 0 == fni.NextEntryOffset )
                break;
            pBase += fni.NextEntryOffset;
        }
    }

    void RegisterEventHandlers( void )
    {GUCEF_TRACE;

        TEventCallback callback( this, &OSSpecificDirectoryWatcher::OnDirWatchPollingCycle );
        SubscribeTo( &m_osPollingTimer              ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }

    void OnDirWatchPollingCycle( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
    {GUCEF_TRACE;

        OverlappedIOCallbackObjMap::iterator i = m_dirsToWatch.begin();
        while ( i != m_dirsToWatch.end() )
        {
            OverlappedIOCallbackObj& watchObj = (*i).second;
            DWORD waitResult = ::WaitForSingleObject( watchObj.overlappedIO.hEvent, 0 );
            if ( WAIT_OBJECT_0 == waitResult )
            {
                GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled an overlapped IO event" );

                DWORD numberOfBytesTransferred = 0;
                if ( TRUE == ::GetOverlappedResult( watchObj.dirHandle, &watchObj.overlappedIO, &numberOfBytesTransferred, TRUE ) )
                {
                    // Check if we overflowed the notification buffer
                    // Sadly this means the notifications that were sent are now gone if this happens.
                    if ( 0 != numberOfBytesTransferred )
                    {
                        RequestDirChangeNotifications( watchObj );
                        ProcessNotifications( watchObj );
                    }
                    else
                    {
                        GUCEF_WARNING_LOG( LOGLEVEL_IMPORTANT, "OSSpecificDirectoryWatcher: The event buffer was not large enough (" +
                                CORE::UInt32ToString( watchObj.notifyBuffer.GetBufferSize() ) + " bytes) to hold all the OS notifications. Event notifications have been lost! Auto-enlarging the buffer x2" );

                        watchObj.notifyBuffer.SetBufferSize( watchObj.notifyBuffer.GetBufferSize() * 2 );
                        RequestDirChangeNotifications( watchObj );
                    }
                }
            }
            ++i;
        }
    }

    OSSpecificDirectoryWatcher( CFileSystemDirectoryWatcher* wrapper )
        : m_osPollingTimer()
        , m_wrapper( wrapper )
        , m_dirsToWatch()
    {GUCEF_TRACE;

        RegisterEventHandlers();
    }

    OSSpecificDirectoryWatcher( CFileSystemDirectoryWatcher* wrapper ,
                                PulseGeneratorPtr pulseGenerator     )
        : m_osPollingTimer( pulseGenerator )
        , m_wrapper( wrapper )
        , m_dirsToWatch()
    {GUCEF_TRACE;

        RegisterEventHandlers();
    }

    OSSpecificDirectoryWatcher( const OSSpecificDirectoryWatcher& src ,
                                CFileSystemDirectoryWatcher* wrapper  )
        : m_osPollingTimer( src.m_osPollingTimer )
        , m_wrapper( wrapper )
        , m_dirsToWatch()
    {GUCEF_TRACE;

        OverlappedIOCallbackObjMap::const_iterator i = src.m_dirsToWatch.begin();
        while ( i != src.m_dirsToWatch.end() )
        {
            const OverlappedIOCallbackObj& watchObj = (*i).second;
            AddDirToWatch( watchObj.dirPath, watchObj.watchOptions );
            ++i;
        }
    }

    ~OSSpecificDirectoryWatcher()
    {GUCEF_TRACE;

        RemoveAllWatches();
    }
};

/*-------------------------------------------------------------------------*/

#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

/*-------------------------------------------------------------------------*/

#define INOTIFY_EVENT_SIZE                  ( sizeof (struct inotify_event) )
#define INOTIFY_EVENT_BUF_LEN               ( 64 * ( INOTIFY_EVENT_SIZE + PATH_MAX + 1 ) )
#define INOTIFY_POLLING_INTERVAL_IN_MS      25

class GUCEF_HIDDEN OSSpecificDirectoryWatcher : public CObserver
{
    public:

    typedef CTEventHandlerFunctor< OSSpecificDirectoryWatcher > TEventCallback;
    typedef CIDirectoryWatcher::CDirWatchOptions CDirWatchOptions;

    class GUCEF_HIDDEN WatchEntry
    {
        public:
        CDirWatchOptions m_options;

        WatchEntry( void )
            : m_options()
        {GUCEF_TRACE;
        }
    };
    typedef GUCEF::map< Int32, WatchEntry > TWatchEntryMap;
    typedef GUCEF::map< CUtf8String, Int32 > TWatchDescriptorMap;

    struct inotify_event_stub
    {
      int wd;		    /* Watch descriptor.  */
      uint32_t mask;	/* Watch mask.  */
      uint32_t cookie;	/* Cookie to synchronize two events.  */
    };

    class GUCEF_HIDDEN WatchCookieEntry
    {
        public:
        struct inotify_event_stub m_event;
        CUtf8String m_eventItemName;

        WatchCookieEntry( void )
            : m_event()
            , m_eventItemName()
        {GUCEF_TRACE;

            memset( &m_event, 0, sizeof m_event );
        }
    };
    typedef GUCEF::map< UInt32, WatchCookieEntry > TWatchCookieEntryMap;

    CTimer m_osPollingTimer;
    CFileSystemDirectoryWatcher* m_wrapper;
    TWatchEntryMap m_dirsToWatch;
    TWatchDescriptorMap m_wdLookupMap;
    TWatchCookieEntryMap m_watchCookieMap;
    int m_inotify_fd;
    CDynamicBuffer m_notifyEventBuffer;
    struct timeval m_select_timeout;

    uint32_t OptionsToNotifyFilter( const CDirWatchOptions& options ) const
    {GUCEF_TRACE;

        uint32_t notifyFilter = 0;
        if ( options.watchForFileCreation || options.watchForDirCreation )
            notifyFilter |= IN_CREATE;
        if ( options.watchForFileDeletion || options.watchForDirDeletion )
            notifyFilter |= IN_DELETE | IN_DELETE_SELF;
        if ( options.watchForFileRenames || options.watchForDirRenames )
            notifyFilter |= IN_MOVE_SELF | IN_MOVED_FROM | IN_MOVED_TO;
        if ( options.watchForFileModifications || options.watchForDirModifications )
            notifyFilter |= IN_ACCESS | IN_ATTRIB | IN_CLOSE_WRITE | IN_CLOSE_NOWRITE | IN_MODIFY | IN_OPEN;
        return notifyFilter;
    }

    void RegisterEventHandlers( void )
    {GUCEF_TRACE;

        TEventCallback callback( this, &OSSpecificDirectoryWatcher::OnDirWatchPollingCycle );
        SubscribeTo( &m_osPollingTimer              ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }

    bool AddDirToWatch( const CString& dirToWatch       ,
                        const CDirWatchOptions& options )
    {GUCEF_TRACE;

        if ( -1 == m_inotify_fd )
            return false;

        CUtf8String utfDirToWatch = ToUtf8String( dirToWatch );
        uint32_t notifyFilter = OptionsToNotifyFilter( options );
        int watchDescriptor = inotify_add_watch( m_inotify_fd, utfDirToWatch.C_String(), notifyFilter );
        if ( -1 != watchDescriptor )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "AddDirToWatch: Added watch of dir \"" + dirToWatch + "\"" );

            m_wdLookupMap[ utfDirToWatch ] = watchDescriptor;
            WatchEntry& entry = m_dirsToWatch[ watchDescriptor ];
            entry.m_options = options;

            m_osPollingTimer.SetEnabled( true );
            return true;
        }
        else
        {

            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "AddDirToWatch: Failed to add watch to dir \"" + dirToWatch + "\"" );
            return false;
        }
    }

    bool RemoveDirToWatch( const CString& dirToWatch )
    {GUCEF_TRACE;

        if ( -1 == m_inotify_fd )
            return false;

        CUtf8String utfDirToWatch = ToUtf8String( dirToWatch );
        TWatchDescriptorMap::iterator i = m_wdLookupMap.find( utfDirToWatch );
        if ( i != m_wdLookupMap.end() )
        {
            TWatchEntryMap::iterator n = m_dirsToWatch.find( (*i).second );
            if ( n != m_dirsToWatch.end() )
            {
                int failed = inotify_rm_watch( m_inotify_fd, (*n).first );
                if ( 0 == failed )
                {
                    m_dirsToWatch.erase( n );
                    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "RemoveDirToWatch: Removed watch of dir \"" + dirToWatch + "\"" );

                    if ( m_dirsToWatch.empty() )
                        m_osPollingTimer.SetEnabled( false );

                    return true;
                }
                else
                {
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "RemoveDirToWatch: Failed to remove watch of dir \"" + dirToWatch + "\"" );
                }
            }
            else
            {
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "RemoveDirToWatch: Request to remove watch of dir \"" + dirToWatch + "\" but we did not the watch entry for the watch descriptor" );
                m_wdLookupMap.erase( i );
            }
        }
        else
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "RemoveDirToWatch: Request to remove watch of dir \"" + dirToWatch + "\" but we did not have any such watch" );
        }
        return false;
    }

    bool RemoveAllWatches( void )
    {GUCEF_TRACE;

        if ( -1 == m_inotify_fd )
            return false;

        m_osPollingTimer.SetEnabled( false );

        int failCount = 0;
        TWatchDescriptorMap::iterator i = m_wdLookupMap.begin();
        while ( i != m_wdLookupMap.end() )
        {
            TWatchEntryMap::iterator n = m_dirsToWatch.find( (*i).second );
            if ( n != m_dirsToWatch.end() )
            {
                int failed = inotify_rm_watch( m_inotify_fd, (*n).first );
                if ( 0 != failed )
                {
                    ++failCount;
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "RemoveAllWatches: Failed to remove watch of dir \"" + ToString( (*i).first ) + "\"" );
                }
                else
                {
                    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "RemoveAllWatches: Removed watch of dir \"" + ToString( (*i).first ) + "\"" );
                }
                ++n;
            }
            ++i;
        }

        m_wdLookupMap.clear();
        m_dirsToWatch.clear();

        return 0 == failCount;
    }

    bool GetAllWatchedDirs( CString::StringSet& dirs ) const
    {GUCEF_TRACE;

        TWatchDescriptorMap::const_iterator i = m_wdLookupMap.begin();
        while ( i != m_wdLookupMap.end() )
        {
            const CUtf8String& watchedDir = (*i).first;
            dirs.insert( ToString( watchedDir ) );
            ++i;
        }
        return true;
    }

    void OnDirWatchPollingCycle( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
    {GUCEF_TRACE;

        fd_set rfds;
        FD_ZERO( &rfds );
        FD_SET( m_inotify_fd, &rfds );

        int selectResult = select( m_inotify_fd + 1, &rfds, NULL, NULL, &m_select_timeout );
        if ( selectResult == -1 )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "OSSpecificDirectoryWatcher:OnDirWatchPollingCycle: select() failed" );
            return;
        }

        if ( 0 == FD_ISSET( m_inotify_fd, &rfds ) )
            return; // nothing to do

        int totalBytes = read( m_inotify_fd, m_notifyEventBuffer.GetBufferPtr(), INOTIFY_EVENT_BUF_LEN );
        if ( totalBytes < 0 )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "OSSpecificDirectoryWatcher:OnDirWatchPollingCycle: Failed to read inotify event messages" );
            return;
        }
        m_notifyEventBuffer.SetDataSize( (UInt32) totalBytes );

        int byteOffset=0;
        while ( byteOffset < totalBytes )
        {
            struct inotify_event* event = (struct inotify_event*) m_notifyEventBuffer.GetBufferPtr( (UInt32) byteOffset );
            if ( event->len > 0 )
            {
                if ( event->mask & IN_Q_OVERFLOW )
                {
                    GUCEF_ERROR_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled IN_Q_OVERFLOW, enlarging the event buffer to twice its size from " + ToString( m_notifyEventBuffer.GetBufferSize() ) );
                    if ( !m_notifyEventBuffer.SetBufferSize( m_notifyEventBuffer.GetBufferSize() * 2, false ) )
                    {
                        GUCEF_ERROR_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: Failed to double the size of the event buffer" );
                    }
                }

                // Obtain the associated data
                TWatchEntryMap::iterator n = m_dirsToWatch.find( event->wd );
                if ( n != m_dirsToWatch.end() )
                {
                    WatchEntry& watchObj = (*n).second;
                    CUtf8String utf8Name = event->name;

                    // Is this a change to a directory or a file?
                    if ( event->mask & IN_ISDIR )
                    {
                        if ( event->mask & IN_CREATE )
                        {
                            GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled IN_CREATE for directory: " + utf8Name );

                            if ( watchObj.m_options.watchForDirCreation )
                            {
                                CIDirectoryWatcher::TDirCreatedEventData eData( utf8Name );
                                if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::DirCreatedEvent, &eData ) ) return;
                            }
                        }
                        else
                        if ( event->mask & IN_DELETE || event->mask & IN_DELETE_SELF )
                        {
                            GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled IN_DELETE for directory: " + utf8Name );

                            if ( watchObj.m_options.watchForDirDeletion )
                            {
                                CIDirectoryWatcher::TDirDeletedEventData eData( utf8Name );
                                if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::DirDeletedEvent, &eData ) ) return;
                            }
                        }
                        else
                        if ( event->mask & IN_MODIFY || event->mask & IN_ATTRIB || event->mask & IN_CLOSE_WRITE || event->mask & IN_CLOSE_NOWRITE || event->mask & IN_OPEN )
                        {
                            GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled IN_MODIFY or IN_ATTRIB or IN_CLOSE_WRITE or IN_CLOSE_NOWRITE or IN_OPEN for directory: " + utf8Name );

                            if ( watchObj.m_options.watchForDirModifications )
                            {
                                CIDirectoryWatcher::TDirModifiedEventData eData( utf8Name );
                                if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::DirModifiedEvent, &eData ) ) return;
                            }
                        }
                        else
                        if ( event->mask & IN_MOVED_FROM || event->mask & IN_MOVE_SELF )
                        {
                            GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled IN_MOVED_FROM or IN_MOVE_SELF for directory: " + utf8Name );

                            if ( watchObj.m_options.watchForDirRenames )
                            {
                                WatchCookieEntry& cookieEntry = m_watchCookieMap[ event->cookie ];
                                memcpy( &cookieEntry.m_event, event, sizeof cookieEntry.m_event );
                                cookieEntry.m_eventItemName = event->name;
                            }
                        }
                        else
                        if ( event->mask & IN_MOVED_TO )
                        {
                            GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled IN_MOVED_TO for directory: " + utf8Name );

                            if ( watchObj.m_options.watchForDirRenames )
                            {
                                TWatchCookieEntryMap::iterator k = m_watchCookieMap.find( event->cookie );
                                if ( k != m_watchCookieMap.end() )
                                {
                                    CIDirectoryWatcher::TDirRenamedEventData eData;
                                    eData.GetData().oldDirName = ToString( (*k).second.m_eventItemName );
                                    eData.GetData().newDirName = ToString( utf8Name );

                                    m_watchCookieMap.erase( k );
                                    if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::DirRenamedEvent, &eData ) ) return;
                                }
                            }
                        }
                    }
                    else
                    {
                        if ( event->mask & IN_CREATE )
                        {
                            GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled IN_CREATE for file: " + utf8Name );

                            if ( watchObj.m_options.watchForFileCreation )
                            {
                                CIDirectoryWatcher::TFileCreatedEventData eData( utf8Name );
                                if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::FileCreatedEvent, &eData ) ) return;
                            }
                        }
                        else
                        if ( event->mask & IN_DELETE || event->mask & IN_DELETE_SELF )
                        {
                            GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled IN_DELETE for file: " + utf8Name );

                            if ( watchObj.m_options.watchForFileDeletion )
                            {
                                CIDirectoryWatcher::TFileDeletedEventData eData( utf8Name );
                                if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::FileDeletedEvent, &eData ) ) return;
                            }
                        }
                        else
                        if ( event->mask & IN_MODIFY || event->mask & IN_ATTRIB || event->mask & IN_CLOSE_WRITE || event->mask & IN_CLOSE_NOWRITE || event->mask & IN_OPEN )
                        {
                            GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled IN_MODIFY or IN_ATTRIB or IN_CLOSE_WRITE or IN_CLOSE_NOWRITE or IN_OPEN for file: " + utf8Name );

                            if ( watchObj.m_options.watchForFileModifications )
                            {
                                CIDirectoryWatcher::TFileModifiedEventData eData( utf8Name );
                                if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::FileModifiedEvent, &eData ) ) return;
                            }
                        }
                        else
                        if ( event->mask & IN_MOVED_FROM || event->mask & IN_MOVE_SELF )
                        {
                            GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled IN_MOVED_FROM or IN_MOVE_SELF for file: " + utf8Name );

                            if ( watchObj.m_options.watchForDirRenames )
                            {
                                WatchCookieEntry& cookieEntry = m_watchCookieMap[ event->cookie ];
                                memcpy( &cookieEntry.m_event, event, sizeof cookieEntry.m_event );
                                cookieEntry.m_eventItemName = event->name;
                            }
                        }
                        else
                        if ( event->mask & IN_MOVED_TO )
                        {
                            GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "OSSpecificDirectoryWatcher: OS Signaled IN_MOVED_TO for directory: " + utf8Name );

                            if ( watchObj.m_options.watchForFileRenames )
                            {
                                TWatchCookieEntryMap::iterator k = m_watchCookieMap.find( event->cookie );
                                if ( k != m_watchCookieMap.end() )
                                {
                                    CIDirectoryWatcher::TFileRenamedEventData eData;
                                    eData.GetData().oldFilename = ToString( (*k).second.m_eventItemName );
                                    eData.GetData().newFilename = ToString( utf8Name );

                                    m_watchCookieMap.erase( k );
                                    if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::FileRenamedEvent, &eData ) ) return;
                                }
                            }
                        }
                    }

                    if ( event->mask & IN_DELETE_SELF || event->mask & IN_IGNORED || event->mask & IN_UNMOUNT )
                        RemoveDirToWatch( utf8Name );
                }
            }

            // Advance by structure header plus variable payload bytes to the next message header
            byteOffset += INOTIFY_EVENT_SIZE + event->len;
        }
    }

    bool Init( void )
    {
        RegisterEventHandlers();

        int fd = inotify_init();
        if ( fd < 0 )
            return false;
        m_inotify_fd = fd;

        if ( !m_notifyEventBuffer.SetBufferSize( INOTIFY_EVENT_BUF_LEN ) )
        {
            close( m_inotify_fd );
            m_inotify_fd = -1;
            return false;
        }

        m_select_timeout.tv_sec = 0;
        m_select_timeout.tv_usec = 0;
        m_osPollingTimer.SetInterval( INOTIFY_POLLING_INTERVAL_IN_MS );

        return true;
    }

    OSSpecificDirectoryWatcher( CFileSystemDirectoryWatcher* wrapper )
        : m_osPollingTimer()
        , m_wrapper( wrapper )
        , m_dirsToWatch()
        , m_wdLookupMap()
        , m_watchCookieMap()
        , m_inotify_fd( -1 )
    {GUCEF_TRACE;

        Init();
    }

    OSSpecificDirectoryWatcher( CFileSystemDirectoryWatcher* wrapper ,
                                PulseGeneratorPtr pulseGenerator     )
        : m_osPollingTimer( pulseGenerator )
        , m_wrapper( wrapper )
        , m_dirsToWatch()
        , m_wdLookupMap()
        , m_watchCookieMap()
        , m_inotify_fd( -1 )
    {GUCEF_TRACE;

        Init();
    }

    OSSpecificDirectoryWatcher( const OSSpecificDirectoryWatcher& src ,
                                CFileSystemDirectoryWatcher* wrapper  )
        : m_osPollingTimer( src.m_osPollingTimer )
        , m_wrapper( wrapper )
        , m_dirsToWatch()
        , m_wdLookupMap()
        , m_watchCookieMap()
        , m_inotify_fd( -1 )
    {GUCEF_TRACE;

        if ( Init() )
        {
            TWatchDescriptorMap::const_iterator i = src.m_wdLookupMap.begin();
            while ( i != src.m_wdLookupMap.end() )
            {
                Int32 watchDescriptor = (*i).second;
                TWatchEntryMap::const_iterator n = src.m_dirsToWatch.find( watchDescriptor );
                if ( n != src.m_dirsToWatch.end() )
                {
                    const WatchEntry& watchObj = (*n).second;
                    AddDirToWatch( (*i).first, watchObj.m_options );
                }
                ++i;
            }
        }
    }

    ~OSSpecificDirectoryWatcher()
    {GUCEF_TRACE;

        RemoveAllWatches();

        if ( m_inotify_fd >= 0 )
        {
            close( m_inotify_fd );
            m_inotify_fd = -1;
        }
    }
};

/*-------------------------------------------------------------------------*/

#else

/*-------------------------------------------------------------------------*/

// Dummy that doesnt do anything as a catch all for unsupported OS
class GUCEF_HIDDEN OSSpecificDirectoryWatcher
{
    public:

    typedef CFileSystemDirectoryWatcher::CDirWatchOptions CDirWatchOptions;

    bool AddDirToWatch( const CString& dirToWatch       ,
                        const CDirWatchOptions& options )
    {GUCEF_TRACE;
        return false;
    }

    bool RemoveDirToWatch( const CString& dirToWatch )
    {GUCEF_TRACE;
        return false;
    }

    bool RemoveAllWatches( void )
    {GUCEF_TRACE;
        return false;
    }

    bool GetAllWatchedDirs( CString::StringSet& dirs ) const
    {GUCEF_TRACE;
        return false;
    }

    OSSpecificDirectoryWatcher( CFileSystemDirectoryWatcher* wrapper )
    {GUCEF_TRACE;

    }

    OSSpecificDirectoryWatcher( CFileSystemDirectoryWatcher* wrapper ,
                                PulseGeneratorPtr pulseGenerator     )
    {GUCEF_TRACE;

    } 

    OSSpecificDirectoryWatcher( const OSSpecificDirectoryWatcher& src ,
                                CFileSystemDirectoryWatcher* wrapper  )
    {GUCEF_TRACE;

    }
};

/*-------------------------------------------------------------------------*/

#endif

/*-------------------------------------------------------------------------*/

bool
CFileSystemDirectoryWatcher::AddDirToWatch( const CString& dirToWatch       ,
                                            const CDirWatchOptions& options ,
                                            bool tryToCreatePathIfNotExists )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_osSpecificImpl )
    {
        CString fullDirToWatch = RelativePath( dirToWatch );

        if ( tryToCreatePathIfNotExists && !DirExists( fullDirToWatch ) )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemDirectoryWatcher:AddDirToWatch: Directory does not exist, will try to create it first: " + fullDirToWatch );
            if ( CreateDirs( fullDirToWatch ) )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemDirectoryWatcher:AddDirToWatch: Directory created: " + fullDirToWatch );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSystemDirectoryWatcher:AddDirToWatch: Failed to create directory: " + fullDirToWatch );
            }
        }

        if ( m_osSpecificImpl->AddDirToWatch( fullDirToWatch, options ) )
        {
            TStartedWatchingDirectoryEventData eData( fullDirToWatch );
            NotifyObservers( StartedWatchingDirectoryEvent, &eData );
            return true;
        }
        return false;
    }
    else
        return false;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDirectoryWatcher::RemoveDirToWatch( const CString& dirToWatch )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_osSpecificImpl )
    {
        if ( m_osSpecificImpl->RemoveDirToWatch( dirToWatch ) )
        {
            TStoppedWatchingDirectoryEventData eData( dirToWatch );
            NotifyObservers( StoppedWatchingDirectoryEvent, &eData );
            return true;
        }
        return false;
    }
    else
        return false;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDirectoryWatcher::RemoveAllWatches( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_osSpecificImpl )
        return m_osSpecificImpl->RemoveAllWatches();
    else
        return false;
}

/*-------------------------------------------------------------------------*/

bool
CFileSystemDirectoryWatcher::GetAllWatchedDirs( CString::StringSet& dirs ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_osSpecificImpl )
    {
        if ( m_osSpecificImpl->GetAllWatchedDirs( dirs ) )
        {
            return true;
        }
        return false;
    }
    else
        return false;
}

/*-------------------------------------------------------------------------*/

CFileSystemDirectoryWatcher::CFileSystemDirectoryWatcher( void )
    : CObservingNotifier()
    , CIDirectoryWatcher()
    , m_osSpecificImpl( GUCEF_NEW OSSpecificDirectoryWatcher( this ) )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileSystemDirectoryWatcher::CFileSystemDirectoryWatcher( const CFileSystemDirectoryWatcher& src )
    : CObservingNotifier()
    , CIDirectoryWatcher( src )
    , m_osSpecificImpl( GUCEF_NEW OSSpecificDirectoryWatcher( *src.m_osSpecificImpl, this ) )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileSystemDirectoryWatcher::CFileSystemDirectoryWatcher( PulseGeneratorPtr pulseGenerator )
    : CObservingNotifier()
    , CIDirectoryWatcher()
    , m_osSpecificImpl( GUCEF_NEW OSSpecificDirectoryWatcher( this, pulseGenerator ) )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileSystemDirectoryWatcher::~CFileSystemDirectoryWatcher()
{GUCEF_TRACE;

    GUCEF_DELETE m_osSpecificImpl;
    m_osSpecificImpl = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CFileSystemDirectoryWatcher&
CFileSystemDirectoryWatcher::operator=( const CFileSystemDirectoryWatcher& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CIDirectoryWatcher::operator=( src );

        GUCEF_DELETE m_osSpecificImpl;
        m_osSpecificImpl = GUCEF_NULL;

        m_osSpecificImpl = GUCEF_NEW OSSpecificDirectoryWatcher( *src.m_osSpecificImpl, this );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

const CString&
CFileSystemDirectoryWatcher::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CFileSystemDirectoryWatcher";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
