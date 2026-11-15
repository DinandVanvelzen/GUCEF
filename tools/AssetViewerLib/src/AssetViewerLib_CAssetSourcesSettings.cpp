/*
 *  AssetViewerLib: the asset sources of the viewer: online stores, the primary store and the transfers
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETVIEWERLIB_CASSETSOURCESSETTINGS_H
#include "AssetViewerLib_CAssetSourcesSettings.h"
#define ASSETVIEWERLIB_CASSETSOURCESSETTINGS_H
#endif /* ASSETVIEWERLIB_CASSETSOURCESSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


CAssetSourcesSettings::CAssetSourcesSettings( void )
    : primaryStore()
    , downloads()
    , downloadRoot()
    , formatPreference( GetDefaultFormatPreference() )
    , mountOnCompletion( true )
    , conversionEnabled( true )
    , conversion()
    , keyringIdleLockInSeconds( 900 )
    , credentialFolder()
    , serviceMode( LocalServiceMode )
    , serviceSettings( "service", GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CAssetSourcesSettings::GetDefaultFormatPreference( void )
{GUCEF_TRACE;

    CORE::CString::StringVector formats;
    formats.push_back( "gltf" );
    formats.push_back( "glb" );
    formats.push_back( "fbx" );
    formats.push_back( "obj" );
    formats.push_back( "usd" );
    return formats;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetSourcesSettings::GetEffectiveDownloadRoot( void ) const
{GUCEF_TRACE;

    if ( !downloadRoot.IsNULLOrEmpty() )
        return downloadRoot;
    const ASSETSTORE::CAssetStorageLocation* location = primaryStore.FindLocation( primaryStore.defaultLocationId );
    if ( GUCEF_NULL == location && !primaryStore.locations.empty() )
        location = &primaryStore.locations[ 0 ];
    return GUCEF_NULL != location ? location->vfsRoot + "/downloads" : CORE::CString( "downloads" );
}

/*-------------------------------------------------------------------------*/

namespace {

bool
IsEngineExport( const ASSETSTORE::CAssetStoreFormat& format )
{GUCEF_TRACE;

    const CORE::CString text = ( format.formatId + " " + format.name + " " + format.group ).Lowercase();
    return text.HasSubstr( "unreal" ) >= 0 || text.HasSubstr( "uasset" ) >= 0 || text.HasSubstr( "engine" ) >= 0;
}

}

/*-------------------------------------------------------------------------*/

void
CAssetSourcesSettings::RankFormats( const ASSETSTORE::TAssetStoreFormatVector& formats, GUCEF::vector< UInt32 >& order ) const
{GUCEF_TRACE;

    order.clear();
    // the rank: the place in the preference list, then the formats outside of it, then the engine exports
    GUCEF::vector< UInt32 > ranks;
    for ( size_t f=0; f<formats.size(); ++f )
    {
        UInt32 rank = (UInt32) formatPreference.size();
        const CORE::CString id = formats[ f ].formatId.Lowercase();
        const CORE::CString name = formats[ f ].name.Lowercase();
        for ( size_t p=0; p<formatPreference.size(); ++p )
        {
            const CORE::CString wanted = formatPreference[ p ].Lowercase();
            if ( id == wanted || name == wanted )
            {
                rank = (UInt32) p;
                break;
            }
        }
        if ( IsEngineExport( formats[ f ] ) && rank >= (UInt32) formatPreference.size() )
            rank = (UInt32) formatPreference.size() + 1;
        ranks.push_back( rank );
    }
    for ( size_t f=0; f<formats.size(); ++f )
        order.push_back( (UInt32) f );
    // stable insertion sort by rank
    for ( size_t i=1; i<order.size(); ++i )
    {
        const UInt32 value = order[ i ];
        size_t j = i;
        while ( j > 0 && ranks[ order[ j - 1 ] ] > ranks[ value ] )
        {
            order[ j ] = order[ j - 1 ];
            --j;
        }
        order[ j ] = value;
    }
}

/*-------------------------------------------------------------------------*/

Int32
CAssetSourcesSettings::ChooseFormat( const ASSETSTORE::TAssetStoreFormatVector& formats ) const
{GUCEF_TRACE;

    if ( formats.empty() )
        return -1;
    GUCEF::vector< UInt32 > order;
    RankFormats( formats, order );
    return (Int32) order[ 0 ];
}

