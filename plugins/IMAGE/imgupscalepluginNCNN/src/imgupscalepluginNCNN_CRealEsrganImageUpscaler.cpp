/*
 *  imgupscalepluginNCNN: GUCEF image plugin that upscales pictures with Real-ESRGAN neural networks running on ncnn
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

#include <stdlib.h>
#include <string.h>

#include "net.h"
#include "gpu.h"

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "imgupscalepluginNCNN_CRealEsrganImageUpscaler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUPSCALEPLUGINNCNN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const CRealEsrganImageUpscaler::ModelDir = "imgupscalepluginNCNN/models";

namespace {

/** The upscaler of the plugin, for the exit handler */
CRealEsrganImageUpscaler* g_instance = GUCEF_NULL;

/** Pixels around a tile that the network sees but that are not taken from its result */
const Int32 TileMargin = 10;
const UInt32 DefaultTileSize = 200;

/** The alpha of the picture scaled 'factor' times, bilinearly, into the alpha of the result */
void
ScaleAlpha( const UInt8* source, Int32 width, Int32 height, Int32 factor, UInt8* target )
{
    const Int32 targetWidth = width * factor;
    const Int32 targetHeight = height * factor;
    for ( Int32 y=0; y<targetHeight; ++y )
    {
        Float32 sy = ( y + 0.5f ) / factor - 0.5f;
        Int32 y0 = sy < 0.0f ? 0 : (Int32) sy;
        Int32 y1 = y0 + 1 < height ? y0 + 1 : height - 1;
        Float32 fy = sy < 0.0f ? 0.0f : sy - (Float32) y0;
        for ( Int32 x=0; x<targetWidth; ++x )
        {
            Float32 sx = ( x + 0.5f ) / factor - 0.5f;
            Int32 x0 = sx < 0.0f ? 0 : (Int32) sx;
            Int32 x1 = x0 + 1 < width ? x0 + 1 : width - 1;
            Float32 fx = sx < 0.0f ? 0.0f : sx - (Float32) x0;
            Float32 top = source[ ( (size_t) y0 * width + x0 ) * 4 + 3 ] * ( 1.0f - fx ) + source[ ( (size_t) y0 * width + x1 ) * 4 + 3 ] * fx;
            Float32 bottom = source[ ( (size_t) y1 * width + x0 ) * 4 + 3 ] * ( 1.0f - fx ) + source[ ( (size_t) y1 * width + x1 ) * 4 + 3 ] * fx;
            target[ ( (size_t) y * targetWidth + x ) * 4 + 3 ] = (UInt8) ( top * ( 1.0f - fy ) + bottom * fy + 0.5f );
        }
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRealEsrganImageUpscaler::CRealEsrganImageUpscaler( void )
    : IMAGE::CImageUpscaler()
    , m_tileSize( DefaultTileSize )
    , m_gpuStarted( false )
    , m_gpuInstance( false )
    , m_useGpu( false )
    , m_lock()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<5; ++i )
        m_models[ i ] = GUCEF_NULL;
    g_instance = this;
}

/*-------------------------------------------------------------------------*/

CRealEsrganImageUpscaler::~CRealEsrganImageUpscaler()
{GUCEF_TRACE;

    ReleaseGpu();
    if ( this == g_instance )
        g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CRealEsrganImageUpscaler::ReleaseNetworksAtExit( void )
{
    if ( GUCEF_NULL != g_instance )
        g_instance->ReleaseNetworks();
}

/*-------------------------------------------------------------------------*/

void
CRealEsrganImageUpscaler::StartGpu( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_gpuStarted )
        return;
    m_gpuStarted = true;
    m_gpuInstance = 0 == ncnn::create_gpu_instance();
    m_useGpu = m_gpuInstance && ncnn::get_gpu_count() > 0;

    // ncnn destroys its GPU instance in an exit handler it registers when the instance is made, which runs before the plugins are
    // unloaded at exit: the networks, which hold GPU memory, have to go before it. Registered after ncnn's handler, this one runs first
    static bool releaseAtExitRegistered = false;
    if ( m_gpuInstance && !releaseAtExitRegistered )
    {
        atexit( ReleaseNetworksAtExit );
        releaseAtExitRegistered = true;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "RealEsrganImageUpscaler: The networks run on " << ( m_useGpu ? "the GPU (Vulkan)" : "the CPU" ) );
}

