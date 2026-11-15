/*
 *  assetpluginFBX_TestApp: Test application for the FBX plugin of the asset converter (assetpluginFBX)
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

#include "TestFbxWriter.h"

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

TText
Number( double value )
{
    return CORE::DoubleToString( value );
}

TText
Numbers( const double* values, UInt32 count )
{
    TText text;
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( i > 0 )
            text += ",";
        text += Number( values[ i ] );
    }
    return text;
}

TText
Ints( const Int32* values, UInt32 count )
{
    TText text;
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( i > 0 )
            text += ",";
        text += CORE::Int32ToString( values[ i ] );
    }
    return text;
}

TText
Id( Int64 id )
{
    return CORE::Int64ToString( id );
}

/** The text of an FBX file in the making: the objects, the connections and the settings */
class CFbxText
{
    public:

    CFbxText( const SFbxAxes& axes )
        : m_axes( axes )
        , m_objects()
        , m_connections()
        , m_modelCount( 0 )
    {
    }

    void AddObject( const TText& text ) { m_objects += text; }

    void AddConnections( const TText& text ) { m_connections += text; }

    void Connect( Int64 child, Int64 parent, const char* property = GUCEF_NULL )
    {
        if ( GUCEF_NULL == property )
            m_connections += "    C: \"OO\"," + Id( child ) + "," + Id( parent ) + "\n";
        else
            m_connections += "    C: \"OP\"," + Id( child ) + "," + Id( parent ) + ", \"" + TText( property ) + "\"\n";
    }

    TText ToText( void ) const
    {
        TText text = "; FBX 7.4.0 project file\n"
                     "FBXHeaderExtension:  {\n    FBXHeaderVersion: 1003\n    FBXVersion: 7400\n    Creator: \"assetpluginFBX_TestApp\"\n}\n"
                     "GlobalSettings:  {\n    Version: 1000\n    Properties70:  {\n";
        text += "        P: \"UpAxis\", \"int\", \"Integer\", \"\"," + CORE::Int32ToString( m_axes.upAxis ) + "\n";
        text += "        P: \"UpAxisSign\", \"int\", \"Integer\", \"\"," + CORE::Int32ToString( m_axes.upSign ) + "\n";
        text += "        P: \"FrontAxis\", \"int\", \"Integer\", \"\"," + CORE::Int32ToString( m_axes.frontAxis ) + "\n";
        text += "        P: \"FrontAxisSign\", \"int\", \"Integer\", \"\"," + CORE::Int32ToString( m_axes.frontSign ) + "\n";
        text += "        P: \"CoordAxis\", \"int\", \"Integer\", \"\"," + CORE::Int32ToString( m_axes.coordAxis ) + "\n";
        text += "        P: \"CoordAxisSign\", \"int\", \"Integer\", \"\"," + CORE::Int32ToString( m_axes.coordSign ) + "\n";
        text += "        P: \"UnitScaleFactor\", \"double\", \"Number\", \"\"," + Number( m_axes.unitScaleFactor ) + "\n";
        text += "        P: \"OriginalUnitScaleFactor\", \"double\", \"Number\", \"\"," + Number( m_axes.unitScaleFactor ) + "\n";
        text += "        P: \"TimeMode\", \"enum\", \"\", \"\",6\n";
        text += "    }\n}\n";
        text += "Objects:  {\n" + m_objects + "}\n";
        text += "Connections:  {\n" + m_connections + "}\n";
        return text;
    }

    private:

    SFbxAxes m_axes;
    TText m_objects;
    TText m_connections;
    UInt32 m_modelCount;
};

TText
Model( Int64 id, const TText& name, const TText& type, const double translation[ 3 ] )
{
    return "    Model: " + Id( id ) + ", \"Model::" + name + "\", \"" + type + "\" {\n        Version: 232\n        Properties70:  {\n"
           "            P: \"Lcl Translation\", \"Lcl Translation\", \"\", \"A\"," + Numbers( translation, 3 ) + "\n        }\n    }\n";
}

/** A geometry of polygons with the data given as text blocks (the layer elements) */
TText
Geometry( Int64 id, const TText& name, const double* vertices, UInt32 vertexCount, const Int32* polygons, UInt32 polygonIndexCount, const TText& layers )
{
    return "    Geometry: " + Id( id ) + ", \"Geometry::" + name + "\", \"Mesh\" {\n"
           "        Vertices: *" + CORE::UInt32ToString( vertexCount * 3 ) + " {\n            a: " + Numbers( vertices, vertexCount * 3 ) + "\n        }\n"
           "        PolygonVertexIndex: *" + CORE::UInt32ToString( polygonIndexCount ) + " {\n            a: " + Ints( polygons, polygonIndexCount ) + "\n        }\n"
           "        GeometryVersion: 124\n" + layers + "    }\n";
}

