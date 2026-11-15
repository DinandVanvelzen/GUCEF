/*
 *  AssetConverterLib: where an asset of the intermediate came from
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

#include "AssetConverterLib_CAssetProvenance.h"

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

CAssetProvenance::CAssetProvenance( void )
    : packId()
    , assetPath()
    , sourceFormat()
    , sourceFormatVersion()
    , sourceId()
    , sourceEngine()
    , importerName()
    , importerVersion( 0 )
    , converterVersion( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetProvenance::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "pack", packId );
    node.SetAttribute( "assetPath", assetPath );
    node.SetAttribute( "sourceFormat", sourceFormat );
    node.SetAttribute( "sourceFormatVersion", sourceFormatVersion );
    node.SetAttribute( "sourceId", sourceId );
    node.SetAttribute( "sourceEngine", sourceEngine );
    node.SetAttribute( "importer", importerName );
    node.SetAttribute( "importerVersion", importerVersion );
    node.SetAttribute( "converterVersion", converterVersion );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetProvenance::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    packId = node.GetAttributeValueOrChildValueByName( "pack" ).AsString();
    assetPath = node.GetAttributeValueOrChildValueByName( "assetPath" ).AsString();
    sourceFormat = node.GetAttributeValueOrChildValueByName( "sourceFormat" ).AsString();
    sourceFormatVersion = node.GetAttributeValueOrChildValueByName( "sourceFormatVersion" ).AsString();
    sourceId = node.GetAttributeValueOrChildValueByName( "sourceId" ).AsString();
    sourceEngine = node.GetAttributeValueOrChildValueByName( "sourceEngine" ).AsString();
    importerName = node.GetAttributeValueOrChildValueByName( "importer" ).AsString();
    importerVersion = node.GetAttributeValueOrChildValueByName( "importerVersion" ).AsUInt32( 0 );
    converterVersion = node.GetAttributeValueOrChildValueByName( "converterVersion" ).AsUInt32( 0 );
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
