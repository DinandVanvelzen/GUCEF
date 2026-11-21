/*
 *  assetpluginUNITY: the Unity project of one import
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

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#include "assetpluginUNITY_CUnityYamlReader.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYMODEL_H
#include "assetpluginUNITY_CUnityModel.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYMODEL_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYMODEL_H ? */

#include "assetpluginUNITY_CUnityProject.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNITY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CUnityProject::GuidIndexName = "unity.guid";
const CORE::CString CUnityProject::SourceName = "unity";
const CORE::CString CUnityProject::SettingsName = "unity";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnityProject::CUnityProject( ASSETCONVERTER::CAssetPackContext& context, ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report, const CORE::CString& assetVfsPath )
    : m_context( context )
    , m_scene( scene )
    , m_report( report )
    , m_assetPath( assetVfsPath )
    , m_settings( SettingsName, GUCEF_DATATYPE_OBJECT )
    , m_metas()
    , m_yamls()
    , m_shaders()
    , m_models()
    , m_cache()
    , m_metaLoads( 0 )
    , m_yamlLoads( 0 )
    , m_modelLoads( 0 )
    , m_reported()
{GUCEF_TRACE;

    const CORE::CDataNode* own = scene.settings.FindChild( SettingsName );
    if ( GUCEF_NULL != own && GUCEF_DATATYPE_OBJECT == own->GetNodeType() )
        m_settings = *own;
}

/*-------------------------------------------------------------------------*/

CUnityProject::~CUnityProject()
{GUCEF_TRACE;

    for ( TMetaMap::iterator i = m_metas.begin(); i != m_metas.end(); ++i )
        GUCEF_DELETE (*i).second;
    for ( TYamlMap::iterator i = m_yamls.begin(); i != m_yamls.end(); ++i )
        GUCEF_DELETE (*i).second;
    for ( TShaderMap::iterator i = m_shaders.begin(); i != m_shaders.end(); ++i )
        GUCEF_DELETE (*i).second;
    for ( TModelMap::iterator i = m_models.begin(); i != m_models.end(); ++i )
        GUCEF_DELETE (*i).second;
}

/*-------------------------------------------------------------------------*/

ASSETCONVERTER::CAssetPackContext& CUnityProject::GetContext( void ) { return m_context; }
ASSETCONVERTER::CAssetScene& CUnityProject::GetScene( void ) { return m_scene; }
ASSETCONVERTER::CConversionReport& CUnityProject::GetReport( void ) { return m_report; }
const CORE::CString& CUnityProject::GetAssetPath( void ) const { return m_assetPath; }
bool CUnityProject::IsCancelled( void ) const { return m_report.IsCancellationRequested(); }
void CUnityProject::ReportProgress( Float32 fraction ) const { m_report.ReportProgress( fraction ); }
const CORE::CDataNode& CUnityProject::GetSettings( void ) const { return m_settings; }

/*-------------------------------------------------------------------------*/

bool
CUnityProject::GetSettingBool( const CORE::CString& name, bool defaultValue ) const
{GUCEF_TRACE;

    const CORE::CVariant& value = m_settings.GetAttributeValueOrChildValueByName( name );
    if ( value.IsNULLOrEmpty() )
        return defaultValue;
    return value.AsBool( defaultValue, true );
}

/*-------------------------------------------------------------------------*/

UInt32 CUnityProject::GetMetaLoadCount( void ) const { return m_metaLoads; }
UInt32 CUnityProject::GetYamlLoadCount( void ) const { return m_yamlLoads; }
UInt32 CUnityProject::GetModelLoadCount( void ) const { return m_modelLoads; }

/*-------------------------------------------------------------------------*/

bool
CUnityProject::ResolveGuid( const CORE::CString& guid, CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    if ( guid.IsNULLOrEmpty() )
        return false;
    return m_context.TryResolve( GuidIndexName, guid.Lowercase(), vfsPath );
}

/*-------------------------------------------------------------------------*/

