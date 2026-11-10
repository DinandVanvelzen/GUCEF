/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#include <cstdio>
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
#include <conio.h>
#else
#include <unistd.h>
#include <sys/select.h>
#endif

#include "AncestryBackup_CConsoleInput.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CConsoleInput::CConsoleInput( void )
    : m_pending()
    , m_closed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CConsoleInput::IsInputClosed( void ) const
{GUCEF_TRACE;

    return m_closed;
}

/*-------------------------------------------------------------------------*/

bool
CConsoleInput::PollLine( std::string& line )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // Windows build unverified
    while ( _kbhit() )
    {
        const int c = _getch();
        if ( c == '\r' || c == '\n' )
        {
            line = m_pending;
            m_pending.clear();
            return true;
        }
        m_pending += (char) c;
    }
    return false;

    #else

    while ( !m_closed )
    {
        fd_set readSet;
        FD_ZERO( &readSet );
        FD_SET( 0, &readSet );
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        if ( select( 1, &readSet, GUCEF_NULL, GUCEF_NULL, &timeout ) <= 0 )
            return false;

        char c = 0;
        const ssize_t got = read( 0, &c, 1 );
        if ( got <= 0 )
        {
            m_closed = true;
            return false;
        }
        if ( c == '\n' )
        {
            line = m_pending;
            m_pending.clear();
            return true;
        }
        if ( c != '\r' )
            m_pending += c;
    }
    return false;

    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
