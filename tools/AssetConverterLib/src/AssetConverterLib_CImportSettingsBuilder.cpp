/*
 *  AssetConverterLib: the effective settings of an asset: defaults, pack rules, presets and its override
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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CImportSettingsBuilder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CImportSettingsBuilder::Merge( CORE::CDataNode& target, const CORE::CDataNode& layer )
{GUCEF_TRACE;

    CORE::CDataNode::TAttributeMap::const_iterator a = layer.AttributeBegin();
    while ( a != layer.AttributeEnd() )
    {
        target.SetAttribute( (*a).first, (*a).second );
        // a value replaces an object or array of the same name
        target.DelChild( (*a).first );
        ++a;
    }
    CORE::CDataNode::const_iterator c = layer.ConstBegin();
    while ( c != layer.ConstEnd() )
    {
        const CORE::CDataNode& child = *(*c);
        CORE::CDataNode* existing = target.FindChild( child.GetName() );
        if ( GUCEF_DATATYPE_OBJECT == child.GetNodeType() && GUCEF_NULL != existing && GUCEF_DATATYPE_OBJECT == existing->GetNodeType() )
        {
            Merge( *existing, child );
        }
        else
        {
            while ( GUCEF_NULL != target.FindChild( child.GetName() ) )
                target.DelChild( child.GetName() );
            target.DelAttribute( child.GetName() );
            target.AddChild( child );
        }
        ++c;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CImportSettingsBuilder::GetOverridePath( const CAssetPackManifest& manifest, const CORE::CString& assetPath )
{GUCEF_TRACE;

    if ( manifest.overrides.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    return CVfsPaths::Join( manifest.GetOverridesPath(), assetPath + ".import.json" );
}

/*-------------------------------------------------------------------------*/

void
CImportSettingsBuilder::Build( const CORE::CDataNode& importerDefaults ,
                               const CAssetPackManifest& manifest      ,
                               const CORE::CString& assetPath          ,
                               const CORE::CString& importerName       ,
                               CORE::CDataNode& settings               ,
                               CORE::CString::StringVector& appliedLayers )
{GUCEF_TRACE;

    appliedLayers.clear();
    settings = CORE::CDataNode( "settings", GUCEF_DATATYPE_OBJECT );

    Merge( settings, importerDefaults );
    appliedLayers.push_back( "defaults" );

    CORE::CDataNode pack;
    manifest.GetPackSettings( pack );
    Merge( settings, pack );
    appliedLayers.push_back( "pack" );

    for ( size_t i=0; i<manifest.presets.size(); ++i )
    {
        const CAssetPreset& preset = manifest.presets[ i ];
        if ( !preset.Matches( assetPath, importerName ) )
            continue;
        Merge( settings, preset.settings );
        appliedLayers.push_back( "preset " + CORE::UInt32ToString( (UInt32) i ) + ": " + preset.match );
    }

    const CORE::CString overridePath = GetOverridePath( manifest, assetPath );
    if ( !overridePath.IsNULLOrEmpty() && VFS::CVfsGlobal::Instance()->GetVfs().FileExists( overridePath ) )
    {
        CORE::CDataNode overrideNode;
        if ( CCanonicalJson::Load( overridePath, overrideNode ) )
        {
            Merge( settings, overrideNode );
            appliedLayers.push_back( "override " + overridePath );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CImportSettingsBuilder::GetPluginSettings( const CORE::CDataNode& effectiveSettings, const CORE::CDataNode& entrySettings, const CORE::CString& pluginName, CORE::CDataNode& pluginSettings )
{GUCEF_TRACE;

    pluginSettings = CORE::CDataNode( "settings", GUCEF_DATATYPE_OBJECT );
    Merge( pluginSettings, entrySettings );
    const CORE::CDataNode* own = effectiveSettings.FindChild( pluginName );
    if ( GUCEF_NULL != own && GUCEF_DATATYPE_OBJECT == own->GetNodeType() )
        Merge( pluginSettings, *own );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
