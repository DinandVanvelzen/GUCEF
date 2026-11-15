/*
 *  assetpluginGODOT_TestApp: Test application for the Godot plugin of the asset converter (assetpluginGODOT)
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_TESTAPP_TESTPROJECT_H
#define GUCEF_ASSETPLUGIN_GODOT_TESTAPP_TESTPROJECT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "TestGodotSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The numbers of the values as the text of a Godot file: "1, 2.5, -3" */
GUCEF::CORE::CString GodotNumbers( const double* values, GUCEF::UInt32 count );

/** Godot's Transform3D( basis by rows, origin ) of a rotation about Y by 'degrees', a uniform scale and a translation */
GUCEF::CORE::CString GodotTransformText( double degreesAboutY, double scale, double x, double y, double z );

/** The bytes of a PackedByteArray as the text of a file, in the numeric form or as base64 */
GUCEF::CORE::CString GodotBytesText( const GUCEF::vector< GUCEF::UInt8 >& bytes, bool base64 );

/** One surface of an ArrayMesh in the packed layout of Godot 4, 16 bit indices, no compression */
struct SGodotSurface
{
    GUCEF::vector< float > positions;       /**< three per vertex */
    GUCEF::vector< float > normals;         /**< three per vertex, or none */
    GUCEF::vector< float > tangents;        /**< four per vertex (xyz and the sign), or none */
    GUCEF::vector< float > uvs;             /**< two per vertex, or none */
    GUCEF::vector< float > uvs2;            /**< two per vertex, or none */
    GUCEF::vector< GUCEF::UInt8 > colours;  /**< four bytes per vertex, or none */
    GUCEF::vector< GUCEF::UInt32 > indices; /**< three per triangle, or none */
    GUCEF::CORE::CString material;          /**< the value of "material", for example SubResource("m")  */
    GUCEF::Int32 primitive;                 /**< 3 triangles, 4 strip, 1 lines */
    bool useBase64;
    GUCEF::UInt64 extraFormat;              /**< flags added to the format that the data does not follow (to make surfaces that are not valid) */
    SGodotSurface( void ) : positions(), normals(), tangents(), uvs(), uvs2(), colours(), indices(), material(), primitive( 3 ), useBase64( false ), extraFormat( 0 ) {}
};

/** A surface of one triangle in the XY plane facing +Z in Godot's clockwise winding, with normals and UV coordinates */
SGodotSurface GodotTriangleSurface( const GUCEF::CORE::CString& material );

/** The packed arrays of a surface and its format flags, as Godot 4 stores them */
void GodotSurfaceData( const SGodotSurface& surface, GUCEF::vector< GUCEF::UInt8 >& vertexData, GUCEF::vector< GUCEF::UInt8 >& attributeData, GUCEF::vector< GUCEF::UInt8 >& indexData, GUCEF::UInt64& format );

/** The text of a dictionary of the "_surfaces" array */
GUCEF::CORE::CString GodotSurfaceText( const SGodotSurface& surface );

/** The text of a [sub_resource] ArrayMesh with the surfaces */
GUCEF::CORE::CString GodotArrayMeshText( const GUCEF::CORE::CString& id, const GUCEF::vector< SGodotSurface >& surfaces );

/** The octahedral packing Godot uses for a unit vector, two values in [ 0, 1 ] */
void GodotOctahedronEncode( float x, float y, float z, float& u, float& v );

/** The project file project.godot in the folder below the work folder */
bool WriteGodotProject( const GUCEF::CORE::CString& relativeFolder );

/**
 *  A glTF file made with the exporter of the plugin glTF, in the folder of the VFS: the scene "name" with the nodes
 *  "Crate" (the root) and below it "Lid" at (0, 1, 0) with a mesh of one triangle and the material "Painted" (a red
 *  base colour, metallic 0.2, roughness 0.7), and "Wheel" at (2, 0, 0) with the same mesh and the animation "Spin" of the wheel (a quarter turn about Y in a second). Written as <folder>/<name>.gltf.
 */
bool WriteGltfModel( const GUCEF::CORE::CString& vfsFolder, const GUCEF::CORE::CString& name );

/** An ASCII FBX of one triangle of 100 units on the node "Tri": in centimetres with Z up, or in metres with Y up */
GUCEF::CORE::CString MakeTriangleFbx( bool centimetresZUp );

/**
 *  Imports a scene or resource of the folder (a VFS folder below the work folder) through the importer "godot": the
 *  pack is indexed first. 'assetPath' is the path in the folder.
 */
bool ImportGodot( const GUCEF::CORE::CString& vfsFolder, const GUCEF::CORE::CString& assetPath, GUCEF::ASSETCONVERTER::CAssetScene& scene, GUCEF::ASSETCONVERTER::CConversionReport& report );

/** Runs the command line converter on the arguments (the program name is added); the exit code, -1 when the arguments are not valid. 'report' gets the report of the run */
GUCEF::Int32 RunConverter( const GUCEF::CORE::CString::StringVector& arguments, GUCEF::ASSETCONVERTER::CConversionReport* report );

/** Finds the node of a scene by its name; -1 when it is not there */
GUCEF::Int32 FindNodeByName( const GUCEF::ASSETCONVERTER::CAssetScene& scene, const char* name );

/** Whether the report of the current asset has an item of the kind whose text has the part */
bool ReportHas( const GUCEF::ASSETCONVERTER::CConversionReport& report, GUCEF::ASSETCONVERTER::TReportItemKind kind, const char* part );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_GODOT_TESTAPP_TESTPROJECT_H ? */
