/*
 *  assetpluginGODOT: the import settings of a file of a Godot project
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#include "assetpluginGODOT_CGodotVariant.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTVARIANT_H ? */

#include "assetpluginGODOT_CGodotImportSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GODOT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGodotImportSettings::CGodotImportSettings( void )
    : m_document()
    , m_settingsPath()
    , m_remap( GUCEF_NULL )
    , m_params( GUCEF_NULL )
    , m_present( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGodotImportSettings::~CGodotImportSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CGodotImportSettings::Load( const ASSETCONVERTER::CAssetPackContext& context, const CORE::CString& assetPath )
{GUCEF_TRACE;

    m_present = false;
    m_remap = GUCEF_NULL;
    m_params = GUCEF_NULL;
    m_settingsPath = assetPath + ".import";
    if ( !context.HasFile( m_settingsPath ) )
        return false;
    if ( !m_document.Load( context, m_settingsPath ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "assetpluginGODOT: " << m_document.GetProblem() );
        return false;
    }
    m_remap = m_document.FindSection( "remap" );
    m_params = m_document.FindSection( "params" );
    m_present = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotImportSettings::IsPresent( void ) const
{GUCEF_TRACE;

    return m_present;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGodotImportSettings::GetSettingsPath( void ) const
{GUCEF_TRACE;

    return m_settingsPath;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGodotImportSettings::GetImporter( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_remap ? CGodotVariant::GetString( CGodotVariant::FindProperty( *m_remap, "importer" ) ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGodotImportSettings::GetUid( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_remap ? CGodotVariant::GetString( CGodotVariant::FindProperty( *m_remap, "uid" ) ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CGodotImportSettings::FindParam( const CORE::CString& key ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_params ? CGodotVariant::FindProperty( *m_params, key ) : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

Float64
CGodotImportSettings::GetRootScale( void ) const
{GUCEF_TRACE;

    return CGodotVariant::GetNumber( FindParam( "nodes/root_scale" ), 1.0 );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGodotImportSettings::GetRootName( void ) const
{GUCEF_TRACE;

    return CGodotVariant::GetString( FindParam( "nodes/root_name" ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGodotImportSettings::GetRootType( void ) const
{GUCEF_TRACE;

    return CGodotVariant::GetString( FindParam( "nodes/root_type" ) );
}

/*-------------------------------------------------------------------------*/

bool
CGodotImportSettings::GetImportAnimations( void ) const
{GUCEF_TRACE;

    return CGodotVariant::GetBool( FindParam( "animation/import" ), true );
}

/*-------------------------------------------------------------------------*/

bool
CGodotImportSettings::GetGenerateLods( void ) const
{GUCEF_TRACE;

    return CGodotVariant::GetBool( FindParam( "meshes/generate_lods" ), true );
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CGodotImportSettings::FindSubresource( const CORE::CString& group, const CORE::CString& name ) const
{GUCEF_TRACE;

    const CORE::CDataNode* groups = CGodotVariant::FindMember( FindParam( "_subresources" ), group );
    return CGodotVariant::FindMember( groups, name );
}

/*-------------------------------------------------------------------------*/

bool
CGodotImportSettings::GetAnimationLoop( const CORE::CString& animationName, bool& loop ) const
{GUCEF_TRACE;

    const CORE::CDataNode* mode = CGodotVariant::FindMember( FindSubresource( "animations", animationName ), "settings/loop_mode" );
    if ( GUCEF_NULL == mode )
        return false;
    loop = CGodotVariant::GetInteger( mode, 0 ) > 0;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGodotImportSettings::GetExternalMaterial( const CORE::CString& materialName, CORE::CString& path, CORE::CString& uid ) const
{GUCEF_TRACE;

    const CORE::CDataNode* material = FindSubresource( "materials", materialName );
    if ( !CGodotVariant::GetBool( CGodotVariant::FindMember( material, "use_external/enabled" ), false ) )
        return false;
    const CORE::CString primary = CGodotVariant::GetString( CGodotVariant::FindMember( material, "use_external/path" ) );
    const CORE::CString fallback = CGodotVariant::GetString( CGodotVariant::FindMember( material, "use_external/fallback_path" ) );
    path = fallback;
    uid.Clear();
    if ( primary.StartsWith( CORE::CString( "uid://" ) ) )
        uid = primary;
    else
    if ( !primary.IsNULLOrEmpty() )
        path = primary;
    return !path.IsNULLOrEmpty() || !uid.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CGodotImportSettings::IsNodeSkipped( const CORE::CString& nodePath ) const
{GUCEF_TRACE;

    const CORE::CDataNode* node = FindSubresource( "nodes", "PATH:" + nodePath );
    return CGodotVariant::GetBool( CGodotVariant::FindMember( node, "import/skip_import" ), false );
}

/*-------------------------------------------------------------------------*/

void
CGodotImportSettings::AddPlain( CORE::CDataNode& object ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_params )
        return;
    const CORE::CDataNode* properties = m_params->FindChild( "properties" );
    if ( GUCEF_NULL == properties )
        return;
    for ( const CORE::CDataNode* property = properties->GetFirstChild(); GUCEF_NULL != property; property = property->GetNext() )
        CGodotVariant::AddPlain( object, property->GetName(), *property );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
