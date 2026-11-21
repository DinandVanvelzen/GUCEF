/*
 *  AssetConverterLib: the manifest of a pack: what to convert, how, and where to
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CAssetPackManifest.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetPackManifest::AutoImporter = "auto";
const CORE::CString CAssetPackManifest::AnySourceFormat = "*";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetPackManifest::CAssetPackManifest( void )
    : id()
    , title()
    , store()
    , licenceRecord()
    , licenceFolder()
    , source()
    , importer( AutoImporter )
    , include()
    , exclude()
    , assets()
    , rules( "rules", GUCEF_DATATYPE_OBJECT )
    , nameHints()
    , presets()
    , overrides()
    , postProcessors()
    , hasValidators( false )
    , validators()
    , exporter( "gltf" )
    , intermediate( "intermediate" )
    , output( "out" )
    , cookTargets()
    , cookProfiles()
    , manifestFolder()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetPackManifest::Load( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CDataNode node;
    if ( !CCanonicalJson::Load( vfsPath, node ) )
        return false;
    if ( !Deserialize( node ) )
        return false;
    manifestFolder = CVfsPaths::FolderOf( vfsPath );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackManifest::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "id", id );
    node.SetAttribute( "title", title );
    node.SetAttribute( "store", store );
    node.SetAttribute( "licenceRecord", licenceRecord );
    if ( !licenceFolder.IsNULLOrEmpty() )
        node.SetAttribute( "licenceFolder", licenceFolder );
    node.SetAttribute( "source", source );
    node.SetAttribute( "importer", importer );
    node.SetValuesOfChildByName( "include", include );
    node.SetValuesOfChildByName( "exclude", exclude );
    node.SetValuesOfChildByName( "assets", assets );
    CORE::CDataNode* rulesNode = node.AddChild( rules );
    rulesNode->SetName( "rules" );
    if ( !nameHints.IsNULLOrEmpty() )
        node.SetAttribute( "nameHints", nameHints );
    CORE::CDataNode* presetList = node.AddChild( "presets", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<presets.size(); ++i )
        presets[ i ].Serialize( *presetList->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    if ( !overrides.IsNULLOrEmpty() )
        node.SetAttribute( "overrides", overrides );
    CORE::CDataNode* stacks = node.AddChild( "postProcessors", GUCEF_DATATYPE_OBJECT );
    TStackMap::const_iterator s = postProcessors.begin();
    while ( s != postProcessors.end() )
    {
        CORE::CDataNode* stack = stacks->AddChild( (*s).first, GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<(*s).second.size(); ++i )
            (*s).second[ i ].Serialize( *stack->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
        ++s;
    }
    if ( hasValidators )
    {
        CORE::CDataNode* list = node.AddChild( "validators", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<validators.size(); ++i )
            validators[ i ].Serialize( *list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    }
    node.SetAttribute( "exporter", exporter );
    node.SetAttribute( "intermediate", intermediate );
    node.SetAttribute( "output", output );
    node.SetValuesOfChildByName( "cookTargets", cookTargets );
    CORE::CDataNode* profiles = node.AddChild( "cookProfiles", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<cookProfiles.size(); ++i )
        cookProfiles[ i ].Serialize( *profiles->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackManifest::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CAssetPackManifest manifest;
    manifest.id = node.GetAttributeValueOrChildValueByName( "id" ).AsString();
    if ( manifest.id.IsNULLOrEmpty() )
        return false;
    manifest.title = node.GetAttributeValueOrChildValueByName( "title" ).AsString();
    manifest.store = node.GetAttributeValueOrChildValueByName( "store" ).AsString();
    manifest.licenceRecord = node.GetAttributeValueOrChildValueByName( "licenceRecord" ).AsString();
    manifest.licenceFolder = node.GetAttributeValueOrChildValueByName( "licenceFolder" ).AsString();
    manifest.source = node.GetAttributeValueOrChildValueByName( "source" ).AsString();
    manifest.importer = node.GetAttributeValueOrChildValueByName( "importer" ).AsString( AutoImporter );
    node.GetStringListByName( "include", manifest.include );
    node.GetStringListByName( "exclude", manifest.exclude );
    node.GetStringListByName( "assets", manifest.assets );
    const CORE::CDataNode* rulesNode = node.FindChild( "rules" );
    if ( GUCEF_NULL != rulesNode )
        manifest.rules = *rulesNode;
    manifest.rules.SetName( "rules" );
    manifest.nameHints = node.GetAttributeValueOrChildValueByName( "nameHints" ).AsString();

    const CORE::CDataNode* presetList = node.FindChild( "presets" );
    if ( GUCEF_NULL != presetList )
    {
        CORE::CDataNode::const_iterator i = presetList->ConstBegin();
        while ( i != presetList->ConstEnd() )
        {
            CAssetPreset preset;
            if ( preset.Deserialize( *(*i) ) )
                manifest.presets.push_back( preset );
            ++i;
        }
    }
    manifest.overrides = node.GetAttributeValueOrChildValueByName( "overrides" ).AsString();

    const CORE::CDataNode* stacks = node.FindChild( "postProcessors" );
    if ( GUCEF_NULL != stacks )
    {
        CORE::CDataNode::const_iterator i = stacks->ConstBegin();
        while ( i != stacks->ConstEnd() )
        {
            CPluginStackEntry::TEntryVector stack;
            CPluginStackEntry::DeserializeList( *(*i), stack );
            manifest.postProcessors[ (*i)->GetName().Lowercase() ] = stack;
            ++i;
        }
    }
    const CORE::CDataNode* validatorList = node.FindChild( "validators" );
    if ( GUCEF_NULL != validatorList )
    {
        manifest.hasValidators = true;
        CPluginStackEntry::DeserializeList( *validatorList, manifest.validators );
    }
    manifest.exporter = node.GetAttributeValueOrChildValueByName( "exporter" ).AsString( "gltf" );
    manifest.intermediate = node.GetAttributeValueOrChildValueByName( "intermediate" ).AsString( "intermediate" );
    manifest.output = node.GetAttributeValueOrChildValueByName( "output" ).AsString( "out" );
    node.GetStringListByName( "cookTargets", manifest.cookTargets );
    const CORE::CDataNode* profiles = node.FindChild( "cookProfiles" );
    if ( GUCEF_NULL != profiles )
    {
        CORE::CDataNode::const_iterator i = profiles->ConstBegin();
        while ( i != profiles->ConstEnd() )
        {
            CCookProfile profile;
            if ( profile.Deserialize( *(*i) ) )
                manifest.cookProfiles.push_back( profile );
            ++i;
        }
    }
    manifest.manifestFolder = manifestFolder;
    *this = manifest;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetPackManifest::ResolvePath( const CORE::CString& path ) const
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    if ( '/' == path[ 0 ] )
        return CVfsPaths::Normalize( path );
    return CVfsPaths::Join( manifestFolder, path );
}

/*-------------------------------------------------------------------------*/

