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

#ifndef GUCEF_PLUGINGLUE_OGRE_COGRELOGADAPTER_H
#define GUCEF_PLUGINGLUE_OGRE_COGRELOGADAPTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreLog.h"

#ifndef GUCEF_PLUGINGLUE_OGRE_MACROS_H
#include "pluginglueOGRE_macros.h"
#define GUCEF_PLUGINGLUE_OGRE_MACROS_H
#endif /* GUCEF_PLUGINGLUE_OGRE_MACROS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Directs the log of Ogre to the GUCEF logs.
 *
 *  The adapter makes the default log of Ogre without a log file and with no output of its own, before the Ogre
 *  Root is made, so the messages of the start up of Root are caught too. The messages are mapped by
 *  COgreLogLevelMapper: trivial and normal messages go to the system log, warnings to the warning log, critical
 *  messages to the error log, all with the prefix "Ogre: ".
 *
 *  When an Ogre log manager exists already when the adapter is made, the adapter adds its log to that manager and
 *  leaves the manager to its owner.
 */
class GUCEF_PLUGINGLUE_OGRE_EXPORT_CPP COgreLogAdapter : public Ogre::LogListener
{
    public:

    COgreLogAdapter( void );

    virtual ~COgreLogAdapter() GUCEF_VIRTUAL_OVERRIDE;

    virtual void messageLogged( const Ogre::String& message ,
                                Ogre::LogMessageLevel lml   ,
                                bool maskDebug              ,
                                const Ogre::String& logName ,
                                bool& skipThisMessage       ) GUCEF_VIRTUAL_OVERRIDE;

    /** The Ogre log that the adapter listens to */
    Ogre::Log* GetLog( void ) const;

    private:

    COgreLogAdapter( const COgreLogAdapter& src );              /**< not implemented */
    COgreLogAdapter& operator=( const COgreLogAdapter& src );   /**< not implemented */

    private:

    Ogre::LogManager* m_logManager;
    bool m_ownsLogManager;
    Ogre::Log* m_log;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_OGRE_COGRELOGADAPTER_H ? */
