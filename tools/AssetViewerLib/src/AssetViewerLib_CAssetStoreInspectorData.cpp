/*
 *  AssetViewerLib: the asset sources of the viewer: online stores, the primary store and the transfers
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */

#ifndef ASSETVIEWERLIB_CASSETSTOREINSPECTORDATA_H
#include "AssetViewerLib_CAssetStoreInspectorData.h"
#define ASSETVIEWERLIB_CASSETSTOREINSPECTORDATA_H
#endif /* ASSETVIEWERLIB_CASSETSTOREINSPECTORDATA_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


namespace {

CORE::CString
Join( const CORE::CString::StringSet& values )
{
    CORE::CString text;
    for ( CORE::CString::StringSet::const_iterator i = values.begin(); i != values.end(); ++i )
        text += ( text.IsNULLOrEmpty() ? "" : ", " ) + *i;
    return text;
}

CORE::CString
Join( const CORE::CString::StringVector& values )
{
    return CORE::CString().Combine( values, ',' ).ReplaceSubstr( ",", ", " );
}

void
AddNumber( CORE::CDataNode& node, const char* name, Int64 value )
{
    if ( value >= 0 )
        node.SetAttribute( name, CORE::Int64ToString( value ) );
}

}

/*-------------------------------------------------------------------------*/

