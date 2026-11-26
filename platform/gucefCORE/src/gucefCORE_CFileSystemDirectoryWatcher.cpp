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

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS )

    #include <CoreServices/CoreServices.h>      /* FSEvents */
    #include <dispatch/dispatch.h>
    #include <sys/stat.h>
    #include <limits.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <string>
    #include <vector>
    #include <set>

    #ifndef GUCEF_MT_CSCOPEMUTEX_H
    #include "gucefMT_CScopeMutex.h"
    #define GUCEF_MT_CSCOPEMUTEX_H
    #endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ? */

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
            overlappedIO.hEvent = ::CreateEvent( GUCEF_NULL, FALSE, FALSE, GUCEF_NULL );
        }

        ~OverlappedIOCallbackObj()
        {GUCEF_TRACE;

            if ( GUCEF_NULL != overlappedIO.hEvent )
            {
                ::CloseHandle( overlappedIO.hEvent );
                overlappedIO.hEvent = GUCEF_NULL;
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
                                                GUCEF_NULL );                                            // completion routine, we use a waitable event instead

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
                                          GUCEF_NULL,
                                          OPEN_EXISTING,
                                          FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                                          GUCEF_NULL                      );
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
                    m_wdLookupMap.erase( i );
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

        int selectResult = select( m_inotify_fd + 1, &rfds, GUCEF_NULL, GUCEF_NULL, &m_select_timeout );
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

#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS )

/*-------------------------------------------------------------------------*/

#define FSEVENTS_POLLING_INTERVAL_IN_MS     25
#define FSEVENTS_LATENCY_IN_SECONDS         0.025

/**
 *  macOS watches the file system with FSEvents. It has no inotify's view of a single directory: it reports what happens at
 *  paths, for a whole tree, from a thread of its own. What it reports is made to look like what inotify gives: a watch is
 *  of the files and directories directly in a directory (the whole tree when the options ask for the sub tree, then the names
 *  are paths relative to the watched directory), and the events are handed over from the polling timer, in the thread of the
 *  application, as for the other platforms.
 *  FSEvents has no cookie to tie the 2 halves of a rename together: the old name (which is gone) and the new name (which is there)
 *  are reported one after the other, and those are paired up.
 */
class GUCEF_HIDDEN OSSpecificDirectoryWatcher : public CObserver
{
    public:

    typedef CTEventHandlerFunctor< OSSpecificDirectoryWatcher > TEventCallback;
    typedef CIDirectoryWatcher::CDirWatchOptions CDirWatchOptions;

    enum EChangeKind
    {
        CHANGE_FILE_CREATED ,
        CHANGE_FILE_DELETED ,
        CHANGE_FILE_MODIFIED ,
        CHANGE_FILE_RENAMED ,
        CHANGE_DIR_CREATED ,
        CHANGE_DIR_DELETED ,
        CHANGE_DIR_MODIFIED ,
        CHANGE_DIR_RENAMED
    };

    struct SChange
    {
        EChangeKind kind;
        CUtf8String name;           // what changed, for a rename the new name
        CUtf8String oldName;        // for a rename the name it had
        CUtf8String watchedDir;     // the watched directory the change was in, as it was given to AddDirToWatch()
    };
    typedef std::vector< SChange > TChangeVector;

    class GUCEF_HIDDEN WatchEntry
    {
        public:

        CDirWatchOptions m_options;
        CUtf8String m_watchedDir;           // as given
        std::string m_realPath;             // as the file system reports paths: no symbolic links and no trailing slash
        FSEventStreamRef m_stream;
        OSSpecificDirectoryWatcher* m_owner;
        std::set< std::string > m_announcedPaths;   // paths whose creation was told: FSEvents keeps the 'created' flag on every later event of a path. Only used by the thread of FSEvents

        WatchEntry( void )
            : m_options()
            , m_watchedDir()
            , m_realPath()
            , m_stream( NULL )
            , m_owner( GUCEF_NULL )
            , m_announcedPaths()
        {GUCEF_TRACE;
        }
    };
    typedef GUCEF::map< CUtf8String, WatchEntry* > TWatchEntryMap;

