/*
 *  assetcookerOGRE_TestApp: Test application for the Ogre 1.x cooker of the asset converter (assetcookerOGRE)
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
#include <string.h>

#include "OgreDefaultHardwareBufferManager.h"
#include "OgreRTShaderSystem.h"
#include "OgreParticleSystemManager.h"
#include "OgreControllerManager.h"

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H
#include "pluginglueOGRE_COgreGlobal.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H ? */

#include "TestCookerSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const TestPluginGroup = "assetcookerOGRE_TestApp";
static const char* const OgrePluginGroup = "assetcookerOGRE_TestApp_Ogre";
static const char* const WorkMountPoint = "work";
static const char* const WorkArchiveName = "assetcookerOGRE_TestApp/work";
static CORE::CString g_workRoot;
static Ogre::DefaultHardwareBufferManager* g_bufferManager = GUCEF_NULL;
static Ogre::DefaultTextureManager* g_textureManager = GUCEF_NULL;
static Ogre::ControllerManager* g_controllerManager = GUCEF_NULL;
static bool g_shaderGenerator = false;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
LoadTestPlugin( const char* moduleName )
{
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    const CORE::CString path = CORE::CString( "$MODULEDIR$/" ) + moduleName;
    return pluginControl.AddPluginFromDir( path + "_d", TestPluginGroup, true ) || pluginControl.AddPluginFromDir( path, TestPluginGroup, true );
}

/*-------------------------------------------------------------------------*/

/** The entry of an Ogre plugin in the bootstrap configuration, as the .ini of an application makes it */
static void
AddOgrePluginEntry( CORE::CDataNode& group, const CORE::CString& pluginName )
{
    CORE::CDataNode* entry = group.AddChild( "PluginMetaData" );
    entry->SetAttribute( "Description", pluginName );
    entry->SetAttribute( "Path", "$MODULEDIR$" );
    entry->SetAttribute( "Filename", pluginName );
    entry->SetAttribute( "AltFilename", pluginName + "_d" );
    entry->SetAttribute( "Type", "OgrePlugin" );
    entry->SetAttribute( "LoadFailAllowed", false );
}

/*-------------------------------------------------------------------------*/

bool
SetUpCookerTests( void )
{GUCEF_TRACE;

    VFS::CVfsGlobal::Instance();
    IMAGE::CImageGlobal::Instance();
    CAssetConverterGlobal::Instance();

    // Ogre without a render system, made before the cooker loads so the cooker uses it as it is
    if ( GUCEF_NULL == PLUGINGLUE::OGRE::COgreGlobal::Instance() || GUCEF_NULL == Ogre::Root::getSingletonPtr() )
        return false;
    CORE::CDataNode config;
    CORE::CDataNode* group = config.AddChild( "PluginGroup" );
    group->SetAttribute( "GroupName", OgrePluginGroup );
    group->SetAttribute( "LoadImmediately", true );
    AddOgrePluginEntry( *group, "OgrePlugin_GucefVFS" );
    AddOgrePluginEntry( *group, "OgrePlugin_DotScene" );
    AddOgrePluginEntry( *group, "OgrePlugin_ParticleFX" );
    if ( !CORE::CCoreGlobal::Instance()->GetPluginControl().LoadConfig( config ) )
        return false;
    g_bufferManager = OGRE_NEW Ogre::DefaultHardwareBufferManager();
    g_textureManager = OGRE_NEW Ogre::DefaultTextureManager();
    Ogre::MaterialManager::getSingleton().initialise();
    // the billboard renderer of particle systems, which Root::initialise() would register: the particle tests load scripts
    if ( !Ogre::ParticleSystemManager::getSingleton().getRendererFactoryIterator().hasMoreElements() )
        Ogre::ParticleSystemManager::getSingleton()._initialise();
    // and the controllers of the time, which a particle system attached to a node uses
    if ( GUCEF_NULL == Ogre::ControllerManager::getSingletonPtr() )
        g_controllerManager = OGRE_NEW Ogre::ControllerManager();
    g_shaderGenerator = Ogre::RTShader::ShaderGenerator::initialize();

    // JSON for every file of the converter, zlib for PNG, FreeImage to read pictures, the glTF plugin and the cooker
    bool loaded = LoadTestPlugin( "dstorepluginJSONPARSER" );
    loaded = LoadTestPlugin( "codecspluginZLIB" ) && loaded;
    loaded = LoadTestPlugin( "imgpluginFreeImage" ) && loaded;
    loaded = LoadTestPlugin( "assetpluginGLTF" ) && loaded;
    loaded = LoadTestPlugin( "assetcookerOGRE" ) && loaded;
    if ( !loaded || !g_shaderGenerator )
        return false;

    g_workRoot = CORE::CombinePath( CORE::TempDir(), "assetcookerOGRE_TestApp" );
    if ( CORE::DirExists( g_workRoot ) )
        CORE::DeleteDir( g_workRoot, true );
    CORE::CreateDirs( g_workRoot );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.MountArchive( WorkArchiveName, g_workRoot, VFS::CVFS::FileSystemArchiveTypeName, WorkMountPoint, true, false, g_workRoot ) )
        return false;
    return !GetOgreCooker().IsNULL();
}

