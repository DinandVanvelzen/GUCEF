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

#ifndef GUCEF_CORE_CIPLUGIN_H
#define GUCEF_CORE_CIPLUGIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CIPLUGINMETADATA_H
#include "gucefCORE_CIPluginMetaData.h"
#define GUCEF_CORE_CIPLUGINMETADATA_H
#endif /* GUCEF_CORE_CIPLUGINMETADATA_H ? */

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
 *  Interface class for plugins
 */
class GUCEF_CORE_PUBLIC_CPP CIPlugin
{
    public:

    CIPlugin( void );

    CIPlugin( const CIPlugin& src );

    virtual ~CIPlugin();

    CIPlugin& operator=( const CIPlugin& src );

    virtual TPluginMetaDataPtr GetMetaData( void ) const = 0;

    virtual void* GetModulePointer( void ) = 0;

    /**
     *  True while code of the module is still needed after the plugin was unregistered, for example because objects
     *  the module made are still referenced. The plugin control then keeps the module loaded and asks again later,
     *  see CPluginControl::ProcessDeferredModuleUnloads(). Plugins whose module can always go return false.
     */
    virtual bool IsModuleInUse( void ) const;

    /**
     *  Called once by the plugin control right before it unloads the module, when IsModuleInUse() is false.
     *  The place for the last calls into the module and for dropping what points into it.
     */
    virtual void OnBeforeModuleUnload( void );
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CIPlugin, MT::CMutex > TPluginPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CIPLUGIN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 27-11-2004 :
        - Dinand: Initial implementation

---------------------------------------------------------------------------*/
