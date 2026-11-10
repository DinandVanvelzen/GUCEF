/*
 *  notifysinkpluginDESKTOP: gucefMULTIPLAY match alert sink plugin for desktop alerts
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

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_CNOTIFYSENDLAUNCHER_H
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_CNOTIFYSENDLAUNCHER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_INOTIFYLAUNCHER_H
#include "notifysinkpluginDESKTOP_INotifyLauncher.h"
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_INOTIFYLAUNCHER_H
#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_INOTIFYLAUNCHER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace DESKTOP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Starts "notify-send" of libnotify directly with the argument vector it is given: no shell, no command line.
 *  The program is started detached from our session with its standard streams on /dev/null and no other descriptor
 *  of ours open in it. Nothing waits for it: finished programs are reaped each time the launcher is used, and one that
 *  runs longer than the maximum run time (a notification daemon that does not answer) is killed.
 *  Not thread safe: use it from one thread at a time.
 */
class GUCEF_MULTIPLAYPLUGIN_DESKTOP_EXPORT_CPP CNotifySendLauncher : public INotifyLauncher
{
    public:

    static const UInt32 DefaultMaxRunTimeMs = 15000;

    /** The name of the program, which is also argv[ 0 ] of every launch */
    static const CORE::CString& GetToolName( void );

    /**
     *  Looks for an executable regular file with the name in the directories of a search path (separated with ':', as PATH is).
     *  @param name         a plain file name, nothing with a '/' in it is ever found
     *  @param searchPath   the directories to look in
     *  @param foundPath    receives the path of the file when found
     */
    static bool FindExecutable( const CORE::CString& name, const CORE::CString& searchPath, CORE::CString& foundPath );

    CNotifySendLauncher( void );

    virtual ~CNotifySendLauncher();

    /** Looks for the program in the directories of the PATH of the process, or in the search path set with SetSearchPath() */
    virtual bool IsAvailable( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Starts the program: refused when argv[ 0 ] is not the name of the program or there are too many running already */
    virtual bool Launch( const TArgumentVector& argv ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetRunningCount( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Directories to look in instead of the PATH of the process, an empty string restores the PATH */
    void SetSearchPath( const CORE::CString& searchPath );

    /** After this long a program that still runs is killed */
    void SetMaxRunTimeMs( UInt32 maxRunTimeMs );

    private:

    struct SChild
    {
        CORE::CProcess* process;
        UInt64 startTick;
    };
    typedef GUCEF::vector< SChild > TChildVector;

    void Reap( void );

    private:

    CORE::CString m_searchPath;
    UInt32 m_maxRunTimeMs;
    TChildVector m_children;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DESKTOP */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_CNOTIFYSENDLAUNCHER_H ? */
