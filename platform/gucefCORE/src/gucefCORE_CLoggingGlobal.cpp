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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#include "gucefCORE_CLoggingGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex CLoggingGlobal::g_dataLock;
CLoggingGlobal* CLoggingGlobal::g_instance = NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLoggingGlobal*
CLoggingGlobal::Instance()
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CLoggingGlobal();
            if ( GUCEF_NULL != g_instance )
            {
                g_instance->Initialize();
            }
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CLoggingGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_dataLock );
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CLoggingGlobal::Initialize( void )
{GUCEF_TRACE;

    m_logManager = GUCEF_NEW CLogManager();                 
}

/*-------------------------------------------------------------------------*/

CLoggingGlobal::CLoggingGlobal( void )
    : m_logManager( GUCEF_NULL )                 
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLoggingGlobal::~CLoggingGlobal()
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_dataLock );

    // make a copy so that we disable logging loop back during destruction of the log manager
    CLogManager* logManager = m_logManager;

    m_logManager->FlushLogs();
    
    // wipe out log manager access first to disable a logging loop back during destruction of the log manager
    m_logManager = GUCEF_NULL;
    GUCEF_DELETE logManager;
}

/*-------------------------------------------------------------------------*/

CLogManager& 
CLoggingGlobal::GetLogManager( void )
{GUCEF_TRACE;

    return *m_logManager;
}

/*-------------------------------------------------------------------------*/

CVariantStreamPtr
CLoggingGlobal::Log( const TLogMsgType logMsgType ,
                     const Int32 logLevel         )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_logManager )
        return m_logManager->Log( logMsgType, logLevel );
    return CVariantStreamPtr();
}

/*-------------------------------------------------------------------------*/

void
CLoggingGlobal::Log( const TLogMsgType logMsgType ,
                     const Int32 logLevel         ,
                     const CString& logMessage    )
{
    if ( GUCEF_NULL != m_logManager  )
        m_logManager->Log( logMsgType, logLevel, logMessage ); 
}

/*-------------------------------------------------------------------------*/

void 
CLoggingGlobal::Log( const TLogMsgType logMsgType ,
                     const Int32 logLevel         ,
                     const CString& logMessage    ,
                     const UInt32 threadId        ,
                     const CTimestamp& timestamp  )
{
    if ( GUCEF_NULL != m_logManager  )
        m_logManager->Log( logMsgType, logLevel, logMessage, threadId, timestamp );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
