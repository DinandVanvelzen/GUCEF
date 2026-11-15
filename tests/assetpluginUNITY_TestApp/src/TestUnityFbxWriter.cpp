/*
 *  assetpluginUNITY_TestApp: Test application for the Unity plugin of the asset converter (assetpluginUNITY)
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

#include "TestUnityFbxWriter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef CORE::CString TText;

TText Number( double value ) { return CORE::DoubleToString( value ); }

TText
Numbers( const double* values, UInt32 count )
{
    TText text;
    for ( UInt32 i=0; i<count; ++i )
        text += ( i > 0 ? "," : "" ) + Number( values[ i ] );
    return text;
}

TText
Ints( const Int32* values, UInt32 count )
{
    TText text;
    for ( UInt32 i=0; i<count; ++i )
        text += ( i > 0 ? "," : "" ) + CORE::Int32ToString( values[ i ] );
    return text;
}

TText Id( Int64 id ) { return CORE::Int64ToString( id ); }

TText
Header( const SFbxAxes& axes, const char* frameRateMode )
{
    TText text = "; FBX 7.4.0 project file\n"
                 "FBXHeaderExtension:  {\n    FBXHeaderVersion: 1003\n    FBXVersion: 7400\n    Creator: \"assetpluginUNITY_TestApp\"\n}\n"
                 "GlobalSettings:  {\n    Version: 1000\n    Properties70:  {\n";
    text += "        P: \"UpAxis\", \"int\", \"Integer\", \"\"," + CORE::Int32ToString( axes.upAxis ) + "\n";
    text += "        P: \"UpAxisSign\", \"int\", \"Integer\", \"\"," + CORE::Int32ToString( axes.upSign ) + "\n";
    text += "        P: \"FrontAxis\", \"int\", \"Integer\", \"\"," + CORE::Int32ToString( axes.frontAxis ) + "\n";
    text += "        P: \"FrontAxisSign\", \"int\", \"Integer\", \"\"," + CORE::Int32ToString( axes.frontSign ) + "\n";
    text += "        P: \"CoordAxis\", \"int\", \"Integer\", \"\"," + CORE::Int32ToString( axes.coordAxis ) + "\n";
    text += "        P: \"CoordAxisSign\", \"int\", \"Integer\", \"\"," + CORE::Int32ToString( axes.coordSign ) + "\n";
    text += "        P: \"UnitScaleFactor\", \"double\", \"Number\", \"\"," + Number( axes.unitScaleFactor ) + "\n";
    text += "        P: \"OriginalUnitScaleFactor\", \"double\", \"Number\", \"\"," + Number( axes.unitScaleFactor ) + "\n";
    text += "        P: \"TimeMode\", \"enum\", \"\", \"\"," + TText( frameRateMode ) + "\n";
    text += "    }\n}\n";
    return text;
}

TText
Model( Int64 id, const TText& name, const TText& type, const double translation[ 3 ] )
{
    return "    Model: " + Id( id ) + ", \"Model::" + name + "\", \"" + type + "\" {\n        Version: 232\n        Properties70:  {\n"
           "            P: \"Lcl Translation\", \"Lcl Translation\", \"\", \"A\"," + Numbers( translation, 3 ) + "\n        }\n    }\n";
}

/** A cube of 'size' at the corner (0, 0, 0): normals per polygon vertex, UVs through an index, material 0 */
TText
CubeGeometry( Int64 id, const TText& name, double size )
{
    double corners[ 24 ];
    for ( UInt32 c=0; c<8; ++c )
    {
        corners[ c * 3 ] = ( c & 1 ) ? size : 0.0;
        corners[ c * 3 + 1 ] = ( c & 2 ) ? size : 0.0;
        corners[ c * 3 + 2 ] = ( c & 4 ) ? size : 0.0;
    }
    const Int32 faces[ 6 ][ 4 ] = { { 0, 2, 3, 1 }, { 4, 5, 7, 6 }, { 0, 1, 5, 4 }, { 2, 6, 7, 3 }, { 0, 4, 6, 2 }, { 1, 3, 7, 5 } };
    const double normals[ 6 ][ 3 ] = { { 0, 0, -1 }, { 0, 0, 1 }, { 0, -1, 0 }, { 0, 1, 0 }, { -1, 0, 0 }, { 1, 0, 0 } };
    Int32 polygons[ 24 ];
    double normalValues[ 72 ];
    Int32 uvIndices[ 24 ];
    for ( UInt32 f=0; f<6; ++f )
    {
        for ( UInt32 c=0; c<4; ++c )
        {
            polygons[ f * 4 + c ] = 3 == c ? -faces[ f ][ c ] - 1 : faces[ f ][ c ];
            for ( UInt32 a=0; a<3; ++a )
                normalValues[ ( f * 4 + c ) * 3 + a ] = normals[ f ][ a ];
            uvIndices[ f * 4 + c ] = (Int32) c;
        }
    }
    const double uvs[ 8 ] = { 0, 0, 1, 0, 1, 1, 0, 1 };
    const Int32 materialIndex = 0;
    return "    Geometry: " + Id( id ) + ", \"Geometry::" + name + "\", \"Mesh\" {\n"
           "        Vertices: *24 {\n            a: " + Numbers( corners, 24 ) + "\n        }\n"
           "        PolygonVertexIndex: *24 {\n            a: " + Ints( polygons, 24 ) + "\n        }\n"
           "        GeometryVersion: 124\n"
           "        LayerElementNormal: 0 {\n            Version: 101\n            Name: \"\"\n            MappingInformationType: \"ByPolygonVertex\"\n            ReferenceInformationType: \"Direct\"\n"
           "            Normals: *72 {\n                a: " + Numbers( normalValues, 72 ) + "\n            }\n        }\n"
           "        LayerElementUV: 0 {\n            Version: 101\n            Name: \"UVMap\"\n            MappingInformationType: \"ByPolygonVertex\"\n            ReferenceInformationType: \"IndexToDirect\"\n"
           "            UV: *8 {\n                a: " + Numbers( uvs, 8 ) + "\n            }\n            UVIndex: *24 {\n                a: " + Ints( uvIndices, 24 ) + "\n            }\n        }\n"
           "        LayerElementMaterial: 0 {\n            Version: 101\n            Name: \"\"\n            MappingInformationType: \"AllSame\"\n            ReferenceInformationType: \"IndexToDirect\"\n"
           "            Materials: *1 {\n                a: " + Ints( &materialIndex, 1 ) + "\n            }\n        }\n"
           "        Layer: 0 {\n            Version: 100\n            LayerElement:  {\n                Type: \"LayerElementNormal\"\n                TypedIndex: 0\n            }\n"
           "            LayerElement:  {\n                Type: \"LayerElementMaterial\"\n                TypedIndex: 0\n            }\n"
           "            LayerElement:  {\n                Type: \"LayerElementUV\"\n                TypedIndex: 0\n            }\n        }\n    }\n";
}

