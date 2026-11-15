/*
 *  pluginglueOGRE: Support library of the Ogre 1.x plugin set: Ogre Root setup, log, plugin loading
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

#ifndef GUCEF_PLUGINGLUE_OGRE_COGRESHADERTECHNIQUERESOLVER_H
#define GUCEF_PLUGINGLUE_OGRE_COGRESHADERTECHNIQUERESOLVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreMaterialManager.h"

#ifndef GUCEF_PLUGINGLUE_OGRE_MACROS_H
#include "pluginglueOGRE_macros.h"
#define GUCEF_PLUGINGLUE_OGRE_MACROS_H
#endif /* GUCEF_PLUGINGLUE_OGRE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace Ogre { namespace RTShader { class ShaderGenerator; } }

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Gives materials a technique of the run time shader system when a viewport asks for a material scheme that the
 *  material has no technique for and the shader generator knows that scheme.
 *
 *  The render systems without a fixed function pipeline (OpenGL 3+, OpenGL ES 2, Vulkan, Direct3D 11) render their
 *  viewports in the scheme of the shader generator. A material of a file or of the application has techniques of
 *  the default scheme only; when a renderable with it is drawn, the material manager asks its listeners for a
 *  technique of the viewport's scheme, and this resolver makes one with generated shaders from the default
 *  technique. The illumination passes of such techniques (stencil shadows) are kept in step with the generator.
 *
 *  It does what OgreBites::SGTechniqueResolverListener of Ogre's application framework does, which GUCEF does not
 *  build. COgreRootSetup installs it when it starts the shader generator.
 */
class GUCEF_PLUGINGLUE_OGRE_EXPORT_CPP COgreShaderTechniqueResolver : public Ogre::MaterialManager::Listener
{
    public:

    explicit COgreShaderTechniqueResolver( Ogre::RTShader::ShaderGenerator& shaderGenerator );

    virtual ~COgreShaderTechniqueResolver();

    /** A technique of the scheme with generated shaders, NULL when the scheme is not one of the shader generator */
    virtual Ogre::Technique* handleSchemeNotFound( unsigned short schemeIndex         ,
                                                   const Ogre::String& schemeName     ,
                                                   Ogre::Material* originalMaterial   ,
                                                   unsigned short lodIndex            ,
                                                   const Ogre::Renderable* renderable ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool afterIlluminationPassesCreated( Ogre::Technique* technique ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool beforeIlluminationPassesCleared( Ogre::Technique* technique ) GUCEF_VIRTUAL_OVERRIDE;

    /** How many techniques this resolver made: for tests */
    UInt32 GetTechniquesCreated( void ) const;

    private:

    COgreShaderTechniqueResolver( const COgreShaderTechniqueResolver& src );              /**< not implemented */
    COgreShaderTechniqueResolver& operator=( const COgreShaderTechniqueResolver& src );   /**< not implemented */

    private:

    Ogre::RTShader::ShaderGenerator& m_shaderGenerator;
    UInt32 m_techniquesCreated;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_OGRE_COGRESHADERTECHNIQUERESOLVER_H ? */