CORE::CString CAssetPackManifest::GetSourcePath( void ) const { return ResolvePath( source ); }
CORE::CString CAssetPackManifest::GetIntermediatePath( void ) const { return ResolvePath( intermediate ); }
CORE::CString CAssetPackManifest::GetOutputPath( void ) const { return ResolvePath( output ); }
CORE::CString CAssetPackManifest::GetOverridesPath( void ) const { return ResolvePath( overrides ); }

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetPackManifest::GetLicenceRecordPath( void ) const
{GUCEF_TRACE;

    if ( licenceRecord.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    const CORE::CString folder = licenceFolder.IsNULLOrEmpty() ? manifestFolder : ResolvePath( licenceFolder );
    return CVfsPaths::Join( folder, licenceRecord + ".licence.json" );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackManifest::IsIncluded( const CORE::CString& assetPath ) const
{GUCEF_TRACE;

    bool included = include.empty();
    for ( size_t i=0; i<include.size() && !included; ++i )
        included = CORE::CGlobPattern::IsMatch( include[ i ], assetPath, false );
    if ( !included )
        return false;
    for ( size_t i=0; i<exclude.size(); ++i )
        if ( CORE::CGlobPattern::IsMatch( exclude[ i ], assetPath, false ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackManifest::IsAssetSelected( const CORE::CString& assetPath ) const
{GUCEF_TRACE;

    if ( assets.empty() )
        return true;
    for ( size_t i=0; i<assets.size(); ++i )
        if ( CORE::CGlobPattern::IsMatch( assets[ i ], assetPath, false ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackManifest::GetPostProcessorStack( const CORE::CString& sourceFormat, CPluginStackEntry::TEntryVector& stack ) const
{GUCEF_TRACE;

    TStackMap::const_iterator i = postProcessors.find( sourceFormat.Lowercase() );
    if ( i == postProcessors.end() )
        i = postProcessors.find( AnySourceFormat );
    if ( i == postProcessors.end() )
        return false;
    stack = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetPackManifest::GetValidators( CPluginStackEntry::TEntryVector& list ) const
{GUCEF_TRACE;

    if ( hasValidators )
    {
        list = validators;
        return;
    }
    list.clear();
    list.push_back( CPluginStackEntry( "MissingTextures" ) );
    list.push_back( CPluginStackEntry( "TextureSize" ) );
    list.push_back( CPluginStackEntry( "BoneLimit" ) );
    list.push_back( CPluginStackEntry( "LayerPenetration" ) );
}

/*-------------------------------------------------------------------------*/

void
CAssetPackManifest::GetPackSettings( CORE::CDataNode& settings ) const
{GUCEF_TRACE;

    settings = rules;
    settings.SetName( "settings" );
    settings.SetNodeType( GUCEF_DATATYPE_OBJECT );
    if ( !nameHints.IsNULLOrEmpty() )
        settings.SetAttribute( "nameHints", nameHints );
}

/*-------------------------------------------------------------------------*/

void
CAssetPackManifest::GetProcessingSettings( CORE::CDataNode& settings ) const
{GUCEF_TRACE;

    CAssetPackManifest processing;
    processing.id = id;
    processing.postProcessors = postProcessors;
    processing.hasValidators = hasValidators;
    processing.validators = validators;
    processing.exporter = exporter;
    processing.cookTargets = cookTargets;
    processing.cookProfiles = cookProfiles;
    CORE::CDataNode full( "processing", GUCEF_DATATYPE_OBJECT );
    processing.Serialize( full );

    settings = CORE::CDataNode( "processing", GUCEF_DATATYPE_OBJECT );
    const char* members[] = { "postProcessors", "validators", "cookTargets", "cookProfiles" };
    for ( UInt32 i=0; i<sizeof( members ) / sizeof( members[ 0 ] ); ++i )
    {
        const CORE::CDataNode* child = full.FindChild( members[ i ] );
        if ( GUCEF_NULL != child )
            settings.AddChild( *child );
    }
    settings.SetAttribute( "exporter", exporter );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetPackManifest::GetLodPolicy( void ) const
{GUCEF_TRACE;

    const CORE::CDataNode* lod = rules.FindChild( "lod" );
    if ( GUCEF_NULL == lod )
        return "source";
    return lod->GetAttributeValueOrChildValueByName( "policy" ).AsString( "source" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetPackManifest::GetColliderPolicy( void ) const
{GUCEF_TRACE;

    const CORE::CDataNode* colliders = rules.FindChild( "colliders" );
    if ( GUCEF_NULL == colliders )
        return "source";
    return colliders->GetAttributeValueOrChildValueByName( "policy" ).AsString( "source" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
