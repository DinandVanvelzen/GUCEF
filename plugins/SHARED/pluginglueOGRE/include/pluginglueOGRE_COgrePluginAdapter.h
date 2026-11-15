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

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREPLUGINADAPTER_H
#define GUCEF_PLUGINGLUE_OGRE_COGREPLUGINADAPTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_CIPLUGIN_H
#include "CIPlugin.h"
#define GUCEF_CORE_CIPLUGIN_H
#endif /* GUCEF_CORE_CIPLUGIN_H ? */

#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#include "gucefCORE_CPluginMetaData.h"
#define GUCEF_CORE_CPLUGINMETADATA_H
#endif /* GUCEF_CORE_CPLUGINMETADATA_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

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
 *  A plugin of Ogre (a render system, a codec, an archive factory) as a plugin of GUCEF.
 *
 *  A module that exports dllStartPlugin() and dllStopPlugin(), as every Ogre plugin does, is linked by calling
 *  dllStartPlugin() which installs the plugin in the Ogre Root; unlinking calls dllStopPlugin().
 *
 *  Where modules cannot be loaded (WebAssembly) the Ogre plugin is linked into the application and has no such
 *  exports. The application then registers the plugin object under the name of the plugin in
 *  COgrePluginManager::RegisterStaticOgrePlugin(), and linking installs that object with Root::installPlugin()
 *  and unlinking uninstalls it. The same registry serves modules that have the exports, when the application
 *  prefers to hand over the object.
 */
class GUCEF_PLUGINGLUE_OGRE_EXPORT_CPP COgrePluginAdapter : public CORE::CIPlugin
{
    public:

    COgrePluginAdapter( void );

    virtual ~COgrePluginAdapter() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TPluginMetaDataPtr GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void* GetModulePointer( void ) GUCEF_VIRTUAL_OVERRIDE;

    void SetPluginParams( const CORE::CValueList& pluginParams );

    const CORE::CValueList& GetPluginParams( void ) const;

    bool Link( void* modulePtr                         ,
               CORE::TPluginMetaDataPtr pluginMetaData );

    bool Unlink( void );

    private:

    COgrePluginAdapter( const COgrePluginAdapter& src );              /**< not implemented */
    COgrePluginAdapter& operator=( const COgrePluginAdapter& src );   /**< not implemented */

    bool IsLoaded( void ) const;

    private:

    enum EFuncPtrType
    {
        FUNCPTR_STARTPLUGIN ,
        FUNCPTR_STOPPLUGIN  ,
        FUNCPTR_COUNT
    };

    void* m_moduleHandle;
    CORE::TDefaultFuncPtr m_funcPointers[ FUNCPTR_COUNT ];
    Ogre::Plugin* m_staticPlugin;
    CORE::CValueList m_params;
    CORE::TPluginMetaDataStoragePtr m_metaData;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< COgrePluginAdapter, MT::CMutex >  TOgrePluginAdapterPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_OGRE_COGREPLUGINADAPTER_H ? */
