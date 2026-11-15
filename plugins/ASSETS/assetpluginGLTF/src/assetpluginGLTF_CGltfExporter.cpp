/*
 *  assetpluginGLTF: the exporter that writes the intermediate as glTF with a binary buffer
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

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFSCENEWRITER_H
#include "assetpluginGLTF_CGltfSceneWriter.h"
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFSCENEWRITER_H
#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFSCENEWRITER_H ? */

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFDOCUMENT_H
#include "assetpluginGLTF_CGltfDocument.h"
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFDOCUMENT_H
#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFDOCUMENT_H ? */

#include "assetpluginGLTF_CGltfExporter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GLTF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGltfExporter::CGltfExporter( void )
    : ASSETCONVERTER::CIAssetExporter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGltfExporter::~CGltfExporter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CGltfExporter::GetName( void ) const
{GUCEF_TRACE;

    return "gltf";
}

/*-------------------------------------------------------------------------*/

UInt32
CGltfExporter::GetVersion( void ) const
{GUCEF_TRACE;

    return CGltfSceneWriter::GetVersion();
}

/*-------------------------------------------------------------------------*/

bool
CGltfExporter::ValidateFile( const CORE::CString& vfsPath, CORE::CString& problem )
{GUCEF_TRACE;

    CGltfDocument document;
    return document.Load( vfsPath, problem ) && document.LoadBuffers( problem ) && document.Validate( problem );
}

/*-------------------------------------------------------------------------*/

bool
CGltfExporter::Export( const ASSETCONVERTER::CAssetScene& scene, const CORE::CString& vfsDir, ASSETCONVERTER::CConversionReport& report )
{GUCEF_TRACE;

    static const char* const source = "gltf";

    const CORE::CString name = scene.name.IsNULLOrEmpty() ? CORE::CString( "scene" ) : scene.name;
    const CORE::CString gltfPath = ASSETCONVERTER::CVfsPaths::Join( vfsDir, name + ".gltf" );
    const CORE::CString binPath = ASSETCONVERTER::CVfsPaths::Join( vfsDir, name + ".bin" );

    CGltfSceneWriter writer( scene, vfsDir, ASSETCONVERTER::CVfsPaths::NameOf( binPath ), report );
    if ( !writer.Write() )
    {
        report.AddError( source, name, "the glTF cannot be built from the scene" );
        return false;
    }

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfsDir.IsNULLOrEmpty() )
        vfs.CreateDir( vfsDir );
    const bool hasBinary = writer.GetBinary().GetDataSize() > 0;
    if ( hasBinary )
    {
        // the buffer is a multiple of 4 bytes long, as the buffer length of the glTF says
        CORE::CDynamicBuffer padded( writer.GetBinary() );
        while ( 0 != padded.GetDataSize() % 4 )
        {
            const char zero = 0;
            padded.Append( &zero, 1 );
        }
        if ( !vfs.StoreAsFile( binPath, padded, 0, true ) )
        {
            report.AddError( source, binPath, "the binary buffer cannot be written" );
            return false;
        }
    }
    if ( !vfs.StoreAsFile( gltfPath, writer.GetText(), 0, true ) )
    {
        report.AddError( source, gltfPath, "the glTF file cannot be written" );
        return false;
    }

    CORE::CString problem;
    if ( !ValidateFile( gltfPath, problem ) )
    {
        report.AddError( source, gltfPath, "cgltf_validate: " + problem );
        return false;
    }

    CORE::CString::StringVector references;
    if ( hasBinary )
        references.push_back( binPath );
    for ( size_t t=0; t<scene.textures.size(); ++t )
        if ( !scene.textures[ t ].bakedPath.IsNULLOrEmpty() )
            references.push_back( scene.textures[ t ].bakedPath );
    report.AddOutputFile( gltfPath, references );
    if ( hasBinary )
        report.AddOutputFile( binPath, CORE::CString::StringVector() );
    report.AddInfo( source, name, "written and validated with cgltf" );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GLTF */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
