/*
 *  assetpluginUNITY: the .meta file of an asset
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#include "assetpluginUNITY_CUnityYamlReader.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H ? */

#include "assetpluginUNITY_CUnityMeta.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnityMeta::CUnityMeta( void )
    : guid()
    , importerType()
    , importerVersion( 0 )
    , folderAsset( false )
    , mainObjectFileId( 0 )
    , idTable()
    , idTableKey()
    , globalScale( 1.0 )
    , hasUseFileScale( false )
    , useFileScale( true )
    , useFileUnits( true )
    , bakeAxisConversion( false )
    , importMaterials( true )
    , materialName( 0 )
    , materialSearch( 1 )
    , materialImportMode( -1 )
    , addColliders( false )
    , importAnimation( true )
    , animationType( 0 )
    , clips()
    , externalObjects()
    , textureType( 0 )
    , sRGBTexture( -1 )
    , linearTexture( -1 )
    , wrapMode( -1 )
    , wrapU( -1 )
    , wrapV( -1 )
    , filterMode( -1 )
    , maxTextureSize( 0 )
    , root( GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnityMeta::Load( const CORE::CString& vfsPath, CORE::CString& problem )
{GUCEF_TRACE;

    CUnityYamlFile file;
    if ( !CUnityYamlReader::Load( vfsPath, file, problem ) && 0 == file.GetDocumentCount() )
        return false;
    if ( 0 == file.GetDocumentCount() )
    {
        problem = "the .meta file is empty";
        return false;
    }
    return Read( file.GetDocument( 0 ).body );
}

/*-------------------------------------------------------------------------*/

bool
CUnityMeta::Parse( const CORE::CString& text, CORE::CString& problem )
{GUCEF_TRACE;

    CUnityYamlFile file;
    if ( !CUnityYamlReader::Parse( text, file, problem ) && 0 == file.GetDocumentCount() )
        return false;
    if ( 0 == file.GetDocumentCount() )
    {
        problem = "the .meta file is empty";
        return false;
    }
    return Read( file.GetDocument( 0 ).body );
}

/*-------------------------------------------------------------------------*/

bool
CUnityMeta::Read( const CORE::CDataNode& mapping )
{GUCEF_TRACE;

    *this = CUnityMeta();
    root = mapping;
    guid = CUnityYamlReader::GetString( mapping, "guid" ).Lowercase();
    const CORE::CString folder = CUnityYamlReader::GetString( mapping, "folderAsset" ).Lowercase();
    folderAsset = "yes" == folder || "true" == folder || "1" == folder;

    // the importer: the one mapping member whose name ends in "Importer"
    const CORE::CDataNode* importer = GUCEF_NULL;
    CORE::CDataNode::const_iterator c = mapping.ConstBegin();
    while ( c != mapping.ConstEnd() )
    {
        const CORE::CString& name = (*c)->GetName();
        if ( name.Length() > 8 && name.EndsWith( "Importer" ) )
        {
            importer = *c;
            importerType = name;
            break;
        }
        ++c;
    }
    if ( GUCEF_NULL == importer )
    {
        // an importer without settings ("DefaultImporter:" with nothing under it)
        CORE::CDataNode::TAttributeMap::const_iterator a = mapping.AttributeBegin();
        while ( a != mapping.AttributeEnd() )
        {
            if ( (*a).first.Length() > 8 && (*a).first.EndsWith( "Importer" ) )
                importerType = (*a).first;
            ++a;
        }
        return !guid.IsNULLOrEmpty();
    }

    importerVersion = CUnityYamlReader::GetInt32( *importer, "serializedVersion", 0 );
    mainObjectFileId = CUnityYamlReader::GetInt64( *importer, "mainObjectFileID", 0 );

    // the ID table, in either of its forms
    const CORE::CDataNode* recycle = importer->FindChild( "fileIDToRecycleName" );
    if ( GUCEF_NULL != recycle && GUCEF_DATATYPE_OBJECT == recycle->GetNodeType() )
    {
        idTableKey = "fileIDToRecycleName";
        CORE::CDataNode::TAttributeMap::const_iterator a = recycle->AttributeBegin();
        while ( a != recycle->AttributeEnd() )
        {
            Int64 id = 0;
            if ( CUnityYamlReader::ParseInt64( (*a).first, id ) )
            {
                SIdEntry entry;
                entry.classId = (Int32) ( id / 100000 );
                entry.name = (*a).second.AsString();
                idTable[ id ] = entry;
            }
            ++a;
        }
    }
    const CORE::CDataNode* internalTable = importer->FindChild( "internalIDToNameTable" );
    if ( GUCEF_NULL != internalTable )
    {
        if ( idTableKey.IsNULLOrEmpty() )
            idTableKey = "internalIDToNameTable";
        CUnityYamlReader::TNodeList items;
        CUnityYamlReader::GetItems( internalTable, items );
        for ( size_t i=0; i<items.size(); ++i )
        {
            const CORE::CDataNode* first = items[ i ]->FindChild( "first" );
            if ( GUCEF_NULL == first || 0 == first->GetAttCount() )
                continue;
            const CORE::CDataNode::TKeyValuePair* pair = first->GetAttribute( (UInt32) 0 );
            Int64 classId = 0;
            Int64 id = 0;
            if ( !CUnityYamlReader::ParseInt64( pair->first, classId ) || !CUnityYamlReader::ParseInt64( pair->second.AsString(), id ) )
                continue;
            SIdEntry entry;
            entry.classId = (Int32) classId;
            entry.name = CUnityYamlReader::GetString( *items[ i ], "second" );
            if ( idTable.find( id ) == idTable.end() )
                idTable[ id ] = entry;
        }
    }

    // ModelImporter
    const CORE::CDataNode* meshes = importer->FindChild( "meshes" );
    if ( GUCEF_NULL != meshes )
    {
        Float64 scale = 1.0;
        if ( CUnityYamlReader::ParseFloat( CUnityYamlReader::GetString( *meshes, "globalScale", "1" ), scale ) )
            globalScale = scale;
        hasUseFileScale = CUnityYamlReader::HasScalar( *meshes, "useFileScale" );
        useFileScale = CUnityYamlReader::GetBool( *meshes, "useFileScale", true );
        useFileUnits = CUnityYamlReader::GetBool( *meshes, "useFileUnits", true );
        addColliders = CUnityYamlReader::GetBool( *meshes, "addColliders", false );
        bakeAxisConversion = CUnityYamlReader::GetBool( *meshes, "bakeAxisConversion", false );
    }
    // a few files of other versions have the scale keys at the top level of the importer
    if ( GUCEF_NULL == meshes && CUnityYamlReader::HasScalar( *importer, "globalScale" ) )
    {
        Float64 scale = 1.0;
        if ( CUnityYamlReader::ParseFloat( CUnityYamlReader::GetString( *importer, "globalScale", "1" ), scale ) )
            globalScale = scale;
    }
    if ( CUnityYamlReader::HasScalar( *importer, "bakeAxisConversion" ) )
        bakeAxisConversion = CUnityYamlReader::GetBool( *importer, "bakeAxisConversion", false );
    const CORE::CDataNode* materials = importer->FindChild( "materials" );
    if ( GUCEF_NULL != materials )
    {
        importMaterials = CUnityYamlReader::GetBool( *materials, "importMaterials", true );
        materialName = CUnityYamlReader::GetInt32( *materials, "materialName", 0 );
        materialSearch = CUnityYamlReader::GetInt32( *materials, "materialSearch", 1 );
        materialImportMode = CUnityYamlReader::GetInt32( *materials, "materialImportMode", -1 );
    }
    if ( CUnityYamlReader::HasScalar( *importer, "materialImportMode" ) )
        materialImportMode = CUnityYamlReader::GetInt32( *importer, "materialImportMode", -1 );
    if ( materialImportMode == 0 )
        importMaterials = false;
    importAnimation = CUnityYamlReader::GetBool( *importer, "importAnimation", true );
    animationType = CUnityYamlReader::GetInt32( *importer, "animationType", 0 );
    const CORE::CDataNode* animations = importer->FindChild( "animations" );
    if ( GUCEF_NULL != animations )
    {
        CUnityYamlReader::TNodeList items;
        CUnityYamlReader::GetItems( animations->FindChild( "clipAnimations" ), items );
        for ( size_t i=0; i<items.size(); ++i )
        {
            CUnityClipAnimation clip;
            if ( clip.Read( *items[ i ] ) )
                clips.push_back( clip );
        }
    }

    // externalObjects: items { first: { type, assembly, name }, second: <reference> }
    CUnityYamlReader::TNodeList externals;
    CUnityYamlReader::GetItems( importer->FindChild( "externalObjects" ), externals );
    for ( size_t i=0; i<externals.size(); ++i )
    {
        const CORE::CDataNode* first = externals[ i ]->FindChild( "first" );
        CUnityReference reference;
        if ( GUCEF_NULL == first || !reference.Read( externals[ i ]->FindChild( "second" ) ) )
            continue;
        const CORE::CString type = CUnityYamlReader::GetString( *first, "type" );
        if ( type.HasSubstr( "Material" ) < 0 )
            continue;
        externalObjects[ CUnityYamlReader::GetString( *first, "name" ) ] = reference;
    }

    // TextureImporter
    textureType = CUnityYamlReader::GetInt32( *importer, "textureType", 0 );
    maxTextureSize = CUnityYamlReader::GetInt32( *importer, "maxTextureSize", 0 );
    const CORE::CDataNode* mipmaps = importer->FindChild( "mipmaps" );
    if ( GUCEF_NULL != mipmaps )
    {
        sRGBTexture = CUnityYamlReader::GetInt32( *mipmaps, "sRGBTexture", -1 );
        linearTexture = CUnityYamlReader::GetInt32( *mipmaps, "linearTexture", -1 );
    }
    const CORE::CDataNode* textureSettings = importer->FindChild( "textureSettings" );
    if ( GUCEF_NULL != textureSettings )
    {
        wrapMode = CUnityYamlReader::GetInt32( *textureSettings, "wrapMode", -1 );
        wrapU = CUnityYamlReader::GetInt32( *textureSettings, "wrapU", -1 );
        wrapV = CUnityYamlReader::GetInt32( *textureSettings, "wrapV", -1 );
        filterMode = CUnityYamlReader::GetInt32( *textureSettings, "filterMode", -1 );
    }
    return !guid.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CUnityMeta::IsModel( void ) const
{GUCEF_TRACE;

    return "ModelImporter" == importerType || "FBXImporter" == importerType;
}

/*-------------------------------------------------------------------------*/

bool
CUnityMeta::IsTexture( void ) const
{GUCEF_TRACE;

    return "TextureImporter" == importerType || "IHVImageFormatImporter" == importerType;
}

/*-------------------------------------------------------------------------*/

bool
CUnityMeta::TryGetIdEntry( Int64 fileId, SIdEntry& entry ) const
{GUCEF_TRACE;

    TIdTable::const_iterator i = idTable.find( fileId );
    if ( i == idTable.end() )
        return false;
    entry = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnityMeta::GetNameRank( Int64 fileId ) const
{GUCEF_TRACE;

    TIdTable::const_iterator self = idTable.find( fileId );
    if ( self == idTable.end() )
        return 0;
    UInt32 rank = 0;
    TIdTable::const_iterator i = idTable.begin();
    while ( i != self )
    {
        if ( (*i).second.classId == (*self).second.classId && (*i).second.name == (*self).second.name )
            ++rank;
        ++i;
    }
    return rank;
}

/*-------------------------------------------------------------------------*/

void
CUnityMeta::GetIdsOfClass( Int32 classId, GUCEF::vector< Int64 >& ids ) const
{GUCEF_TRACE;

    ids.clear();
    TIdTable::const_iterator i = idTable.begin();
    while ( i != idTable.end() )
    {
        if ( (*i).second.classId == classId )
            ids.push_back( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CUnityMeta::AppliesFileScale( void ) const
{GUCEF_TRACE;

    // files before useFileScale (serializedVersion 18) take the units of the file as Unity units: useFileUnits
    // concerns other model formats; checked against the child positions of the prefabs of real packs
    return hasUseFileScale && useFileScale;
}

/*-------------------------------------------------------------------------*/

Float64
CUnityMeta::GetEffectiveScale( Float64 fileUnitMetres ) const
{GUCEF_TRACE;

    const Float64 unit = fileUnitMetres > 0.0 ? fileUnitMetres : 0.01;
    return globalScale * ( AppliesFileScale() ? unit : 1.0 );
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
