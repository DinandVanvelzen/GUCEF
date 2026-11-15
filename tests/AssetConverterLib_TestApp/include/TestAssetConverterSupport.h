/*
 *  AssetConverterLib_TestApp: Test application for the asset converter library (AssetConverterLib)
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

#ifndef ASSETCONVERTERLIB_TESTAPP_TESTSUPPORT_H
#define ASSETCONVERTERLIB_TESTAPP_TESTSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#ifndef GUCEF_MATH_H
#include "gucefMATH.h"
#define GUCEF_MATH_H
#endif /* GUCEF_MATH_H ? */

#ifndef ASSETCONVERTERLIB_H
#include "AssetConverterLib.h"
#define ASSETCONVERTERLIB_H
#endif /* ASSETCONVERTERLIB_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The importer of the tests, "stub": reads files with the extension .stub, which are JSON written by the tests:
 *
 *  { "system": "(+X, +Y, +Z, 1)", "normalMaps": "directx", "engine": "unreal",
 *    "nodes": [ { "name": "Box", "parent": -1, "mesh": true, "skinned": false, "size": 1, "position": [ 0, 0, 0 ], "material": 0 } ],
 *    "materials": [ { "name": "M", "baseColor": "tex.png", "normal": "normal.png" } ],
 *    "animations": [ "Walk" ], "dependencies": [ "notes.txt" ], "approximation": "what", "fail": false }
 *
 *  Paths are relative to the folder of the .stub file. A node with a mesh gets a box of the size with normals and UVs,
 *  no tangents. The settings of the asset go into the extras of the scene ("settings"), so the tests see what reached
 *  the importer. Its version can be changed by the tests, which changes the cache keys.
 */
class CStubImporter : public GUCEF::ASSETCONVERTER::CIAssetImporter
{
    public:

    GUCEF::UInt32 version;
    GUCEF::UInt32 importCount;

    CStubImporter( void );

