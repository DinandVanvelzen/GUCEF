/*
 *  assetpluginGODOT: the importer of Godot projects
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

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTPATHS_H
#include "assetpluginGODOT_CGodotPaths.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTPATHS_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTPATHS_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTPRIMITIVEMESHES_H
#include "assetpluginGODOT_CGodotPrimitiveMeshes.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTPRIMITIVEMESHES_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTPRIMITIVEMESHES_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTSCENEREADER_H
#include "assetpluginGODOT_CGodotSceneReader.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTSCENEREADER_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTSCENEREADER_H ? */

#include "AssetConverterLib_CAssetPackListing.h"
#include "AssetConverterLib_CAssetPackProfile.h"

#include "assetpluginGODOT_CGodotImporter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GODOT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGodotImporter::CGodotImporter( void )
    : ASSETCONVERTER::CIAssetImporter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGodotImporter::~CGodotImporter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CGodotImporter::GetName( void ) const
{GUCEF_TRACE;

    return "godot";
}

/*-------------------------------------------------------------------------*/

UInt32
CGodotImporter::GetVersion( void ) const
{GUCEF_TRACE;

    // 3: records the normalized relations of the res:// and uid:// references it resolves, those that are not in the pack kept unresolved
    return 3;
}

/*-------------------------------------------------------------------------*/

bool
CGodotImporter::CanImport( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& /* context */ ) const
{GUCEF_TRACE;

    const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( vfsPath );
    if ( "tscn" == extension )
        return true;
    if ( "tres" == extension )
    {
        // a resource file is an asset when it is a mesh
        const CORE::CString header = CGodotPaths::ReadHeaderLine( vfsPath );
        if ( header.HasSubstr( "gd_resource" ) < 0 )
            return false;
        return header.HasSubstr( "type=\"ArrayMesh\"" ) >= 0 || header.HasSubstr( "Mesh\"" ) >= 0;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGodotImporter::Import( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& context, ASSETCONVERTER::CAssetScene& out, ASSETCONVERTER::CConversionReport& report )
{GUCEF_TRACE;

    CORE::CString assetPath;
    if ( !context.ToAssetPath( vfsPath, assetPath ) )
    {
        report.AddError( "godot", vfsPath, "the file is not in the source of the pack" );
        return false;
    }
    out.provenance.importerName = GetName();
    out.provenance.importerVersion = GetVersion();
    CGodotSceneReader reader( assetPath, context, out, report );
    return reader.Read();
}

/*-------------------------------------------------------------------------*/

bool
CGodotImporter::IndexPack( ASSETCONVERTER::CAssetPackContext& context )
{GUCEF_TRACE;

    // the uids are read when a path leads nowhere (CGodotPaths::Resolve()): a pack can be an archive, whose files are
    // read one by one by scanning up to them, and most paths of a project are right
    (void) context;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGodotImporter::GetFileKinds( ASSETCONVERTER::TAssetFileKindVector& kinds ) const
{GUCEF_TRACE;

    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "tscn", ASSETCONVERTER::ASSETKIND_PREFAB ) );
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "scn", ASSETCONVERTER::ASSETKIND_PREFAB ) );
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "tres", ASSETCONVERTER::ASSETKIND_DATA ) );
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "res", ASSETCONVERTER::ASSETKIND_DATA ) );
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "gdshader", ASSETCONVERTER::ASSETKIND_SHADER ) );
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "gd", ASSETCONVERTER::ASSETKIND_DATA ) );
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "godot", ASSETCONVERTER::ASSETKIND_DATA ) );
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "pck", ASSETCONVERTER::ASSETKIND_PACKAGE ) );
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "svg", ASSETCONVERTER::ASSETKIND_IMAGE ) );
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "ogv", ASSETCONVERTER::ASSETKIND_DATA ) );
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "fnt", ASSETCONVERTER::ASSETKIND_FONT ) );
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "fontdata", ASSETCONVERTER::ASSETKIND_FONT ) );
    // "script.gd.uid" belongs to "script.gd"
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "uid", ASSETCONVERTER::ASSETKIND_METADATA ) );
    // "icon.png.import" belongs to "icon.png"
    kinds.push_back( ASSETCONVERTER::CAssetFileKind( "import", ASSETCONVERTER::ASSETKIND_METADATA ) );
}

/*-------------------------------------------------------------------------*/

