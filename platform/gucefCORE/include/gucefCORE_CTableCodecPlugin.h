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

#ifndef GUCEF_CORE_CTABLECODECPLUGIN_H
#define GUCEF_CORE_CTABLECODECPLUGIN_H

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

#ifndef GUCEF_CORE_CGUARDEDTABLECODEC_H
#include "gucefCORE_CGuardedTableCodec.h"
#define GUCEF_CORE_CGUARDEDTABLECODEC_H
#endif /* GUCEF_CORE_CGUARDEDTABLECODEC_H ? */

#ifndef GUCEF_CORE_CTABLECODECREGISTRY_H
#include "gucefCORE_CTableCodecRegistry.h"
#define GUCEF_CORE_CTABLECODECREGISTRY_H
#endif /* GUCEF_CORE_CTABLECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_TABLECODECPLUGINAPI_H
#include "gucefCORE_TableCodecPluginAPI.h"
#define GUCEF_CORE_TABLECODECPLUGINAPI_H
#endif /* GUCEF_CORE_TABLECODECPLUGINAPI_H ? */

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
 *  A table codec plugin module, linked through its TABLECODECPLUGIN_* exports (gucefCORE_TableCodecPluginAPI.h).
 *
 *  The module is a C++ link-back module: it links gucefCORE and implements CITableCodec directly, so batches, pooled
 *  buffers and in place spans cross the module boundary without marshalling. The API version ties the module to the
 *  C++ interface of this gucefCORE; a module of another major version is refused and nothing else of it is called.
 *
 *  The plugin registers the codecs of the module in a CTableCodecRegistry with TryRegister() and takes them out with
 *  TryUnregister() when the plugin is unregistered. The module itself may only go when no codec object of it is
 *  referenced any more and no object made by its code is alive: IsModuleInUse() says so and the plugin control keeps
 *  the module loaded until it is false, then calls OnBeforeModuleUnload(), which drops the codecs and calls
 *  TABLECODECPLUGIN_Shutdown(). So no object outlives its code.
 *
 *  The codecs of the module are registered behind a CGuardedTableCodec. Readers, writers, file metadata and clones that
 *  a codec makes have their code in the module as well; the guard hands out stand-ins of its own for them (code of
 *  gucefCORE) that forward to the objects of the module and count themselves in a CTableCodecModuleLifetime. The module
 *  therefore stays loaded as long as any of them is alive, whether or not the codec pointer they came from is still held,
 *  and the count drops only after the destructor of the object of the module has returned. The unload itself never runs
 *  inside such a destructor: it is done by CPluginControl::ProcessDeferredModuleUnloads() on the thread that calls it.
 */
class GUCEF_CORE_PUBLIC_CPP CTableCodecPlugin : public CIPlugin
{
    public:

    typedef CTableCodecRegistry::TTableCodecPtr         TTableCodecPtr;
    typedef GUCEF::vector< TTableCodecPtr >             TTableCodecPtrVector;

    CTableCodecPlugin( void );

    virtual ~CTableCodecPlugin() GUCEF_VIRTUAL_OVERRIDE;

    virtual TPluginMetaDataPtr GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void* GetModulePointer( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsModuleInUse( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnBeforeModuleUnload( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Looks up the exports of the module, checks the API version, initializes the module and makes its codecs.
     *  False, with the module left alone, when it is not a table codec plugin (no exports), is of another API
     *  version (logged with both versions), refuses to initialize or is still in use from an earlier load.
     */
    bool Link( void* modulePtr                   ,
               TPluginMetaDataPtr pluginMetaData );

    /** True when the plugin API version of a module works with the one of this host: same major, minor not above the host's */
    static bool IsApiVersionCompatible( UInt32 pluginApiVersion, UInt32 hostApiVersion );

    /**
     *  Registers the codecs of the module with TryRegister(). A codec whose type name is taken is logged and left out.
     *  Returns the number registered.
     */
    UInt32 RegisterCodecs( CTableCodecRegistry& registry );

    /** Takes the codecs of this plugin out of the registry, only those that are registered under their name by this plugin */
    void UnregisterCodecs( CTableCodecRegistry& registry );

    UInt32 GetCodecCount( void ) const;

    /** The type names of the codecs registered by RegisterCodecs() */
    void GetRegisteredTypeNames( CString::StringVector& typeNames ) const;

    /** True from a successful Link() until the module is shut down */
    bool IsLinked( void ) const;

    private:

    CTableCodecPlugin( const CTableCodecPlugin& src );              /**< not implemented */
    CTableCodecPlugin& operator=( const CTableCodecPlugin& src );   /**< not implemented */

    void Shutdown( void );

    private:

    void* m_moduleHandle;
    TDefaultFuncPtr m_funcPointers[ 8 ];
    TTableCodecModuleLifetimePtr m_lifetime;            /**< the objects made by the code of the module that are alive */
    TTableCodecPtrVector m_codecs;
    CString::StringVector m_registeredTypeNames;      /**< per entry of m_codecs, empty when it was not registered */
    TPluginMetaDataStoragePtr m_metaData;
    bool m_initialized;
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CTableCodecPlugin, MT::CMutex >    TTableCodecPluginPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTABLECODECPLUGIN_H ? */