TText
LayerElement( const char* kind, const TText& name, const char* mapping, const char* reference, const TText& data )
{
    return "        LayerElement" + TText( kind ) + ": 0 {\n            Version: 101\n            Name: \"" + name + "\"\n            MappingInformationType: \"" + TText( mapping ) +
           "\"\n            ReferenceInformationType: \"" + TText( reference ) + "\"\n" + data + "        }\n";
}

TText
DataBlock( const char* name, UInt32 count, const TText& values )
{
    return "            " + TText( name ) + ": *" + CORE::UInt32ToString( count ) + " {\n                a: " + values + "\n            }\n";
}

TText
Texture( Int64 id, const TText& name, const TText& file )
{
    return "    Texture: " + Id( id ) + ", \"Texture::" + name + "\", \"\" {\n        Type: \"TextureVideoClip\"\n        Version: 202\n        TextureName: \"Texture::" + name + "\"\n"
           "        Media: \"Video::" + name + "\"\n        FileName: \"" + file + "\"\n        RelativeFilename: \"" + file + "\"\n    }\n";
}

TText
Matrix( double x, double y, double z )
{
    const double values[ 16 ] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, z, 1 };
    return Numbers( values, 16 );
}

TText
MakeCubeText( const SFbxAxes& axes, double size, const double translation[ 3 ], const TText& material, const TText& extraObjects, const TText& extraConnections )
{GUCEF_TRACE;

    CFbxText file( axes );

    double corners[ 24 ];
    for ( UInt32 c=0; c<8; ++c )
    {
        corners[ c * 3 ] = ( c & 1 ) ? size : 0.0;
        corners[ c * 3 + 1 ] = ( c & 2 ) ? size : 0.0;
        corners[ c * 3 + 2 ] = ( c & 4 ) ? size : 0.0;
    }
    // the faces as the corners in order, the last of each face negated and one less
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

    TText layers = LayerElement( "Normal", "", "ByPolygonVertex", "Direct", DataBlock( "Normals", 72, Numbers( normalValues, 72 ) ) );
    layers += LayerElement( "UV", "UVMap", "ByPolygonVertex", "IndexToDirect", DataBlock( "UV", 8, Numbers( uvs, 8 ) ) + DataBlock( "UVIndex", 24, Ints( uvIndices, 24 ) ) );
    const Int32 materialIndex = 0;
    layers += LayerElement( "Material", "", "AllSame", "IndexToDirect", DataBlock( "Materials", 1, Ints( &materialIndex, 1 ) ) );
    layers += "        Layer: 0 {\n            Version: 100\n            LayerElement:  {\n                Type: \"LayerElementNormal\"\n                TypedIndex: 0\n            }\n"
              "            LayerElement:  {\n                Type: \"LayerElementMaterial\"\n                TypedIndex: 0\n            }\n"
              "            LayerElement:  {\n                Type: \"LayerElementUV\"\n                TypedIndex: 0\n            }\n        }\n";
    file.AddObject( Geometry( 1001, "Cube", corners, 8, polygons, 24, layers ) );
    file.AddObject( Model( 2001, "Cube", "Mesh", translation ) );

    file.AddObject( material );
    file.AddObject( extraObjects );
    file.Connect( 1001, 2001 );
    file.Connect( 3001, 2001 );
    file.Connect( 2001, 0 );
    file.AddConnections( extraConnections );
    return file.ToText();
}


}

