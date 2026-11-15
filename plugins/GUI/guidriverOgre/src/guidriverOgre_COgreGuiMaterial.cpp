/*
 *  guidriverOgre: gucefGUI window backend that gives Ogre render windows on the platform windows
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

#include <Ogre.h>
#include <OgreGpuProgramManager.h>
#include <OgreHardwarePixelBuffer.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "guidriverOgre_COgreGuiMaterial.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEROGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString COgreGuiMaterial::DataModuleName = "guidriverOgre";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const VertexProgramName = "GucefGui/VertexProgram";
const char* const FragmentProgramName = "GucefGui/FragmentProgram";
const char* const WhiteTextureName = "GucefGui/White";

/** What the materials share: the programs of the render system and the white texture */
struct SSharedResources
{
    Ogre::GpuProgramPtr vertexProgram;
    Ogre::GpuProgramPtr fragmentProgram;
    Ogre::TexturePtr whiteTexture;
    UInt32 users;
    UInt32 materialCounter;
};

SSharedResources g_shared = { Ogre::GpuProgramPtr(), Ogre::GpuProgramPtr(), Ogre::TexturePtr(), 0, 0 };

/** The text of a shader file of the data of the plugin */
bool
LoadShaderSource( const CORE::CString& fileName, Ogre::String& source )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.MountModuleData( COgreGuiMaterial::DataModuleName ) )
        return false;
    const CORE::CString path = COgreGuiMaterial::DataModuleName + "/shaders/" + fileName;
    CORE::CDynamicBuffer buffer;
    if ( !vfs.LoadFile( buffer, path ) || 0 == buffer.GetDataSize() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreGuiMaterial: The shader file " << path << " cannot be read" );
        return false;
    }
    source.assign( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
    return true;
}

Ogre::GpuProgramPtr
MakeProgram( const char* name, const CORE::CString& language, Ogre::GpuProgramType type, const CORE::CString& fileName, const char* entryPoint, const char* target )
{
    Ogre::String source;
    if ( !LoadShaderSource( fileName, source ) )
        return Ogre::GpuProgramPtr();
    Ogre::GpuProgramManager& programs = Ogre::GpuProgramManager::getSingleton();
    Ogre::GpuProgramPtr program = programs.createProgram( name, Ogre::RGN_INTERNAL, language.C_String(), type );
    program->setSource( source );
    if ( GUCEF_NULL != entryPoint )
        program->setParameter( "entry_point", entryPoint );
    if ( GUCEF_NULL != target )
        program->setParameter( "target", target );
    program->load();
    if ( program->hasCompileError() || !program->isSupported() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreGuiMaterial: The " << language << " program " << fileName << " does not compile on this render system" );
        programs.remove( program );
        return Ogre::GpuProgramPtr();
    }
    return program;
}

bool
AcquireSharedResources( void )
{
    if ( g_shared.users > 0 )
    {
        ++g_shared.users;
        return true;
    }

    const CORE::CString language = COgreGuiMaterial::GetShadingLanguage();
    if ( language.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreGuiMaterial: The render system has none of the shading languages of the GUI programs (GLSL, GLSL ES, HLSL)" );
        return false;
    }
    try
    {
        if ( "hlsl" == language )
        {
            g_shared.vertexProgram = MakeProgram( VertexProgramName, language, Ogre::GPT_VERTEX_PROGRAM, "GucefGui.hlsl", "GucefGuiVS", "vs_4_0" );
            g_shared.fragmentProgram = MakeProgram( FragmentProgramName, language, Ogre::GPT_FRAGMENT_PROGRAM, "GucefGui.hlsl", "GucefGuiPS", "ps_4_0" );
        }
        else
        {
            g_shared.vertexProgram = MakeProgram( VertexProgramName, language, Ogre::GPT_VERTEX_PROGRAM, "GucefGui_vs." + language, GUCEF_NULL, GUCEF_NULL );
            g_shared.fragmentProgram = MakeProgram( FragmentProgramName, language, Ogre::GPT_FRAGMENT_PROGRAM, "GucefGui_fs." + language, GUCEF_NULL, GUCEF_NULL );
        }
        if ( !g_shared.vertexProgram || !g_shared.fragmentProgram )
        {
            COgreGuiMaterial::ReleaseSharedResources();
            return false;
        }

        // geometry without a texture samples white
        g_shared.whiteTexture = Ogre::TextureManager::getSingleton().createManual( WhiteTextureName, Ogre::RGN_INTERNAL, Ogre::TEX_TYPE_2D,
                                                                                   1, 1, 0, Ogre::PF_BYTE_RGBA );
        Ogre::uint8 white[ 4 ] = { 255, 255, 255, 255 };
        g_shared.whiteTexture->getBuffer()->blitFromMemory( Ogre::PixelBox( 1, 1, 1, Ogre::PF_BYTE_RGBA, white ) );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreGuiMaterial: The GUI programs cannot be made: " << e.getFullDescription() );
        COgreGuiMaterial::ReleaseSharedResources();
        return false;
    }
    g_shared.users = 1;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "OgreGuiMaterial: Made the GUI programs in " << language );
    return true;
}