bool
CGodotImporter::RecognisePack( const ASSETCONVERTER::CAssetPackListing& listing, ASSETCONVERTER::CAssetPackProfile& profile ) const
{GUCEF_TRACE;

    // a project has a project.godot at its root, or below one folder of a pack that holds the project
    CORE::CString projectPath;
    CORE::CString root;
    if ( !listing.FindFile( "project.godot", projectPath ) )
    {
        CORE::CString::StringVector folders;
        listing.GetSubFolders( CORE::CString::Empty, folders );
        bool found = false;
        for ( size_t f=0; f<folders.size() && !found; ++f )
        {
            if ( listing.FindFile( folders[ f ] + "/project.godot", projectPath ) )
            {
                root = folders[ f ];
                found = true;
            }
        }
        if ( !found )
            return false;
    }

    profile.Clear();
    profile.engine = "Godot";
    profile.packType = "godot-project";
    profile.packTypeName = "Godot project";
    profile.root = root;
    profile.confidence = 90;
    const CORE::CString prefix = root.IsNULLOrEmpty() ? CORE::CString::Empty : root + "/";

    // the version is the first feature that starts with a digit: config/features=PackedStringArray("4.2", "Forward Plus"); the main scene is run/main_scene="res://scenes/main.tscn"
    CORE::CString text;
    if ( listing.ReadText( projectPath, text, 65536 ) )
    {
        const Int32 features = text.HasSubstr( "config/features=" );
        if ( features >= 0 )
        {
            const CORE::CString rest = text.SubstrFromRange( (UInt32) features, text.Length() );
            const Int32 lineEnd = rest.HasChar( '\n', true );
            const CORE::CString line = lineEnd >= 0 ? rest.SubstrFromRange( 0, (UInt32) lineEnd ) : rest;
            const CORE::CString::StringVector parts = line.ParseElements( '"', false );
            for ( size_t p=1; p<parts.size() && profile.engineVersion.IsNULLOrEmpty(); ++p )
                if ( parts[ p ].Length() > 0 && parts[ p ][ 0 ] >= '0' && parts[ p ][ 0 ] <= '9' )
                    profile.engineVersion = parts[ p ];
        }
        const Int32 mainScene = text.HasSubstr( "run/main_scene=\"" );
        if ( mainScene >= 0 )
        {
            CORE::CString scene = text.SubstrFromRange( (UInt32) mainScene + 16, text.Length() );
            const Int32 quote = scene.HasChar( '"', true );
            if ( quote > 0 )
            {
                scene = scene.SubstrFromRange( 0, (UInt32) quote );
                if ( scene.HasSubstr( "res://" ) == 0 )
                    scene = scene.SubstrFromRange( 6, scene.Length() );
                CORE::CString listed;
                if ( listing.FindFile( prefix + scene, listed ) )
                    profile.showcaseCandidates.push_back( listed.SubstrFromRange( prefix.Length(), listed.Length() ) );
            }
        }
    }

    // then the other scenes, the demo and sample ones first
    CORE::CString::StringVector keywords;
    keywords.push_back( "demo" );
    keywords.push_back( "sample" );
    keywords.push_back( "main" );
    keywords.push_back( "example" );
    CORE::CString::StringVector scenes;
    listing.FindFilesRanked( prefix + "*.tscn", keywords, scenes, 16 );
    for ( size_t s=0; s<scenes.size(); ++s )
    {
        const CORE::CString relative = scenes[ s ].SubstrFromRange( prefix.Length(), scenes[ s ].Length() );
        bool present = false;
        for ( size_t c=0; c<profile.showcaseCandidates.size() && !present; ++c )
            present = profile.showcaseCandidates[ c ] == relative;
        if ( !present )
            profile.showcaseCandidates.push_back( relative );
    }
    listing.GetSubFolders( root, profile.groupingHints );
    profile.noisePatterns.push_back( "*.import" );
    // what the editor leaves behind
    profile.noisePatterns.push_back( "*.unwrap_cache" );
    profile.noisePatterns.push_back( "*.depren" );
    profile.noisePatterns.push_back( "*.failed_import" );
    profile.noisePatterns.push_back( "*.bak" );
    profile.noisePatterns.push_back( "*.uid" );
    profile.noisePatterns.push_back( ".godot/*" );
    profile.noisePatterns.push_back( ".import/*" );
    return true;
}


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