const TText MaterialObject = "    Material: 3001, \"Material::Mat\", \"\" {\n        Version: 102\n        ShadingModel: \"phong\"\n        MultiLayer: 0\n        Properties70:  {\n"
                             "            P: \"DiffuseColor\", \"Color\", \"\", \"A\",0.8,0.4,0.2\n            P: \"DiffuseFactor\", \"Number\", \"\", \"A\",1\n        }\n    }\n"
                             "    Texture: 4001, \"Texture::Albedo\", \"\" {\n        Type: \"TextureVideoClip\"\n        Version: 202\n        TextureName: \"Texture::Albedo\"\n"
                             "        FileName: \"U:/Author/Textures/albedo.psd\"\n        RelativeFilename: \"..\\\\Textures\\\\albedo.psd\"\n    }\n";

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      FILES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

SFbxAxes
FbxYUpCentimetres( void )
{
    SFbxAxes axes = { 1, 1, 2, 1, 0, 1, 1.0 };
    return axes;
}

/*-------------------------------------------------------------------------*/

SFbxAxes
FbxZUpCentimetres( void )
{
    SFbxAxes axes = { 2, 1, 1, -1, 0, 1, 1.0 };
    return axes;
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeNodesFbx( const SFbxAxes& axes, const STestFbxNode* nodes, UInt32 count )
{GUCEF_TRACE;

    TText objects = MaterialObject;
    TText connections = "    C: \"OP\",4001,3001, \"DiffuseColor\"\n";
    for ( UInt32 n=0; n<count; ++n )
    {
        const Int64 model = 2000 + n;
        const Int64 geometry = 1000 + n;
        objects += Model( model, nodes[ n ].name, nodes[ n ].size > 0.0 ? "Mesh" : "Null", nodes[ n ].translation );
        if ( nodes[ n ].size > 0.0 )
        {
            objects += CubeGeometry( geometry, nodes[ n ].name, nodes[ n ].size );
            connections += "    C: \"OO\"," + Id( geometry ) + "," + Id( model ) + "\n";
            connections += "    C: \"OO\",3001," + Id( model ) + "\n";
        }
        connections += "    C: \"OO\"," + Id( model ) + "," + Id( nodes[ n ].parent < 0 ? 0 : 2000 + nodes[ n ].parent ) + "\n";
    }
    return Header( axes, "6" ) + "Objects:  {\n" + objects + "}\nConnections:  {\n" + connections + "}\n";
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeMoverFbx( const SFbxAxes& axes )
{GUCEF_TRACE;

    const Int64 second = 46186158000LL;
    const double origin[ 3 ] = { 0, 0, 0 };
    TText objects = MaterialObject;
    objects += Model( 2000, "Mover", "Mesh", origin );
    objects += CubeGeometry( 1000, "Mover", 10.0 );
    objects += "    AnimationStack: 7001, \"AnimStack::Take 001\", \"\" {\n        Properties70:  {\n            P: \"LocalStart\", \"KTime\", \"Time\", \"\",0\n"
               "            P: \"LocalStop\", \"KTime\", \"Time\", \"\"," + Id( 2 * second ) + "\n            P: \"ReferenceStart\", \"KTime\", \"Time\", \"\",0\n"
               "            P: \"ReferenceStop\", \"KTime\", \"Time\", \"\"," + Id( 2 * second ) + "\n        }\n    }\n";
    objects += "    AnimationLayer: 7002, \"AnimLayer::BaseLayer\", \"\" {\n    }\n";
    objects += "    AnimationCurveNode: 7003, \"AnimCurveNode::T\", \"\" {\n        Properties70:  {\n            P: \"d|X\", \"Number\", \"\", \"A\",0\n"
               "            P: \"d|Y\", \"Number\", \"\", \"A\",0\n            P: \"d|Z\", \"Number\", \"\", \"A\",0\n        }\n    }\n";
    objects += "    AnimationCurve: 7004, \"AnimCurve::\", \"\" {\n        Default: 0\n        KeyVer: 4009\n        KeyTime: *3 {\n            a: 0," + Id( second ) + "," + Id( 2 * second ) + "\n        }\n"
               "        KeyValueFloat: *3 {\n            a: 0,50,100\n        }\n        KeyAttrFlags: *1 {\n            a: 260\n        }\n"
               "        KeyAttrDataFloat: *4 {\n            a: 0,0,0,0\n        }\n        KeyAttrRefCount: *1 {\n            a: 3\n        }\n    }\n";
    TText connections = "    C: \"OP\",4001,3001, \"DiffuseColor\"\n";
    connections += "    C: \"OO\",1000,2000\n    C: \"OO\",3001,2000\n    C: \"OO\",2000,0\n";
    connections += "    C: \"OO\",7002,7001\n    C: \"OO\",7003,7002\n    C: \"OP\",7003,2000, \"Lcl Translation\"\n    C: \"OP\",7004,7003, \"d|X\"\n";
    return Header( axes, "6" ) + "Objects:  {\n" + objects + "}\nConnections:  {\n" + connections + "}\n";
}

/*-------------------------------------------------------------------------*/
