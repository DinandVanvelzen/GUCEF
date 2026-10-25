/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#include "gucefMULTIPLAY_CLinkProof.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CLinkProof::MaxCount;
const UInt32 CLinkProof::MaxProviderNameLength;
const UInt32 CLinkProof::MaxKindLength;
const UInt32 CLinkProof::MaxDataSize;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLinkProof::CLinkProof( void )
    : providerName()
    , proof()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLinkProof::CLinkProof( const CORE::CString& provider, const AUTH::CAuthProof& authProof )
    : providerName( provider )
    , proof( authProof )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLinkProof::CLinkProof( const CLinkProof& src )
    : providerName( src.providerName )
    , proof( src.proof )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLinkProof::~CLinkProof()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLinkProof&
CLinkProof::operator=( const CLinkProof& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        providerName = src.providerName;
        proof = src.proof;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CLinkProof::IsSendable( void ) const
{GUCEF_TRACE;

    // ByteSize() counts the terminating zero
    if ( providerName.IsNULLOrEmpty() || providerName.ByteSize() - 1 > MaxProviderNameLength )
        return false;
    if ( proof.IsEmpty() || proof.kind.IsNULLOrEmpty() || proof.kind.ByteSize() - 1 > MaxKindLength )
        return false;
    return proof.data.Reveal().ByteSize() - 1 <= MaxDataSize;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
