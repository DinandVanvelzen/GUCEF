/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNRPCTRANSPORTSETTINGS_H
#define GUCEF_DRN_CDRNRPCTRANSPORTSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Settings of the "drn" RPC transport and of its listener (CDRNRpcTransport, CDRNRpcTransportListener).
 *  The timing of the RPC session (ack delay, keepalive) is the endpoint's: the transport profile "drn" of
 *  COMCORE::CRpcTransportProfile, or the endpoint's own profile.
 *  Config section "DRNRpcTransportSettings".
 */
class GUCEF_DRN_EXPORT_CPP CDRNRpcTransportSettings : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ClassTypeName;

    UInt32 maxFrameSize;            /**< largest RPC frame, default 16384; datagram channels use the link's unfragmented payload below it */
    bool requireProtectedLink;      /**< refuse a member link without packet protection, default true */
    UInt32 maxSessionsPerMember;    /**< listener: RPC sessions one member may hold open, default 4, 0 = no maximum */

    CDRNRpcTransportSettings( void );
    CDRNRpcTransportSettings( const CDRNRpcTransportSettings& src );
    virtual ~CDRNRpcTransportSettings();
    CDRNRpcTransportSettings& operator=( const CDRNRpcTransportSettings& src );

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRPCTRANSPORTSETTINGS_H ? */
