/*
 *  AssetWorkbenchLib: the material view: the materials of a script on a sphere, a cube or a plane, with their values in the inspector
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
#include <OgreShaderGenerator.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H
#include "gucefGUI_CPropertyRowDescriptor.h"
#define GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H
#endif /* GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H ? */

#include "AssetWorkbenchLib_CMaterialView.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString
ColourText( const Ogre::ColourValue& colour )
{
    return CORE::UInt32ToString( (UInt32) ( colour.r * 255.0f + 0.5f ) ) + "," + CORE::UInt32ToString( (UInt32) ( colour.g * 255.0f + 0.5f ) ) + "," +
           CORE::UInt32ToString( (UInt32) ( colour.b * 255.0f + 0.5f ) ) + "," + CORE::UInt32ToString( (UInt32) ( colour.a * 255.0f + 0.5f ) );
}

bool
ParseColour( const CORE::CString& text, Ogre::ColourValue& colour )
{
    GUI::CColour parsed;
    if ( !GUI::CColour::FromString( text, parsed ) )
        return false;
    colour = Ogre::ColourValue( (Float32) parsed.r / 255.0f, (Float32) parsed.g / 255.0f, (Float32) parsed.b / 255.0f, (Float32) parsed.a / 255.0f );
    return true;
}

const char* const CullingNames[] = { "None", "Clockwise", "Anticlockwise" };

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMaterialView::CMaterialView( CWorkbenchStage& stage )
    : m_stage( stage )
    , m_preview( stage )
    , m_materialNames()
    , m_selectedMaterial()
    , m_groupName()
    , m_shape( SHAPE_SPHERE )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMaterialView::~CMaterialView()
{GUCEF_TRACE;

    Unload();
}

/*-------------------------------------------------------------------------*/

void
CMaterialView::Unload( void )
{GUCEF_TRACE;

    m_preview.Unload();
    m_materialNames.clear();
    m_selectedMaterial.Clear();
}

/*-------------------------------------------------------------------------*/

CModelView&
CMaterialView::GetPreview( void )
{GUCEF_TRACE;

    return m_preview;
}

/*-------------------------------------------------------------------------*/

