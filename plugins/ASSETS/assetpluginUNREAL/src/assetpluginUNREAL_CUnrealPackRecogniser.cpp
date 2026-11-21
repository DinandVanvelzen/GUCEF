/*
 *  assetpluginUNREAL: the file kinds of Unreal Engine and the recogniser of its projects and MetaHuman packages
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKRECOGNISER_H
#include "assetpluginUNREAL_CUnrealPackRecogniser.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKRECOGNISER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKRECOGNISER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The quoted value of a key of a JSON text: "key": "value"; empty when it is not there */
static CORE::CString
ReadJsonString( const CORE::CString& text, const CORE::CString& key )
{GUCEF_TRACE;

    const Int32 at = text.HasSubstr( "\"" + key + "\"" );
    if ( at < 0 )
        return CORE::CString::Empty;
    const CORE::CString rest = text.SubstrFromRange( (UInt32) at + key.Length() + 2, text.Length() );
    const Int32 open = rest.HasChar( '"', true );
    if ( open < 0 )
        return CORE::CString::Empty;
    const CORE::CString value = rest.SubstrFromRange( (UInt32) open + 1, rest.Length() );
    const Int32 close = value.HasChar( '"', true );
    return close < 0 ? CORE::CString::Empty : value.SubstrFromRange( 0, (UInt32) close );
}

/*-------------------------------------------------------------------------*/

