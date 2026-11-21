/*
 *  assetpluginUNREAL: the hidden face map of an outfit: which faces of the body the outfit hides
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

#include <math.h>

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALHIDDENFACEMAP_H
#include "assetpluginUNREAL_CUnrealHiddenFaceMap.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALHIDDENFACEMAP_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALHIDDENFACEMAP_H ? */

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const Float32 CUnrealHiddenFaceMap::DefaultMaxCullValue = 0.1f;
const Float32 CUnrealHiddenFaceMap::DefaultMinKeepValue = 0.9f;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnrealHiddenFaceMap::SResult::SResult( void )
    : triangles( 0 )
    , hiddenTriangles( 0 )
    , otherTriangles( 0 )
    , newSubMeshes( 0 )
    , bandVertices( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnrealHiddenFaceMap::CUnrealHiddenFaceMap( void )
    : m_values()
    , m_width( 0 )
    , m_height( 0 )
    , m_maxCullValue( DefaultMaxCullValue )
    , m_minKeepValue( DefaultMinKeepValue )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnrealHiddenFaceMap::SetImage( const TByteVector& pixels, UInt32 width, UInt32 height, UInt32 channels )
{GUCEF_TRACE;

    m_values.clear();
    m_width = m_height = 0;
    if ( 0 == width || 0 == height || 0 == channels || pixels.size() != (size_t) width * height * channels )
        return false;
    m_values.resize( (size_t) width * height );
    for ( size_t i=0; i<m_values.size(); ++i )
        m_values[ i ] = pixels[ i * channels ];
    m_width = width;
    m_height = height;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealHiddenFaceMap::GetWidth( void ) const
{GUCEF_TRACE;

    return m_width;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealHiddenFaceMap::GetHeight( void ) const
{GUCEF_TRACE;

    return m_height;
}

/*-------------------------------------------------------------------------*/

void
CUnrealHiddenFaceMap::SetMaxCullValue( Float32 value )
{GUCEF_TRACE;

    m_maxCullValue = value;
}

/*-------------------------------------------------------------------------*/

Float32
CUnrealHiddenFaceMap::GetMaxCullValue( void ) const
{GUCEF_TRACE;

    return m_maxCullValue;
}

/*-------------------------------------------------------------------------*/

void
CUnrealHiddenFaceMap::SetMinKeepValue( Float32 value )
{GUCEF_TRACE;

    m_minKeepValue = value;
}

/*-------------------------------------------------------------------------*/

Float32
CUnrealHiddenFaceMap::GetMinKeepValue( void ) const
{GUCEF_TRACE;

    return m_minKeepValue;
}

/*-------------------------------------------------------------------------*/

UInt8
CUnrealHiddenFaceMap::Texel( Int32 x, Int32 y ) const
{GUCEF_TRACE;

    if ( m_values.empty() )
        return 255;
    x = x < 0 ? 0 : ( x >= (Int32) m_width ? (Int32) m_width - 1 : x );
    y = y < 0 ? 0 : ( y >= (Int32) m_height ? (Int32) m_height - 1 : y );
    return m_values[ (size_t) y * m_width + (size_t) x ];
}

/*-------------------------------------------------------------------------*/

Float32
CUnrealHiddenFaceMap::Sample( Float32 u, Float32 v ) const
{GUCEF_TRACE;

    return Texel( (Int32) floor( u * (Float32) m_width ), (Int32) floor( v * (Float32) m_height ) ) / 255.0f;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealHiddenFaceMap::CoversAtLeast( const Float32* a, const Float32* b, const Float32* c, UInt32 threshold ) const
{GUCEF_TRACE;

    // the corners, the middles of the edges and the centre always count
    const Float32 points[ 7 ][ 2 ] =
    {
        { a[ 0 ], a[ 1 ] }, { b[ 0 ], b[ 1 ] }, { c[ 0 ], c[ 1 ] },
        { ( a[ 0 ] + b[ 0 ] ) * 0.5f, ( a[ 1 ] + b[ 1 ] ) * 0.5f }, { ( b[ 0 ] + c[ 0 ] ) * 0.5f, ( b[ 1 ] + c[ 1 ] ) * 0.5f },
        { ( c[ 0 ] + a[ 0 ] ) * 0.5f, ( c[ 1 ] + a[ 1 ] ) * 0.5f }, { ( a[ 0 ] + b[ 0 ] + c[ 0 ] ) / 3.0f, ( a[ 1 ] + b[ 1 ] + c[ 1 ] ) / 3.0f }
    };
    for ( UInt32 p=0; p<7; ++p )
        if ( Texel( (Int32) floor( points[ p ][ 0 ] ), (Int32) floor( points[ p ][ 1 ] ) ) >= threshold )
            return true;

    // the texels whose centres lie inside the triangle (on an edge counts)
    Float32 minX = a[ 0 ], maxX = a[ 0 ], minY = a[ 1 ], maxY = a[ 1 ];
    const Float32* corners[ 2 ] = { b, c };
    for ( UInt32 k=0; k<2; ++k )
    {
        minX = corners[ k ][ 0 ] < minX ? corners[ k ][ 0 ] : minX;
        maxX = corners[ k ][ 0 ] > maxX ? corners[ k ][ 0 ] : maxX;
        minY = corners[ k ][ 1 ] < minY ? corners[ k ][ 1 ] : minY;
        maxY = corners[ k ][ 1 ] > maxY ? corners[ k ][ 1 ] : maxY;
    }
    Int32 x0 = (Int32) floor( minX ), x1 = (Int32) floor( maxX ), y0 = (Int32) floor( minY ), y1 = (Int32) floor( maxY );
    x0 = x0 < 0 ? 0 : x0;
    y0 = y0 < 0 ? 0 : y0;
    x1 = x1 >= (Int32) m_width ? (Int32) m_width - 1 : x1;
    y1 = y1 >= (Int32) m_height ? (Int32) m_height - 1 : y1;
    for ( Int32 y=y0; y<=y1; ++y )
    {
        const Float32 py = (Float32) y + 0.5f;
        for ( Int32 x=x0; x<=x1; ++x )
        {
            if ( m_values[ (size_t) y * m_width + (size_t) x ] < threshold )
                continue;
            const Float32 px = (Float32) x + 0.5f;
            const Float32 e0 = ( b[ 0 ] - a[ 0 ] ) * ( py - a[ 1 ] ) - ( b[ 1 ] - a[ 1 ] ) * ( px - a[ 0 ] );
            const Float32 e1 = ( c[ 0 ] - b[ 0 ] ) * ( py - b[ 1 ] ) - ( c[ 1 ] - b[ 1 ] ) * ( px - b[ 0 ] );
            const Float32 e2 = ( a[ 0 ] - c[ 0 ] ) * ( py - c[ 1 ] ) - ( a[ 1 ] - c[ 1 ] ) * ( px - c[ 0 ] );
            if ( ( e0 >= 0.0f && e1 >= 0.0f && e2 >= 0.0f ) || ( e0 <= 0.0f && e1 <= 0.0f && e2 <= 0.0f ) )
                return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealHiddenFaceMap::IsHidden( const Float32* uv0, const Float32* uv1, const Float32* uv2 ) const
{GUCEF_TRACE;

    if ( m_values.empty() )
        return false;
    const Float32 w = (Float32) m_width, h = (Float32) m_height;
    const Float32 a[ 2 ] = { uv0[ 0 ] * w, uv0[ 1 ] * h };
    const Float32 b[ 2 ] = { uv1[ 0 ] * w, uv1[ 1 ] * h };
    const Float32 c[ 2 ] = { uv2[ 0 ] * w, uv2[ 1 ] * h };
    // a texel value of at least the cull value (in steps of 1/255) keeps the triangle
    const Float32 scaled = m_maxCullValue * 255.0f;
    Int32 threshold = (Int32) ceil( scaled - 1e-4f );
    threshold = threshold < 0 ? 0 : threshold;
    if ( threshold > 255 )
        return true;
    return !CoversAtLeast( a, b, c, (UInt32) threshold );
}

/*-------------------------------------------------------------------------*/

void
CUnrealHiddenFaceMap::GetCoverage( Float32& hiddenShare, Float32& bandShare ) const
{GUCEF_TRACE;

    hiddenShare = bandShare = 0.0f;
    if ( m_values.empty() )
        return;
    UInt64 hidden = 0, band = 0;
    for ( size_t i=0; i<m_values.size(); ++i )
    {
        const Float32 value = m_values[ i ] / 255.0f;
        if ( value < m_maxCullValue )
            ++hidden;
        else if ( value < m_minKeepValue )
            ++band;
    }
    hiddenShare = (Float32) ( (Float64) hidden / (Float64) m_values.size() );
    bandShare = (Float32) ( (Float64) band / (Float64) m_values.size() );
}

/*-------------------------------------------------------------------------*/

void
CUnrealHiddenFaceMap::HideFaces( ASSETCONVERTER::CAssetMesh& mesh, UInt32 setBit, Int32 tile, const TInt32Set& leaveAlone, SResult& result ) const
{GUCEF_TRACE;

    const size_t subMeshCount = mesh.subMeshes.size();
    for ( size_t s=0; s<subMeshCount; ++s )
    {
        ASSETCONVERTER::CAssetSubMesh& subMesh = mesh.subMeshes[ s ];
        const UInt32 triangleCount = subMesh.GetTriangleCount();
        const UInt32 vertexCount = subMesh.GetVertexCount();
        if ( leaveAlone.find( subMesh.materialIndex ) != leaveAlone.end() || subMesh.uvSets[ 0 ].size() != (size_t) vertexCount * 2 || m_values.empty() )
        {
            result.otherTriangles += triangleCount;
            continue;
        }
        ASSETCONVERTER::CAssetSubMesh::TUInt32Vector hidden;
        ASSETCONVERTER::CAssetSubMesh::TUInt32Vector shown;
        GUCEF::vector< bool > band( vertexCount, false );
        const ASSETCONVERTER::CAssetSubMesh::TFloatVector& uv = subMesh.uvSets[ 0 ];
        for ( UInt32 t=0; t<triangleCount; ++t )
        {
            const UInt32 i0 = subMesh.indices[ (size_t) t * 3 ], i1 = subMesh.indices[ (size_t) t * 3 + 1 ], i2 = subMesh.indices[ (size_t) t * 3 + 2 ];
            if ( i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount )
            {
                shown.push_back( t );
                ++result.otherTriangles;
                continue;
            }
            const Float32 centre = ( uv[ (size_t) i0 * 2 ] + uv[ (size_t) i1 * 2 ] + uv[ (size_t) i2 * 2 ] ) / 3.0f;
            if ( (Int32) floor( centre ) != tile )
            {
                shown.push_back( t );
                ++result.otherTriangles;
                continue;
            }
            ++result.triangles;
            const Float32 a[ 2 ] = { uv[ (size_t) i0 * 2 ] - (Float32) tile, uv[ (size_t) i0 * 2 + 1 ] };
            const Float32 b[ 2 ] = { uv[ (size_t) i1 * 2 ] - (Float32) tile, uv[ (size_t) i1 * 2 + 1 ] };
            const Float32 c[ 2 ] = { uv[ (size_t) i2 * 2 ] - (Float32) tile, uv[ (size_t) i2 * 2 + 1 ] };
            if ( IsHidden( a, b, c ) )
            {
                hidden.push_back( t );
                continue;
            }
            shown.push_back( t );
            const UInt32 corners[ 3 ] = { i0, i1, i2 };
            const Float32* cornerUvs[ 3 ] = { a, b, c };
            for ( UInt32 k=0; k<3; ++k )
            {
                const Float32 value = Sample( cornerUvs[ k ][ 0 ], cornerUvs[ k ][ 1 ] );
                if ( value >= m_maxCullValue && value < m_minKeepValue )
                    band[ corners[ k ] ] = true;
            }
        }
        for ( size_t v=0; v<band.size(); ++v )
            if ( band[ v ] )
                ++result.bandVertices;
        result.hiddenTriangles += (UInt32) hidden.size();
        if ( hidden.empty() )
            continue;
        if ( shown.empty() )
        {
            subMesh.hiddenBy |= setBit;
            continue;
        }
        ASSETCONVERTER::CAssetSubMesh hiddenPart;
        ASSETCONVERTER::CAssetSubMesh shownPart;
        subMesh.ExtractTriangles( hidden, hiddenPart );
        subMesh.ExtractTriangles( shown, shownPart );
        hiddenPart.hiddenBy |= setBit;
        mesh.subMeshes[ s ] = shownPart;
        mesh.subMeshes.push_back( hiddenPart );
        ++result.newSubMeshes;
    }
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
