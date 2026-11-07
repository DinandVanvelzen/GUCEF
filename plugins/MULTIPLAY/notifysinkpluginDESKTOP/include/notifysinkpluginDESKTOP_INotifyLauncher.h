/*
 *  notifysinkpluginDESKTOP: gucefMULTIPLAY match alert sink plugin for desktop alerts
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

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_INOTIFYLAUNCHER_H
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_INOTIFYLAUNCHER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_DESKTOP_MACROS_H
#include "notifysinkpluginDESKTOP_macros.h"
#define GUCEF_MULTIPLAYPLUGIN_DESKTOP_MACROS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace DESKTOP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The only way the desktop match alert sink starts the program that shows a notification. The sink builds the
 *  argument vector, the launcher starts the program with exactly those arguments: there is no shell, no command
 *  line to parse and nothing to quote. A test of the sink gives it a launcher that records the vector.
 */
class GUCEF_MULTIPLAYPLUGIN_DESKTOP_EXPORT_CPP INotifyLauncher
{
    public:

    typedef GUCEF::vector< CORE::CString > TArgumentVector;

    INotifyLauncher( void );

    virtual ~INotifyLauncher();

    /** Whether the program that shows notifications can be started on this system */
    virtual bool IsAvailable( void ) = 0;

    /**
     *  Starts the program without waiting for it to end.
     *  @param argv  argv[ 0 ] is the name of the program, the other entries are its arguments, one each
     *  @return true when the program was started
     */
    virtual bool Launch( const TArgumentVector& argv ) = 0;

    /** The number of programs started by this launcher which are still running */
    virtual UInt32 GetRunningCount( void ) = 0;

    private:

    INotifyLauncher( const INotifyLauncher& src );
    INotifyLauncher& operator=( const INotifyLauncher& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DESKTOP */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_DESKTOP_INOTIFYLAUNCHER_H ? */
