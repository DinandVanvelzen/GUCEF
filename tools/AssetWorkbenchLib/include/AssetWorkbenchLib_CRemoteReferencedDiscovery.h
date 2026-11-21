/*
 *  AssetWorkbenchLib: the files that a model of a remote store refers to by name
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

#ifndef ASSETWORKBENCHLIB_CREMOTEREFERENCEDDISCOVERY_H
#define ASSETWORKBENCHLIB_CREMOTEREFERENCEDDISCOVERY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CIREMOTEFILEDISCOVERY_H
#include "gucefASSETSTORE_CIRemoteFileDiscovery.h"
#define GUCEF_ASSETSTORE_CIREMOTEFILEDISCOVERY_H
#endif /* GUCEF_ASSETSTORE_CIREMOTEFILEDISCOVERY_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The pictures, material libraries and data buffers that a model of a remote store names, which lie in the folder tree of its pack in the
 *  original area. A store does not always know the relations between the assets of a pack, and the model formats name their files in
 *  their own ways, so the model is read (through the VFS, from the cache) and every file of that kind in the tree of the pack whose name
 *  occurs in it is asked for. A material library that was found is read in the same way (a few rounds), as it names the pictures of the
 *  materials. It runs on the thread of the transfer, after the files of the asset are in the cache. It is the fallback for assets without
 *  relations: the companion files of an asset come from its relations in the catalogue first (CAssetFileSet, converter design 5.8).
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CRemoteReferencedDiscovery : public ASSETSTORE::CIRemoteFileDiscovery
{
    public:

    /** 'clientPath' is the VFS path of the model as the views open it, 'clientPackFolder' the folder of its pack in the original area */
    CRemoteReferencedDiscovery( const CORE::CString& clientPath, const CORE::CString& clientPackFolder );

    /**
     *  A discovery that learns its model from the file set of the asset (OnAssetFileSet()): the fallback of a download or of keep offline of
     *  one asset (CRemoteFileTransfer::AddFallbackDiscovery()), which the transfer asks only when the asset has no relations in the catalogue.
     *  The folder of the pack is that of the model in the original area ("<location>/original/<pack folder>"), else the folder of the model
     */
    CRemoteReferencedDiscovery( void );

    virtual void OnAssetFileSet( const ASSETSTORE::CAssetFileSet& fileSet ) GUCEF_VIRTUAL_OVERRIDE;

    /** The folder of the pack of a file of the service in the original area: "<location>/original/<pack folder>", else the folder of the file */
    static CORE::CString GetServicePackFolder( const CORE::CString& servicePath );

    virtual ~CRemoteReferencedDiscovery() GUCEF_VIRTUAL_OVERRIDE;

    virtual void Discover( ASSETSTORE::CRemoteAssetFileSystem& system, CORE::CString::StringVector& servicePaths ) GUCEF_VIRTUAL_OVERRIDE;

    /** How many files the last Discover() found */
    UInt32 GetFoundCount( void ) const;

    /** Whether files of the kind that a model refers to are looked for with this extension (lower case, without the dot) */
    static bool IsReferenceableExtension( const CORE::CString& extension );

    private:

    CRemoteReferencedDiscovery( const CRemoteReferencedDiscovery& src );
    CRemoteReferencedDiscovery& operator=( const CRemoteReferencedDiscovery& src );

    private:

    CORE::CString m_clientPath;
    CORE::CString m_clientPackFolder;
    CORE::CString m_servicePath;          /**< the model, as the service has it: from the file set when no client path was given */
    CORE::CString m_servicePackFolder;
    UInt32 m_found;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CREMOTEREFERENCEDDISCOVERY_H ? */
