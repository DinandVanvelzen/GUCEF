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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

#include "gucefDRN_CIDRNAccessPolicy.h"

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

CIDRNAccessPolicy::CIDRNAccessPolicy( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIDRNAccessPolicy::~CIDRNAccessPolicy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIDRNAccessPolicy::MaySubscribeAs( UInt32 peerId, const CDRNAuthResult& /* auth */, const CORE::CString& itemName )
{GUCEF_TRACE;

    return MaySubscribe( peerId, itemName );
}

/*-------------------------------------------------------------------------*/

bool
CIDRNAccessPolicy::MayWriteAs( UInt32 peerId, const CDRNAuthResult& /* auth */, const CORE::CString& itemName, UInt32 keyIndex )
{GUCEF_TRACE;

    return MayWrite( peerId, itemName, keyIndex );
}

/*-------------------------------------------------------------------------*/

static bool
IsRoleIn( const CORE::CString::StringSet& roles, const CORE::CString& role )
{GUCEF_TRACE;

    return roles.empty() || roles.find( role ) != roles.end();
}

/*-------------------------------------------------------------------------*/

CDRNDefaultAccessPolicy::CDRNDefaultAccessPolicy( void )
    : CIDRNAccessPolicy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDefaultAccessPolicy::~CDRNDefaultAccessPolicy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNDefaultAccessPolicy::SetJoinRoles( const CORE::CString::StringSet& roles )
{GUCEF_TRACE;

    m_joinRoles = roles;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringSet&
CDRNDefaultAccessPolicy::GetJoinRoles( void ) const
{GUCEF_TRACE;

    return m_joinRoles;
}

/*-------------------------------------------------------------------------*/

void
CDRNDefaultAccessPolicy::SetSubscribeRoles( const CORE::CString::StringSet& roles )
{GUCEF_TRACE;

    m_subscribeRoles = roles;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringSet&
CDRNDefaultAccessPolicy::GetSubscribeRoles( void ) const
{GUCEF_TRACE;

    return m_subscribeRoles;
}

/*-------------------------------------------------------------------------*/

void
CDRNDefaultAccessPolicy::SetWriteRoles( const CORE::CString::StringSet& roles )
{GUCEF_TRACE;

    m_writeRoles = roles;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringSet&
CDRNDefaultAccessPolicy::GetWriteRoles( void ) const
{GUCEF_TRACE;

    return m_writeRoles;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDefaultAccessPolicy::MayJoin( const CDRNPeerLink& link, const CORE::CString& /* sessionName */ )
{GUCEF_TRACE;

    return IsRoleIn( m_joinRoles, link.GetAuthResult().role );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDefaultAccessPolicy::MaySubscribeAs( UInt32 /* peerId */, const CDRNAuthResult& auth, const CORE::CString& /* itemName */ )
{GUCEF_TRACE;

    return IsRoleIn( m_subscribeRoles, auth.role );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDefaultAccessPolicy::MayWriteAs( UInt32 /* peerId */, const CDRNAuthResult& auth, const CORE::CString& /* itemName */, UInt32 /* keyIndex */ )
{GUCEF_TRACE;

    return IsRoleIn( m_writeRoles, auth.role );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDefaultAccessPolicy::MaySubscribe( UInt32 /* peerId */, const CORE::CString& /* itemName */ )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDefaultAccessPolicy::MayWrite( UInt32 /* peerId */, const CORE::CString& /* itemName */, UInt32 /* keyIndex */ )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