/*-------------------------------------------------------------------------*/

const CORE::CString CAssetSourcesSettings::SectionName = "AssetSources";
const CORE::CString CAssetSourcesSettings::LocalServiceMode = "local";

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesSettings::Save( CORE::CDataNode& globalConfig ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = globalConfig.Structure( SectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;
    section->SetAttribute( "downloadRoot", downloadRoot );
    section->SetAttribute( "formatPreference", CORE::CString().Combine( formatPreference, ',' ) );
    section->SetAttribute( "mountOnCompletion", mountOnCompletion );
    section->SetAttribute( "conversionEnabled", conversionEnabled );
    section->SetAttribute( "keyringIdleLockInSeconds", keyringIdleLockInSeconds );
    section->SetAttribute( "credentialFolder", credentialFolder );
    CORE::CDataNodeSerializableSettings serializerSettings;
    CORE::CDataNode* store = section->AddChild( "primaryStore" );
    CORE::CDataNode* downloadNode = section->AddChild( "downloads" );
    CORE::CDataNode* conversionNode = section->AddChild( "conversion" );
    section->DelChild( "service" );
    CORE::CDataNode* serviceNode = section->AddChild( serviceSettings );
    if ( GUCEF_NULL != serviceNode )
    {
        serviceNode->SetName( "service" );
        serviceNode->SetAttribute( "mode", serviceMode.IsNULLOrEmpty() ? LocalServiceMode : serviceMode );
    }
    return GUCEF_NULL != serviceNode && primaryStore.Serialize( *store, serializerSettings ) && downloads.Serialize( *downloadNode, serializerSettings ) && conversion.Serialize( *conversionNode, serializerSettings );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSourcesSettings::Load( const CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    const CORE::CDataNode* section = globalConfig.FindChild( SectionName );
    if ( GUCEF_NULL == section )
        return true;
    downloadRoot = section->GetAttributeValueOrChildValueByName( "downloadRoot" ).AsString( downloadRoot, true );
    mountOnCompletion = section->GetAttributeValueOrChildValueByName( "mountOnCompletion" ).AsBool( mountOnCompletion, true );
    conversionEnabled = section->GetAttributeValueOrChildValueByName( "conversionEnabled" ).AsBool( conversionEnabled, true );
    keyringIdleLockInSeconds = section->GetAttributeValueOrChildValueByName( "keyringIdleLockInSeconds" ).AsUInt32( keyringIdleLockInSeconds, true );
    credentialFolder = section->GetAttributeValueOrChildValueByName( "credentialFolder" ).AsString( credentialFolder, false );
    const CORE::CString preference = section->GetAttributeValueOrChildValueByName( "formatPreference" ).AsString( CORE::CString::Empty, true );
    if ( !preference.IsNULLOrEmpty() )
    {
        formatPreference.clear();
        const CORE::CString::StringVector parts = preference.ParseElements( ',', false );
        for ( size_t i=0; i<parts.size(); ++i )
        {
            const CORE::CString part = parts[ i ].Trim( true ).Trim( false );
            if ( !part.IsNULLOrEmpty() )
                formatPreference.push_back( part );
        }
    }
    CORE::CDataNodeSerializableSettings serializerSettings;
    bool result = true;
    const CORE::CDataNode* store = section->FindChild( "primaryStore" );
    if ( GUCEF_NULL != store )
        result = primaryStore.Deserialize( *store, serializerSettings ) && result;
    const CORE::CDataNode* downloadNode = section->FindChild( "downloads" );
    if ( GUCEF_NULL != downloadNode )
        result = downloads.Deserialize( *downloadNode, serializerSettings ) && result;
    const CORE::CDataNode* conversionNode = section->FindChild( "conversion" );
    if ( GUCEF_NULL != conversionNode )
        result = conversion.Deserialize( *conversionNode, serializerSettings ) && result;
    const CORE::CDataNode* serviceNode = section->FindChild( "service" );
    if ( GUCEF_NULL != serviceNode )
    {
        serviceSettings.Copy( *serviceNode );
        serviceSettings.SetName( "service" );
        serviceMode = serviceNode->GetAttributeValueOrChildValueByName( "mode" ).AsString( LocalServiceMode, true );
        if ( serviceMode.IsNULLOrEmpty() )
            serviceMode = LocalServiceMode;
    }
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
