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

#ifndef GUCEF_ASSETSTORE_CIREMOTEFILEDISCOVERY_H
#define GUCEF_ASSETSTORE_CIREMOTEFILEDISCOVERY_H

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

class CRemoteAssetFileSystem;
class CAssetFileSet;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a CRemoteFileTransfer of the mode PREFETCH asks after the files it was given are in the cache: more files that the
 *  application knows to be needed (the cooked output of a model, found by reading the files that are in the cache now). It runs on the
 *  thread of the transfer, so it may read through the VFS and wait for the service. A fallback discovery
 *  (CRemoteFileTransfer::AddFallbackDiscovery()) is asked in the other modes too, but only for assets without relations.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CIRemoteFileDiscovery
{
    public:

    virtual ~CIRemoteFileDiscovery() {}

    /** Adds the paths of the service (as the service has them) of the files that are needed as well */
    virtual void Discover( CRemoteAssetFileSystem& system, CORE::CString::StringVector& servicePaths ) = 0;

    /**
     *  The file set of an asset that the transfer added (CRemoteFileTransfer::AddAsset()), before Discover() is asked: a discovery
     *  that was made without knowing the asset (a fallback discovery of a download) takes its main file from it. Nothing by default
     */
    virtual void OnAssetFileSet( const CAssetFileSet& fileSet ) { (void) fileSet; }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CIREMOTEFILEDISCOVERY_H ? */
