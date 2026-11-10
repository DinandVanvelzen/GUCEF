/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H
#include "gucefAUTH_CAuthProviderDescriptor.h"
#define GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H
#endif /* GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H ? */

#ifndef GUCEF_AUTH_CAUTHPROVIDERREGISTRY_H
#include "gucefAUTH_CAuthProviderRegistry.h"
#define GUCEF_AUTH_CAUTHPROVIDERREGISTRY_H
#endif /* GUCEF_AUTH_CAUTHPROVIDERREGISTRY_H ? */

#include "gucefAUTH_Names.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
GetProviderDisplayName( const CORE::CString& providerName )
{GUCEF_TRACE;

    CAuthProviderDescriptor descriptor;
    if ( CAuthGlobal::Instance()->GetDescriptorRegistry().GetDescriptor( providerName, descriptor ) && !descriptor.displayName.IsNULLOrEmpty() )
        return descriptor.displayName;
    if ( providerName.IsNULLOrEmpty() )
        return providerName;
    return providerName.SubstrToIndex( 1, true ).Uppercase() + providerName.SubstrToIndex( 1, false );
}

/*-------------------------------------------------------------------------*/

CORE::CString
GetAccountProviderDisplayName( const CORE::CString& accountProvider )
{GUCEF_TRACE;

    // the descriptor with that name makes accounts of that namespace in most cases, else look for one that does
    CAuthProviderRegistry& registry = CAuthGlobal::Instance()->GetDescriptorRegistry();
    CAuthProviderDescriptor descriptor;
    if ( registry.GetDescriptor( accountProvider, descriptor ) && descriptor.accountProviderId == accountProvider && !descriptor.displayName.IsNULLOrEmpty() )
        return descriptor.displayName;
    CORE::CString::StringVector names;
    registry.GetNames( names );
    CORE::CString found;
    UInt32 matches = 0;
    for ( size_t i=0; i<names.size(); ++i )
    {
        if ( registry.GetDescriptor( names[ i ], descriptor ) && descriptor.accountProviderId == accountProvider && !descriptor.displayName.IsNULLOrEmpty() )
        {
            if ( 0 == matches )
                found = descriptor.displayName;
            ++matches;
        }
    }
    // one descriptor gives the name; several that make accounts of one namespace share its name
    if ( 1 == matches )
        return found;
    if ( accountProvider.IsNULLOrEmpty() )
        return accountProvider;
    if ( matches > 1 && accountProvider.Length() <= 4 )
        return accountProvider.Uppercase();
    return accountProvider.SubstrToIndex( 1, true ).Uppercase() + accountProvider.SubstrToIndex( 1, false );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
