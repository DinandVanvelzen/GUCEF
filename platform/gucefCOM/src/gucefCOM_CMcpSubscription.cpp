/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#include "gucefCOM_CMcpSubscription.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpSubscription::CMcpSubscription( void )
    : id()
    , idKey()
    , channelId( 0 )
    , principal()
    , toolsListChanged( false )
    , promptsListChanged( false )
    , resourcesListChanged( false )
    , resourceSubscriptions()
    , hadResourceSubscriptions( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMcpSubscription::FilterToDataNode( CORE::CDataNode& filter ) const
{GUCEF_TRACE;

    filter.Clear();
    filter.SetNodeType( GUCEF_DATATYPE_OBJECT );
    if ( toolsListChanged )
        filter.SetAttribute( "toolsListChanged", true );
    if ( promptsListChanged )
        filter.SetAttribute( "promptsListChanged", true );
    if ( resourcesListChanged )
        filter.SetAttribute( "resourcesListChanged", true );
    if ( !resourceSubscriptions.empty() )
    {
        CORE::CDataNode* uris = filter.AddChild( "resourceSubscriptions", GUCEF_DATATYPE_ARRAY );
        for ( CORE::CString::StringSet::const_iterator i = resourceSubscriptions.begin(); i != resourceSubscriptions.end(); ++i )
            uris->AddValueAsChild( CORE::CVariant( (*i) ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMcpSubscription::IsSubscribedToResource( const CORE::CString& uri ) const
{GUCEF_TRACE;

    return resourceSubscriptions.find( uri ) != resourceSubscriptions.end();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