    CTimer m_osPollingTimer;
    CFileSystemDirectoryWatcher* m_wrapper;
    TWatchEntryMap m_dirsToWatch;
    dispatch_queue_t m_queue;
    MT::CMutex m_pendingLock;
    TChangeVector m_pendingChanges;

    static bool PathExists( const std::string& path )
    {GUCEF_TRACE;

        struct stat info;
        return 0 == ::lstat( path.c_str(), &info );
    }

    /** What FSEvents told, from its own thread: made into changes of the watched directory and queued for the polling timer */
    static void StreamCallback( ConstFSEventStreamRef stream                 ,
                                void* clientCallBackInfo                     ,
                                size_t numEvents                             ,
                                void* eventPaths                             ,
                                const FSEventStreamEventFlags eventFlags[]   ,
                                const FSEventStreamEventId eventIds[]        )
    {
        WatchEntry* entry = static_cast< WatchEntry* >( clientCallBackInfo );
        if ( GUCEF_NULL == entry || GUCEF_NULL == entry->m_owner )
            return;

        const char* const* paths = static_cast< const char* const* >( eventPaths );
        TChangeVector changes;
        bool haveRenamedFrom = false;
        std::string renamedFrom;
        bool renamedFromIsDir = false;

        for ( size_t i=0; i<numEvents; ++i )
        {
            const FSEventStreamEventFlags flags = eventFlags[ i ];
            std::string path = paths[ i ];

            if ( 0 != ( flags & ( kFSEventStreamEventFlagMustScanSubDirs | kFSEventStreamEventFlagUserDropped | kFSEventStreamEventFlagKernelDropped |
                                  kFSEventStreamEventFlagRootChanged | kFSEventStreamEventFlagMount | kFSEventStreamEventFlagUnmount | kFSEventStreamEventFlagHistoryDone ) ) )
                continue;

            // what is in the watched directory: a path below it
            const std::string& root = entry->m_realPath;
            if ( path.size() <= root.size() + 1 || 0 != path.compare( 0, root.size(), root ) || '/' != path[ root.size() ] )
                continue;
            std::string relative = path.substr( root.size() + 1 );
            if ( !entry->m_options.watchSubTree && std::string::npos != relative.find( '/' ) )
                continue;

            const bool isDir = 0 != ( flags & kFSEventStreamEventFlagItemIsDir );

            // What FSEvents reports for a path is what has happened to it so far, and not only since the last report: the flags of
            // an earlier create stay on the events of the path after it. What an event is about now is told by the file system:
            // a path that is gone was deleted (or moved away), one that is there and has not been told about is new.
            const bool exists = PathExists( path );
            if ( !exists && 0 != ( flags & kFSEventStreamEventFlagItemRemoved ) )
            {
                // a path that is not there (any more) and was removed: deleted, also when it has been renamed before
                SChange change;
                change.kind = isDir ? CHANGE_DIR_DELETED : CHANGE_FILE_DELETED;
                change.name = relative.c_str();
                change.watchedDir = entry->m_watchedDir;
                changes.push_back( change );
                entry->m_announcedPaths.erase( path );
                continue;
            }
            else
            if ( 0 != ( flags & kFSEventStreamEventFlagItemRenamed ) )
            {
                entry->m_announcedPaths.erase( path );
                if ( exists )
                    entry->m_announcedPaths.insert( path );
            }
            else
            if ( !exists )
            {
                entry->m_announcedPaths.erase( path );
                continue;
            }
            else
            if ( 0 != ( flags & kFSEventStreamEventFlagItemCreated ) )
            {
                if ( entry->m_announcedPaths.insert( path ).second )
                {
                    SChange change;
                    change.kind = isDir ? CHANGE_DIR_CREATED : CHANGE_FILE_CREATED;
                    change.name = relative.c_str();
                    change.watchedDir = entry->m_watchedDir;
                    changes.push_back( change );

                    // creating a directory is not a change of it
                    if ( isDir )
                        continue;
                }
            }

            if ( 0 != ( flags & kFSEventStreamEventFlagItemRenamed ) )
            {
                // the old name is not there any more, the new one is: they are reported one after the other
                if ( !PathExists( path ) )
                {
                    haveRenamedFrom = true;
                    renamedFrom = relative;
                    renamedFromIsDir = isDir;
                }
                else
                if ( haveRenamedFrom && renamedFromIsDir == isDir )
                {
                    SChange change;
                    change.kind = isDir ? CHANGE_DIR_RENAMED : CHANGE_FILE_RENAMED;
                    change.name = relative.c_str();
                    change.oldName = renamedFrom.c_str();
                    change.watchedDir = entry->m_watchedDir;
                    changes.push_back( change );
                    haveRenamedFrom = false;
                }
            }
            // the rename of a path is told as that, whatever else the flags still say about it
            if ( 0 == ( flags & kFSEventStreamEventFlagItemRenamed ) &&
                 0 != ( flags & ( kFSEventStreamEventFlagItemModified | kFSEventStreamEventFlagItemInodeMetaMod | kFSEventStreamEventFlagItemXattrMod |
                                  kFSEventStreamEventFlagItemChangeOwner | kFSEventStreamEventFlagItemFinderInfoMod ) ) )
            {
                SChange change;
                change.kind = isDir ? CHANGE_DIR_MODIFIED : CHANGE_FILE_MODIFIED;
                change.name = relative.c_str();
                change.watchedDir = entry->m_watchedDir;
                changes.push_back( change );
            }
        }

        if ( !changes.empty() )
        {
            MT::CScopeMutex lock( entry->m_owner->m_pendingLock );
            entry->m_owner->m_pendingChanges.insert( entry->m_owner->m_pendingChanges.end(), changes.begin(), changes.end() );
        }
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

        CUtf8String utfDirToWatch = ToUtf8String( dirToWatch );
        if ( m_dirsToWatch.find( utfDirToWatch ) != m_dirsToWatch.end() )
            return false;       // already watched

        // FSEvents reports the paths the file system has: without symbolic links, as /private/var and not /var
        char realPathBuffer[ PATH_MAX + 1 ];
        if ( GUCEF_NULL == ::realpath( utfDirToWatch.C_String(), realPathBuffer ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "AddDirToWatch: Failed to add watch to dir \"" + dirToWatch + "\": it does not exist" );
            return false;
        }
        std::string realPath = realPathBuffer;
        while ( realPath.size() > 1 && '/' == realPath[ realPath.size()-1 ] )
            realPath.erase( realPath.size()-1 );

        WatchEntry* entry = GUCEF_NEW WatchEntry();
        entry->m_options = options;
        entry->m_watchedDir = utfDirToWatch;
        entry->m_realPath = realPath;
        entry->m_owner = this;

        CFStringRef cfPath = ::CFStringCreateWithCString( kCFAllocatorDefault, realPath.c_str(), kCFStringEncodingUTF8 );
        CFArrayRef cfPaths = GUCEF_NULL != cfPath ? ::CFArrayCreate( kCFAllocatorDefault, (const void**) &cfPath, 1, &kCFTypeArrayCallBacks ) : NULL;
        if ( NULL != cfPath )
            ::CFRelease( cfPath );
        if ( NULL == cfPaths )
        {
            GUCEF_DELETE entry;
            return false;
        }

        FSEventStreamContext context;
        memset( &context, 0, sizeof( context ) );
        context.info = entry;
        entry->m_stream = ::FSEventStreamCreate( kCFAllocatorDefault                                                   ,
                                                 &OSSpecificDirectoryWatcher::StreamCallback                           ,
                                                 &context                                                              ,
                                                 cfPaths                                                               ,
                                                 kFSEventStreamEventIdSinceNow                                         ,
                                                 FSEVENTS_LATENCY_IN_SECONDS                                           ,
                                                 kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagNoDefer  );
        ::CFRelease( cfPaths );
        if ( NULL == entry->m_stream )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "AddDirToWatch: Failed to add watch to dir \"" + dirToWatch + "\"" );
            GUCEF_DELETE entry;
            return false;
        }

        ::FSEventStreamSetDispatchQueue( entry->m_stream, m_queue );
        if ( !::FSEventStreamStart( entry->m_stream ) )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "AddDirToWatch: Failed to start the watch of dir \"" + dirToWatch + "\"" );
            ::FSEventStreamInvalidate( entry->m_stream );
            ::FSEventStreamRelease( entry->m_stream );
            GUCEF_DELETE entry;
            return false;
        }

        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "AddDirToWatch: Added watch of dir \"" + dirToWatch + "\"" );
        m_dirsToWatch[ utfDirToWatch ] = entry;
        m_osPollingTimer.SetEnabled( true );
        return true;
    }

    /** Stops the stream: what it had in flight is delivered first, the callback is not called any more when this returns */
    static void StopWatchEntry( WatchEntry* entry )
    {GUCEF_TRACE;

        if ( NULL != entry->m_stream )
        {
            ::FSEventStreamFlushSync( entry->m_stream );
            ::FSEventStreamStop( entry->m_stream );
            ::FSEventStreamInvalidate( entry->m_stream );
            ::FSEventStreamRelease( entry->m_stream );
            entry->m_stream = NULL;
        }
        entry->m_owner = GUCEF_NULL;
    }

    bool RemoveDirToWatch( const CString& dirToWatch )
    {GUCEF_TRACE;

        CUtf8String utfDirToWatch = ToUtf8String( dirToWatch );
        TWatchEntryMap::iterator i = m_dirsToWatch.find( utfDirToWatch );
        if ( i == m_dirsToWatch.end() )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "RemoveDirToWatch: Request to remove watch of dir \"" + dirToWatch + "\" but we did not have any such watch" );
            return false;
        }

        StopWatchEntry( (*i).second );
        GUCEF_DELETE (*i).second;
        m_dirsToWatch.erase( i );
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "RemoveDirToWatch: Removed watch of dir \"" + dirToWatch + "\"" );

        if ( m_dirsToWatch.empty() )
            m_osPollingTimer.SetEnabled( false );
        return true;
    }

    bool RemoveAllWatches( void )
    {GUCEF_TRACE;

        m_osPollingTimer.SetEnabled( false );

        TWatchEntryMap::iterator i = m_dirsToWatch.begin();
        while ( i != m_dirsToWatch.end() )
        {
            StopWatchEntry( (*i).second );
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "RemoveAllWatches: Removed watch of dir \"" + ToString( (*i).first ) + "\"" );
            GUCEF_DELETE (*i).second;
            ++i;
        }
        m_dirsToWatch.clear();

        // what was not handed over yet belongs to watches that are gone
        MT::CScopeMutex lock( m_pendingLock );
        m_pendingChanges.clear();
        return true;
    }

    bool GetAllWatchedDirs( CString::StringSet& dirs ) const
    {GUCEF_TRACE;

        TWatchEntryMap::const_iterator i = m_dirsToWatch.begin();
        while ( i != m_dirsToWatch.end() )
        {
            dirs.insert( ToString( (*i).first ) );
            ++i;
        }
        return true;
    }

    void OnDirWatchPollingCycle( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
    {GUCEF_TRACE;

        TChangeVector changes;
        {
            MT::CScopeMutex lock( m_pendingLock );
            changes.swap( m_pendingChanges );
        }

        for ( size_t i=0; i<changes.size(); ++i )
        {
            const SChange& change = changes[ i ];
            TWatchEntryMap::iterator n = m_dirsToWatch.find( change.watchedDir );
            if ( n == m_dirsToWatch.end() )
                continue;   // watch removed in the meantime
            const CDirWatchOptions& options = (*n).second->m_options;

            switch ( change.kind )
            {
                case CHANGE_FILE_CREATED:
                {
                    if ( options.watchForFileCreation )
                    {
                        CIDirectoryWatcher::TFileCreatedEventData eData( change.name );
                        if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::FileCreatedEvent, &eData ) ) return;
                    }
                    break;
                }
                case CHANGE_FILE_DELETED:
                {
                    if ( options.watchForFileDeletion )
                    {
                        CIDirectoryWatcher::TFileDeletedEventData eData( change.name );
                        if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::FileDeletedEvent, &eData ) ) return;
                    }
                    break;
                }
                case CHANGE_FILE_MODIFIED:
                {
                    if ( options.watchForFileModifications )
                    {
                        CIDirectoryWatcher::TFileModifiedEventData eData( change.name );
                        if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::FileModifiedEvent, &eData ) ) return;
                    }
                    break;
                }
                case CHANGE_FILE_RENAMED:
                {
                    if ( options.watchForFileRenames )
                    {
                        CIDirectoryWatcher::TFileRenamedEventData eData;
                        eData.GetData().oldFilename = ToString( change.oldName );
                        eData.GetData().newFilename = ToString( change.name );
                        if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::FileRenamedEvent, &eData ) ) return;
                    }
                    break;
                }
                case CHANGE_DIR_CREATED:
                {
                    if ( options.watchForDirCreation )
                    {
                        CIDirectoryWatcher::TDirCreatedEventData eData( change.name );
                        if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::DirCreatedEvent, &eData ) ) return;
                    }
                    break;
                }
                case CHANGE_DIR_DELETED:
                {
                    if ( options.watchForDirDeletion )
                    {
                        CIDirectoryWatcher::TDirDeletedEventData eData( change.name );
                        if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::DirDeletedEvent, &eData ) ) return;
                    }
                    break;
                }
                case CHANGE_DIR_MODIFIED:
                {
                    if ( options.watchForDirModifications )
                    {
                        CIDirectoryWatcher::TDirModifiedEventData eData( change.name );
                        if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::DirModifiedEvent, &eData ) ) return;
                    }
                    break;
                }
                case CHANGE_DIR_RENAMED:
                {
                    if ( options.watchForDirRenames )
                    {
                        CIDirectoryWatcher::TDirRenamedEventData eData;
                        eData.GetData().oldDirName = ToString( change.oldName );
                        eData.GetData().newDirName = ToString( change.name );
                        if ( !m_wrapper->NotifyObservers( CIDirectoryWatcher::DirRenamedEvent, &eData ) ) return;
                    }
                    break;
                }
            }
        }
    }

    void Init( void )
    {GUCEF_TRACE;

        m_queue = ::dispatch_queue_create( "gucef.core.fsevents", DISPATCH_QUEUE_SERIAL );
        RegisterEventHandlers();
        m_osPollingTimer.SetInterval( FSEVENTS_POLLING_INTERVAL_IN_MS );
    }

    OSSpecificDirectoryWatcher( CFileSystemDirectoryWatcher* wrapper )
        : m_osPollingTimer()
        , m_wrapper( wrapper )
        , m_dirsToWatch()
        , m_queue( NULL )
        , m_pendingLock()
        , m_pendingChanges()
    {GUCEF_TRACE;

        Init();
    }

    OSSpecificDirectoryWatcher( CFileSystemDirectoryWatcher* wrapper ,
                                PulseGeneratorPtr pulseGenerator     )
        : m_osPollingTimer( pulseGenerator )
        , m_wrapper( wrapper )
        , m_dirsToWatch()
        , m_queue( NULL )
        , m_pendingLock()
        , m_pendingChanges()
    {GUCEF_TRACE;

        Init();
    }

    OSSpecificDirectoryWatcher( const OSSpecificDirectoryWatcher& src ,
                                CFileSystemDirectoryWatcher* wrapper  )
        : m_osPollingTimer( src.m_osPollingTimer )
        , m_wrapper( wrapper )
        , m_dirsToWatch()
        , m_queue( NULL )
        , m_pendingLock()
        , m_pendingChanges()
    {GUCEF_TRACE;

        Init();
        TWatchEntryMap::const_iterator i = src.m_dirsToWatch.begin();
        while ( i != src.m_dirsToWatch.end() )
        {
            AddDirToWatch( ToString( (*i).first ), (*i).second->m_options );
            ++i;
        }
    }

    ~OSSpecificDirectoryWatcher()
    {GUCEF_TRACE;

        RemoveAllWatches();
        if ( NULL != m_queue )
        {
            ::dispatch_release( m_queue );
            m_queue = NULL;
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
