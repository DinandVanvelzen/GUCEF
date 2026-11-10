/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CTABLECODECPLUGINMANAGER_H
#define GUCEF_CORE_CTABLECODECPLUGINMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CPLUGINMANAGER_H
#include "CPluginManager.h"
#define GUCEF_CORE_CPLUGINMANAGER_H
#endif /* GUCEF_CORE_CPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_CTABLECODECPLUGIN_H
#include "gucefCORE_CTableCodecPlugin.h"
#define GUCEF_CORE_CTABLECODECPLUGIN_H
#endif /* GUCEF_CORE_CTABLECODECPLUGIN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Plugin manager for table codec plugins, plugin type "GucefTableCodecPlugin". Owned by CCoreGlobal.
 *
 *  Table codec plugins are C++ link-back modules that implement CITableCodec directly, see CTableCodecPlugin and
 *  gucefCORE_TableCodecPluginAPI.h. Registering a plugin puts its codecs in the table codec registry; unregistering
 *  takes them out again. The module is unloaded by the plugin control, which waits while codec objects of the module
 *  are still referenced (CTableCodecPlugin::IsModuleInUse()).
 */
class GUCEF_CORE_PUBLIC_CPP CTableCodecPluginManager : public CPluginManager
{
    public:

    virtual CString GetPluginType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual TPluginPtr RegisterPlugin( void* modulePtr                   ,
                                       TPluginMetaDataPtr pluginMetaData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool UnregisterPlugin( TPluginPtr plugin ) GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CCoreGlobal;

    CTableCodecPluginManager( void );

    virtual ~CTableCodecPluginManager() GUCEF_VIRTUAL_OVERRIDE;

    CTableCodecPluginManager( const CTableCodecPluginManager& src );            /**< not implemented */
    CTableCodecPluginManager& operator=( const CTableCodecPluginManager& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTABLECODECPLUGINMANAGER_H ? */
