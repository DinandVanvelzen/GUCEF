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

#ifndef GUCEF_DRN_CDRNRESOURCESHARERULES_H
#define GUCEF_DRN_CDRNRESOURCESHARERULES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
 *  What a provider shares, and with whom: nothing unless said so. Given with the provider when it is registered
 *  (CDRNResourceSessionComponent::RegisterProvider()).
 *
 *  - roles: the roles (CDRNAuthResult::role) of the members that may read; AnyRole for every member of the session,
 *    whatever its role; empty: nobody
 *  - subtrees: the parts of the provider's namespace that are shared, as resource names (CDRNResourceName): a name is
 *    shared when it is one of them or lies below one ("maps" shares "maps/a.bin" and "maps/x/b.bin", not "mapsx");
 *    WholeProvider shares every name; empty: nothing
 *
 *  The node's access policy has the last word (CIDRNAccessPolicy::MayReadResourceAs()), told what these rules decided.
 */
class GUCEF_DRN_EXPORT_CPP CDRNResourceShareRules
{
    public:

    static const CORE::CString AnyRole;         /**< "*": every member of the session */
    static const CORE::CString WholeProvider;   /**< "/": every name of the provider */

    CORE::CString::StringSet roles;
    CORE::CString::StringVector subtrees;

    CDRNResourceShareRules( void );

    /** Rules that share the given subtree with the given role */
    CDRNResourceShareRules( const CORE::CString& subtree, const CORE::CString& role );

    CDRNResourceShareRules& AllowRole( const CORE::CString& role );
    CDRNResourceShareRules& AllowSubtree( const CORE::CString& subtree );

    bool AllowsRole( const CORE::CString& role ) const;
    bool AllowsName( const CORE::CString& resourceName ) const;

    /** A member with this role may read this name */
    bool Allows( const CORE::CString& role, const CORE::CString& resourceName ) const;

    /**
     *  Whether a directory may be browsed (listed, statted): the root (the empty name), a directory that is shared, or one
     *  that lies above a shared subtree, which has to be walked through to get there. Whether it is browsable says
     *  nothing about what can be read in it: that is AllowsName().
     */
    bool AllowsBrowsingDirectory( const CORE::CString& dirName ) const;

    /** Whether a file or directory shows up in a listing: what AllowsName() shares, and directories above a shared subtree */
    bool ShowsEntry( const CORE::CString& entryName, bool isDirectory ) const;

    /** A member with this role may browse this directory, and see this entry */
    bool AllowsBrowsing( const CORE::CString& role, const CORE::CString& dirName ) const;

    /** Whether every subtree is WholeProvider or a valid resource name */
    bool IsValid( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRESOURCESHARERULES_H ? */