void
ReleaseSharedResourcesOfUser( void )
{
    if ( g_shared.users > 0 && 0 == --g_shared.users )
        COgreGuiMaterial::ReleaseSharedResources();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreGuiMaterial::COgreGuiMaterial( TBlendMode blendMode )
    : m_blendMode( blendMode )
    , m_initialized( false )
    , m_nearestFiltering( true )
    , m_material()
    , m_vertexParameters()
    , m_fragmentParameters()
    , m_sampler()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreGuiMaterial::~COgreGuiMaterial()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

CORE::CString
COgreGuiMaterial::GetShadingLanguage( void )
{GUCEF_TRACE;

    Ogre::GpuProgramManager* programs = Ogre::GpuProgramManager::getSingletonPtr();
    if ( GUCEF_NULL == programs )
        return CORE::CString::Empty;
    if ( programs->isLanguageSupported( "hlsl" ) )
        return "hlsl";
    if ( programs->isLanguageSupported( "glsles" ) )
        return "glsles";
    if ( programs->isLanguageSupported( "glsl" ) )
        return "glsl";
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

Ogre::TexturePtr
COgreGuiMaterial::GetWhiteTexture( void )
{GUCEF_TRACE;

    return g_shared.whiteTexture;
}

/*-------------------------------------------------------------------------*/

void
COgreGuiMaterial::ReleaseSharedResources( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != Ogre::Root::getSingletonPtr() )
    {
        try
        {
            if ( g_shared.whiteTexture )
                Ogre::TextureManager::getSingleton().remove( g_shared.whiteTexture );
            if ( g_shared.vertexProgram )
                Ogre::GpuProgramManager::getSingleton().remove( g_shared.vertexProgram );
            if ( g_shared.fragmentProgram )
                Ogre::GpuProgramManager::getSingleton().remove( g_shared.fragmentProgram );
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreGuiMaterial: Releasing the GUI programs failed: " << e.getFullDescription() );
        }
    }
    g_shared.whiteTexture.reset();
    g_shared.vertexProgram.reset();
    g_shared.fragmentProgram.reset();
    g_shared.users = 0;
}

/*-------------------------------------------------------------------------*/

bool
COgreGuiMaterial::Initialize( void )
{GUCEF_TRACE;

    if ( m_initialized )
        return true;
    if ( GUCEF_NULL == Ogre::Root::getSingletonPtr() || !Ogre::Root::getSingleton().isInitialised() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreGuiMaterial: There is no initialised Ogre Root" );
        return false;
    }
    if ( !AcquireSharedResources() )
        return false;

    try
    {
        // the pass, for drivers that also draw through a scene manager; BeginDrawing() sets the same state directly
        const Ogre::String name = "GucefGui/Material/" + Ogre::StringConverter::toString( ++g_shared.materialCounter );
        m_material = Ogre::MaterialManager::getSingleton().create( name, Ogre::RGN_INTERNAL );
        Ogre::Pass* pass = m_material->getTechnique( 0 )->getPass( 0 );
        pass->setVertexProgram( VertexProgramName );
        pass->setFragmentProgram( FragmentProgramName );
        pass->setCullingMode( Ogre::CULL_NONE );
        pass->setManualCullingMode( Ogre::MANUAL_CULL_NONE );
        pass->setDepthCheckEnabled( false );
        pass->setDepthWriteEnabled( false );
        pass->setLightingEnabled( false );
        pass->setFog( true, Ogre::FOG_NONE );
        pass->setVertexColourTracking( Ogre::TVC_DIFFUSE );
        pass->setSceneBlendingOperation( Ogre::SBO_ADD );
        if ( BLENDMODE_PREMULTIPLIED == m_blendMode )
            pass->setSeparateSceneBlending( Ogre::SBF_ONE, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA, Ogre::SBF_ONE, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA );
        else
            pass->setSeparateSceneBlending( Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA, Ogre::SBF_ONE, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA );
        Ogre::TextureUnitState* unit = pass->createTextureUnitState();
        unit->setTextureAddressingMode( Ogre::TAM_CLAMP );
        unit->setTextureFiltering( m_nearestFiltering ? Ogre::TFO_NONE : Ogre::TFO_BILINEAR );
        unit->setTexture( g_shared.whiteTexture );

        m_vertexParameters = g_shared.vertexProgram->createParameters();
        m_vertexParameters->setIgnoreMissingParams( true );
        m_vertexParameters->setNamedConstant( "projection", Ogre::Matrix4::IDENTITY );
        m_vertexParameters->setNamedConstant( "transform", Ogre::Matrix4::IDENTITY );
        m_vertexParameters->setNamedConstant( "translation", Ogre::Vector2( 0.0f, 0.0f ) );
        m_fragmentParameters = g_shared.fragmentProgram->createParameters();
        m_fragmentParameters->setIgnoreMissingParams( true );
        m_fragmentParameters->setNamedConstant( "guiTexture", 0 );

        m_sampler = Ogre::TextureManager::getSingleton().createSampler();
        m_sampler->setAddressingMode( Ogre::TAM_CLAMP );
        m_sampler->setFiltering( m_nearestFiltering ? Ogre::TFO_NONE : Ogre::TFO_BILINEAR );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreGuiMaterial: The GUI material cannot be made: " << e.getFullDescription() );
        m_initialized = true;
        Release();
        return false;
    }
    m_initialized = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
COgreGuiMaterial::Release( void )
{GUCEF_TRACE;

    if ( !m_initialized )
        return;
    m_initialized = false;
    if ( m_material && GUCEF_NULL != Ogre::MaterialManager::getSingletonPtr() )
        Ogre::MaterialManager::getSingleton().remove( m_material );
    m_material.reset();
    m_vertexParameters.reset();
    m_fragmentParameters.reset();
    m_sampler.reset();
    ReleaseSharedResourcesOfUser();
}

/*-------------------------------------------------------------------------*/

bool
COgreGuiMaterial::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_initialized;
}

/*-------------------------------------------------------------------------*/

COgreGuiMaterial::TBlendMode
COgreGuiMaterial::GetBlendMode( void ) const
{GUCEF_TRACE;

    return m_blendMode;
}

/*-------------------------------------------------------------------------*/

Ogre::MaterialPtr
COgreGuiMaterial::GetMaterial( void ) const
{GUCEF_TRACE;

    return m_material;
}

/*-------------------------------------------------------------------------*/

Ogre::Pass*
COgreGuiMaterial::GetPass( void ) const
{GUCEF_TRACE;

    return m_material ? m_material->getTechnique( 0 )->getPass( 0 ) : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
COgreGuiMaterial::SetNearestFiltering( bool nearest )
{GUCEF_TRACE;

    m_nearestFiltering = nearest;
    const Ogre::TextureFilterOptions filtering = nearest ? Ogre::TFO_NONE : Ogre::TFO_BILINEAR;
    if ( m_sampler )
        m_sampler->setFiltering( filtering );
    Ogre::Pass* pass = GetPass();
    if ( GUCEF_NULL != pass && pass->getNumTextureUnitStates() > 0 )
        pass->getTextureUnitState( 0 )->setTextureFiltering( filtering );
}

/*-------------------------------------------------------------------------*/

bool
COgreGuiMaterial::GetNearestFiltering( void ) const
{GUCEF_TRACE;

    return m_nearestFiltering;
}

/*-------------------------------------------------------------------------*/

Ogre::Matrix4
COgreGuiMaterial::MakeProjection( float offsetX       ,
                                  float offsetY       ,
                                  float scaleX        ,
                                  float scaleY        ,
                                  UInt32 targetWidth  ,
                                  UInt32 targetHeight ,
                                  bool flipY          )
{GUCEF_TRACE;

    // target pixel = offset + scale * GUI pixel; clip x = 2 * target x / width - 1, clip y = 1 - 2 * target y / height,
    // or its negation where the render target is stored upside down (OpenGL render textures)
    const float width = targetWidth > 0 ? (float) targetWidth : 1.0f;
    const float height = targetHeight > 0 ? (float) targetHeight : 1.0f;
    const float sy = flipY ? 1.0f : -1.0f;
    return Ogre::Matrix4( 2.0f * scaleX / width, 0.0f,                       0.0f, 2.0f * offsetX / width - 1.0f,
                          0.0f,                  sy * 2.0f * scaleY / height, 0.0f, sy * ( 2.0f * offsetY / height - 1.0f ),
                          0.0f,                  0.0f,                       1.0f, 0.0f,
                          0.0f,                  0.0f,                       0.0f, 1.0f );
}

/*-------------------------------------------------------------------------*/

void
COgreGuiMaterial::BeginDrawing( Ogre::RenderSystem& renderSystem, const Ogre::Matrix4& projection )
{GUCEF_TRACE;

    if ( !m_initialized )
        return;

    renderSystem.bindGpuProgram( g_shared.vertexProgram->_getBindingDelegate() );
    renderSystem.bindGpuProgram( g_shared.fragmentProgram->_getBindingDelegate() );

    SetColourWriteEnabled( renderSystem, true );
    renderSystem._setDepthBufferParams( false, false, Ogre::CMPF_ALWAYS_PASS );
    renderSystem._setCullingMode( Ogre::CULL_NONE );
    renderSystem._setPolygonMode( Ogre::PM_SOLID );
    renderSystem._setAlphaRejectSettings( Ogre::CMPF_ALWAYS_PASS, 0, false );
    renderSystem.setStencilState( Ogre::StencilState() );
    renderSystem.setScissorTest( false );
    renderSystem._disableTextureUnitsFrom( 1 );

    m_vertexParameters->setNamedConstant( "projection", projection );
    m_vertexParameters->setNamedConstant( "transform", Ogre::Matrix4::IDENTITY );
    m_vertexParameters->setNamedConstant( "translation", Ogre::Vector2( 0.0f, 0.0f ) );
    SetTexture( renderSystem, Ogre::TexturePtr() );
}

/*-------------------------------------------------------------------------*/

void
COgreGuiMaterial::SetColourWriteEnabled( Ogre::RenderSystem& renderSystem, bool enabled )
{GUCEF_TRACE;

    Ogre::ColourBlendState blend;
    blend.writeR = blend.writeG = blend.writeB = blend.writeA = enabled;
    blend.operation = Ogre::SBO_ADD;
    blend.alphaOperation = Ogre::SBO_ADD;
    blend.sourceFactor = BLENDMODE_PREMULTIPLIED == m_blendMode ? Ogre::SBF_ONE : Ogre::SBF_SOURCE_ALPHA;
    blend.destFactor = Ogre::SBF_ONE_MINUS_SOURCE_ALPHA;
    blend.sourceFactorAlpha = Ogre::SBF_ONE;
    blend.destFactorAlpha = Ogre::SBF_ONE_MINUS_SOURCE_ALPHA;
    renderSystem.setColourBlendState( blend );
}

/*-------------------------------------------------------------------------*/

void
COgreGuiMaterial::SetProjection( const Ogre::Matrix4& projection )
{GUCEF_TRACE;

    if ( m_vertexParameters )
        m_vertexParameters->setNamedConstant( "projection", projection );
}

/*-------------------------------------------------------------------------*/

void
COgreGuiMaterial::SetTransform( const Ogre::Matrix4* transform )
{GUCEF_TRACE;

    if ( m_vertexParameters )
        m_vertexParameters->setNamedConstant( "transform", GUCEF_NULL != transform ? *transform : Ogre::Matrix4::IDENTITY );
}

/*-------------------------------------------------------------------------*/

void
COgreGuiMaterial::SetTranslation( float x, float y )
{GUCEF_TRACE;

    if ( m_vertexParameters )
        m_vertexParameters->setNamedConstant( "translation", Ogre::Vector2( x, y ) );
}

/*-------------------------------------------------------------------------*/

void
COgreGuiMaterial::SetTexture( Ogre::RenderSystem& renderSystem, const Ogre::TexturePtr& texture )
{GUCEF_TRACE;

    if ( !m_initialized )
        return;
    renderSystem._setTexture( 0, true, texture ? texture : g_shared.whiteTexture );
    renderSystem._setSampler( 0, *m_sampler );
}

/*-------------------------------------------------------------------------*/

void
COgreGuiMaterial::Render( Ogre::RenderSystem& renderSystem, const Ogre::RenderOperation& operation )
{GUCEF_TRACE;

    if ( !m_initialized )
        return;
    renderSystem.bindGpuProgramParameters( Ogre::GPT_VERTEX_PROGRAM, m_vertexParameters, Ogre::GPV_ALL );
    renderSystem.bindGpuProgramParameters( Ogre::GPT_FRAGMENT_PROGRAM, m_fragmentParameters, Ogre::GPV_ALL );
    renderSystem._render( operation );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEROGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
