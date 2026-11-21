/*
 *  AssetWorkbenchLib: the cooked output of an asset of a remote store as files to fetch
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

#ifndef ASSETWORKBENCHLIB_CREMOTECOOKEDDISCOVERY_H
#define ASSETWORKBENCHLIB_CREMOTECOOKEDDISCOVERY_H

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
 *  The files that a model of a remote store needs besides its own when it is shown from the cooked output of the pack: the cache entry
 *  of the converter, the cooked files of the profile and the conversion sidecar (CCookedOutputResolver on the VFS path of the original, in
 *  the layout of the store: "original/", "intermediate/" and "gucef-ogre3d/" per pack). It runs after the files of the asset are in the
 *  cache, on the thread of the transfer, so reading the cache entry through the VFS may wait for the service. What the resolver asked of
 *  the VFS is known to the file system afterwards (the lists of the folders), so the thread of the interface finds the cooked output
 *  without a wait.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CRemoteCookedDiscovery : public ASSETSTORE::CIRemoteFileDiscovery
{
    public:

    /** 'clientPath' is the VFS path of the original as the views open it; 'profile' the cook profile, empty for the default */
    CRemoteCookedDiscovery( const CORE::CString& clientPath, const CORE::CString& profile );

    virtual ~CRemoteCookedDiscovery() GUCEF_VIRTUAL_OVERRIDE;

    virtual void Discover( ASSETSTORE::CIRemoteFileSource& system, CORE::CString::StringVector& servicePaths ) GUCEF_VIRTUAL_OVERRIDE;

    /** How many cooked files the last Discover() found */
    UInt32 GetFoundCount( void ) const;

    private:

    CRemoteCookedDiscovery( const CRemoteCookedDiscovery& src );
    CRemoteCookedDiscovery& operator=( const CRemoteCookedDiscovery& src );

    private:

    CORE::CString m_clientPath;
    CORE::CString m_profile;
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

#endif /* ASSETWORKBENCHLIB_CREMOTECOOKEDDISCOVERY_H ? */
