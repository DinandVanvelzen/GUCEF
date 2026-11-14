/*
 *  guidriverMyGUI: glue module for the MyGUI GUI backend
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#include "guidriverMyGUI_CLogAdapter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MYGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CLogAdapter::CLogAdapter()
    : MyGUI::LogManager() ,
      m_logManager( &CORE::CCoreGlobal::Instance()->GetLogManager() )
{
}

/*-------------------------------------------------------------------------*/
	
CLogAdapter::~CLogAdapter()
{
    m_logManager = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CLogAdapter::flush()
{
    m_logManager->FlushLogs();
}

/*-------------------------------------------------------------------------*/

void
CLogAdapter::ConvertLogLevel( MyGUI::LogLevel myGuilevel ,
                              CORE::TLogMsgType& msgType ,
                              CORE::Int32& logLevel      )
{
    if ( myGuilevel == MyGUI::LogLevel( MyGUI::LogLevel::Info ) )
    {
        msgType = CORE::LOG_SYSTEM;
        logLevel = CORE::LOGLEVEL_NORMAL;
    }
    else
    if ( myGuilevel == MyGUI::LogLevel( MyGUI::LogLevel::Warning ) )
    {
        msgType = CORE::LOG_WARNING;
        logLevel = CORE::LOGLEVEL_NORMAL;
    }
    else
    if ( myGuilevel == MyGUI::LogLevel( MyGUI::LogLevel::Error ) )
    {
        msgType = CORE::LOG_ERROR;
        logLevel = CORE::LOGLEVEL_NORMAL;
    } 
    else
    if ( myGuilevel == MyGUI::LogLevel( MyGUI::LogLevel::Critical ) )
    {
        msgType = CORE::LOG_ERROR;
        logLevel = CORE::LOGLEVEL_IMPORTANT;
    }
}

/*-------------------------------------------------------------------------*/

void
CLogAdapter::log( const std::string& _section , 
                  MyGUI::LogLevel _level      , 
                  const std::string& _message ,
                  const char* _file           ,
                  int _line                   )
{
    CORE::CString newMessage( "MyGUI: " + _message );

    CORE::TLogMsgType msgType;
    CORE::Int32 logLevel;

    ConvertLogLevel( _level, msgType, logLevel );

    m_logManager->Log( msgType, logLevel, newMessage );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MYGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