void
CMaterialView::ParseMaterialNames( const CORE::CString& scriptText, CORE::CString::StringVector& names )
{GUCEF_TRACE;

    names.clear();
    const CORE::CString::StringVector lines = scriptText.ReplaceChar( '\r', '\n' ).ParseElements( '\n', false );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        CORE::CString line = lines[ i ].Trim( true ).Trim( false );
        if ( line.Length() < 10 || line.SubstrFromRange( 0, 9 ) != "material " )
            continue;
        line = line.CutChars( 9, true ).Trim( true ).Trim( false );
        // "material Name : Parent {" and quoted names: the name is the first word
        if ( line.Length() > 1 && '"' == line[ 0 ] )
        {
            const Int32 closing = line.HasChar( '"', 1, true );
            if ( closing > 1 )
                names.push_back( line.SubstrFromRange( 1, (UInt32) closing ) );
            continue;
        }
        UInt32 end = 0;
        while ( end < line.Length() && ' ' != line[ end ] && '\t' != line[ end ] && ':' != line[ end ] && '{' != line[ end ] )
            ++end;
        if ( end > 0 )
            names.push_back( line.SubstrFromRange( 0, end ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMaterialView::LoadScript( const CORE::CString& groupName, const CORE::CString& scriptName )
{GUCEF_TRACE;

    Unload();
    try
    {
        Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource( scriptName.STL_String(), groupName.STL_String() );
        const CORE::CString text = stream->getAsString();
        ParseMaterialNames( text, m_materialNames );
        // a script that was read before is read again: the materials of it that are there are taken out, as Ogre refuses the same name twice
        for ( size_t i=0; i<m_materialNames.size(); ++i )
            if ( Ogre::MaterialManager::getSingleton().resourceExists( m_materialNames[ i ].STL_String(), groupName.STL_String() ) )
                Ogre::MaterialManager::getSingleton().remove( m_materialNames[ i ].STL_String(), groupName.STL_String() );
        stream->seek( 0 );
        Ogre::MaterialManager::getSingleton().parseScript( stream, groupName.STL_String() );
        m_groupName = groupName;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CMaterialView: The script " << scriptName << " cannot be read: " << e.getDescription() );
        return false;
    }
    if ( m_materialNames.empty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CMaterialView: The script " << scriptName << " defines no material" );
        return false;
    }
    return ShowMaterial( m_materialNames[ 0 ], groupName );
}

/*-------------------------------------------------------------------------*/

bool
CMaterialView::ShowShape( void )
{GUCEF_TRACE;

    const char* const names[ SHAPE_COUNT ] = { Ogre::SceneManager::PT_SPHERE, Ogre::SceneManager::PT_CUBE, Ogre::SceneManager::PT_PLANE };
    if ( !m_preview.LoadMesh( Ogre::RGN_DEFAULT, names[ m_shape ] ) )
        return false;
    if ( !m_selectedMaterial.IsNULLOrEmpty() )
        m_preview.SetSubMeshMaterial( 0, m_selectedMaterial, m_groupName );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMaterialView::ShowMaterial( const CORE::CString& materialName, const CORE::CString& groupName )
{GUCEF_TRACE;

    if ( !Ogre::MaterialManager::getSingleton().resourceExists( materialName.STL_String(), groupName.STL_String() ) &&
         !Ogre::MaterialManager::getSingleton().resourceExists( materialName.STL_String() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CMaterialView: There is no material " << materialName );
        return false;
    }
    m_selectedMaterial = materialName;
    m_groupName = groupName;
    if ( m_materialNames.empty() )
        m_materialNames.push_back( materialName );
    return ShowShape();
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector&
CMaterialView::GetMaterialNames( void ) const
{GUCEF_TRACE;

    return m_materialNames;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMaterialView::GetSelectedMaterial( void ) const
{GUCEF_TRACE;

    return m_selectedMaterial;
}

/*-------------------------------------------------------------------------*/

void
CMaterialView::SetShape( EShape shape )
{GUCEF_TRACE;

    if ( (Int32) shape < 0 || (Int32) shape >= (Int32) SHAPE_COUNT || shape == m_shape )
        return;
    m_shape = shape;
    if ( !m_selectedMaterial.IsNULLOrEmpty() )
        ShowShape();
}

/*-------------------------------------------------------------------------*/

CMaterialView::EShape
CMaterialView::GetShape( void ) const
{GUCEF_TRACE;

    return m_shape;
}

/*-------------------------------------------------------------------------*/

const char*
CMaterialView::GetShapeName( EShape shape )
{GUCEF_TRACE;

    switch ( shape )
    {
        case SHAPE_SPHERE:  return "Sphere";
        case SHAPE_CUBE:    return "Cube";
        default:            return "Plane";
    }
}

/*-------------------------------------------------------------------------*/

Ogre::MaterialPtr
CMaterialView::GetMaterial( void ) const
{GUCEF_TRACE;

    if ( m_selectedMaterial.IsNULLOrEmpty() )
        return Ogre::MaterialPtr();
    return Ogre::MaterialManager::getSingleton().getByName( m_selectedMaterial.STL_String(), m_groupName.STL_String() );
}

/*-------------------------------------------------------------------------*/

void
CMaterialView::WriteInspectorData( CORE::CDataNode& root ) const
{GUCEF_TRACE;

    root.SetName( "Material" );
    root.SetAttribute( "Name", m_selectedMaterial );
    Ogre::MaterialPtr material = GetMaterial();
    if ( !material )
        return;

    CORE::CDataNode* general = root.AddChild( "General" );
    general->SetAttribute( "Group", CORE::CString( material->getGroup() ) );
    general->SetAttribute( "Techniques", CORE::UInt32ToString( material->getNumTechniques() ) );
    general->SetAttribute( "Shape", GetShapeName( m_shape ) );
    if ( material->getNumTechniques() > 0 )
        general->SetAttribute( "Passes", CORE::UInt32ToString( material->getTechnique( 0 )->getNumPasses() ) );
    if ( m_materialNames.size() > 1 )
    {
        CORE::CString all;
        for ( size_t i=0; i<m_materialNames.size(); ++i )
            all += ( 0 == i ? "" : ", " ) + m_materialNames[ i ];
        general->SetAttribute( "InScript", all );
    }

    if ( 0 == material->getNumTechniques() || 0 == material->getTechnique( 0 )->getNumPasses() )
        return;
    Ogre::Pass* pass = material->getTechnique( 0 )->getPass( 0 );
    CORE::CDataNode* parameters = root.AddChild( "Pass" );
    parameters->SetAttribute( "Lighting", pass->getLightingEnabled() ? "true" : "false" );
    parameters->SetAttribute( "Ambient", ColourText( pass->getAmbient() ) );
    parameters->SetAttribute( "Diffuse", ColourText( pass->getDiffuse() ) );
    parameters->SetAttribute( "Specular", ColourText( pass->getSpecular() ) );
    parameters->SetAttribute( "Emissive", ColourText( pass->getSelfIllumination() ) );
    parameters->SetAttribute( "Shininess", GUI::CPropertyRowDescriptor::NumberToText( pass->getShininess(), 1 ) );
    parameters->SetAttribute( "DepthWrite", pass->getDepthWriteEnabled() ? "true" : "false" );
    parameters->SetAttribute( "DepthCheck", pass->getDepthCheckEnabled() ? "yes" : "no" );
    const UInt32 culling = (UInt32) pass->getCullingMode() > 0 ? (UInt32) pass->getCullingMode() - 1 : 0;
    parameters->SetAttribute( "Culling", CullingNames[ culling < 3 ? culling : 0 ] );
    parameters->SetAttribute( "Programs", pass->hasGpuProgram( Ogre::GPT_VERTEX_PROGRAM ) || pass->hasGpuProgram( Ogre::GPT_FRAGMENT_PROGRAM ) ? "yes" : "generated" );

    CORE::CDataNode* textures = root.AddChild( "Textures" );
    for ( unsigned short u=0; u<pass->getNumTextureUnitStates(); ++u )
    {
        Ogre::TextureUnitState* unit = pass->getTextureUnitState( u );
        CORE::CDataNode* slot = textures->AddChild( "Slot" + CORE::UInt32ToString( u ) );
        const CORE::CString textureName = unit->getNumFrames() > 0 ? CORE::CString( unit->getFrameTextureName( 0 ) ) : CORE::CString::Empty;
        slot->SetAttribute( "Texture", textureName );
        if ( !textureName.IsNULLOrEmpty() )
        {
            Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName( textureName.STL_String(), material->getGroup() );
            if ( texture && texture->isLoaded() )
            {
                slot->SetAttribute( "Size", CORE::UInt32ToString( texture->getWidth() ) + " x " + CORE::UInt32ToString( texture->getHeight() ) );
                slot->SetAttribute( "Format", CORE::CString( Ogre::PixelUtil::getFormatName( texture->getFormat() ) ) );
                slot->SetAttribute( "Memory", CMeshInspector::FormatByteCount( texture->getSize() ) );
            }
            else
                slot->SetAttribute( "Size", "not loaded" );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMaterialView::GetInspectorDescriptors( GUI::CPropertyInspectorForm::TDescriptorMap& descriptors ) const
{GUCEF_TRACE;

    descriptors.clear();
    CORE::CString::StringVector culling;
    for ( UInt32 i=0; i<3; ++i )
        culling.push_back( CullingNames[ i ] );
    descriptors[ "Pass@Lighting" ] = GUI::CPropertyRowDescriptor::Boolean();
    descriptors[ "Pass@Ambient" ] = GUI::CPropertyRowDescriptor::Colour();
    descriptors[ "Pass@Diffuse" ] = GUI::CPropertyRowDescriptor::Colour();
    descriptors[ "Pass@Specular" ] = GUI::CPropertyRowDescriptor::Colour();
    descriptors[ "Pass@Emissive" ] = GUI::CPropertyRowDescriptor::Colour();
    descriptors[ "Pass@Shininess" ] = GUI::CPropertyRowDescriptor::NumberInRange( 0.0, 128.0, 1 );
    descriptors[ "Pass@DepthWrite" ] = GUI::CPropertyRowDescriptor::Boolean();
    descriptors[ "Pass@Culling" ] = GUI::CPropertyRowDescriptor::Choice( culling );
}

/*-------------------------------------------------------------------------*/

bool
CMaterialView::ApplyProperty( const CORE::CString& path, const CORE::CString& value )
{GUCEF_TRACE;

    Ogre::MaterialPtr material = GetMaterial();
    if ( !material || 0 == material->getNumTechniques() || 0 == material->getTechnique( 0 )->getNumPasses() )
        return false;
    Ogre::Pass* pass = material->getTechnique( 0 )->getPass( 0 );
    Ogre::ColourValue colour;
    bool applied = true;
    if ( "Pass@Lighting" == path )
        pass->setLightingEnabled( CORE::StringToBool( value, true ) );
    else
    if ( "Pass@Ambient" == path && ParseColour( value, colour ) )
        pass->setAmbient( colour );
    else
    if ( "Pass@Diffuse" == path && ParseColour( value, colour ) )
        pass->setDiffuse( colour );
    else
    if ( "Pass@Specular" == path && ParseColour( value, colour ) )
        pass->setSpecular( colour );
    else
    if ( "Pass@Emissive" == path && ParseColour( value, colour ) )
        pass->setSelfIllumination( colour );
    else
    if ( "Pass@Shininess" == path )
        pass->setShininess( CORE::StringToFloat( value, 0.0f ) );
    else
    if ( "Pass@DepthWrite" == path )
        pass->setDepthWriteEnabled( CORE::StringToBool( value, true ) );
    else
    if ( "Pass@Culling" == path )
    {
        Ogre::CullingMode mode = Ogre::CULL_NONE;
        if ( "Clockwise" == value )
            mode = Ogre::CULL_CLOCKWISE;
        else
        if ( "Anticlockwise" == value )
            mode = Ogre::CULL_ANTICLOCKWISE;
        pass->setCullingMode( mode );
    }
    else
        applied = false;

    if ( applied )
    {
        // the generated shaders have the old values until the generator is told to make them again
        if ( GUCEF_NULL != Ogre::RTShader::ShaderGenerator::getSingletonPtr() )
            Ogre::RTShader::ShaderGenerator::getSingleton().invalidateMaterial( Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME, *material );
        m_stage.MarkDirty();
    }
    return applied;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