/*-------------------------------------------------------------------------*/

void
TearDownCookerTests( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( TestPluginGroup );
    if ( GUCEF_NULL != Ogre::Root::getSingletonPtr() )
    {
        Ogre::ResourceGroupManager::getSingleton().shutdownAll();
        // the templates hold emitters and affectors of ParticleFX: they go before the plugin (a Root that was not
        // initialised does not remove them in its shutdown)
        Ogre::ParticleSystemManager::getSingleton().removeAllTemplates( true );
    }
    if ( g_shaderGenerator )
        Ogre::RTShader::ShaderGenerator::destroy();
    g_shaderGenerator = false;
    VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( WorkArchiveName );
    OGRE_DELETE g_controllerManager;
    g_controllerManager = GUCEF_NULL;
    OGRE_DELETE g_textureManager;
    g_textureManager = GUCEF_NULL;
    OGRE_DELETE g_bufferManager;
    g_bufferManager = GUCEF_NULL;
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( OgrePluginGroup );
    PLUGINGLUE::OGRE::COgreGlobal::Deinstance();
}

/*-------------------------------------------------------------------------*/

CORE::CString
WorkDiskPath( const CORE::CString& relativePath )
{GUCEF_TRACE;

    return CORE::CombinePath( g_workRoot, relativePath );
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeWorkFolder( const CORE::CString& name )
{GUCEF_TRACE;

    const CORE::CString folder = WorkDiskPath( name );
    if ( CORE::DirExists( folder ) )
        CORE::DeleteDir( folder, true );
    CORE::CreateDirs( folder );
    return folder;
}

/*-------------------------------------------------------------------------*/

bool
WriteWorkText( const CORE::CString& relativePath, const CORE::CString& text )
{GUCEF_TRACE;

    const CORE::CString path = WorkDiskPath( relativePath );
    CORE::CreateDirs( CORE::StripFilename( path ) );
    return CORE::WriteStringAsTextFile( path, text );
}

/*-------------------------------------------------------------------------*/

CORE::CString
ReadVfsText( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( buffer, vfsPath, "rb" ) )
        return CORE::CString::Empty;
    const char terminator = '\0';
    buffer.Append( &terminator, 1, true );
    return CORE::CString( static_cast< const char* >( buffer.GetConstBufferPtr() ) );
}

/*-------------------------------------------------------------------------*/

bool
VfsFileExists( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    return VFS::CVfsGlobal::Instance()->GetVfs().FileExists( vfsPath );
}

/*-------------------------------------------------------------------------*/

