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

#ifndef __LogManager_H__
#include "OgreLogManager.h"
#define __LogManager_H__
#endif /* __LogManager_H__ ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "guidriverOgre_COgreLogAdapter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEROGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreLogAdapter::COgreLogAdapter( void )
    : Ogre::LogListener() ,
      m_log( GUCEF_NULL )
{GUCEF_TRACE;

    Ogre::LogManager* logManager = Ogre::LogManager::getSingletonPtr();
    if ( GUCEF_NULL == logManager )
    {
        logManager = OGRE_NEW Ogre::LogManager();
    }
    
    m_log = logManager->createLog( "OgreLogAdapter", true , true, true ); 
    if ( GUCEF_NULL != m_log )
    {
        m_log->addListener( this );
    }
}

/*-------------------------------------------------------------------------*/

COgreLogAdapter::~COgreLogAdapter()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_log )
    {
        m_log->removeListener( this );
        Ogre::LogManager::getSingleton().destroyLog( m_log );
    }
}

/*-------------------------------------------------------------------------*/

void
COgreLogAdapter::messageLogged( const Ogre::String& message , 
                                Ogre::LogMessageLevel lml   , 
                                bool maskDebug              , 
                                const Ogre::String &logName , 
                                bool& skipThisMessage       )
{GUCEF_TRACE;

    switch ( lml )
    {
        case Ogre::LML_CRITICAL :
        {        
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_CRITICAL, message );
            break;
        }
        case Ogre::LML_TRIVIAL :
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, message );
            break;
        }
        case Ogre::LML_NORMAL :
        default:
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, message );
            break;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEROGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
