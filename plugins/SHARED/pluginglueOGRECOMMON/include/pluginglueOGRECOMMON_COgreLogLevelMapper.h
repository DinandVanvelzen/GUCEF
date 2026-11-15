/*
 *  pluginglueOGRECOMMON: Header only adapters between GUCEF and the Ogre 1.x API, compiled into the plugins that use them
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

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGRELOGLEVELMAPPER_H
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGRELOGLEVELMAPPER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreLog.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_MACROS_H
#include "pluginglueOGRECOMMON_macros.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_MACROS_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRECOMMON {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The mapping of the message levels of the Ogre log to the logs and levels of GUCEF:
 *
 *      LML_TRIVIAL     system log, LOGLEVEL_BELOW_NORMAL
 *      LML_NORMAL      system log, LOGLEVEL_NORMAL
 *      LML_WARNING     warning log, LOGLEVEL_NORMAL
 *      LML_CRITICAL    error log, LOGLEVEL_CRITICAL
 *
 *  Every message is logged with the prefix "Ogre: ".
 */
class COgreLogLevelMapper
{
    public:

    enum ELogKind
    {
        LOGKIND_SYSTEM  ,
        LOGKIND_WARNING ,
        LOGKIND_ERROR
    };
    typedef enum ELogKind TLogKind;

    static void Map( Ogre::LogMessageLevel lml ,
                     TLogKind& kind            ,
                     Int32& logLevel           )
    {GUCEF_TRACE;

        switch ( lml )
        {
            case Ogre::LML_TRIVIAL :
            {
                kind = LOGKIND_SYSTEM;
                logLevel = CORE::LOGLEVEL_BELOW_NORMAL;
                break;
            }
            case Ogre::LML_WARNING :
            {
                kind = LOGKIND_WARNING;
                logLevel = CORE::LOGLEVEL_NORMAL;
                break;
            }
            case Ogre::LML_CRITICAL :
            {
                kind = LOGKIND_ERROR;
                logLevel = CORE::LOGLEVEL_CRITICAL;
                break;
            }
            case Ogre::LML_NORMAL :
            default :
            {
                kind = LOGKIND_SYSTEM;
                logLevel = CORE::LOGLEVEL_NORMAL;
                break;
            }
        }
    }

    /** Sends the message to the GUCEF log the level maps to */
    static void Log( Ogre::LogMessageLevel lml, const Ogre::String& message )
    {GUCEF_TRACE;

        TLogKind kind = LOGKIND_SYSTEM;
        Int32 logLevel = CORE::LOGLEVEL_NORMAL;
        Map( lml, kind, logLevel );

        switch ( kind )
        {
            case LOGKIND_WARNING :
            {
                GUCEF_WARNING_LOG( logLevel, "Ogre: " << message );
                break;
            }
            case LOGKIND_ERROR :
            {
                GUCEF_ERROR_LOG( logLevel, "Ogre: " << message );
                break;
            }
            case LOGKIND_SYSTEM :
            default :
            {
                GUCEF_SYSTEM_LOG( logLevel, "Ogre: " << message );
                break;
            }
        }
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRECOMMON */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGRELOGLEVELMAPPER_H ? */
