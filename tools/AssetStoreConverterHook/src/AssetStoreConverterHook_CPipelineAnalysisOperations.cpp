/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#include "gucefASSETSTORE_CAssetRootedPath.h"
#define GUCEF_ASSETSTORE_CASSETROOTEDPATH_H
#endif /* GUCEF_ASSETSTORE_CASSETROOTEDPATH_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#include "gucefASSETSTORE_CAssetToolUris.h"
#define GUCEF_ASSETSTORE_CASSETTOOLURIS_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLURIS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#include "AssetConverterLib_CAssetConverterGlobal.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#include "AssetConverterLib_CAssetPluginRegistry.h"
#define ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPIPELINE_H
#include "AssetConverterLib_CAssetPipeline.h"
#define ASSETCONVERTERLIB_CASSETPIPELINE_H
#endif /* ASSETCONVERTERLIB_CASSETPIPELINE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H
#include "AssetConverterLib_CAssetPipelineOptions.h"
#define ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H
#endif /* ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKPLAN_H
#include "AssetConverterLib_CAssetPackPlan.h"
#define ASSETCONVERTERLIB_CASSETPACKPLAN_H
#endif /* ASSETCONVERTERLIB_CASSETPACKPLAN_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#include "AssetConverterLib_CAssetPackContext.h"
#define ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#endif /* ASSETCONVERTERLIB_CASSETPACKCONTEXT_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H
#include "AssetConverterLib_CConversionSidecar.h"
#define ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONSIDECAR_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEINSPECTIONOPERATIONS_H
#include "AssetStoreConverterHook_CPipelineInspectionOperations.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINEINSPECTIONOPERATIONS_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEINSPECTIONOPERATIONS_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINELICENCEOPERATIONS_H
#include "AssetStoreConverterHook_CPipelineLicenceOperations.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINELICENCEOPERATIONS_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINELICENCEOPERATIONS_H ? */

#include "AssetStoreConverterHook_CPipelineAnalysisOperations.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The summary of a route of compare_routes */
struct SRouteSummary
{
    CORE::CString label;
    bool found;
    bool hasBounds;
    Float64 min[ 3 ];
    Float64 max[ 3 ];
    UInt32 triangles;
    UInt32 vertices;
    UInt32 materials;
};

/** What is known of a property of the source materials of a shader */
struct SPropertyInfo
{
    CORE::CString type;
    UInt32 count;
    bool hasRange;
    Float64 min;
    Float64 max;
    CORE::CString::StringSet textureFiles;
    CORE::CString mappedTo;
};

typedef GUCEF::map< CORE::CString, SPropertyInfo > TPropertyMap;

/** What is known of the source materials of one shader */
struct SShaderInfo
{
    UInt32 materialCount;
    TPropertyMap properties;
};

typedef GUCEF::map< CORE::CString, SShaderInfo > TShaderMap;

/** A heuristic of material_map_suggest: a part of a property name and the slot it suggests for a kind of value */
struct SSlotPattern
{
    const char* type;       /**< "texture", "color", "float" */
    const char* pattern;
    bool exact;             /**< the whole name (without leading '_'), not a part of it */
    const char* slot;
};

const SSlotPattern SlotPatterns[] =
{
    { "texture", "maintex", true, "baseColorTexture" }, { "texture", "basemap", true, "baseColorTexture" },
    { "texture", "basecolormap", true, "baseColorTexture" }, { "texture", "bumpmap", true, "normalTexture" },
    { "texture", "albedo", false, "baseColorTexture" }, { "texture", "basecolor", false, "baseColorTexture" },
    { "texture", "diffuse", false, "baseColorTexture" }, { "texture", "maintex", false, "baseColorTexture" },
    { "texture", "texture", false, "baseColorTexture" }, { "texture", "normal", false, "normalTexture" },
    { "texture", "bump", false, "normalTexture" }, { "texture", "emiss", false, "emissiveTexture" },
    { "texture", "metal", false, "metallicRoughnessTexture" }, { "texture", "occlusion", false, "occlusionTexture" },
    { "color", "color", true, "baseColorFactor" }, { "color", "basecolor", true, "baseColorFactor" },
    { "color", "emiss", false, "emissiveFactor" }, { "color", "albedo", false, "baseColorFactor" },
    { "color", "maincolor", false, "baseColorFactor" }, { "color", "tint", false, "baseColorFactor" },
    { "color", "diffuse", false, "baseColorFactor" },
    { "float", "bumpscale", false, "normalScale" }, { "float", "normalscale", false, "normalScale" },
    { "float", "normalstrength", false, "normalScale" }, { "float", "metal", false, "metallicFactor" },
    { "float", "smooth", false, "smoothnessFactor" }, { "float", "gloss", false, "smoothnessFactor" },
    { "float", "rough", false, "roughnessFactor" }, { "float", "cutoff", false, "alphaCutoff" },
    { "float", "opacity", false, "opacity" }
};