void
CAssetStoreInspectorData::DescribeRestriction( const CORE::CString& flag, CORE::CString& subject, CORE::CString& statement )
{GUCEF_TRACE;

    if ( ASSETSTORE::CAssetLicence::NoMachineLearning == flag )
    {
        subject = "Machine learning";
        statement = "not allowed";
    }
    else
    if ( ASSETSTORE::CAssetLicence::AttributionRequired == flag )
    {
        subject = "Attribution";
        statement = "required";
    }
    else
    if ( ASSETSTORE::CAssetLicence::NoCommercialUse == flag )
    {
        subject = "Commercial use";
        statement = "not allowed";
    }
    else
    if ( ASSETSTORE::CAssetLicence::NoRedistribution == flag )
    {
        subject = "Redistribution of the files";
        statement = "not allowed";
    }
    else
    if ( ASSETSTORE::CAssetLicence::EditorialOnly == flag )
    {
        subject = "Editorial use only";
        statement = "yes";
    }
    else
    {
        subject = flag;
        statement = "restricted";
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreInspectorData::DescribeRestrictions( const CORE::CString::StringSet& flags )
{GUCEF_TRACE;

    if ( flags.empty() )
        return "none";
    CORE::CString text;
    for ( CORE::CString::StringSet::const_iterator i = flags.begin(); i != flags.end(); ++i )
    {
        CORE::CString subject, statement;
        DescribeRestriction( *i, subject, statement );
        text += ( text.IsNULLOrEmpty() ? "" : "; " ) + subject + " " + statement;
    }
    return text;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreInspectorData::FillLicence( const ASSETSTORE::CAssetLicence& licence, CORE::CDataNode& node )
{GUCEF_TRACE;

    CORE::CDataNode* child = node.AddChild( "Licence" );
    child->SetAttribute( "Name", licence.name.IsNULLOrEmpty() ? CORE::CString( "unknown" ) : licence.name );
    child->SetAttribute( "Class", licence.licenceClass.IsNULLOrEmpty() ? CORE::CString( "unknown" ) : licence.licenceClass );
    if ( !licence.url.IsNULLOrEmpty() )
        child->SetAttribute( "Address", licence.url );
    // every restriction that the licence has, in words; a licence without a flag says what is allowed
    for ( CORE::CString::StringSet::const_iterator i = licence.restrictions.begin(); i != licence.restrictions.end(); ++i )
    {
        CORE::CString subject, statement;
        DescribeRestriction( *i, subject, statement );
        child->SetAttribute( subject, statement );
    }
    if ( !licence.HasRestriction( ASSETSTORE::CAssetLicence::NoMachineLearning ) )
        child->SetAttribute( "Machine learning", "allowed" );
    if ( !licence.HasRestriction( ASSETSTORE::CAssetLicence::NoCommercialUse ) )
        child->SetAttribute( "Commercial use", "allowed" );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreInspectorData::FillPack( const ASSETSTORE::CAssetPackRecord& pack, CORE::CDataNode& node )
{GUCEF_TRACE;

    node.SetName( "Pack" );
    node.SetAttribute( "Title", pack.title );
    node.SetAttribute( "Seller", pack.seller );
    node.SetAttribute( "Source", pack.sourceId );
    if ( !pack.sourceUrl.IsNULLOrEmpty() )
        node.SetAttribute( "Address", pack.sourceUrl );
    if ( !pack.sourceVersion.IsNULLOrEmpty() )
        node.SetAttribute( "Version", pack.sourceVersion );
    node.SetAttribute( "State", CORE::CString( ASSETSTORE::AssetPackStateToString( pack.state ) ) );
    if ( !pack.stateReason.IsNULLOrEmpty() )
        node.SetAttribute( "Reason", pack.stateReason );
    node.SetAttribute( "Keeps", CORE::CString( ASSETSTORE::AssetKeepPolicyToString( pack.keep ) ) );
    node.SetAttribute( "Location", pack.locationId );
    node.SetAttribute( "Files", CORE::Int64ToString( pack.fileCount ) );
    node.SetAttribute( "Assets", CORE::Int64ToString( pack.assetCount ) );
    node.SetAttribute( "Size", CORE::Int64ToString( pack.sizeBytes ) + " bytes" );
    if ( !pack.description.IsNULLOrEmpty() )
        node.SetAttribute( "Description", pack.description );
    CORE::CDataNode* classification = node.AddChild( "Classification" );
    classification->SetAttribute( "Categories", Join( pack.categories ) );
    classification->SetAttribute( "Kinds", Join( pack.kinds ) );
    classification->SetAttribute( "Themes", Join( pack.themes ) );
    classification->SetAttribute( "Art styles", Join( pack.artStyles ) );
    if ( !pack.unmappedCategories.empty() )
        classification->SetAttribute( "Not mapped", Join( pack.unmappedCategories ) );
    CORE::CDataNode* formats = node.AddChild( "Formats" );
    formats->SetAttribute( "Original", Join( pack.formatsOriginal ) );
    formats->SetAttribute( "Converted", pack.formatsConverted.empty() ? CORE::CString( "none" ) : Join( pack.formatsConverted ) );
    if ( pack.conversion.IsSet() )
    {
        CORE::CDataNode* conversion = node.AddChild( "Conversion" );
        conversion->SetAttribute( "Outcome", pack.conversion.outcome );
        conversion->SetAttribute( "Profile", pack.conversion.profile );
        if ( !pack.conversion.reason.IsNULLOrEmpty() )
            conversion->SetAttribute( "Reason", pack.conversion.reason );
        conversion->SetAttribute( "Converted", CORE::UInt32ToString( pack.conversion.convertedCount + pack.conversion.cachedCount ) );
        if ( pack.conversion.approximatedCount > 0 )
            conversion->SetAttribute( "Approximated", CORE::UInt32ToString( pack.conversion.approximatedCount ) );
        if ( pack.conversion.failedCount > 0 )
            conversion->SetAttribute( "Failed", CORE::UInt32ToString( pack.conversion.failedCount ) );
        if ( !pack.conversion.converters.empty() )
            conversion->SetAttribute( "Made with", Join( pack.conversion.converters ) );
        if ( !pack.conversion.reportPath.IsNULLOrEmpty() )
            conversion->SetAttribute( "Report", pack.conversion.reportPath );
    }
    FillLicence( pack.licence, node );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreInspectorData::FillAsset( const ASSETSTORE::CAssetRecord& asset, CORE::CDataNode& node )
{GUCEF_TRACE;

    node.SetName( "Asset" );
    node.SetAttribute( "Name", asset.name );
    node.SetAttribute( "Kind", asset.kind );
    if ( !asset.categoryId.IsNULLOrEmpty() )
        node.SetAttribute( "Category", asset.categoryId );
    node.SetAttribute( "Format", asset.sourceFormat );
    if ( !asset.convertedFormat.IsNULLOrEmpty() )
        node.SetAttribute( "Converted format", asset.convertedFormat );
    if ( !asset.convertedPath.IsNULLOrEmpty() )
        node.SetAttribute( "Converted path", asset.convertedPath );
    node.SetAttribute( "Size", CORE::Int64ToString( asset.sizeBytes ) + " bytes" );
    node.SetAttribute( "Path", asset.originalPath );
    if ( asset.model.isSet )
    {
        CORE::CDataNode* model = node.AddChild( "Model" );
        AddNumber( *model, "Triangles", asset.model.triangles );
        AddNumber( *model, "Vertices", asset.model.vertices );
        AddNumber( *model, "Sub meshes", asset.model.subMeshes );
        AddNumber( *model, "Bones", asset.model.bones );
        AddNumber( *model, "Animations", asset.model.animations );
        AddNumber( *model, "Levels of detail", asset.model.lods );
        model->SetAttribute( "Rigged", asset.model.rigged ? "yes" : "no" );
        model->SetAttribute( "Collision", asset.model.hasCollision ? "yes" : "no" );
    }
    if ( asset.texture.isSet )
    {
        CORE::CDataNode* texture = node.AddChild( "Texture" );
        texture->SetAttribute( "Size", CORE::Int64ToString( asset.texture.width ) + " x " + CORE::Int64ToString( asset.texture.height ) );
        if ( !asset.texture.role.IsNULLOrEmpty() )
            texture->SetAttribute( "Role", asset.texture.role );
        if ( !asset.texture.colourSpace.IsNULLOrEmpty() )
            texture->SetAttribute( "Colour space", asset.texture.colourSpace );
        if ( !asset.texture.compression.IsNULLOrEmpty() )
            texture->SetAttribute( "Compression", asset.texture.compression );
    }
    if ( !asset.files.empty() )
    {
        CORE::CDataNode* files = node.AddChild( "Files" );
        for ( size_t i=0; i<asset.files.size(); ++i )
            files->SetAttribute( asset.files[ i ].path, CORE::Int64ToString( asset.files[ i ].sizeBytes ) + " bytes" );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreInspectorData::FillStoreItem( const ASSETSTORE::CAssetStoreItem& item, const CORE::CString& formatId, CORE::CDataNode& node )
{GUCEF_TRACE;

    node.SetName( "Item" );
    node.SetAttribute( "Title", item.title );
    node.SetAttribute( "Seller", item.seller );
    node.SetAttribute( "Type", item.listingType );
    if ( !item.version.IsNULLOrEmpty() )
        node.SetAttribute( "Version", item.version );
    if ( !item.url.IsNULLOrEmpty() )
        node.SetAttribute( "Address", item.url );
    if ( !item.description.IsNULLOrEmpty() )
        node.SetAttribute( "Description", item.description );
    if ( !item.categories.empty() )
        node.SetAttribute( "Categories", Join( item.categories ) );
    if ( !item.tags.empty() )
        node.SetAttribute( "Tags", Join( item.tags ) );

    ASSETSTORE::CAssetLicence licence;
    licence.name = item.licenceName;
    licence.url = item.licenceUrl;
    licence.restrictions = item.licenceRestrictions;
    FillLicence( licence, node );

    CORE::CDataNode* formats = node.AddChild( "Formats" );
    for ( size_t f=0; f<item.formats.size(); ++f )
    {
        const ASSETSTORE::CAssetStoreFormat& format = item.formats[ f ];
        formats->SetAttribute( format.name.IsNULLOrEmpty() ? format.formatId : format.name,
                               ( format.group.IsNULLOrEmpty() ? CORE::CString::Empty : format.group + ", " ) + CORE::UInt32ToString( (UInt32) format.files.size() ) +
                               ( 1 == format.files.size() ? " file, " : " files, " ) + CORE::UInt64ToString( format.GetTotalSize() ) + " bytes" );
    }
    const ASSETSTORE::CAssetStoreFormat* chosen = formatId.IsNULLOrEmpty() ? GUCEF_NULL : item.FindFormat( formatId );
    if ( GUCEF_NULL != chosen )
    {
        CORE::CDataNode* files = node.AddChild( "Files" );
        files->SetAttribute( "Format", chosen->name.IsNULLOrEmpty() ? chosen->formatId : chosen->name );
        for ( size_t i=0; i<chosen->files.size(); ++i )
            files->SetAttribute( chosen->files[ i ].name, chosen->files[ i ].size > 0 ? CORE::UInt64ToString( chosen->files[ i ].size ) + " bytes" : CORE::CString( "size not known" ) );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
