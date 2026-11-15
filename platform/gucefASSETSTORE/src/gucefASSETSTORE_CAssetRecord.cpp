/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#include "gucefASSETSTORE_CAssetRecord.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetRecord::CAssetRecord( void )
    : CORE::CIDataNodeSerializable()
    , assetId()
    , packId()
    , kind()
    , name()
    , originalPath()
    , convertedPath()
    , sourceFormat()
    , convertedFormat()
    , sizeBytes( 0 )
    , digest()
    , categoryId()
    , files()
    , keywords()
    , model()
    , texture()
    , material()
    , animation()
    , audio()
    , analysed( false )
    , analyzers()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetRecord::~CAssetRecord()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetRecord::Serialize( CORE::CDataNode& domRootNode                        ,
                         const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "assetId", assetId );
    success = domRootNode.SetAttribute( "packId", packId ) && success;
    success = domRootNode.SetAttribute( "kind", kind ) && success;
    success = domRootNode.SetAttribute( "name", name ) && success;
    success = domRootNode.SetAttribute( "originalPath", originalPath ) && success;
    success = domRootNode.SetAttribute( "convertedPath", convertedPath ) && success;
    success = domRootNode.SetAttribute( "sourceFormat", sourceFormat ) && success;
    success = domRootNode.SetAttribute( "convertedFormat", convertedFormat ) && success;
    success = domRootNode.SetAttribute( "sizeBytes", sizeBytes ) && success;
    success = domRootNode.SetAttribute( "digest", digest ) && success;
    success = domRootNode.SetAttribute( "categoryId", categoryId ) && success;
    success = domRootNode.SetAttribute( "analysed", analysed ) && success;
    success = domRootNode.SetValuesOfChildByName( "analyzers", analyzers ) && success;

    CORE::CDataNode* filesNode = domRootNode.AddChild( "files", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; GUCEF_NULL != filesNode && i<files.size(); ++i )
        success = files[ i ].Serialize( *filesNode->AddChild( "file", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    CORE::CDataNode* keywordsNode = domRootNode.AddChild( "keywords", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; GUCEF_NULL != keywordsNode && i<keywords.size(); ++i )
        success = keywords[ i ].Serialize( *keywordsNode->AddChild( "keyword", GUCEF_DATATYPE_OBJECT ), settings ) && success;

    if ( model.isSet )
        success = model.Serialize( *domRootNode.AddChild( "model", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    if ( texture.isSet )
        success = texture.Serialize( *domRootNode.AddChild( "texture", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    if ( material.isSet )
        success = material.Serialize( *domRootNode.AddChild( "material", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    if ( animation.isSet )
        success = animation.Serialize( *domRootNode.AddChild( "animation", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    if ( audio.isSet )
        success = audio.Serialize( *domRootNode.AddChild( "audio", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetRecord::Deserialize( const CORE::CDataNode& domRootNode                  ,
                           const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    *this = CAssetRecord();
    assetId = domRootNode.GetAttributeValueOrChildValueByName( "assetId" ).AsString();
    packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString();
    kind = domRootNode.GetAttributeValueOrChildValueByName( "kind" ).AsString();
    name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString();
    originalPath = domRootNode.GetAttributeValueOrChildValueByName( "originalPath" ).AsString();
    convertedPath = domRootNode.GetAttributeValueOrChildValueByName( "convertedPath" ).AsString();
    sourceFormat = domRootNode.GetAttributeValueOrChildValueByName( "sourceFormat" ).AsString();
    convertedFormat = domRootNode.GetAttributeValueOrChildValueByName( "convertedFormat" ).AsString();
    sizeBytes = domRootNode.GetAttributeValueOrChildValueByName( "sizeBytes" ).AsInt64( 0, true );
    digest = domRootNode.GetAttributeValueOrChildValueByName( "digest" ).AsString();
    categoryId = domRootNode.GetAttributeValueOrChildValueByName( "categoryId" ).AsString();
    analysed = domRootNode.GetAttributeValueOrChildValueByName( "analysed" ).AsBool( false, true );
    domRootNode.GetValuesOfChildByName( "analyzers", analyzers );

    const CORE::CDataNode* filesNode = domRootNode.FindChild( "files" );
    if ( GUCEF_NULL != filesNode )
    {
        CORE::CDataNode::const_iterator i = filesNode->ConstBegin();
        while ( i != filesNode->ConstEnd() )
        {
            CAssetFileRecord file;
            if ( file.Deserialize( *(*i), settings ) )
                files.push_back( file );
            ++i;
        }
    }
    const CORE::CDataNode* keywordsNode = domRootNode.FindChild( "keywords" );
    if ( GUCEF_NULL != keywordsNode )
    {
        CORE::CDataNode::const_iterator i = keywordsNode->ConstBegin();
        while ( i != keywordsNode->ConstEnd() )
        {
            CAssetKeyword keyword;
            if ( keyword.Deserialize( *(*i), settings ) )
                keywords.push_back( keyword );
            ++i;
        }
    }
    const CORE::CDataNode* node = domRootNode.FindChild( "model" );
    if ( GUCEF_NULL != node )
        model.Deserialize( *node, settings );
    node = domRootNode.FindChild( "texture" );
    if ( GUCEF_NULL != node )
        texture.Deserialize( *node, settings );
    node = domRootNode.FindChild( "material" );
    if ( GUCEF_NULL != node )
        material.Deserialize( *node, settings );
    node = domRootNode.FindChild( "animation" );
    if ( GUCEF_NULL != node )
        animation.Deserialize( *node, settings );
    node = domRootNode.FindChild( "audio" );
    if ( GUCEF_NULL != node )
        audio.Deserialize( *node, settings );
    return !assetId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetRecord::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetRecord( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetRecord::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetRecord::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetRecord";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetRecord::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetRecord";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