    virtual GUCEF::CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool CanImport( const GUCEF::CORE::CString& vfsPath, GUCEF::ASSETCONVERTER::CAssetPackContext& context ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Import( const GUCEF::CORE::CString& vfsPath, GUCEF::ASSETCONVERTER::CAssetPackContext& context, GUCEF::ASSETCONVERTER::CAssetScene& out, GUCEF::ASSETCONVERTER::CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IndexPack( GUCEF::ASSETCONVERTER::CAssetPackContext& context ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetDefaultSettings( GUCEF::CORE::CDataNode& settings ) const GUCEF_VIRTUAL_OVERRIDE;
};

/** The exporter of the tests, "gltf": writes <name>.gltf as canonical JSON of what the scene holds, referencing its baked textures */
class CStubExporter : public GUCEF::ASSETCONVERTER::CIAssetExporter
{
    public:

    virtual GUCEF::CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Export( const GUCEF::ASSETCONVERTER::CAssetScene& scene, const GUCEF::CORE::CString& vfsDir, GUCEF::ASSETCONVERTER::CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;
};

/**
 *  The cooker of the tests, "stubcook": per asset <out>/<pack>/<stem>.mesh (referencing the material),
 *  <stem>.material (referencing a texture of its own and one that every asset shares) and the two textures, with
 *  known sizes: the mesh MeshSize bytes, the material MaterialSize, the own texture OwnTextureSize, the shared one
 *  SharedTextureSize.
 */
class CStubCooker : public GUCEF::ASSETCONVERTER::CIAssetCooker
{
    public:

    static const GUCEF::UInt32 MeshSize = 40;
    static const GUCEF::UInt32 MaterialSize = 30;
    static const GUCEF::UInt32 OwnTextureSize = 50;
    static const GUCEF::UInt32 SharedTextureSize = 100;

    GUCEF::UInt32 version;
    GUCEF::UInt32 maxBones;
    GUCEF::UInt32 cookCalls;
    GUCEF::CORE::CString::StringVector cookedAssets;

    CStubCooker( void );

    virtual GUCEF::CORE::CString GetTargetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::UInt32 GetMaxBonesPerSubMesh( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Cook( const GUCEF::CORE::CString& intermediateDir, const GUCEF::CORE::CString& outDir, const GUCEF::ASSETCONVERTER::CCookSettings& settings, GUCEF::ASSETCONVERTER::CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;
};

/** A post-processor of the tests that appends its name to the extra "order" of the scene: the order of a stack */
class COrderPostProcessor : public GUCEF::ASSETCONVERTER::CIAssetPostProcessor
{
    public:

    explicit COrderPostProcessor( const GUCEF::CORE::CString& name );

    virtual GUCEF::CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Process( GUCEF::ASSETCONVERTER::CAssetScene& scene, const GUCEF::CORE::CDataNode& settings, GUCEF::ASSETCONVERTER::CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    GUCEF::CORE::CString m_name;
};

/*-------------------------------------------------------------------------*/

/** Loads the plugins (JSON, zlib, FreeImage, zip), mounts the work folder and registers the plugins of the tests */
bool SetUpConverterTests( void );

void TearDownConverterTests( void );

CStubImporter& GetStubImporter( void );
CStubCooker& GetStubCooker( void );

/** A new, empty folder of the test case on the disk, below the work folder; its disk path. Its VFS path is "work/<name>" */
GUCEF::CORE::CString MakeWorkFolder( const GUCEF::CORE::CString& name );

/** The disk path of a path below the work folder */
GUCEF::CORE::CString WorkDiskPath( const GUCEF::CORE::CString& relativePath );

/** Writes a text file below the work folder (folders made as needed) */
bool WriteWorkFile( const GUCEF::CORE::CString& relativePath, const GUCEF::CORE::CString& text );

/** Reads a file of the VFS as text; empty when it cannot be read */
GUCEF::CORE::CString ReadVfsText( const GUCEF::CORE::CString& vfsPath );

/** Writes an RGBA PNG through the VFS, every texel the colour, and returns whether it worked */
bool WriteTestPng( const GUCEF::CORE::CString& vfsPath, GUCEF::UInt32 width, GUCEF::UInt32 height, GUCEF::UInt8 r, GUCEF::UInt8 g, GUCEF::UInt8 b, GUCEF::UInt8 a );

/** Writes the licence record <folder>/<id>.licence.json below the work folder */
bool WriteLicence( const GUCEF::CORE::CString& relativeFolder, const GUCEF::CORE::CString& id, bool ueOnly, bool referenceOnly, bool restricted, bool reviewed );

/** A stub source with one box node "<name>" and a material with a base colour texture, as JSON text */
GUCEF::CORE::CString MakeStubSource( const GUCEF::CORE::CString& nodeName, const GUCEF::CORE::CString& baseColorTexture );

/**
 *  Writes the pack of a test case below the work folder: <name>/src/props/rock.stub (with the texture rock.png and the
 *  dependency rock.notes.txt) and crate.stub (with crate.png), a readme that no importer takes, the licence record "rec"
 *  (unless 'withLicence' is false) and the manifest <name>/pack.assetpack.json with the extra members given, which
 *  cooks for "stubcook" with the profile "desktop". The VFS path of the manifest.
 */
GUCEF::CORE::CString WriteStandardPack( const GUCEF::CORE::CString& name, const GUCEF::CORE::CString& extraMembers, bool withLicence = true );

/** An RGBA8 picture of 'width' x 'height' texels, each texel (r, g, b, a) */
GUCEF::IMAGE::TPixelMapPtr MakePicture( GUCEF::UInt32 width, GUCEF::UInt32 height, GUCEF::UInt8 r, GUCEF::UInt8 g, GUCEF::UInt8 b, GUCEF::UInt8 a );

/** The bytes of a picture */
const GUCEF::UInt8* Texels( const GUCEF::IMAGE::CPixelMap& picture );

/** Whether two floats are within the tolerance */
bool Near( GUCEF::Float32 a, GUCEF::Float32 b, GUCEF::Float32 tolerance = 1e-4f );

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_TESTAPP_TESTSUPPORT_H ? */