/*-------------------------------------------------------------------------*/

void
CRealEsrganImageUpscaler::ReleaseGpu( void )
{GUCEF_TRACE;

    ReleaseNetworks();
    MT::CScopeMutex lock( m_lock );
    if ( m_gpuInstance )
        ncnn::destroy_gpu_instance();
    m_gpuInstance = false;
    m_useGpu = false;
    m_gpuStarted = false;
}

/*-------------------------------------------------------------------------*/

void
CRealEsrganImageUpscaler::ReleaseNetworks( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<5; ++i )
    {
        if ( GUCEF_NULL != m_models[ i ] )
        {
            delete m_models[ i ]->net;
            delete m_models[ i ];
            m_models[ i ] = GUCEF_NULL;
        }
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRealEsrganImageUpscaler::GetName( void ) const
{GUCEF_TRACE;

    return "realesrgan";
}

/*-------------------------------------------------------------------------*/

IMAGE::CImageUpscaler::TFactorVector
CRealEsrganImageUpscaler::GetFactors( void ) const
{GUCEF_TRACE;

    TFactorVector factors;
    factors.push_back( 2 );
    factors.push_back( 3 );
    factors.push_back( 4 );
    return factors;
}

/*-------------------------------------------------------------------------*/

bool
CRealEsrganImageUpscaler::IsUsingGpu( void ) const
{GUCEF_TRACE;

    StartGpu();
    return m_useGpu;
}

/*-------------------------------------------------------------------------*/

void
CRealEsrganImageUpscaler::SetTileSize( UInt32 tileSize )
{GUCEF_TRACE;

    m_tileSize = tileSize;
}

/*-------------------------------------------------------------------------*/

CRealEsrganImageUpscaler::TModel*
CRealEsrganImageUpscaler::GetModel( UInt32 factor )
{GUCEF_TRACE;

    StartGpu();
    MT::CScopeMutex lock( m_lock );
    if ( GUCEF_NULL != m_models[ factor ] )
        return m_models[ factor ];

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.MountModuleData( "imgupscalepluginNCNN" ) )
        return GUCEF_NULL;

    CORE::CString name = CORE::CString( ModelDir ) + "/realesr-animevideov3-x" + CORE::UInt32ToString( factor );
    TModel* model = new TModel();
    model->net = GUCEF_NULL;
    if ( !vfs.LoadFile( model->param, name + ".param" ) || !vfs.LoadFile( model->weights, name + ".bin" ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RealEsrganImageUpscaler: Cannot read the network " << name );
        delete model;
        return GUCEF_NULL;
    }
    // the network text is read as a C string
    UInt8 terminator = 0;
    model->param.Append( &terminator, 1 );

    model->net = new ncnn::Net();
    model->net->opt.use_vulkan_compute = m_useGpu;
    if ( 0 != model->net->load_param_mem( static_cast< const char* >( model->param.GetConstBufferPtr() ) ) ||
         0 == model->net->load_model( static_cast< const unsigned char* >( model->weights.GetConstBufferPtr() ) ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RealEsrganImageUpscaler: The network " << name << " cannot be loaded" );
        delete model->net;
        delete model;
        return GUCEF_NULL;
    }
    m_models[ factor ] = model;
    return model;
}

/*-------------------------------------------------------------------------*/

bool
CRealEsrganImageUpscaler::Upscale( const IMAGE::CPixelMap& source, UInt32 factor, IMAGE::TPixelMapPtr& result, IMAGE::CIUpscaleProgress* progress )
{GUCEF_TRACE;

    if ( factor < 2 || factor > 4 )
        return false;
    TModel* model = GetModel( factor );
    if ( GUCEF_NULL == model )
        return false;

    // RGBA with 8 bits per channel, the picture of the caller stays as it is
    const IMAGE::CPixelMap* rgba = &source;
    IMAGE::CPixelMap converted( source );
    if ( IMAGE::PSF_RGBA != source.GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != source.GetPixelComponentDataType() )
    {
        if ( !converted.ConvertFormatTo( IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) )
            return false;
        rgba = &converted;
    }

    const Int32 width = (Int32) rgba->GetWidthInPixels();
    const Int32 height = (Int32) rgba->GetHeightInPixels();
    const Int32 f = (Int32) factor;
    if ( width <= 0 || height <= 0 )
        return false;
    const UInt8* pixels = static_cast< const UInt8* >( rgba->GetDataPtr() );

    IMAGE::TPixelMapPtr upscaled( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, (UInt32) ( width * f ), (UInt32) ( height * f ), IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8* out = static_cast< UInt8* >( upscaled->GetDataPtr() );
    ScaleAlpha( pixels, width, height, f, out );

    const Int32 tile = 0 == m_tileSize ? ( width > height ? width : height ) : (Int32) m_tileSize;
    const Int32 tilesX = ( width + tile - 1 ) / tile;
    const Int32 tilesY = ( height + tile - 1 ) / tile;
    const float normalize[ 3 ] = { 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f };
    for ( Int32 ty=0; ty<tilesY; ++ty )
    {
        for ( Int32 tx=0; tx<tilesX; ++tx )
        {
            if ( GUCEF_NULL != progress && !progress->OnUpscaleProgress( (Float32) ( ty * tilesX + tx ) / (Float32) ( tilesX * tilesY ) ) )
                return false;

            // the tile with a margin, within the picture
            const Int32 x0 = tx * tile, y0 = ty * tile;
            const Int32 x1 = x0 + tile < width ? x0 + tile : width;
            const Int32 y1 = y0 + tile < height ? y0 + tile : height;
            const Int32 inX0 = x0 - TileMargin > 0 ? x0 - TileMargin : 0;
            const Int32 inY0 = y0 - TileMargin > 0 ? y0 - TileMargin : 0;
            const Int32 inX1 = x1 + TileMargin < width ? x1 + TileMargin : width;
            const Int32 inY1 = y1 + TileMargin < height ? y1 + TileMargin : height;

            ncnn::Mat in = ncnn::Mat::from_pixels( pixels + ( (size_t) inY0 * width + inX0 ) * 4, ncnn::Mat::PIXEL_RGBA2RGB,
                                                   inX1 - inX0, inY1 - inY0, width * 4 );
            in.substract_mean_normalize( GUCEF_NULL, normalize );
            ncnn::Extractor extractor = model->net->create_extractor();
            ncnn::Mat upscaledTile;
            if ( 0 != extractor.input( "data", in ) || 0 != extractor.extract( "output", upscaledTile ) ||
                 3 != upscaledTile.c || ( inX1 - inX0 ) * f != upscaledTile.w || ( inY1 - inY0 ) * f != upscaledTile.h )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RealEsrganImageUpscaler: The network did not upscale a tile" );
                return false;
            }

            // the tile without its margin into the result
            const Int32 offsetX = ( x0 - inX0 ) * f, offsetY = ( y0 - inY0 ) * f;
            for ( Int32 y=0; y<( y1 - y0 ) * f; ++y )
            {
                UInt8* row = out + ( (size_t) ( y0 * f + y ) * width * f + x0 * f ) * 4;
                for ( Int32 c=0; c<3; ++c )
                {
                    const float* channel = upscaledTile.channel( c ).row( offsetY + y ) + offsetX;
                    for ( Int32 x=0; x<( x1 - x0 ) * f; ++x )
                    {
                        float value = channel[ x ] * 255.0f + 0.5f;
                        row[ x * 4 + c ] = (UInt8) ( value < 0.0f ? 0.0f : ( value > 255.0f ? 255.0f : value ) );
                    }
                }
            }
        }
    }

    if ( GUCEF_NULL != progress && !progress->OnUpscaleProgress( 1.0f ) )
        return false;
    result = upscaled;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUPSCALEPLUGINNCNN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
