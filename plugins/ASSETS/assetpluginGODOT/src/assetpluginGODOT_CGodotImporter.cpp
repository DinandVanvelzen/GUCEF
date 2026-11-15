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

    return 2;
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
