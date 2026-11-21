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

#ifndef GUCEF_DRN_CDRNRESOURCENAME_H
#include "gucefDRN_CDRNResourceName.h"
#define GUCEF_DRN_CDRNRESOURCENAME_H
#endif /* GUCEF_DRN_CDRNRESOURCENAME_H ? */

#include "gucefDRN_CDRNResourceShareRules.h"

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

const CORE::CString CDRNResourceShareRules::AnyRole = "*";
const CORE::CString CDRNResourceShareRules::WholeProvider = "/";

/*-------------------------------------------------------------------------*/

CDRNResourceShareRules::CDRNResourceShareRules( void )
    : roles()
    , subtrees()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNResourceShareRules::CDRNResourceShareRules( const CORE::CString& subtree, const CORE::CString& role )
    : roles()
    , subtrees()
{GUCEF_TRACE;

    AllowSubtree( subtree );
    AllowRole( role );
}

/*-------------------------------------------------------------------------*/

CDRNResourceShareRules&
CDRNResourceShareRules::AllowRole( const CORE::CString& role )
{GUCEF_TRACE;

    roles.insert( role );
    return *this;
}

/*-------------------------------------------------------------------------*/

CDRNResourceShareRules&
CDRNResourceShareRules::AllowSubtree( const CORE::CString& subtree )
{GUCEF_TRACE;

    subtrees.push_back( subtree );
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceShareRules::AllowsRole( const CORE::CString& role ) const
{GUCEF_TRACE;

    if ( roles.find( AnyRole ) != roles.end() || roles.find( role ) != roles.end() )
        return true;

    // an authentication result can carry several roles, comma separated (as the RPC transport reads them)
    CORE::CString::StringVector memberRoles = role.ParseElements( ',', false );
    for ( UInt32 i=0; i<memberRoles.size(); ++i )
    {
        if ( roles.find( memberRoles[ i ] ) != roles.end() )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceShareRules::AllowsName( const CORE::CString& resourceName ) const
{GUCEF_TRACE;

    if ( !CDRNResourceName::IsValidResourceName( resourceName ) )
        return false;
    for ( UInt32 i=0; i<subtrees.size(); ++i )
    {
        const CORE::CString& subtree = subtrees[ i ];
        if ( subtree == WholeProvider || subtree == resourceName )
            return true;
        if ( CDRNResourceName::IsValidResourceName( subtree ) && resourceName.StartsWith( subtree + "/" ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceShareRules::Allows( const CORE::CString& role, const CORE::CString& resourceName ) const
{GUCEF_TRACE;

    return AllowsRole( role ) && AllowsName( resourceName );
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceShareRules::AllowsBrowsingDirectory( const CORE::CString& dirName ) const
{GUCEF_TRACE;

    // the root, a directory that is shared, or above a shared subtree
    if ( dirName.IsNULLOrEmpty() )
        return !subtrees.empty();
    return ShowsEntry( dirName, true );
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceShareRules::ShowsEntry( const CORE::CString& entryName, bool isDirectory ) const
{GUCEF_TRACE;

    if ( !CDRNResourceName::IsValidResourceName( entryName ) )
        return false;
    if ( AllowsName( entryName ) )
        return true;
    if ( !isDirectory )
        return false;
    for ( UInt32 i=0; i<subtrees.size(); ++i )
    {
        const CORE::CString& subtree = subtrees[ i ];
        if ( subtree != WholeProvider && CDRNResourceName::IsValidResourceName( subtree ) && subtree.StartsWith( entryName + "/" ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceShareRules::AllowsBrowsing( const CORE::CString& role, const CORE::CString& dirName ) const
{GUCEF_TRACE;

    return AllowsRole( role ) && AllowsBrowsingDirectory( dirName );
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceShareRules::IsValid( void ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<subtrees.size(); ++i )
        if ( subtrees[ i ] != WholeProvider && !CDRNResourceName::IsValidResourceName( subtrees[ i ] ) )
            return false;
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
