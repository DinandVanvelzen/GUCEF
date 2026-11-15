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

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREPLUGINMANAGER_H
#define GUCEF_PLUGINGLUE_OGRE_COGREPLUGINMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPLUGINMANAGER_H
#include "CPluginManager.h"
#define GUCEF_CORE_CPLUGINMANAGER_H
#endif /* GUCEF_CORE_CPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_MACROS_H
#include "pluginglueOGRE_macros.h"
#define GUCEF_PLUGINGLUE_OGRE_MACROS_H
#endif /* GUCEF_PLUGINGLUE_OGRE_MACROS_H ? */

namespace Ogre { class Plugin; }

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
 *  Plugin manager of the plugin type "OgrePlugin": the Ogre plugins that the bootstrap configuration lists
 *  (render systems, codecs, archive factories) are loaded through GUCEF plugin control and installed in the Ogre
 *  Root, in the order of the bootstrap configuration.
 */
class GUCEF_PLUGINGLUE_OGRE_EXPORT_CPP COgrePluginManager : public CORE::CPluginManager
{
    public:

    COgrePluginManager( void );

    virtual ~COgrePluginManager() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetPluginType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Makes an Ogre plugin object known under the name of its module file (without directory and extension), for
     *  platforms where Ogre plugins are linked into the application. The object must stay valid until it is
     *  unregistered or the plugin is unloaded
     */
    static bool RegisterStaticOgrePlugin( const CORE::CString& pluginName, Ogre::Plugin* plugin );

    static bool UnregisterStaticOgrePlugin( const CORE::CString& pluginName );

    /** The plugin object registered under the name, NULL when there is none */
    static Ogre::Plugin* GetStaticOgrePlugin( const CORE::CString& pluginName );

    /**
     *  For an Ogre plugin that is linked into the application (WebAssembly, Android): makes the module file name of
     *  the bootstrap entry known to the module loader of gucefCORE (CORE::RegisterStaticModule(), so loading the entry
     *  finds the application instead of a file) and registers the plugin object under the name of the module
     *  (RegisterStaticOgrePlugin()). 'moduleFilename' is the name as the bootstrap configuration gives it to the
     *  loader. The object must stay valid until the plugin is unloaded
     */
    static bool RegisterLinkedOgrePlugin( const CORE::CString& moduleFilename, Ogre::Plugin* plugin );

    /** The name of an Ogre plugin by its module file: without directory, extension, "lib" prefix and "_d" suffix */
    static CORE::CString GetOgrePluginNameOfModule( const CORE::CString& moduleFilename );

    protected:

    virtual CORE::TPluginPtr RegisterPlugin( void* modulePtr                         ,
                                             CORE::TPluginMetaDataPtr pluginMetaData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool UnregisterPlugin( CORE::TPluginPtr plugin ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    COgrePluginManager( const COgrePluginManager& src );              /**< not implemented */
    COgrePluginManager& operator=( const COgrePluginManager& src );   /**< not implemented */
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

#endif /* GUCEF_PLUGINGLUE_OGRE_COGREPLUGINMANAGER_H ? */
