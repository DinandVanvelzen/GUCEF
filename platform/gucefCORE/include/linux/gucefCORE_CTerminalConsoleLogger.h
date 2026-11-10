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

#ifndef GUCEF_CORE_CTERMINALCONSOLELOGGER_H
#define GUCEF_CORE_CTERMINALCONSOLELOGGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTEXTLINECONSOLELOGGER_H
#include "gucefCORE_CTextLineConsoleLogger.h"
#define GUCEF_CORE_CTEXTLINECONSOLELOGGER_H
#endif /* GUCEF_CORE_CTEXTLINECONSOLELOGGER_H ? */

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
 *  Console logger for an application that is run from a terminal: the log lines go to the terminal that
 *  stdout or stderr is connected to. A terminal that cannot take any more data at the moment, for example
 *  because it was paused with ctrl-s, makes the logger drop lines instead of blocking the application.
 */
class GUCEF_CORE_PUBLIC_CPP CTerminalConsoleLogger : public CTextLineConsoleLogger
{
    public:

    /** @return true if stdout or stderr is connected to a terminal */
    static bool IsRunningInTerminal( void );

    /** @return the file descriptor of the terminal, stdout preferred unless it is claimed (CStandardOutputWriter), or -1 if there is none */
    static int GetTerminalFileDescriptor( void );

    /**
     *  @param fileDescriptor   where to write to. -1 picks the terminal as per GetTerminalFileDescriptor().
     *                          The file descriptor is not owned and stays open
     */
    CTerminalConsoleLogger( int fileDescriptor = -1 );

    virtual ~CTerminalConsoleLogger() GUCEF_VIRTUAL_OVERRIDE;

    /** Number of lines that were dropped because the terminal was not ready for them */
    UInt32 GetDroppedLineCount( void ) const;

    protected:

    virtual void WriteLine( const CString& line ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CTerminalConsoleLogger& operator=( const CTerminalConsoleLogger& src );
    CTerminalConsoleLogger( const CTerminalConsoleLogger& src );

    private:

    int m_fileDescriptor;
    UInt32 m_droppedLines;
    bool m_notATerminal;        /**< the file descriptor is a pipe or a file: a write can raise SIGPIPE */
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTERMINALCONSOLELOGGER_H ? */
