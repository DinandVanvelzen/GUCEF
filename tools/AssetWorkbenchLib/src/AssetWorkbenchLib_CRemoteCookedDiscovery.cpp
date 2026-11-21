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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#include "gucefASSETSTORE_CIRemoteFileSource.h"
#define GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H
#endif /* GUCEF_ASSETSTORE_CREMOTEASSETFILESYSTEM_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCACHEENTRY_H
#include "AssetConverterLib_CAssetCacheEntry.h"
#define ASSETCONVERTERLIB_CASSETCACHEENTRY_H
#endif /* ASSETCONVERTERLIB_CASSETCACHEENTRY_H ? */

#ifndef ASSETWORKBENCHLIB_CCOOKEDOUTPUTRESOLVER_H
#include "AssetWorkbenchLib_CCookedOutputResolver.h"
#define ASSETWORKBENCHLIB_CCOOKEDOUTPUTRESOLVER_H
#endif /* ASSETWORKBENCHLIB_CCOOKEDOUTPUTRESOLVER_H ? */

#include "AssetWorkbenchLib_CRemoteCookedDiscovery.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteCookedDiscovery::CRemoteCookedDiscovery( const CORE::CString& clientPath, const CORE::CString& profile )
    : ASSETSTORE::CIRemoteFileDiscovery()
    , m_clientPath( clientPath )
    , m_profile( profile )
    , m_found( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteCookedDiscovery::~CRemoteCookedDiscovery()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteCookedDiscovery::GetFoundCount( void ) const
{
    return m_found;
}

/*-------------------------------------------------------------------------*/

void
CRemoteCookedDiscovery::Discover( ASSETSTORE::CIRemoteFileSource& system, CORE::CString::StringVector& servicePaths )
{GUCEF_TRACE;

    m_found = 0;
    CCookedOutputResolver::SOptions options;
    if ( !m_profile.IsNULLOrEmpty() )
        options.profile = m_profile;
    CCookedOutputResolver::SCookedOutput output;
    CCookedOutputResolver::LocateInStore( m_clientPath, CORE::CString::Empty, options, output );
    if ( !output.IsLocated() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CRemoteCookedDiscovery: " << m_clientPath << ": " << CCookedOutputResolver::Describe( output ) );
        return;
    }
    CORE::CString servicePath;
    const CORE::CString entryPath = ASSETCONVERTER::CAssetCacheEntry::GetEntryPath( output.intermediateFolder, output.assetPath );
    if ( system.ToServicePath( entryPath, servicePath ) )
        servicePaths.push_back( servicePath );
    for ( size_t i=0; i<output.files.size(); ++i )
    {
        if ( system.ToServicePath( output.files[ i ], servicePath ) )
        {
            servicePaths.push_back( servicePath );
            ++m_found;
        }
    }
    if ( !output.sidecarFile.IsNULLOrEmpty() && system.ToServicePath( output.sidecarFile, servicePath ) )
        servicePaths.push_back( servicePath );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CRemoteCookedDiscovery: " << m_clientPath << " has " << m_found << " cooked files for the profile " << options.profile );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
