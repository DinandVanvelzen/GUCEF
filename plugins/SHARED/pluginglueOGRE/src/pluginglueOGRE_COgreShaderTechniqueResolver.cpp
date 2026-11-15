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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreTechnique.h"
#include "OgreMaterial.h"
#include "OgreShaderGenerator.h"

#include "pluginglueOGRE_COgreShaderTechniqueResolver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreShaderTechniqueResolver::COgreShaderTechniqueResolver( Ogre::RTShader::ShaderGenerator& shaderGenerator )
    : Ogre::MaterialManager::Listener()
    , m_shaderGenerator( shaderGenerator )
    , m_techniquesCreated( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreShaderTechniqueResolver::~COgreShaderTechniqueResolver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Ogre::Technique*
COgreShaderTechniqueResolver::handleSchemeNotFound( unsigned short schemeIndex         ,
                                                    const Ogre::String& schemeName     ,
                                                    Ogre::Material* originalMaterial   ,
                                                    unsigned short lodIndex            ,
                                                    const Ogre::Renderable* renderable )
{GUCEF_TRACE;

    if ( GUCEF_NULL == originalMaterial || !m_shaderGenerator.hasRenderState( schemeName ) )
        return GUCEF_NULL;

    // a technique of the scheme with generated shaders, made from the technique of the default scheme
    if ( !m_shaderGenerator.createShaderBasedTechnique( *originalMaterial, Ogre::MaterialManager::DEFAULT_SCHEME_NAME, schemeName ) )
        return GUCEF_NULL;

    // the shaders are made now, not at the first frame that draws the material
    m_shaderGenerator.validateMaterial( schemeName, *originalMaterial );

    const Ogre::Material::Techniques& techniques = originalMaterial->getTechniques();
    for ( size_t i=0; i<techniques.size(); ++i )
    {
        if ( techniques[ i ]->getSchemeName() == schemeName )
        {
            ++m_techniquesCreated;
            return techniques[ i ];
        }
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
COgreShaderTechniqueResolver::afterIlluminationPassesCreated( Ogre::Technique* technique )
{GUCEF_TRACE;

    if ( GUCEF_NULL == technique || !m_shaderGenerator.hasRenderState( technique->getSchemeName() ) )
        return false;

    const Ogre::Material* material = technique->getParent();
    m_shaderGenerator.validateMaterialIlluminationPasses( technique->getSchemeName(), material->getName(), material->getGroup() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COgreShaderTechniqueResolver::beforeIlluminationPassesCleared( Ogre::Technique* technique )
{GUCEF_TRACE;

    if ( GUCEF_NULL == technique || !m_shaderGenerator.hasRenderState( technique->getSchemeName() ) )
        return false;

    const Ogre::Material* material = technique->getParent();
    m_shaderGenerator.invalidateMaterialIlluminationPasses( technique->getSchemeName(), material->getName(), material->getGroup() );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
COgreShaderTechniqueResolver::GetTechniquesCreated( void ) const
{GUCEF_TRACE;

    return m_techniquesCreated;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
