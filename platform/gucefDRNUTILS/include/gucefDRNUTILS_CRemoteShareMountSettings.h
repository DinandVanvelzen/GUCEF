/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CREMOTESHAREMOUNTSETTINGS_H
#define GUCEF_DRNUTILS_CREMOTESHAREMOUNTSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a share of a remote service needs to be mounted into the VFS (CRemoteShareMount), and the form it has in a
 *  configuration ("Share" entries of the section "RemoteShares", see CRemoteShareMounts):
 *
 *    service        the credential slot of the paired service the share belongs to (CRemoteShareMounts reads it, CRemoteShareMount does not)
 *    share          the name the service shares the files under
 *    mountPoint     the VFS path the share is mounted at
 *    channel        "auto" (default) or the index of a channel of the connection: 0 is the control channel, 1.. are the bulk channels. "auto" is
 *                   the first bulk channel when the connection has bulk channels, else the control channel. The channel is the DRN session whose
 *                   resource transfer carries the files, and that session is the one that is registered by name in the DRN session registry
 *    BridgeSettings child with the settings of the bridge archive as attributes (metadataTtlMs, maxOpenHandles, verifyMode, ...), by the names
 *                   that the README of the VFS plugin for DRN lists; they are handed to the archive as its archive specific settings
 *
 *  LoadConfig() refuses a key it does not know (logged) and keeps the settings as they were.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CRemoteShareMountSettings : public CORE::CIConfigurable
{
    public:

    /** The value of 'channel' that stands for "auto" */
    static const UInt32 AutoChannel;

    CORE::CString service;                  /**< the credential slot of the service; default empty */
    CORE::CString share;                    /**< the share's name at the service; default empty */
    CORE::CString mountPoint;               /**< the VFS path to mount at; default empty */
    UInt32 channel;                         /**< AutoChannel, or the index of the channel; default AutoChannel */
    CORE::CDataNode bridgeSettings;         /**< the attributes are the archive specific settings of the bridge archive; default none (the bridge's defaults) */

    CRemoteShareMountSettings( void );
    CRemoteShareMountSettings( const CRemoteShareMountSettings& src );
    virtual ~CRemoteShareMountSettings() GUCEF_VIRTUAL_OVERRIDE;
    CRemoteShareMountSettings& operator=( const CRemoteShareMountSettings& src );

    /** Whether the settings can be mounted: a share and a mount point are there; the reason when not */
    bool IsValid( CORE::CString& reason ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CREMOTESHAREMOUNTSETTINGS_H ? */