/** The value of a line "Key=value" of an ini text; empty when the key is not there */
static CORE::CString
ReadIniValue( const CORE::CString& text, const CORE::CString& key )
{GUCEF_TRACE;

    const CORE::CString::StringVector lines = text.ParseElements( '\n', false );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        const CORE::CString line = ASSETCONVERTER::CAssetPackListing::TrimText( lines[ i ] );
        if ( line.Length() > key.Length() + 1 && 0 == line.HasSubstr( key + "=" ) )
            return ASSETCONVERTER::CAssetPackListing::TrimText( line.SubstrFromRange( key.Length() + 1, line.Length() ) );
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CUnrealPackRecogniser::GetFileKinds( ASSETCONVERTER::TAssetFileKindVector& kinds )
{GUCEF_TRACE;

    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "uasset", ASSETCONVERTER::ASSETKIND_PACKAGE ) );
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "umap", ASSETCONVERTER::ASSETKIND_SCENE ) );
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "mhpkg", ASSETCONVERTER::ASSETKIND_PACKAGE ) );
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "uproject", ASSETCONVERTER::ASSETKIND_DATA ) );
    // "Rock.uexp", "Rock.ubulk" and "Rock.uptnl" belong to "Rock.uasset" or "Rock.umap"
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "uexp", ASSETCONVERTER::ASSETKIND_METADATA, "uasset umap" ) );
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "ubulk", ASSETCONVERTER::ASSETKIND_METADATA, "uasset umap" ) );
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "uptnl", ASSETCONVERTER::ASSETKIND_METADATA, "uasset umap" ) );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackRecogniser::Recognise( const ASSETCONVERTER::CAssetPackListing& listing, ASSETCONVERTER::CAssetPackProfile& profile )
{GUCEF_TRACE;

    CORE::CString::StringVector keywords;
    keywords.push_back( "demo" );
    keywords.push_back( "sample" );
    keywords.push_back( "main" );
    keywords.push_back( "example" );

    // a MetaHuman package: the manifest and the file list of an .mhpkg next to its packages
    CORE::CString manifestPath;
    CORE::CString::StringVector packages;
    listing.FindFiles( "*.uasset", packages );
    if ( listing.HasFile( "FileList.json" ) && listing.FindFile( "Manifest.json", manifestPath ) && !packages.empty() )
    {
        profile.Clear();
        profile.engine = "Unreal Engine";
        profile.packType = "metahuman-package";
        profile.packTypeName = "MetaHuman package";
        profile.confidence = 95;
        CORE::CString text;
        if ( listing.ReadText( manifestPath, text, 65536 ) )
            profile.engineVersion = ReadJsonString( text, "engineVersion" );
        // the root character asset is the one in the folder closest to the root with a name of the package
        listing.FindFilesRanked( "*.uasset", CORE::CString::StringVector(), profile.showcaseCandidates, 1 );
        listing.GetSubFolders( CORE::CString::Empty, profile.groupingHints );
        return true;
    }

    // a project: the .uproject at the root, or in the one folder that holds the project
    CORE::CString::StringVector projects;
    listing.FindFiles( "*.uproject", projects );
    CORE::CString projectPath;
    for ( size_t i=0; i<projects.size(); ++i )
        if ( projectPath.IsNULLOrEmpty() || projects[ i ].GetCharacterCount( '/' ) < projectPath.GetCharacterCount( '/' ) )
            projectPath = projects[ i ];
    CORE::CString root;
    if ( !projectPath.IsNULLOrEmpty() )
    {
        const Int32 slash = projectPath.HasChar( '/', false );
        if ( slash > 0 )
            root = projectPath.SubstrFromRange( 0, (UInt32) slash );
    }
    const CORE::CString prefix = root.IsNULLOrEmpty() ? CORE::CString::Empty : root + "/";

    CORE::CString::StringVector maps;
    listing.FindFilesRanked( prefix + "*.umap", keywords, maps, 16 );
    if ( projectPath.IsNULLOrEmpty() && packages.empty() && maps.empty() )
        return false;

    profile.Clear();
    profile.engine = "Unreal Engine";
    profile.root = root;
    if ( projectPath.IsNULLOrEmpty() )
    {
        profile.packType = "unreal-package";
        profile.packTypeName = "Unreal package";
        profile.confidence = 50;
    }
    else
    {
        profile.packType = "unreal-project";
        profile.packTypeName = "Unreal project";
        profile.confidence = 90;
        CORE::CString text;
        if ( listing.ReadText( projectPath, text, 65536 ) )
            profile.engineVersion = ReadJsonString( text, "EngineAssociation" );

        // the default map: GameDefaultMap=/Game/Maps/Entry.Entry is Content/Maps/Entry.umap
        CORE::CString ini;
        CORE::CString defaultMap;
        if ( listing.ReadText( prefix + "Config/DefaultEngine.ini", ini, 262144 ) )
        {
            defaultMap = ReadIniValue( ini, "GameDefaultMap" );
            if ( defaultMap.IsNULLOrEmpty() )
                defaultMap = ReadIniValue( ini, "EditorStartupMap" );
        }
        if ( defaultMap.HasSubstr( "/Game/" ) == 0 )
        {
            CORE::CString path = defaultMap.SubstrFromRange( 6, defaultMap.Length() );
            const Int32 dot = path.HasChar( '.', false );
            if ( dot > 0 )
                path = path.SubstrFromRange( 0, (UInt32) dot );
            CORE::CString listed;
            if ( listing.FindFile( prefix + "Content/" + path + ".umap", listed ) )
                profile.showcaseCandidates.push_back( listed.SubstrFromRange( prefix.Length(), listed.Length() ) );
        }
    }
    for ( size_t i=0; i<maps.size(); ++i )
    {
        const CORE::CString relative = maps[ i ].SubstrFromRange( prefix.Length(), maps[ i ].Length() );
        bool present = false;
        for ( size_t c=0; c<profile.showcaseCandidates.size() && !present; ++c )
            present = profile.showcaseCandidates[ c ] == relative;
        if ( !present )
            profile.showcaseCandidates.push_back( relative );
    }
    listing.GetSubFolders( prefix + "Content", profile.groupingHints );
    profile.noisePatterns.push_back( "Saved/*" );
    profile.noisePatterns.push_back( "Intermediate/*" );
    profile.noisePatterns.push_back( "DerivedDataCache/*" );
    profile.noisePatterns.push_back( "Binaries/*" );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
