/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#include "dvcppstringutils.h"

#include "gucefASSETSTORE_CAssetStorePluginLoadReport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetStorePluginLoadReport::CAssetStorePluginLoadReport( void )
    : directory()
    , directoryGiven( false )
    , directoryFound( false )
    , libraryCount( 0 )
    , loaded()
    , refused()
    , ignored()
    , stores()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetStorePluginLoadReport::Clear( void )
{GUCEF_TRACE;

    directory.Clear();
    directoryGiven = false;
    directoryFound = false;
    libraryCount = 0;
    loaded.clear();
    refused.clear();
    ignored.clear();
    stores.clear();
}

/*-------------------------------------------------------------------------*/

bool
CAssetStorePluginLoadReport::IsClean( void ) const
{GUCEF_TRACE;

    return directoryGiven && directoryFound && refused.empty();
}

/*-------------------------------------------------------------------------*/

void
CAssetStorePluginLoadReport::GetLines( CORE::CString::StringVector& lines ) const
{GUCEF_TRACE;

    if ( !directoryGiven )
    {
        lines.push_back( "No folder of private store plugins is set" );
        return;
    }
    if ( !directoryFound )
    {
        lines.push_back( "The folder of private store plugins does not exist: " + directory );
        return;
    }
    lines.push_back( CORE::CString( "Folder " ) + directory + ": " + CORE::ToString( libraryCount ) + ( 1 == libraryCount ? " library, " : " libraries, " ) +
                     CORE::ToString( (UInt32) loaded.size() ) + " loaded as store plugins" );
    for ( size_t i=0; i<loaded.size(); ++i )
        lines.push_back( "Loaded " + loaded[ i ] );
    for ( size_t i=0; i<stores.size(); ++i )
        lines.push_back( "Store registered: " + stores[ i ] );
    for ( size_t i=0; i<refused.size(); ++i )
        lines.push_back( "Refused " + refused[ i ] );
    for ( size_t i=0; i<ignored.size(); ++i )
        lines.push_back( "Left alone, no store plugin: " + ignored[ i ] );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