/** The kind of value of a source material property */
CORE::CString
TypeOfProperty( const CORE::CDataNode* child, const CORE::CVariant* value, bool hasTexture )
{GUCEF_TRACE;

    if ( hasTexture )
        return "texture";
    if ( GUCEF_NULL != value )
    {
        if ( value->IsNumber() )
            return "float";
        if ( value->IsBoolean() )
            return "boolean";
        return "string";
    }
    if ( GUCEF_NULL != child )
    {
        if ( GUCEF_DATATYPE_ARRAY == child->GetNodeType() )
            return 4 == child->GetNrOfDirectChildNodes() || 3 == child->GetNrOfDirectChildNodes() ? "color" : "array";
        if ( 0 != child->GetAttCount() && GUCEF_NULL != child->GetAttribute( "r" ) )
            return "color";
        return "object";
    }
    return "unknown";
}

void
NoteNumber( SPropertyInfo& info, Float64 number )
{GUCEF_TRACE;

    if ( !info.hasRange )
    {
        info.min = info.max = number;
        info.hasRange = true;
        return;
    }
    if ( number < info.min )
        info.min = number;
    if ( number > info.max )
        info.max = number;
}

/** The slot a heuristic suggests for a property, with a confidence; false when none */
bool
SuggestSlot( const CORE::CString& property, const CORE::CString& type, CORE::CString& slot, Float64& confidence )
{GUCEF_TRACE;

    CORE::CString name = property.Lowercase();
    while ( name.Length() > 0 && '_' == name[ 0 ] )
        name = name.CutChars( 1, true );
    for ( size_t p=0; p<sizeof( SlotPatterns ) / sizeof( SlotPatterns[ 0 ] ); ++p )
    {
        if ( type != SlotPatterns[ p ].type )
            continue;
        const CORE::CString pattern = SlotPatterns[ p ].pattern;
        if ( SlotPatterns[ p ].exact ? name == pattern : name.HasSubstr( pattern, true ) >= 0 )
        {
            slot = SlotPatterns[ p ].slot;
            confidence = SlotPatterns[ p ].exact ? 0.9 : 0.6;
            return true;
        }
    }
    return false;
}

Float64
NumberAt( const CORE::CDataNode* node, UInt32 index )
{GUCEF_TRACE;

    if ( GUCEF_NULL == node )
        return 0.0;
    const CORE::CDataNode* child = node->GetChildAtIndex( index );
    return GUCEF_NULL == child ? 0.0 : child->GetValue().AsFloat64( 0.0, true );
}

/** Whether two bounds are within the tolerance of each other, for the size of the larger */
bool
Near( Float64 a, Float64 b, Float64 scale, Float64 tolerance )
{GUCEF_TRACE;

    return fabs( a - b ) <= tolerance * ( scale > 0.0 ? scale : 1.0 );
}

