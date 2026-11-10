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

#include "gucefDRN_CDRNAnonymousAuthenticator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNAnonymousAuthenticator::MethodName = "anonymous";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNAnonymousAuthenticator::CDRNAnonymousAuthenticator( void )
    : CIDRNAuthenticator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNAnonymousAuthenticator::~CDRNAnonymousAuthenticator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNAnonymousAuthenticator::GetMethodName( void ) const
{GUCEF_TRACE;

    return MethodName;
}

/*-------------------------------------------------------------------------*/

bool
CDRNAnonymousAuthenticator::CreateProof( const CDRNAuthContext& /* context */, CORE::CDynamicBuffer& proof )
{GUCEF_TRACE;

    proof.SetDataSize( 0 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNAnonymousAuthenticator::VerifyProof( const CDRNAuthContext& /* context */, const UInt8* /* proof */, UInt32 /* proofSize */, CDRNAuthResult& result )
{GUCEF_TRACE;

    result.userId.Clear();
    result.role.Clear();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNAnonymousAuthenticator::SaveConfig( CORE::CDataNode& /* cfg */ ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNAnonymousAuthenticator::LoadConfig( const CORE::CDataNode& /* cfg */ )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNAnonymousAuthenticator::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNAnonymousAuthenticator";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