const CUnityMeta*
CUnityProject::GetMeta( const CORE::CString& assetVfsPath )
{GUCEF_TRACE;

    TMetaMap::const_iterator known = m_metas.find( assetVfsPath );
    if ( known != m_metas.end() )
        return (*known).second;

    CUnityMeta* meta = GUCEF_NULL;
    const CORE::CString metaPath = assetVfsPath + ".meta";
    if ( VFS::CVfsGlobal::Instance()->GetVfs().FileExists( metaPath ) )
    {
        ++m_metaLoads;
        meta = GUCEF_NEW CUnityMeta();
        CORE::CString problem;
        if ( !meta->Load( metaPath, problem ) )
        {
            Warn( Describe( metaPath ), "the .meta file cannot be read: " + problem );
            GUCEF_DELETE meta;
            meta = GUCEF_NULL;
        }
        else
        {
            AddDependency( metaPath );
        }
    }
    m_metas[ assetVfsPath ] = meta;
    return meta;
}

/*-------------------------------------------------------------------------*/

const CUnityYamlFile*
CUnityProject::GetYaml( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    TYamlMap::const_iterator known = m_yamls.find( vfsPath );
    if ( known != m_yamls.end() )
        return (*known).second;

    if ( IsCancelled() )
        return GUCEF_NULL;
    ++m_yamlLoads;
    CUnityYamlFile* file = GUCEF_NEW CUnityYamlFile();
    CORE::CString problem;
    if ( !CUnityYamlReader::Load( vfsPath, *file, problem, &m_report ) )
    {
        if ( m_report.IsCancellationRequested() )
        {
            GUCEF_DELETE file;
            return GUCEF_NULL;
        }
        if ( 0 == file->GetDocumentCount() )
        {
            Warn( Describe( vfsPath ), "the file cannot be read as Unity YAML: " + problem );
            GUCEF_DELETE file;
            file = GUCEF_NULL;
        }
        else
        {
            Warn( Describe( vfsPath ), "a document of the file cannot be read, it is left out: " + problem );
        }
    }
    if ( GUCEF_NULL != file )
        AddDependency( vfsPath );
    m_yamls[ vfsPath ] = file;
    return file;
}

/*-------------------------------------------------------------------------*/