CORE::CString
OrientationOf( const SRouteSummary& a, const SRouteSummary& b, Float64 tolerance )
{GUCEF_TRACE;

    if ( !a.hasBounds || !b.hasBounds )
        return "unknown";
    Float64 scale = 0.0;
    for ( UInt32 i=0; i<3; ++i )
    {
        const Float64 size = a.max[ i ] - a.min[ i ];
        if ( size > scale )
            scale = size;
    }
    bool same = true;
    bool mirroredX = true;
    bool mirroredZ = true;
    bool halfTurnY = true;
    for ( UInt32 i=0; i<3; ++i )
    {
        const bool kept = Near( a.min[ i ], b.min[ i ], scale, tolerance ) && Near( a.max[ i ], b.max[ i ], scale, tolerance );
        const bool flipped = Near( a.min[ i ], -b.max[ i ], scale, tolerance ) && Near( a.max[ i ], -b.min[ i ], scale, tolerance );
        same = same && kept;
        mirroredX = mirroredX && ( 0 == i ? flipped : kept );
        mirroredZ = mirroredZ && ( 2 == i ? flipped : kept );
        halfTurnY = halfTurnY && ( 1 == i ? kept : flipped );
    }
    if ( same )
        return "same";
    if ( halfTurnY )
        return "half-turn-y";
    if ( mirroredX )
        return "mirror-x";
    if ( mirroredZ )
        return "mirror-z";
    return "unknown";
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineAnalysisOperations::ConvertPlan( CPipelineOperationCall& call, CPipelinePlanStore& plans )
{GUCEF_TRACE;

    CConverterPack pack;
    if ( !call.FindRequestPack( pack, true ) )
        return call.result.status;
    UInt32 first = 0;
    UInt32 count = 0;
    if ( !call.GetPage( call.settings.planSampleEntries, first, count ) )
        return call.result.status;

    ASSETCONVERTER::CAssetPipelineOptions options;
    options.assetGlobs = call.request.assets;
    options.cookTargets = call.request.cookTargets;
    options.reimport = call.request.reimport;
    options.force = call.request.force;
    ASSETCONVERTER::CAssetPipeline pipeline;
    ASSETCONVERTER::CAssetPackPlan plan;
    if ( !pipeline.PlanPack( pack.manifest, options, plan ) )
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "the source of the pack " + pack.packId + " cannot be opened" );

    CORE::CDataNode& document = call.Document();
    document.SetAttribute( "packId", pack.packId );
    CORE::CDataNode* licence = document.AddChild( "licence", GUCEF_DATATYPE_OBJECT );
    licence->SetAttribute( "conversionAllowed", plan.conversionAllowed );
    CPipelineOperationCall::SetTextOrNull( *licence, "reason", plan.conversionAllowed ? CORE::CString::Empty : pack.licence.reason );
    licence->SetValuesOfChildByName( "restrictions", plan.restrictions );
    CORE::CDataNode* counts = document.AddChild( "counts", GUCEF_DATATYPE_OBJECT );
    counts->SetAttribute( "import", plan.GetCount( ASSETCONVERTER::PLANACTION_IMPORT ) );
    counts->SetAttribute( "cached", plan.GetCount( ASSETCONVERTER::PLANACTION_CACHED ) );
    counts->SetAttribute( "cook", plan.GetCount( ASSETCONVERTER::PLANACTION_COOK ) );
    counts->SetAttribute( "refused", plan.GetCount( ASSETCONVERTER::PLANACTION_REFUSED ) );
    counts->SetAttribute( "failed", plan.GetCount( ASSETCONVERTER::PLANACTION_FAILED ) );
    counts->SetAttribute( "validate", plan.GetCount( ASSETCONVERTER::PLANACTION_VALIDATE ) );
    counts->SetAttribute( "noIntermediate", plan.GetCount( ASSETCONVERTER::PLANACTION_NO_INTERMEDIATE ) );
    counts->SetAttribute( "filteredOut", plan.filteredOut );
    CPipelineOperationCall::SetTextOrNull( document, "lockHolder", plan.lockHolder );
    document.SetValuesOfChildByName( "problems", plan.problems );
    document.SetValuesOfChildByName( "cookTargets", plan.cookTargets );
    CORE::CDataNode* items = document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
    UInt32 shown = 0;
    for ( UInt32 i=first; i<(UInt32) plan.entries.size() && shown<count; ++i, ++shown )
    {
        const ASSETCONVERTER::CAssetPlanEntry& entry = plan.entries[ i ];
        CORE::CDataNode* item = items->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( "assetPath", entry.assetPath );
        item->SetAttribute( "importer", entry.importer );
        item->SetAttribute( "action", ASSETCONVERTER::ToString( entry.action ) );
        item->SetAttribute( "reason", ASSETCONVERTER::ToString( entry.reason ) );
        item->SetAttribute( "detail", entry.detail );
    }
    call.FinishPage( first, shown, (UInt32) plan.entries.size() );

    // the run time of the last run is not recorded by the pipeline; the time its assets took is
    ASSETCONVERTER::CConversionReport report;
    if ( pack.LoadReport( report ) )
    {
        UInt64 assetTime = 0;
        for ( size_t e=0; e<report.GetEntries().size(); ++e )
            assetTime += report.GetEntries()[ e ].durationInMs;
        document.SetAttribute( "lastRunAssetTimeMs", assetTime );
    }
    else
        document.SetAttribute( "lastRunAssetTimeMs", CORE::CVariant() );
    document.SetAttribute( "lastRunDurationMs", CORE::CVariant() );
    CPipelineInspectionOperations::AddLink( document, ASSETSTORE::CAssetToolUris::ForPack( pack.packId, "plan" ), "plan", "application/json" );

    CORE::CDataNode planNode( "plan", GUCEF_DATATYPE_OBJECT );
    plan.Serialize( planNode );
    plans.Store( pack.packId, planNode );
    return call.Succeed();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineAnalysisOperations::CompareRoutes( CPipelineOperationCall& call )
{GUCEF_TRACE;

    const CORE::CDataNode* routes = call.GetNodeArgument( "routes" );
    if ( GUCEF_NULL == routes || routes->GetNrOfDirectChildNodes() < 2 || routes->GetNrOfDirectChildNodes() > 4 )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "routes is a list of 2 to 4 routes" );
    Float64 tolerance = call.request.arguments.GetAttributeValueOrChildValueByName( "tolerance" ).AsFloat64( 0.01, true );
    if ( tolerance < 0.0 || tolerance > 0.5 )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "tolerance lies between 0 and 0.5" );

    CORE::CDataNode& document = call.Document();
    CORE::CDataNode* list = document.AddChild( "routes", GUCEF_DATATYPE_ARRAY );
    GUCEF::vector< SRouteSummary > summaries;
    UInt32 index = 0;
    CORE::CDataNode::const_iterator r = routes->ConstBegin();
    for ( ; r != routes->ConstEnd(); ++r, ++index )
    {
        const CORE::CDataNode& route = *(*r);
        SRouteSummary summary;
        summary.found = false;
        summary.hasBounds = false;
        summary.triangles = summary.vertices = summary.materials = 0;
        for ( UInt32 i=0; i<3; ++i )
            summary.min[ i ] = summary.max[ i ] = 0.0;
        summary.label = route.GetAttributeValueOrChildValueByName( "label" ).AsString( CORE::CString::Empty, true );
        if ( summary.label.IsNULLOrEmpty() )
            summary.label = "route" + CORE::ToString( index + 1 );
        const CORE::CString packId = route.GetAttributeValueOrChildValueByName( "packId" ).AsString( CORE::CString::Empty, true );
        const CORE::CString assetPath = route.GetAttributeValueOrChildValueByName( "assetPath" ).AsString( CORE::CString::Empty, true );
        CORE::CString stage = route.GetAttributeValueOrChildValueByName( "stage" ).AsString( "intermediate", true );
        if ( stage.IsNULLOrEmpty() )
            stage = "intermediate";

        CORE::CDataNode* item = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( "label", summary.label );
        item->SetAttribute( "packId", packId );
        item->SetAttribute( "assetPath", assetPath );
        item->SetAttribute( "stage", stage );

        CORE::CString reason;
        CConverterPack pack;
        CORE::CString problem;
        CORE::CDataNode sidecar;
        TConverterPackVector packs;
        call.GetPacks( packs );
        const CConverterPack* found = GUCEF_NULL;
        for ( size_t p=0; p<packs.size(); ++p )
            if ( packs[ p ].packId == packId && !packs[ p ].ambiguous )
                found = &packs[ p ];
        if ( !ASSETSTORE::CAssetRootedPath::IsSafeRelativePath( assetPath, false, true, problem ) )
            reason = "the asset path is refused: " + problem;
        else
        if ( GUCEF_NULL == found )
            reason = "there is no (unambiguous) pack " + packId;
        else
        if ( !found->runnable )
            reason = "the pack cannot be read through the tools: " + found->runnableReason;
        else
        if ( "intermediate" != stage )
            reason = "the " + stage + " stage keeps no summary of its own: compare the intermediate stage";
        else
        {
            const CORE::CString path = ASSETCONVERTER::CConversionSidecar::GetPath( found->manifest.GetIntermediatePath(), assetPath );
            if ( !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( path ) || !ASSETCONVERTER::CCanonicalJson::Load( path, sidecar ) )
                reason = "the asset has no intermediate in the pack (it was not converted yet)";
        }
        item->SetAttribute( "found", reason.IsNULLOrEmpty() );
        if ( !reason.IsNULLOrEmpty() )
        {
            item->SetAttribute( "reason", reason );
            summaries.push_back( summary );
            continue;
        }
        summary.found = true;
        const CORE::CDataNode* sum = sidecar.FindChild( "summary" );
        CORE::CDataNode* counts = item->AddChild( "counts", GUCEF_DATATYPE_OBJECT );
        const char* names[][ 2 ] = { { "nodes", "nodes" }, { "meshes", "meshes" }, { "submeshes", "subMeshes" }, { "vertices", "vertices" },
                                     { "triangles", "triangles" }, { "materials", "materials" }, { "textures", "textures" }, { "bones", "bones" },
                                     { "colliders", "colliders" }, { "lodLevels", "lodLevels" } };
        for ( UInt32 n=0; n<10; ++n )
            counts->SetAttribute( names[ n ][ 0 ], GUCEF_NULL == sum ? (UInt32) 0 : sum->GetAttributeValueOrChildValueByName( names[ n ][ 1 ] ).AsUInt32( 0, true ) );
        CORE::CDataNode* clips = item->AddChild( "clips", GUCEF_DATATYPE_ARRAY );
        const CORE::CDataNode* clipList = GUCEF_NULL == sum ? GUCEF_NULL : sum->FindChild( "clips" );
        UInt32 clipCount = 0;
        if ( GUCEF_NULL != clipList )
        {
            CORE::CDataNode::const_iterator c = clipList->ConstBegin();
            for ( ; c != clipList->ConstEnd(); ++c, ++clipCount )
            {
                CORE::CDataNode* clip = clips->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                clip->SetAttribute( "name", (*c)->GetAttributeValueOrChildValueByName( "name" ).AsString( CORE::CString::Empty, true ) );
                clip->SetAttribute( "duration", (*c)->GetAttributeValueOrChildValueByName( "duration" ) );
            }
        }
        counts->SetAttribute( "clips", clipCount );
        if ( GUCEF_NULL != sum )
        {
            summary.triangles = sum->GetAttributeValueOrChildValueByName( "triangles" ).AsUInt32( 0, true );
            summary.vertices = sum->GetAttributeValueOrChildValueByName( "vertices" ).AsUInt32( 0, true );
            summary.materials = sum->GetAttributeValueOrChildValueByName( "materials" ).AsUInt32( 0, true );
        }
        const CORE::CDataNode* bounds = GUCEF_NULL == sum ? GUCEF_NULL : sum->FindChild( "bounds" );
        if ( GUCEF_NULL != bounds && GUCEF_NULL != bounds->FindChild( "min" ) && GUCEF_NULL != bounds->FindChild( "max" ) )
        {
            summary.hasBounds = true;
            CORE::CDataNode* out = item->AddChild( "bounds", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode::TVariantVector minimum, maximum, size;
            for ( UInt32 i=0; i<3; ++i )
            {
                summary.min[ i ] = NumberAt( bounds->FindChild( "min" ), i );
                summary.max[ i ] = NumberAt( bounds->FindChild( "max" ), i );
                minimum.push_back( CORE::CVariant( summary.min[ i ] ) );
                maximum.push_back( CORE::CVariant( summary.max[ i ] ) );
                size.push_back( CORE::CVariant( summary.max[ i ] - summary.min[ i ] ) );
            }
            out->SetValuesOfChildByName( "min", minimum );
            out->SetValuesOfChildByName( "max", maximum );
            out->SetValuesOfChildByName( "size", size );
        }
        else
            item->SetAttribute( "bounds", CORE::CVariant() );
        // the axes are not recorded as such: the coordinate systems of the normalisation are
        item->SetAttribute( "upAxis", CORE::CVariant() );
        item->SetAttribute( "forwardAxis", CORE::CVariant() );
        const CORE::CDataNode* normalisation = sidecar.FindChild( "normalisation" );
        if ( GUCEF_NULL != normalisation )
        {
            CORE::CDataNode* system = item->AddChild( "coordinateSystem", GUCEF_DATATYPE_OBJECT );
            system->SetAttribute( "from", normalisation->GetAttributeValueOrChildValueByName( "from" ).AsString( CORE::CString::Empty, true ) );
            system->SetAttribute( "to", normalisation->GetAttributeValueOrChildValueByName( "to" ).AsString( CORE::CString::Empty, true ) );
            system->SetAttribute( "mirrored", normalisation->GetAttributeValueOrChildValueByName( "mirrored" ).AsBool( false, true ) );
        }
        const CORE::CDataNode* provenance = sidecar.FindChild( "provenance" );
        item->SetAttribute( "importer", GUCEF_NULL == provenance ? CORE::CString::Empty : provenance->GetAttributeValueOrChildValueByName( "importer" ).AsString( CORE::CString::Empty, true ) );
        item->SetAttribute( "importerVersion", GUCEF_NULL == provenance ? (UInt32) 0 : provenance->GetAttributeValueOrChildValueByName( "importerVersion" ).AsUInt32( 0, true ) );
        summaries.push_back( summary );
    }

    CORE::CDataNode* pairs = document.AddChild( "pairs", GUCEF_DATATYPE_ARRAY );
    for ( size_t a=0; a<summaries.size(); ++a )
    {
        for ( size_t b=a+1; b<summaries.size(); ++b )
        {
            const SRouteSummary& first = summaries[ a ];
            const SRouteSummary& second = summaries[ b ];
            if ( !first.found || !second.found )
                continue;
            CORE::CDataNode* pair = pairs->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            pair->SetAttribute( "a", first.label );
            pair->SetAttribute( "b", second.label );
            CORE::CDataNode* ratio = pair->AddChild( "sizeRatio", GUCEF_DATATYPE_ARRAY );
            bool within = first.hasBounds && second.hasBounds;
            for ( UInt32 i=0; i<3; ++i )
            {
                const Float64 sizeA = first.max[ i ] - first.min[ i ];
                const Float64 sizeB = second.max[ i ] - second.min[ i ];
                if ( !first.hasBounds || !second.hasBounds || sizeA <= 0.0 )
                {
                    ratio->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_NULL );
                    if ( !( sizeA <= 0.0 && sizeB <= 0.0 ) )
                        within = false;
                    continue;
                }
                const Float64 value = sizeB / sizeA;
                ratio->AddValueAsChild( CORE::CVariant( value ) );
                if ( fabs( value - 1.0 ) > tolerance )
                    within = false;
            }
            pair->SetAttribute( "triangleDelta", (Int32) second.triangles - (Int32) first.triangles );
            pair->SetAttribute( "vertexDelta", (Int32) second.vertices - (Int32) first.vertices );
            pair->SetAttribute( "materialDelta", (Int32) second.materials - (Int32) first.materials );
            pair->SetAttribute( "orientation", OrientationOf( first, second, tolerance ) );
            pair->SetAttribute( "withinTolerance", within );
        }
    }
    return call.Succeed();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineAnalysisOperations::MaterialMapSuggest( CPipelineOperationCall& call )
{GUCEF_TRACE;

    CConverterPack pack;
    if ( !call.FindRequestPack( pack, true ) )
        return call.result.status;
    const CORE::CString shaderName = call.GetArgument( "shaderName" );
    ASSETCONVERTER::CAssetPipelineOptions filter;
    filter.assetGlobs = call.request.assets;

    ASSETCONVERTER::CConversionReport report;
    if ( !pack.LoadReport( report ) )
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "the pack " + pack.packId + " was not converted yet: its sidecars hold the source materials" );
    const CORE::CDataNode* maps = pack.manifest.rules.FindChild( "shaderPropertyMaps" );

    TShaderMap shaders;
    const ASSETCONVERTER::CConversionReport::TEntryVector& entries = report.GetEntries();
    for ( size_t e=0; e<entries.size(); ++e )
    {
        if ( entries[ e ].assetPath.IsNULLOrEmpty() || !filter.IsInAssetFilter( entries[ e ].assetPath ) )
            continue;
        CORE::CDataNode sidecar;
        const CORE::CString path = ASSETCONVERTER::CConversionSidecar::GetPath( pack.manifest.GetIntermediatePath(), entries[ e ].assetPath );
        if ( !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( path ) || !ASSETCONVERTER::CCanonicalJson::Load( path, sidecar ) )
            continue;
        const CORE::CDataNode* materials = sidecar.FindChild( "materials" );
        if ( GUCEF_NULL == materials )
            continue;
        CORE::CDataNode::const_iterator m = materials->ConstBegin();
        for ( ; m != materials->ConstEnd(); ++m )
        {
            const CORE::CDataNode* source = (*m)->FindChild( "source" );
            if ( GUCEF_NULL == source )
                continue;
            const CORE::CString shader = source->GetAttributeValueOrChildValueByName( "shader" ).AsString( CORE::CString::Empty, true );
            if ( shader.IsNULLOrEmpty() )
                continue;
            if ( !shaderName.IsNULLOrEmpty() && shader != shaderName )
                continue;
            if ( shaderName.IsNULLOrEmpty() && GUCEF_NULL != maps && GUCEF_NULL != maps->FindChild( shader ) )
                continue;
            SShaderInfo& info = shaders[ shader ];
            ++info.materialCount;
            const CORE::CDataNode* properties = source->FindChild( "properties" );
            const CORE::CDataNode* textures = source->FindChild( "textures" );
            CORE::CString::StringSet names;
            if ( GUCEF_NULL != properties )
            {
                for ( CORE::CDataNode::TAttributeMap::const_iterator a=properties->AttributeBegin(); a!=properties->AttributeEnd(); ++a )
                    names.insert( (*a).first );
                for ( CORE::CDataNode::const_iterator c=properties->ConstBegin(); c!=properties->ConstEnd(); ++c )
                    names.insert( (*c)->GetName() );
            }
            if ( GUCEF_NULL != textures )
            {
                for ( CORE::CDataNode::TAttributeMap::const_iterator a=textures->AttributeBegin(); a!=textures->AttributeEnd(); ++a )
                    names.insert( (*a).first );
                for ( CORE::CDataNode::const_iterator c=textures->ConstBegin(); c!=textures->ConstEnd(); ++c )
                    names.insert( (*c)->GetName() );
            }
            for ( CORE::CString::StringSet::const_iterator n=names.begin(); n!=names.end(); ++n )
            {
                if ( (*n).IsNULLOrEmpty() )
                    continue;
                const CORE::CDataNode::TKeyValuePair* value = GUCEF_NULL == properties ? GUCEF_NULL : properties->GetAttribute( (*n) );
                const CORE::CDataNode* child = GUCEF_NULL == properties || GUCEF_NULL != value ? GUCEF_NULL : properties->FindChild( (*n) );
                const CORE::CDataNode::TKeyValuePair* textureValue = GUCEF_NULL == textures ? GUCEF_NULL : textures->GetAttribute( (*n) );
                const CORE::CDataNode* textureChild = GUCEF_NULL == textures || GUCEF_NULL != textureValue ? GUCEF_NULL : textures->FindChild( (*n) );
                const bool hasTexture = GUCEF_NULL != textureValue || GUCEF_NULL != textureChild;
                SPropertyInfo& property = info.properties[ (*n) ];
                if ( 0 == property.count )
                {
                    property.hasRange = false;
                    property.min = property.max = 0.0;
                }
                ++property.count;
                const CORE::CString type = TypeOfProperty( child, GUCEF_NULL != value ? &(*value).second : GUCEF_NULL, hasTexture );
                if ( property.type.IsNULLOrEmpty() || "texture" == type )
                    property.type = type;
                if ( GUCEF_NULL != value && (*value).second.IsNumber() )
                    NoteNumber( property, (*value).second.AsFloat64( 0.0, true ) );
                if ( hasTexture && property.textureFiles.size() < 10 )
                {
                    const CORE::CString file = CPipelineInspectionOperations::DescribeValue( textureChild, GUCEF_NULL != textureValue ? &(*textureValue).second : GUCEF_NULL );
                    if ( !file.IsNULLOrEmpty() )
                        property.textureFiles.insert( file );
                }
            }
        }
    }

    CORE::CDataNode& document = call.Document();
    document.SetAttribute( "packId", pack.packId );
    CORE::CDataNode* list = document.AddChild( "shaders", GUCEF_DATATYPE_ARRAY );
    CORE::CDataNode* fragment = document.AddChild( "fragment", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* fragmentMaps = fragment->AddChild( "shaderPropertyMaps", GUCEF_DATATYPE_OBJECT );
    for ( TShaderMap::const_iterator s=shaders.begin(); s!=shaders.end(); ++s )
    {
        const CORE::CDataNode* rule = GUCEF_NULL == maps ? GUCEF_NULL : maps->FindChild( (*s).first );
        CORE::CDataNode* item = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        item->SetAttribute( "shaderName", (*s).first );
        item->SetAttribute( "materialCount", (*s).second.materialCount );
        CORE::CDataNode* properties = item->AddChild( "properties", GUCEF_DATATYPE_ARRAY );
        CORE::CDataNode* mapped = item->AddChild( "mapped", GUCEF_DATATYPE_ARRAY );
        CORE::CDataNode* unmapped = item->AddChild( "unmapped", GUCEF_DATATYPE_ARRAY );
        CORE::CDataNode* proposals = item->AddChild( "proposals", GUCEF_DATATYPE_ARRAY );
        CORE::CDataNode* shaderFragment = fragmentMaps->AddChild( (*s).first, GUCEF_DATATYPE_OBJECT );
        for ( TPropertyMap::const_iterator p=(*s).second.properties.begin(); p!=(*s).second.properties.end(); ++p )
        {
            const SPropertyInfo& info = (*p).second;
            CORE::CDataNode* property = properties->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            property->SetAttribute( "name", (*p).first );
            property->SetAttribute( "type", info.type );
            property->SetAttribute( "count", info.count );
            if ( info.hasRange )
            {
                property->SetAttribute( "min", CORE::CVariant( info.min ) );
                property->SetAttribute( "max", CORE::CVariant( info.max ) );
            }
            else
            {
                property->SetAttribute( "min", CORE::CVariant() );
                property->SetAttribute( "max", CORE::CVariant() );
            }
            property->SetValuesOfChildByName( "textureFiles", info.textureFiles );
            const CORE::CDataNode::TKeyValuePair* current = GUCEF_NULL == rule ? GUCEF_NULL : rule->GetAttribute( (*p).first );
            if ( GUCEF_NULL != current )
            {
                CORE::CDataNode* entry = mapped->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                entry->SetAttribute( "property", (*p).first );
                entry->SetAttribute( "role", (*current).second.AsString( CORE::CString::Empty, true ) );
                shaderFragment->SetAttribute( (*p).first, (*current).second.AsString( CORE::CString::Empty, true ) );
                continue;
            }
            unmapped->AddValueAsChild( (*p).first );
            CORE::CString slot;
            Float64 confidence = 0.0;
            if ( SuggestSlot( (*p).first, info.type, slot, confidence ) )
            {
                CORE::CDataNode* proposal = proposals->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                proposal->SetAttribute( "property", (*p).first );
                proposal->SetAttribute( "role", slot );
                proposal->SetAttribute( "confidence", CORE::CVariant( confidence ) );
                shaderFragment->SetAttribute( (*p).first, slot );
            }
        }
    }
    return call.Succeed();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineAnalysisOperations::ManifestPropose( CPipelineOperationCall& call )
{GUCEF_TRACE;

    CORE::CString sourceVfs;
    const ASSETSTORE::CAssetToolRoot* root = GUCEF_NULL;
    CORE::CString rooted;
    if ( !call.ResolveRootedPath( call.request.sourcePath, "source|staging", false, sourceVfs, root, rooted ) )
        return call.result.status;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.DirExists( sourceVfs ) && !vfs.FileExists( sourceVfs ) )
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "there is nothing at " + rooted );

    const CORE::CString stem = ASSETCONVERTER::CVfsPaths::StemOf( sourceVfs );
    const CORE::CString id = call.request.packId.IsNULLOrEmpty() ? CPipelineLicenceOperations::MakeId( stem ) : call.request.packId;
    CORE::CString title = call.GetArgument( "title" );
    if ( title.IsNULLOrEmpty() )
        title = ASSETCONVERTER::CVfsPaths::NameOf( sourceVfs );

    // the source as the pipeline sees it: a folder, or an archive mounted read only for the time of the call
    ASSETCONVERTER::CAssetPackManifest probe;
    probe.id = id;
    probe.source = "/" + ASSETCONVERTER::CVfsPaths::Normalize( sourceVfs );
    probe.manifestFolder = ASSETCONVERTER::CVfsPaths::FolderOf( sourceVfs );
    ASSETCONVERTER::CAssetPackContext context;
    if ( !context.Open( probe ) )
        return call.Fail( CPipelineOperationCall::ErrorNotFound, "the source " + rooted + " cannot be opened as a folder or an archive" );

    CORE::CDataNode& document = call.Document();
    CORE::CDataNode* counts = document.AddChild( "counts", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* extensions = counts->AddChild( "extensions", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* importerCounts = counts->AddChild( "importers", GUCEF_DATATYPE_OBJECT );
    const CORE::CString::StringVector& files = context.GetFiles();
    GUCEF::map< CORE::CString, UInt32 > extensionCount;
    bool unityProject = false;
    bool godot = false;
    bool unrealExport = false;
    for ( size_t f=0; f<files.size(); ++f )
    {
        const CORE::CString extension = ASSETCONVERTER::CVfsPaths::ExtensionOf( files[ f ] ).Lowercase();
        ++extensionCount[ extension.IsNULLOrEmpty() ? CORE::CString( "(none)" ) : extension ];
        const CORE::CString name = ASSETCONVERTER::CVfsPaths::NameOf( files[ f ] );
        if ( "meta" == extension && 0 == files[ f ].HasSubstr( "Assets/", true ) )
            unityProject = true;
        if ( "project.godot" == name )
            godot = true;
        if ( "material-params.json" == name )
            unrealExport = true;
    }
    for ( GUCEF::map< CORE::CString, UInt32 >::const_iterator e=extensionCount.begin(); e!=extensionCount.end(); ++e )
        extensions->SetAttribute( (*e).first, (*e).second );

    GUCEF::vector< ASSETCONVERTER::TAssetImporterPtr > importers;
    ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry().GetImporters( importers );
    CORE::CString bestImporter;
    UInt32 bestCount = 0;
    for ( size_t i=0; i<importers.size(); ++i )
    {
        importers[ i ]->IndexPack( context );
        UInt32 taken = 0;
        for ( size_t f=0; f<files.size(); ++f )
            if ( importers[ i ]->CanImport( context.ToVfsPath( files[ f ] ), context ) )
                ++taken;
        importerCounts->SetAttribute( importers[ i ]->GetName(), taken );
        if ( taken > bestCount )
        {
            bestCount = taken;
            bestImporter = importers[ i ]->GetName();
        }
    }

    CORE::CString::StringVector detected;
    if ( unityProject )
        detected.push_back( "unity-project" );
    if ( "unitypackage" == ASSETCONVERTER::CVfsPaths::ExtensionOf( sourceVfs ).Lowercase() )
        detected.push_back( "unity-package" );
    if ( godot )
        detected.push_back( "godot" );
    if ( unrealExport )
        detected.push_back( "unreal-export" );
    if ( detected.empty() )
        detected.push_back( "loose-files" );
    document.SetValuesOfChildByName( "detected", detected );

    // the folders below the first roots of their kinds
    CORE::CString::StringVector notes;
    ASSETCONVERTER::CAssetPackManifest proposal;
    proposal.id = id;
    proposal.title = title;
    proposal.store = call.GetArgument( "store" );
    proposal.source = probe.source;
    proposal.importer = bestImporter.IsNULLOrEmpty() ? ASSETCONVERTER::CAssetPackManifest::AutoImporter : bestImporter;
    if ( bestImporter.IsNULLOrEmpty() )
        notes.push_back( "no importer takes a file of the source: the importer is left on auto" );
    const char* kinds[] = { "intermediate", "output", "overrides" };
    CORE::CString* members[] = { &proposal.intermediate, &proposal.output, &proposal.overrides };
    for ( UInt32 k=0; k<3; ++k )
    {
        const ASSETSTORE::CAssetToolRoot* kindRoot = ASSETSTORE::CAssetToolRoot::FindFirstOfKind( call.settings.roots, kinds[ k ] );
        if ( GUCEF_NULL == kindRoot )
        {
            notes.push_back( CORE::CString( "there is no root of the kind " ) + kinds[ k ] + ": the folder is left empty" );
            continue;
        }
        *members[ k ] = "/" + ASSETCONVERTER::CVfsPaths::Join( kindRoot->vfsPath, id );
    }
    const ASSETSTORE::CAssetToolRoot* licences = ASSETSTORE::CAssetToolRoot::FindFirstOfKind( call.settings.roots, ASSETSTORE::CAssetToolRoot::KindLicences );
    bool recordFound = false;
    if ( GUCEF_NULL != licences )
    {
        proposal.licenceFolder = "/" + ASSETCONVERTER::CVfsPaths::Normalize( licences->vfsPath );
        recordFound = vfs.FileExists( ASSETCONVERTER::CVfsPaths::Join( licences->vfsPath, id + ".licence.json" ) );
        if ( recordFound )
            proposal.licenceRecord = id;
        else
            notes.push_back( "there is no licence record " + id + ".licence.json: the owner has to make one (licence_draft_record gives a draft) before the pack can be converted" );
    }
    else
        notes.push_back( "there is no root of the kind licences" );
    proposal.cookProfiles.push_back( ASSETCONVERTER::CCookProfile( "desktop" ) );
    CORE::CString::StringVector cookers;
    ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry().GetCookers().GetRegisteredObjNames( cookers );
    if ( !cookers.empty() )
        notes.push_back( "cook targets that are loaded: " + CORE::CString().Combine( cookers, ',' ) + "; none is set in the proposal" );

    CORE::CDataNode* manifestNode = document.AddChild( "manifest", GUCEF_DATATYPE_OBJECT );
    proposal.Serialize( *manifestNode );
    manifestNode->SetName( "manifest" );
    CORE::CDataNode* paths = document.AddChild( "paths", GUCEF_DATATYPE_OBJECT );
    paths->SetAttribute( "source", rooted );
    call.SetRooted( *paths, "intermediate", proposal.intermediate );
    call.SetRooted( *paths, "output", proposal.output );
    call.SetRooted( *paths, "overrides", proposal.overrides );
    call.SetRooted( *paths, "licenceFolder", proposal.licenceFolder );
    document.SetValuesOfChildByName( "notes", notes );
    document.SetAttribute( "licenceRecordFound", recordFound );
    document.SetAttribute( "licenceRecordRequired", !recordFound );
    document.SetAttribute( "fileCount", (UInt32) files.size() );
    return call.Succeed();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
