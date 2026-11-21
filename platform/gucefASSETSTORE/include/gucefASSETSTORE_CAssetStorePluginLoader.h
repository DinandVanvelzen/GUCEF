/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADER_H
#define GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPLUGINHOST_H
#include "gucefASSETSTORE_CIAssetStorePluginHost.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPLUGINHOST_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPLUGINHOST_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADREPORT_H
#include "gucefASSETSTORE_CAssetStorePluginLoadReport.h"
#define GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADREPORT_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADREPORT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Loads the asset store plugins of a folder that is not part of the application: the store plugins an owner keeps apart from the
 *  framework, for example in a private repository. The application gets the folder from its settings (empty by default, so a build or an
 *  installation without such plugins is as it was) and calls LoadFromDirectory() before it makes its asset store service.
 *
 *  A plugin declares itself an asset store plugin with the exports every plugin of the framework has: it is a generic plugin whose
 *  GUCEFPlugin_GetLinkBackModuleDependencies() names gucefASSETSTORE, the module it registers its store with (the provider factory
 *  and the registry of CAssetStoreGlobal). Every library of the folder is looked at without making it a plugin first; only what
 *  declares itself is loaded, with LoadFailAllowed set: a library that does not load is refused with its reason and the others go on.
 *  A library that is no plugin (a helper library of a plugin, loaded by the plugin itself) and a plugin of another kind are left alone.
 *  Everything that is loaded, refused or left alone is logged and in the report; the report holds no secret.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStorePluginLoader
{
    public:

    /** The module whose name in the link back dependencies of a plugin makes it an asset store plugin: "gucefASSETSTORE" */
    static const CORE::CString StorePluginDependencyName;

    /** The plugin group of the plugin control that the plugins are loaded in: "AssetStorePrivate" */
    static const CORE::CString PluginGroupName;

    /** 'host' does the inspecting and loading (not owned, it has to be there as long as the loader); GUCEF_NULL for the system host */
    explicit CAssetStorePluginLoader( CIAssetStorePluginHost* host = GUCEF_NULL );

    ~CAssetStorePluginLoader();

    /**
     *  Loads the asset store plugins of the folder. An empty folder is no error (nothing is asked for), a folder that does not exist is
     *  reported and logged as a warning. True when a folder was set, it exists and no store plugin in it was refused
     */
    bool LoadFromDirectory( const CORE::CString& directory, CAssetStorePluginLoadReport& report );

    CIAssetStorePluginHost& GetHost( void );

    /** The host that uses the plugin control of gucefCORE and the dynamic module loading of the platform */
    static CIAssetStorePluginHost& GetSystemHost( void );

    private:

    CAssetStorePluginLoader( const CAssetStorePluginLoader& src );              /**< not implemented */
    CAssetStorePluginLoader& operator=( const CAssetStorePluginLoader& src );   /**< not implemented */

    private:

    CIAssetStorePluginHost* m_host;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADER_H ? */