bool
CUnityProject::LoadText( const CORE::CString& vfsPath, CORE::CString& text )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( buffer, vfsPath, "rb" ) )
        return false;
    if ( 0 == buffer.GetDataSize() )
    {
        text.Clear();
        return true;
    }
    text = CORE::CString( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

const CUnityShaderInfo*
CUnityProject::GetShader( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    TShaderMap::const_iterator known = m_shaders.find( vfsPath );
    if ( known != m_shaders.end() )
        return (*known).second;

    CUnityShaderInfo* shader = GUCEF_NEW CUnityShaderInfo();
    CORE::CString text;
    bool ok = LoadText( vfsPath, text );
    if ( ok )
    {
        if ( "shadergraph" == ASSETCONVERTER::CVfsPaths::ExtensionOf( vfsPath ) )
            ok = shader->ParseShaderGraph( text, ASSETCONVERTER::CVfsPaths::StemOf( vfsPath ) );
        else
            ok = shader->ParseShaderLab( text );
    }
    if ( !ok )
    {
        Warn( Describe( vfsPath ), "the shader cannot be read: its properties are not known" );
        GUCEF_DELETE shader;
        shader = GUCEF_NULL;
    }
    else
    {
        AddDependency( vfsPath );
    }
    m_shaders[ vfsPath ] = shader;
    return shader;
}

/*-------------------------------------------------------------------------*/

CUnityModel*
CUnityProject::GetModel( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    TModelMap::const_iterator known = m_models.find( vfsPath );
    if ( known != m_models.end() )
        return (*known).second;

    if ( IsCancelled() )
        return GUCEF_NULL;
    ++m_modelLoads;
    CUnityModel* model = GUCEF_NEW CUnityModel();
    if ( !model->Load( *this, vfsPath ) )
    {
        GUCEF_DELETE model;
        model = GUCEF_NULL;
    }
    m_models[ vfsPath ] = model;
    return model;
}

/*-------------------------------------------------------------------------*/

bool
CUnityProject::TryGetCached( const CORE::CString& key, Int32& index ) const
{GUCEF_TRACE;

    TIndexMap::const_iterator i = m_cache.find( key );
    if ( i == m_cache.end() )
        return false;
    index = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CUnityProject::SetCached( const CORE::CString& key, Int32 index )
{GUCEF_TRACE;

    m_cache[ key ] = index;
}

/*-------------------------------------------------------------------------*/

void
CUnityProject::AddDependency( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    if ( vfsPath != m_assetPath )
        m_scene.AddDependency( vfsPath );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnityProject::GuidVia( const CORE::CString& guid )
{GUCEF_TRACE;

    return "guid:" + guid.Lowercase();
}

/*-------------------------------------------------------------------------*/

void
CUnityProject::AddReference( const CORE::CString& kind, const CORE::CString& vfsPath, const CORE::CString& guid, bool optional )
{GUCEF_TRACE;

    if ( vfsPath.IsNULLOrEmpty() || vfsPath == m_assetPath )
        return;
    m_scene.AddReference( kind, vfsPath, guid.IsNULLOrEmpty() ? CORE::CString::Empty : GuidVia( guid ), optional );
}

/*-------------------------------------------------------------------------*/

void
CUnityProject::AddNestedReferences( const ASSETCONVERTER::CAssetScene& nested )
{GUCEF_TRACE;

    for ( size_t r=0; r<nested.references.size(); ++r )
    {
        const ASSETCONVERTER::CAssetReference& reference = nested.references[ r ];
        if ( !reference.IsResolved() )
            m_scene.AddUnresolvedReference( reference.kind, reference.via, reference.optional );
        else
        if ( reference.target != m_assetPath )
            m_scene.AddReference( reference.kind, reference.target, reference.via, reference.optional );
    }
}

/*-------------------------------------------------------------------------*/

void
CUnityProject::AddUnresolvedReference( const CORE::CString& kind, const CORE::CString& guid, bool optional )
{GUCEF_TRACE;

    if ( guid.IsNULLOrEmpty() )
        return;
    m_scene.AddUnresolvedReference( kind, GuidVia( guid ), optional );
}

/*-------------------------------------------------------------------------*/

bool
CUnityProject::Once( const CORE::CString& kind, const CORE::CString& subject, const CORE::CString& message )
{GUCEF_TRACE;

    return m_reported.insert( kind + "|" + subject + "|" + message ).second;
}

/*-------------------------------------------------------------------------*/

void
CUnityProject::Warn( const CORE::CString& subject, const CORE::CString& message )
{GUCEF_TRACE;

    if ( Once( "w", subject, message ) )
        m_report.AddWarning( SourceName, subject, message );
}

/*-------------------------------------------------------------------------*/

void
CUnityProject::Info( const CORE::CString& subject, const CORE::CString& message )
{GUCEF_TRACE;

    if ( Once( "i", subject, message ) )
        m_report.AddInfo( SourceName, subject, message );
}

/*-------------------------------------------------------------------------*/

void
CUnityProject::Approximate( const CORE::CString& subject, const CORE::CString& message )
{GUCEF_TRACE;

    if ( Once( "a", subject, message ) )
        m_report.AddApproximation( SourceName, subject, message );
}

/*-------------------------------------------------------------------------*/

void
CUnityProject::Skip( const CORE::CString& subject, const CORE::CString& message )
{GUCEF_TRACE;

    if ( Once( "s", subject, message ) )
        m_report.AddSkipped( SourceName, subject, message );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnityProject::Describe( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    CORE::CString assetPath;
    if ( m_context.ToAssetPath( vfsPath, assetPath ) )
        return assetPath;
    return vfsPath;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
