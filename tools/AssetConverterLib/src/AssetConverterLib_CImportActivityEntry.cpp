/*
 *  AssetConverterLib: an entry of the import activity log
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

#include "AssetConverterLib_CImportActivityEntry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImportActivityEntry::CImportActivityEntry( void )
    : assetPath()
    , action( ACTIVITYACTION_IMPORTED )
    , reason( ACTIVITYREASON_NEW )
    , detail()
    , durationInMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CImportActivityEntry::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "asset", assetPath );
    node.SetAttribute( "action", ToString( action ) );
    node.SetAttribute( "reason", ToString( reason ) );
    if ( !detail.IsNULLOrEmpty() )
        node.SetAttribute( "detail", detail );
    node.SetAttribute( "durationMs", durationInMs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImportActivityEntry::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CImportActivityEntry entry;
    entry.assetPath = node.GetAttributeValueOrChildValueByName( "asset" ).AsString();
    if ( !FromString( node.GetAttributeValueOrChildValueByName( "action" ).AsString(), entry.action ) ||
         !FromString( node.GetAttributeValueOrChildValueByName( "reason" ).AsString(), entry.reason ) )
        return false;
    entry.detail = node.GetAttributeValueOrChildValueByName( "detail" ).AsString();
    entry.durationInMs = node.GetAttributeValueOrChildValueByName( "durationMs" ).AsUInt32( 0 );
    *this = entry;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
