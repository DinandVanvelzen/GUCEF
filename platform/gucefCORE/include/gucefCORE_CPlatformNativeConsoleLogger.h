/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#define GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONSOLELOGGER_H
#include "gucefCORE_CIConsoleLogger.h"
#define GUCEF_CORE_CICONSOLELOGGER_H
#endif /* GUCEF_CORE_CICONSOLELOGGER_H ? */

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
 *  Class which uses the strategy pattern to allocate a platform
 *  native console logger which can then be obtained using GetLogger().
 *  Note that it does not automatically register the logger with the 
 *  log manager.
 */
class GUCEF_CORE_PUBLIC_CPP CPlatformNativeConsoleLogger
{
    public:

    /** The consoles a Linux application can log to */
    enum EConsoleKind
    {
        CONSOLEKIND_NONE = 0,               /**< no console: headless, or switched off */
        CONSOLEKIND_TERMINAL,               /**< the terminal the application was started from */
        CONSOLEKIND_DESKTOP_TERMINAL,       /**< a terminal window of its own in the desktop session */
        CONSOLEKIND_XTERM,                  /**< an xterm of its own */
        CONSOLEKIND_STDERR                  /**< text lines on stderr whether it is a terminal or not, which is what a client that started the application with pipes sees of its log */
    };

    /**
     *  Which console a Linux application logs to. 'forced' is the environment variable GUCEF_CONSOLE_LOGGER (terminal,
     *  desktop, xterm, stderr, none or auto / empty) and goes over the rest. Otherwise a window of its own when 'ownWindow' asks for it
     *  and the desktop has one to give, else the terminal it was started from; without 'ownWindow' the terminal first.
     *  While stdout is claimed as a protocol channel (CStandardOutputWriter::IsClaimed(), 'stdoutClaimed') the console is
     *  stderr, and no window of its own is opened unless 'forced' asks for one; the terminal logger then never writes to stdout
     */
    static EConsoleKind ChooseConsoleKind( const char* forced, bool runningInTerminal, bool desktopSessionAvailable, bool ownWindow, bool stdoutClaimed = false );

    /** @param ownWindow  log in a console window of its own, also when the application was started from a console */
    explicit CPlatformNativeConsoleLogger( bool ownWindow = false );

    ~CPlatformNativeConsoleLogger();

    CIConsoleLogger* GetLogger( void );

    private:

    CPlatformNativeConsoleLogger& operator=( const CPlatformNativeConsoleLogger& src );
    CPlatformNativeConsoleLogger( const CPlatformNativeConsoleLogger& src );

    private:

    CIConsoleLogger* m_nativeConsoleLogger;
    bool m_ownWindow;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 19-05-2007 :
        - Dinand: Added this class

---------------------------------------------------------------------------*/
