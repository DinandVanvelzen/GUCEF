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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLICENCE_H
#include "gucefASSETSTORE_CAssetLicence.h"
#define GUCEF_ASSETSTORE_CASSETLICENCE_H
#endif /* GUCEF_ASSETSTORE_CASSETLICENCE_H ? */

#include "gucefASSETSTORE_CAssetFileServingGate.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetFileServingGate::SCaller::SCaller( void )
    : deviceId(), owner( false ), mcp( false ), learning( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetFileServingGate::SCaller::NeedsLook( void ) const
{GUCEF_TRACE;

    return !owner || learning || mcp;
}

/*-------------------------------------------------------------------------*/

CAssetFileServingGate::CAssetFileServingGate( CIAssetStoreService* service )
    : m_service( service )
    , m_lock()
    , m_granted()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetFileServingGate::SetService( CIAssetStoreService* service )
{
    m_service = service;
}

/*-------------------------------------------------------------------------*/

bool
CAssetFileServingGate::IsThumbnailPath( const CORE::CString& path )
{GUCEF_TRACE;

    return path.HasSubstr( "/thumbnails/" ) >= 0;
}

/*-------------------------------------------------------------------------*/

CAssetFileServingGate::EDecision
CAssetFileServingGate::Decide( const SCaller& caller, const CORE::CString::StringSet& flags, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( flags.find( CAssetLicence::NoRedistribution ) != flags.end() && !caller.owner )
    {
        reason = "The licence of this pack does not allow its files to leave the devices of the owner";
        return RESTRICTED_NO_REDISTRIBUTION;
    }
    if ( flags.find( CAssetLicence::NoMachineLearning ) != flags.end() && caller.learning )
    {
        reason = "There is no such file";
        return RESTRICTED_NO_MACHINE_LEARNING;
    }
    return ALLOWED;
}

/*-------------------------------------------------------------------------*/

CAssetFileServingGate::EDecision
CAssetFileServingGate::CheckFile( const SCaller& caller, const CORE::CString& servicePath, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !caller.NeedsLook() || GUCEF_NULL == m_service )
        return ALLOWED;
    if ( caller.mcp && !caller.owner )
    {
        reason = "The content of the files is withheld from the machine role: the metadata of every pack is available";
        return WITHHELD_FROM_MCP;
    }
    CORE::CString::StringSet flags;
    const bool inPack = m_service->GetRestrictionsOfPath( servicePath, flags );
    if ( !inPack )
    {
        // the file of a thumbnail is in no pack: allowed for the pictures that the device was given
        if ( IsThumbnailPath( servicePath ) && !caller.owner )
        {
            MT::CScopeMutex lock( m_lock );
            GUCEF::map< CORE::CString, CORE::CString::StringSet >::const_iterator g = m_granted.find( caller.deviceId );
            if ( g == m_granted.end() || g->second.find( servicePath ) == g->second.end() )
            {
                reason = "This picture was not given to this device";
                return RESTRICTED_NO_REDISTRIBUTION;
            }
        }
        return ALLOWED;
    }
    return Decide( caller, flags, reason );
}

/*-------------------------------------------------------------------------*/

CAssetFileServingGate::EDecision
CAssetFileServingGate::CheckPack( const SCaller& caller, const CORE::CString& packId, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !caller.NeedsLook() || GUCEF_NULL == m_service )
        return ALLOWED;
    if ( caller.mcp && !caller.owner )
    {
        reason = "The pictures of the assets are withheld from the machine role: the metadata of every pack is available";
        return WITHHELD_FROM_MCP;
    }
    CAssetLicence licence;
    if ( ASSETSTORE_STATUS_OK != m_service->GetLicence( packId, licence ) )
        return ALLOWED;
    return Decide( caller, licence.restrictions, reason );
}

/*-------------------------------------------------------------------------*/

CAssetFileServingGate::EDecision
CAssetFileServingGate::CheckThumbnailId( const SCaller& caller, const CORE::CString& id, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( !caller.NeedsLook() || GUCEF_NULL == m_service )
        return ALLOWED;
    if ( caller.mcp && !caller.owner )
    {
        reason = "The pictures of the assets are withheld from the machine role: the metadata of every pack is available";
        return WITHHELD_FROM_MCP;
    }
    CORE::CString packId;
    if ( 0 == id.HasSubstr( "pack:" ) )
        packId = id.CutChars( 5, true );
    else
    {
        CAssetRecord asset;
        if ( ASSETSTORE_STATUS_OK != m_service->GetAsset( id, asset ) )
            return ALLOWED;
        packId = asset.packId;
    }
    return CheckPack( caller, packId, reason );
}

/*-------------------------------------------------------------------------*/

void
CAssetFileServingGate::GrantThumbnail( const CORE::CString& deviceId, const CORE::CString& vfsPath )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CORE::CString::StringSet& granted = m_granted[ deviceId ];
    if ( granted.size() >= 8192 )
        granted.clear();
    granted.insert( vfsPath );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
