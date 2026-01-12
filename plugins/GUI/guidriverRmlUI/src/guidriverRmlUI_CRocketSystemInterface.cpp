/*
 *  guidriverRmlUI: GUI backend using RmlUI
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "guidriverRmlUI_CRmlUISystemInterface.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverRmlUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CRmlUISystemInterface::CRmlUISystemInterface( void )
    : RmlUI::Core::SystemInterface()
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CRmlUISystemInterface::~CRmlUISystemInterface()
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

float
CRmlUISystemInterface::GetElapsedTime()
{GUCEF_TRACE;

    return CORE::GUCEFGetTickCount() / 1000.0f;
}

/*---------------------------------------------------------------------------*/

bool
CRmlUISystemInterface::LogMessage( RmlUI::Core::Log::Type type        , 
                                    const RmlUI::Core::String& message )
{GUCEF_TRACE;

    static CORE::CString logPrefix = "RmlUI: ";
    
    switch ( type )
    {
        case RmlUI::Core::Log::LT_ALWAYS :
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, logPrefix + message.CString() );
            return true;
        }
        case RmlUI::Core::Log::LT_ERROR :
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, logPrefix + message.CString() );
            return true;
        }
        case RmlUI::Core::Log::LT_ASSERT :
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, logPrefix + message.CString() );
            return true;
        }
        case RmlUI::Core::Log::LT_WARNING :
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, logPrefix + message.CString() );
            return true;
        }
        case RmlUI::Core::Log::LT_INFO :
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, logPrefix + message.CString() );
            return true;
        }
        case RmlUI::Core::Log::LT_DEBUG :
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, logPrefix + message.CString() );
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

