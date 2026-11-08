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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

#include "gucefDRN_CDRNRpcTransportSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNRpcTransportSettings::ClassTypeName = "GUCEF::DRN::CDRNRpcTransportSettings";

/*-------------------------------------------------------------------------*/

CDRNRpcTransportSettings::CDRNRpcTransportSettings( void )
    : CORE::CIConfigurable()
    , maxFrameSize( 16384 )
    , requireProtectedLink( true )
    , maxSessionsPerMember( 4 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNRpcTransportSettings::CDRNRpcTransportSettings( const CDRNRpcTransportSettings& src )
    : CORE::CIConfigurable( src )
    , maxFrameSize( src.maxFrameSize )
    , requireProtectedLink( src.requireProtectedLink )
    , maxSessionsPerMember( src.maxSessionsPerMember )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNRpcTransportSettings::~CDRNRpcTransportSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNRpcTransportSettings&
CDRNRpcTransportSettings::operator=( const CDRNRpcTransportSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        maxFrameSize = src.maxFrameSize;
        requireProtectedLink = src.requireProtectedLink;
        maxSessionsPerMember = src.maxSessionsPerMember;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransportSettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = cfg.SetAttribute( "maxFrameSize", maxFrameSize ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "requireProtectedLink", requireProtectedLink ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxSessionsPerMember", maxSessionsPerMember ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcTransportSettings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    maxFrameSize = cfg.GetAttributeValueOrChildValueByName( "maxFrameSize" ).AsUInt32( maxFrameSize, true );
    requireProtectedLink = cfg.GetAttributeValueOrChildValueByName( "requireProtectedLink" ).AsBool( requireProtectedLink, true );
    maxSessionsPerMember = cfg.GetAttributeValueOrChildValueByName( "maxSessionsPerMember" ).AsUInt32( maxSessionsPerMember, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNRpcTransportSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