/*-------------------------------------------------------------------------//
//                                                                         //
//      FILES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

SFbxAxes
FbxYUpMetres( void )
{
    SFbxAxes axes = { 1, 1, 2, 1, 0, 1, 100.0 };
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
MakeCubeFbx( const SFbxAxes& axes, double size, const double translation[ 3 ], const CORE::CString& diffuseTexture, const CORE::CString& normalTexture )
{GUCEF_TRACE;

    const TText material = "    Material: 3001, \"Material::Mat\", \"\" {\n        Version: 102\n        ShadingModel: \"phong\"\n        MultiLayer: 0\n        Properties70:  {\n"
                           "            P: \"DiffuseColor\", \"Color\", \"\", \"A\",0.8,0.4,0.2\n            P: \"DiffuseFactor\", \"Number\", \"\", \"A\",1\n"
                           "            P: \"ShininessExponent\", \"Number\", \"\", \"A\",20\n        }\n    }\n";
    TText objects;
    TText connections;
    if ( !diffuseTexture.IsNULLOrEmpty() )
    {
        objects += Texture( 4001, "Albedo", diffuseTexture );
        connections += "    C: \"OP\",4001,3001, \"DiffuseColor\"\n";
    }
    if ( !normalTexture.IsNULLOrEmpty() )
    {
        objects += Texture( 4002, "Normals", normalTexture );
        connections += "    C: \"OP\",4002,3001, \"NormalMap\"\n";
    }
    return MakeCubeText( axes, size, translation, material, objects, connections );
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeSkinnedFbx( const SFbxAxes& axes, UInt32 boneCount )
{GUCEF_TRACE;

    CFbxText file( axes );

    // a strip of two quads: 0 1 / 2 3 / 4 5
    const double vertices[ 18 ] = { 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 2, 0, 1, 2, 0 };
    const Int32 polygons[ 8 ] = { 0, 1, 3, -3, 2, 3, 5, -5 };
    file.AddObject( Geometry( 1001, "Strip", vertices, 6, polygons, 8, CORE::CString::Empty ) );
    const double origin[ 3 ] = { 0, 0, 0 };
    file.AddObject( Model( 2001, "Strip", "Mesh", origin ) );
    file.Connect( 1001, 2001 );
    file.Connect( 2001, 0 );

    // the weights of every vertex over the bones; the first bone is the only one that moves the first vertex fully
    const double weightTable[ 6 ][ 5 ] = { { 0.3, 0.25, 0.2, 0.15, 0.1 }, { 1.0, 0.0, 0.0, 0.0, 0.0 }, { 0.0, 0.5, 0.5, 0.0, 0.0 },
                                           { 0.0, 0.0, 1.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0, 0.5, 0.5 }, { 0.0, 0.0, 0.0, 0.0, 1.0 } };
    file.AddObject( "    Deformer: 5001, \"Deformer::Skin\", \"Skin\" {\n        Version: 101\n        Link_DeformAcuracy: 50\n    }\n" );
    file.Connect( 5001, 1001 );

    for ( UInt32 b=0; b<boneCount; ++b )
    {
        const Int64 boneId = 6000 + b;
        const Int64 attributeId = 6100 + b;
        const Int64 clusterId = 5100 + b;
        const double position[ 3 ] = { 0, 0 == b ? 0.0 : 1.0, 0 };
        file.AddObject( "    NodeAttribute: " + Id( attributeId ) + ", \"NodeAttribute::Bone" + CORE::UInt32ToString( b ) + "\", \"LimbNode\" {\n        TypeFlags: \"Skeleton\"\n    }\n" );
        file.AddObject( Model( boneId, "Bone" + CORE::UInt32ToString( b ), "LimbNode", position ) );
        file.Connect( attributeId, boneId );
        file.Connect( boneId, 0 == b ? 0 : 6000 + b - 1 );

        Int32 indexes[ 6 ];
        double weights[ 6 ];
        UInt32 count = 0;
        for ( UInt32 v=0; v<6; ++v )
        {
            if ( b < 5 && weightTable[ v ][ b ] > 0.0 )
            {
                indexes[ count ] = (Int32) v;
                weights[ count ] = weightTable[ v ][ b ];
                ++count;
            }
        }
        file.AddObject( "    Deformer: " + Id( clusterId ) + ", \"SubDeformer::Cluster" + CORE::UInt32ToString( b ) + "\", \"Cluster\" {\n        Version: 100\n        UserData: \"\", \"\"\n"
                        "        Indexes: *" + CORE::UInt32ToString( count ) + " {\n            a: " + Ints( indexes, count ) + "\n        }\n"
                        "        Weights: *" + CORE::UInt32ToString( count ) + " {\n            a: " + Numbers( weights, count ) + "\n        }\n"
                        "        Transform: *16 {\n            a: " + Matrix( 0, -(double) b, 0 ) + "\n        }\n"
                        "        TransformLink: *16 {\n            a: " + Matrix( 0, (double) b, 0 ) + "\n        }\n    }\n" );
        file.Connect( clusterId, 5001 );
        file.Connect( boneId, clusterId );
    }

    // one take: the second bone turns about Z from 0 to 90 degrees in one second
    const Int64 second = 46186158000LL;
    file.AddObject( "    AnimationStack: 7001, \"AnimStack::Take 001\", \"\" {\n        Properties70:  {\n            P: \"LocalStart\", \"KTime\", \"Time\", \"\",0\n"
                    "            P: \"LocalStop\", \"KTime\", \"Time\", \"\"," + Id( second ) + "\n            P: \"ReferenceStart\", \"KTime\", \"Time\", \"\",0\n"
                    "            P: \"ReferenceStop\", \"KTime\", \"Time\", \"\"," + Id( second ) + "\n        }\n    }\n" );
    file.AddObject( "    AnimationLayer: 7002, \"AnimLayer::BaseLayer\", \"\" {\n    }\n" );
    file.AddObject( "    AnimationCurveNode: 7003, \"AnimCurveNode::R\", \"\" {\n        Properties70:  {\n            P: \"d|X\", \"Number\", \"\", \"A\",0\n"
                    "            P: \"d|Y\", \"Number\", \"\", \"A\",0\n            P: \"d|Z\", \"Number\", \"\", \"A\",0\n        }\n    }\n" );
    file.AddObject( "    AnimationCurve: 7004, \"AnimCurve::\", \"\" {\n        Default: 0\n        KeyVer: 4009\n        KeyTime: *2 {\n            a: 0," + Id( second ) + "\n        }\n"
                    "        KeyValueFloat: *2 {\n            a: 0,90\n        }\n        KeyAttrFlags: *1 {\n            a: 2328\n        }\n"
                    "        KeyAttrDataFloat: *4 {\n            a: 0,0,0,0\n        }\n        KeyAttrRefCount: *1 {\n            a: 2\n        }\n    }\n" );
    file.Connect( 7002, 7001 );
    file.Connect( 7003, 7002 );
    file.Connect( 7003, 6001, "Lcl Rotation" );
    file.Connect( 7004, 7003, "d|Z" );
    return file.ToText();
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeEmbeddedTextureFbx( const SFbxAxes& axes, const CORE::CString& fileName, const CORE::CString& base64 )
{GUCEF_TRACE;

    const TText material = "    Material: 3001, \"Material::Mat\", \"\" {\n        Version: 102\n        ShadingModel: \"phong\"\n        MultiLayer: 0\n        Properties70:  {\n"
                           "            P: \"DiffuseColor\", \"Color\", \"\", \"A\",0.8,0.4,0.2\n        }\n    }\n";
    TText objects = Texture( 4001, "Albedo", fileName );
    objects += "    Video: 8001, \"Video::Albedo\", \"Clip\" {\n        Type: \"Clip\"\n        Properties70:  {\n            P: \"Path\", \"KString\", \"XRefUrl\", \"\", \"" + fileName + "\"\n        }\n"
               "        UseMipMap: 0\n        Filename: \"" + fileName + "\"\n        RelativeFilename: \"" + fileName + "\"\n        Content: \"" + base64 + "\"\n    }\n";
    const TText connections = "    C: \"OP\",4001,3001, \"DiffuseColor\"\n    C: \"OO\",8001,4001\n";
    const double translation[ 3 ] = { 0, 0, 0 };
    return MakeCubeText( axes, 1.0, translation, material, objects, connections );
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakePbrFbx( const SFbxAxes& axes, const CORE::CString& baseColorTexture, const CORE::CString& normalTexture )
{GUCEF_TRACE;

    // a 3ds Max PBR (metal/rough) material: the class of the material says which one it is
    const Int64 classA = 0xd00f1e00LL;
    const Int64 classB = 0xbe77e500LL;
    const TText material = "    Material: 3001, \"Material::PbrMat\", \"\" {\n        Version: 102\n        ShadingModel: \"unknown\"\n        MultiLayer: 0\n        Properties70:  {\n"
                           "            P: \"3dsMax|ClassIDa\", \"int\", \"Integer\", \"\"," + Id( classA ) + "\n            P: \"3dsMax|ClassIDb\", \"int\", \"Integer\", \"\"," + Id( classB ) + "\n"
                           "            P: \"3dsMax|main|base_color\", \"Color\", \"\", \"A\",0.2,0.6,0.9\n            P: \"3dsMax|main|roughness\", \"Number\", \"\", \"A\",0.35\n"
                           "            P: \"3dsMax|main|metalness\", \"Number\", \"\", \"A\",0.9\n            P: \"3dsMax|main|useGlossiness\", \"Number\", \"\", \"A\",0\n        }\n    }\n";
    TText objects;
    TText connections;
    if ( !baseColorTexture.IsNULLOrEmpty() )
    {
        objects += Texture( 4001, "Albedo", baseColorTexture );
        connections += "    C: \"OP\",4001,3001, \"3dsMax|main|base_color_map\"\n";
    }
    if ( !normalTexture.IsNULLOrEmpty() )
    {
        objects += Texture( 4002, "Normals", normalTexture );
        connections += "    C: \"OP\",4002,3001, \"3dsMax|main|norm_map\"\n";
    }
    const double translation[ 3 ] = { 0, 0, 0 };
    return MakeCubeText( axes, 1.0, translation, material, objects, connections );
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeBlendShapeFbx( const SFbxAxes& axes, bool animated )
{GUCEF_TRACE;

    CFbxText file( axes );
    const double vertices[ 12 ] = { 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1 };
    const Int32 polygons[ 4 ] = { 0, 1, 2, -4 };
    file.AddObject( Geometry( 1001, "Plane", vertices, 4, polygons, 4, CORE::CString::Empty ) );
    const double origin[ 3 ] = { 0, 0, 0 };
    file.AddObject( Model( 2001, "Plane", "Mesh", origin ) );
    file.Connect( 1001, 2001 );
    file.Connect( 2001, 0 );

    const Int32 indexes[ 4 ] = { 0, 1, 2, 3 };
    const double offsets[ 12 ] = { 0, 0.5, 0, 0, 0.5, 0, 0, 0.5, 0, 0, 0.5, 0 };
    const double normals[ 12 ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    file.AddObject( "    Deformer: 5001, \"Deformer::Shapes\", \"BlendShape\" {\n        Version: 100\n    }\n" );
    file.AddObject( "    Deformer: 5002, \"SubDeformer::Lift\", \"BlendShapeChannel\" {\n        Version: 100\n        DeformPercent: 50\n        FullWeights: *1 {\n            a: 100\n        }\n    }\n" );
    file.AddObject( "    Geometry: 5003, \"Geometry::Lift\", \"Shape\" {\n        Version: 100\n        Indexes: *4 {\n            a: " + Ints( indexes, 4 ) + "\n        }\n"
                    "        Vertices: *12 {\n            a: " + Numbers( offsets, 12 ) + "\n        }\n        Normals: *12 {\n            a: " + Numbers( normals, 12 ) + "\n        }\n    }\n" );
    file.Connect( 5001, 1001 );
    file.Connect( 5002, 5001 );
    file.Connect( 5003, 5002 );

    if ( animated )
    {
        const Int64 second = 46186158000LL;
        file.AddObject( "    AnimationStack: 7001, \"AnimStack::Morph\", \"\" {\n        Properties70:  {\n            P: \"LocalStart\", \"KTime\", \"Time\", \"\",0\n"
                        "            P: \"LocalStop\", \"KTime\", \"Time\", \"\"," + Id( second ) + "\n            P: \"ReferenceStart\", \"KTime\", \"Time\", \"\",0\n"
                        "            P: \"ReferenceStop\", \"KTime\", \"Time\", \"\"," + Id( second ) + "\n        }\n    }\n" );
        file.AddObject( "    AnimationLayer: 7002, \"AnimLayer::BaseLayer\", \"\" {\n    }\n" );
        file.AddObject( "    AnimationCurveNode: 7003, \"AnimCurveNode::DeformPercent\", \"\" {\n        Properties70:  {\n            P: \"d|DeformPercent\", \"Number\", \"\", \"A\",0\n        }\n    }\n" );
        file.AddObject( "    AnimationCurve: 7004, \"AnimCurve::\", \"\" {\n        Default: 0\n        KeyVer: 4009\n        KeyTime: *2 {\n            a: 0," + Id( second ) + "\n        }\n"
                        "        KeyValueFloat: *2 {\n            a: 0,100\n        }\n        KeyAttrFlags: *1 {\n            a: 2328\n        }\n"
                        "        KeyAttrDataFloat: *4 {\n            a: 0,0,0,0\n        }\n        KeyAttrRefCount: *1 {\n            a: 2\n        }\n    }\n" );
        file.Connect( 7002, 7001 );
        file.Connect( 7003, 7002 );
        file.Connect( 7003, 5002, "DeformPercent" );
        file.Connect( 7004, 7003, "d|DeformPercent" );
    }
    return file.ToText();
}

/*-------------------------------------------------------------------------*/
