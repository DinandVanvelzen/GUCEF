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

#ifndef GUCEF_CORE_CDESKTOPTERMINALCONSOLELOGGER_H
#define GUCEF_CORE_CDESKTOPTERMINALCONSOLELOGGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <deque>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTEXTLINECONSOLELOGGER_H
#include "gucefCORE_CTextLineConsoleLogger.h"
#define GUCEF_CORE_CTEXTLINECONSOLELOGGER_H
#endif /* GUCEF_CORE_CTEXTLINECONSOLELOGGER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

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
 *  Console logger for graphical Linux desktop sessions for applications which were not started from a terminal,
 *  which is what a console window is on Windows. It opens a terminal emulator window which shows the log.
 *
 *  Which terminal emulator is used is discovered: the configured one, $TERMINAL, xdg-terminal-exec,
 *  x-terminal-emulator and then a list of well known ones. The log lines travel to the window via a private
 *  FIFO (permissions 0600) which a small shell viewer in the window reads and displays.
 *
 *  The logger never blocks the application: lines are kept in a bounded buffer until the window has opened its
 *  end of the FIFO and while the window cannot keep up. When the buffer is full the oldest lines are dropped, which is
 *  marked in the log. If the window does not show up within the connect timeout or is closed by the user the
 *  logger disables itself, other loggers are not affected.
 */
class GUCEF_CORE_PUBLIC_CPP CDesktopTerminalConsoleLogger : public CTextLineConsoleLogger ,
                                                            public CIConfigurable         
{
    public:

    /**
     *  True if there is a graphical session in which a window can be opened, as per WAYLAND_DISPLAY, DISPLAY and
     *  XDG_SESSION_TYPE
     */
    static bool IsDesktopSessionAvailable( void );

    /**
     *  Finds an executable using the search path in the environment
     *  @param name         file name, or a path in which case only that path is checked
     *  @param fullPath     receives the path of the executable
     */
    static bool FindExecutableInPath( const CString& name ,
                                      CString& fullPath   );

    /**
     *  Finds a terminal emulator and builds the command line which makes it run the given command in a window.
     *
     *  @param preferredTerminal    name or path of the terminal emulator to use if available, empty for none
     *  @param windowTitle          title for the window, for terminals which have a command line option for it
     *  @param command              the program and its arguments the terminal is to run
     *  @param exePath              receives the path of the terminal emulator
     *  @param args                 receives the arguments to start the terminal emulator with
     *  @param terminalName         receives the name of the terminal that was discovered
     *  @return true if a terminal emulator was found
     */
    static bool DiscoverTerminal( const CString& preferredTerminal            ,
                                  const CString& windowTitle                  ,
                                  const CProcess::TStringVector& command      ,
                                  CString& exePath                            ,
                                  CProcess::TStringVector& args               ,
                                  CString& terminalName                       );

    public:

    /**
     *  @param initialize   whether to open the window right away. If not call Initialize() after the configuration
     *                      was done
     */
    CDesktopTerminalConsoleLogger( bool initialize = true );

    virtual ~CDesktopTerminalConsoleLogger() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Starts the terminal window. The window is not necessarily there when this returns, lines are kept until it is.
     *  @return false if the logger is disabled or no terminal could be started
     */
    bool Initialize( void );

    /** Whether a window is started or open and lines are shown or about to be shown */
    bool IsActive( void ) const;

    /** Whether the window has opened the log and is receiving lines */
    bool IsConnected( void ) const;

    /** Number of lines which were dropped because the window could not keep up */
    UInt32 GetDroppedLineCount( void ) const;

    /** Name of the terminal emulator that is used, empty if none */
    CString GetTerminalName( void ) const;

    /** Terminal to use if it is available, name or path. Applies to the next Initialize() */
    void SetPreferredTerminal( const CString& terminal );

    CString GetPreferredTerminal( void ) const;

    /** Whether the window stays after the application ended its log until the user closes it, default false */
    void SetKeepWindowOpenOnExit( bool keepOpen );

    bool GetKeepWindowOpenOnExit( void ) const;

    /** Disabled loggers do not open a window, default enabled */
    void SetEnabled( bool enabled );

    bool GetEnabled( void ) const;

    void SetWindowTitle( const CString& title );

    /** How long to wait for the window to open before giving up, default 10 seconds */
    void SetConnectTimeoutInMs( UInt32 timeoutInMs );

    /** Size of the buffer that holds lines the window did not take yet, default 256KiB */
    void SetMaxBufferedBytes( UInt32 maxBytes );

    virtual bool SaveConfig( CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void WriteLine( const CString& line ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnFlushLog( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    enum EState
    {
        STATE_IDLE = 0      ,   /**< nothing started yet */
        STATE_CONNECTING    ,   /**< window was started but did not open the log yet */
        STATE_CONNECTED     ,   /**< window shows the log */
        STATE_DISABLED          /**< ended or failed, nothing more will be shown */
    };

    CDesktopTerminalConsoleLogger& operator=( const CDesktopTerminalConsoleLogger& src );
    CDesktopTerminalConsoleLogger( const CDesktopTerminalConsoleLogger& src );

    bool InitializeImp( void );
    void Pump( void );
    void Disable( const CString& reason );
    void QueueLine( const CString& line );
    bool FlushPending( void );
    void Cleanup( void );

    private:

    mutable MT::CMutex m_lock;
    EState m_state;
    CString m_preferredTerminal;
    CString m_windowTitle;
    CString m_terminalName;
    bool m_keepWindowOpen;
    bool m_enabled;
    UInt32 m_connectTimeoutInMs;
    UInt32 m_maxBufferedBytes;
    CString m_fifoPath;
    int m_fifoFd;
    CProcess m_terminalProcess;
    UInt64 m_startTicks;
    std::deque< CString > m_pending;
    UInt32 m_pendingBytes;
    UInt32 m_droppedLines;
    UInt32 m_droppedSinceLastMarker;
    UInt32 m_partialOffset;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDESKTOPTERMINALCONSOLELOGGER_H ? */
