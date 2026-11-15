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

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H
#define GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUETASKCONSUMER_H
#include "pluginglueOGRE_COgreWorkQueueTaskConsumer.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUETASKCONSUMER_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREWORKQUEUETASKCONSUMER_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREFUTUREBRIDGE_H
#include "pluginglueOGRE_COgreFutureBridge.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREFUTUREBRIDGE_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREFUTUREBRIDGE_H ? */

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

class COgreLogAdapter;
class COgrePluginManager;
class COgreRootSetup;

/*-------------------------------------------------------------------------*/

/**
 *  Singular singleton providing access to the global systems of the Ogre support library.
 *
 *  The task type of the Ogre work queue is registered with the task manager first, then the log adapter is made, so the
 *  Ogre log manager exists before the Root, then the root setup (which makes the Root and gives it the work queue on a
 *  GUCEF thread pool), the plugin manager and the future bridge. They are taken down in the opposite order, so the Ogre
 *  plugins that are loaded through the plugin manager are uninstalled while the Root and its log still exist, and the
 *  work queue is shut down with the Root before its task type goes.
 */
class GUCEF_PLUGINGLUE_OGRE_EXPORT_CPP COgreGlobal
{
    public:

    static COgreGlobal* Instance( void );

    COgreLogAdapter& GetLogAdapter( void );

    COgreRootSetup& GetRootSetup( void );

    COgrePluginManager& GetPluginManager( void );

    /** Ogre's background resource preparation and loading with GUCEF futures */
    COgreFutureBridge& GetFutureBridge( void );

    /** Destroys the singleton and with it the Root that the root setup made, called when the library is unloaded */
    static void Deinstance( void );

    private:

    COgreGlobal( void );
    ~COgreGlobal();
    COgreGlobal( const COgreGlobal& src );              /**< not implemented */
    COgreGlobal& operator=( const COgreGlobal& src );   /**< not implemented */

    void Initialize( void );

    private:

    COgreLogAdapter* m_logAdapter;
    COgreRootSetup* m_rootSetup;
    COgrePluginManager* m_pluginManager;
    COgreFutureBridgePtr m_futureBridge;
    TOgreWorkQueueTaskConsumerFactory m_workQueueTaskConsumerFactory;

    static COgreGlobal* g_instance;
    static MT::CMutex g_dataLock;
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

#endif /* GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H ? */
