/*
 *  assetpluginUNREAL: Brings the materials of the slots of a mesh into the scene: resolves the instance chains and maps the parameters
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALINSTANCEREADER_H
#include "assetpluginUNREAL_CUnrealMaterialInstanceReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALINSTANCEREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALINSTANCEREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALMAPPER_H
#include "assetpluginUNREAL_CUnrealMaterialMapper.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALMAPPER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALMAPPER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALIMPORTER_H
#include "assetpluginUNREAL_CUnrealMaterialImporter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALIMPORTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALIMPORTER_H ? */

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The index of the material that was made for the object path before, or -1 */
static Int32
FindMaterial( const ASSETCONVERTER::CAssetScene& scene, const CORE::CString& objectPath )
{
    for ( size_t m=0; m<scene.materials.size(); ++m )
        if ( scene.materials[ m ].source.properties.GetAttributeValue( "objectPath" ).AsString() == objectPath )
            return (Int32) m;
    return -1;
}

/*-------------------------------------------------------------------------*/

void
CUnrealMaterialImporter::AddMaterialsOfMesh( const CUnrealPackage& package                 ,
                                             const CUnrealSkeletalMesh& mesh               ,
                                             const CUnrealObjectPathResolver& resolver     ,
                                             ASSETCONVERTER::CAssetScene& scene            ,
                                             ASSETCONVERTER::CConversionReport& report     ,
                                             GUCEF::vector< Int32 >& slotMaterials         )
{GUCEF_TRACE;

    AddMaterialsOfSlots( package, mesh.materialSlots, resolver, scene, report, slotMaterials );
}

/*-------------------------------------------------------------------------*/

void
CUnrealMaterialImporter::AddMaterialsOfSlots( const CUnrealPackage& package                                  ,
                                              const CUnrealSkeletalMesh::TMaterialSlotVector& materialSlots  ,
                                              const CUnrealObjectPathResolver& resolver                      ,
                                              ASSETCONVERTER::CAssetScene& scene                             ,
                                              ASSETCONVERTER::CConversionReport& report                      ,
                                              GUCEF::vector< Int32 >& slotMaterials                          )
{GUCEF_TRACE;

    slotMaterials.assign( materialSlots.size(), -1 );
    for ( size_t s=0; s<materialSlots.size(); ++s )
    {
        const CUnrealSkeletalMesh::SMaterialSlot& slot = materialSlots[ s ];
        CORE::CString objectPath;
        if ( slot.material.IsImport() && slot.material.ToImportIndex() < package.GetImports().size() )
            objectPath = package.GetImportPath( slot.material.ToImportIndex() );
        else
        if ( slot.material.IsExport() && slot.material.ToExportIndex() < package.GetExports().size() )
            objectPath = package.GetExportPath( slot.material.ToExportIndex() );

        const CORE::CString slotName = slot.slotName.IsNULLOrEmpty() ? "slot" + CORE::UInt64ToString( s ) : slot.slotName;
        Int32 known = objectPath.IsNULLOrEmpty() ? -1 : FindMaterial( scene, objectPath );
        if ( known >= 0 )
        {
            slotMaterials[ s ] = known;
            continue;
        }

        ASSETCONVERTER::CAssetMaterial material;
        CORE::CString vfsPath;
        if ( !objectPath.IsNULLOrEmpty() && resolver.ResolveReference( objectPath, ASSETCONVERTER::CAssetReference::KindUsesMaterial, vfsPath ) )
        {
            CUnrealResolvedMaterial resolved;
            if ( CUnrealMaterialInstanceReader::Resolve( vfsPath, resolver, resolved ) )
            {
                CUnrealMaterialMapper::Map( resolved, resolved.name.IsNULLOrEmpty() ? slotName : resolved.name, material, report );
                material.source.properties.SetAttribute( "objectPath", objectPath );
            }
            else
            {
                report.AddWarning( "unreal", slotName, "The material " + objectPath + " cannot be read: " + ( resolved.problems.empty() ? CORE::CString( "unknown reason" ) : resolved.problems[ 0 ] ) );
                material.name = slotName;
                material.source.properties.SetAttribute( "objectPath", objectPath );
            }
        }
        else
        {
            // engine content or content of a plugin that is not in the pack
            report.AddWarning( "unreal", slotName, objectPath.IsNULLOrEmpty() ? CORE::CString( "The material slot refers to no material: a default material is used" ) : "The material " + objectPath + " is not in the pack: a default material is used" );
            material.name = slotName;
            if ( !objectPath.IsNULLOrEmpty() )
                material.source.properties.SetAttribute( "objectPath", objectPath );
        }
        scene.materials.push_back( material );
        slotMaterials[ s ] = (Int32) scene.materials.size() - 1;
    }
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