bool
WriteTestPng( const CORE::CString& vfsPath, UInt32 width, UInt32 height, UInt8 r, UInt8 g, UInt8 b, UInt8 a )
{GUCEF_TRACE;

    IMAGE::TPixelMapPtr picture( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8* texels = static_cast< UInt8* >( picture->GetDataPtr() );
    for ( UInt32 i=0; i<width*height; ++i )
    {
        texels[ i*4 ] = r;
        texels[ i*4+1 ] = g;
        texels[ i*4+2 ] = b;
        texels[ i*4+3 ] = a;
    }
    return CTextureBaker::SavePng( vfsPath, *picture );
}

/*-------------------------------------------------------------------------*/

bool
ReadTestPicture( const CORE::CString& vfsPath, UInt32& width, UInt32& height, UInt8 firstTexel[ 4 ] )
{GUCEF_TRACE;

    IMAGE::TPixelMapPtr picture;
    if ( !CTextureBaker::LoadPicture( vfsPath, picture ) || picture.IsNULL() )
        return false;
    width = picture->GetWidthInPixels();
    height = picture->GetHeightInPixels();
    const UInt32 channels = picture->GetNumberOfChannelsPerPixel();
    const UInt8* texels = static_cast< const UInt8* >( picture->GetDataPtr() );
    for ( UInt32 c=0; c<4; ++c )
        firstTexel[ c ] = c < channels ? texels[ c ] : 255;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
WriteLicence( const CORE::CString& relativeFolder, const CORE::CString& id )
{GUCEF_TRACE;

    CLicenceRecord record;
    record.id = id;
    record.store = "Test store";
    record.product = "Test pack";
    record.licenceName = "Test licence";
    record.seats = 1;
    CORE::CDataNode node( "licence", GUCEF_DATATYPE_OBJECT );
    record.Serialize( node );
    return WriteWorkText( relativeFolder + "/" + id + ".licence.json", CCanonicalJson::ToJson( node ) );
}

/*-------------------------------------------------------------------------*/

TAssetCookerPtr
GetOgreCooker( void )
{GUCEF_TRACE;

    TAssetCookerPtr cooker;
    CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetCooker( "ogre", cooker );
    return cooker;
}

/*-------------------------------------------------------------------------*/

bool
MakeIntermediate( const CAssetScene& source, const CORE::CString& caseFolder, const CORE::CString& assetPath, CCookSettings::SAsset& asset )
{GUCEF_TRACE;

    TAssetExporterPtr exporter;
    if ( !CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetExporter( "gltf", exporter ) )
        return false;
    CAssetScene scene( source );
    scene.name = CVfsPaths::StemOf( assetPath );
    const CORE::CString intermediate = "work/" + caseFolder + "/inter";
    const CORE::CString exportDir = CVfsPaths::Join( intermediate, CVfsPaths::FolderOf( assetPath ) );
    CConversionReport report;
    CConversionReportEntry& entry = report.SelectAsset( assetPath );
    if ( !exporter->Export( scene, exportDir, report ) )
        return false;

    // the sidecar, as the pipeline writes it: the prefabs of the scene among what it holds
    CORE::CreateDirs( WorkDiskPath( caseFolder + "/src" ) );
    CAssetPackManifest manifest;
    manifest.id = "pack";
    manifest.source = "/work/" + caseFolder + "/src";
    manifest.intermediate = "/" + intermediate;
    CAssetPackContext context;
    if ( !context.Open( manifest ) )
        return false;
    CORE::CDataNode sidecar;
    CConversionSidecar::Build( scene, entry, CORE::CString::StringVector(), "licence", context, intermediate, sidecar );
    asset.assetPath = assetPath;
    asset.sidecarFile = CConversionSidecar::GetPath( intermediate, assetPath );
    if ( !CCanonicalJson::Store( asset.sidecarFile, sidecar ) )
        return false;
    asset.files.clear();
    asset.files.push_back( CVfsPaths::Join( exportDir, scene.name + ".gltf" ) );
    asset.files.push_back( CVfsPaths::Join( exportDir, scene.name + ".bin" ) );
    asset.files.push_back( asset.sidecarFile );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
GetProfileDir( const CORE::CString& caseFolder, const CORE::CString& profile )
{GUCEF_TRACE;

    return "work/" + caseFolder + "/out/" + profile;
}

/*-------------------------------------------------------------------------*/

bool
CookIntermediate( const CORE::CString& caseFolder, CCookSettings& settings, CConversionReport& report )
{GUCEF_TRACE;

    TAssetCookerPtr cooker = GetOgreCooker();
    if ( cooker.IsNULL() )
        return false;
    settings.target = "ogre";
    if ( settings.packId.IsNULLOrEmpty() )
        settings.packId = "pack";
    settings.sourceRoot = "work/" + caseFolder + "/src";
    return cooker->Cook( "work/" + caseFolder + "/inter", GetProfileDir( caseFolder, settings.profile.name ), settings, report );
}

/*-------------------------------------------------------------------------*/

bool
UseOgreGroup( const Ogre::String& group, const CORE::CString& vfsFolder, bool parseScripts )
{GUCEF_TRACE;

    try
    {
        DropOgreGroup( group );
        Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
        groups.createResourceGroup( group, false );
        groups.addResourceLocation( ( "/" + vfsFolder ).STL_String(), "GucefVFS", group, true );
        if ( parseScripts )
            groups.initialiseResourceGroup( group );
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "The resource group " << group << " cannot be made: " << e.getDescription() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
DropOgreGroup( const Ogre::String& group )
{GUCEF_TRACE;

    try
    {
        Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
        if ( groups.resourceGroupExists( group ) )
            groups.destroyResourceGroup( group );
    }
    catch ( const Ogre::Exception& )
    {
    }
}

/*-------------------------------------------------------------------------*/

bool
ReportHas( const CConversionReport& report, TReportItemKind kind, const CORE::CString& text )
{GUCEF_TRACE;

    const CConversionReport::TEntryVector& entries = report.GetEntries();
    for ( size_t e=0; e<entries.size(); ++e )
        for ( size_t i=0; i<entries[ e ].items.size(); ++i )
            if ( entries[ e ].items[ i ].kind == kind && entries[ e ].items[ i ].message.HasSubstr( text ) >= 0 )
                return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
ReportHasFile( const CConversionReport& report, const CORE::CString& path )
{GUCEF_TRACE;

    const CConversionReport::TEntryVector& entries = report.GetEntries();
    for ( size_t e=0; e<entries.size(); ++e )
        for ( size_t f=0; f<entries[ e ].files.size(); ++f )
            if ( entries[ e ].files[ f ].path == path )
                return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
Near( float a, float b, float tolerance )
{
    return fabs( a - b ) <= tolerance;
}

/*-------------------------------------------------------------------------*/
