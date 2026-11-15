/*
 *  pluginglueOGRE: Support library of the Ogre 1.x plugin set: Ogre Root setup, log, plugin loading
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

#include "OgreLogManager.h"

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGRELOGLEVELMAPPER_H
#include "pluginglueOGRECOMMON_COgreLogLevelMapper.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGRELOGLEVELMAPPER_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGRELOGLEVELMAPPER_H ? */

#include "pluginglueOGRE_COgreLogAdapter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreLogAdapter::COgreLogAdapter( void )
    : Ogre::LogListener()
    , m_logManager( GUCEF_NULL )
    , m_ownsLogManager( false )
    , m_log( GUCEF_NULL )
{GUCEF_TRACE;

    m_logManager = Ogre::LogManager::getSingletonPtr();
    if ( GUCEF_NULL == m_logManager )
    {
        m_logManager = new Ogre::LogManager();
        m_ownsLogManager = true;
    }

    // the default log, no file and no output to the console of its own: GUCEF decides where the messages go
    m_log = m_logManager->createLog( "OgreGucef", true, false, true );
    if ( GUCEF_NULL != m_log )
    {
        // every message is passed on, the levels of GUCEF decide which of them are kept
        m_log->setMinLogLevel( Ogre::LML_TRIVIAL );
        m_log->addListener( this );
    }
}

/*-------------------------------------------------------------------------*/

COgreLogAdapter::~COgreLogAdapter()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_log )
    {
        m_log->removeListener( this );
        m_logManager->destroyLog( m_log );
        m_log = GUCEF_NULL;
    }
    if ( m_ownsLogManager )
    {
        delete m_logManager;
    }
    m_logManager = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
COgreLogAdapter::messageLogged( const Ogre::String& message ,
                                Ogre::LogMessageLevel lml   ,
                                bool maskDebug              ,
                                const Ogre::String& logName ,
                                bool& skipThisMessage       )
{GUCEF_TRACE;

    OGRECOMMON::COgreLogLevelMapper::Log( lml, message );
}

/*-------------------------------------------------------------------------*/

Ogre::Log*
COgreLogAdapter::GetLog( void ) const
{GUCEF_TRACE;

    return m_log;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
