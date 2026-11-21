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

#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPLUGINHOST_H
#define GUCEF_ASSETSTORE_CIASSETSTOREPLUGINHOST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

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

/** What a library of a plugin folder turned out to be */
enum EAssetStorePluginKind
{
    ASSETSTORE_PLUGINKIND_BROKEN = 0    ,   /**< the library cannot be loaded (damaged, not a library, a library it needs is missing) */
    ASSETSTORE_PLUGINKIND_NOT_A_PLUGIN  ,   /**< a library that is no plugin of the framework */
    ASSETSTORE_PLUGINKIND_OTHER_PLUGIN  ,   /**< a plugin of the framework that is no asset store plugin */
    ASSETSTORE_PLUGINKIND_STORE_PLUGIN      /**< a plugin that declares itself an asset store plugin */
};
typedef enum EAssetStorePluginKind TAssetStorePluginKind;

/**
 *  What CAssetStorePluginLoader needs of the system, so that it can be driven by a fake: finding out what a library is, and loading
 *  it as a plugin. The loader has a host that does this with the plugin control of gucefCORE (CAssetStorePluginLoader::GetSystemHost()).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CIAssetStorePluginHost
{
    public:

    CIAssetStorePluginHost( void );

    virtual ~CIAssetStorePluginHost();

    /** Finds out what the library at the path is, without making it a plugin; 'reason' says why for a broken library */
    virtual TAssetStorePluginKind InspectModule( const CORE::CString& modulePath, CORE::CString& reason ) = 0;

    /** Loads the library as a plugin and lets it register; false with the reason when it fails */
    virtual bool LoadPlugin( const CORE::CString& directory, const CORE::CString& moduleFileName, CORE::CString& reason ) = 0;

    private:

    CIAssetStorePluginHost( const CIAssetStorePluginHost& src );                /**< not implemented */
    CIAssetStorePluginHost& operator=( const CIAssetStorePluginHost& src );     /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPLUGINHOST_H ? */
