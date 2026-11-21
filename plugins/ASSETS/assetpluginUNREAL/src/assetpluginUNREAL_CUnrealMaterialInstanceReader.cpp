/*
 *  assetpluginUNREAL: Follows the parent chain of a material instance to its master material and collects the parameters
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#include "assetpluginUNREAL_CUnrealPropertyReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#include "assetpluginUNREAL_CUnrealObjectProperties.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALINSTANCEREADER_H
#include "assetpluginUNREAL_CUnrealMaterialInstanceReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALINSTANCEREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMATERIALINSTANCEREADER_H ? */

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

/** The value of a property without the prefix of its enumeration: EMaterialShadingModel::MSM_Cloth and MSM_Cloth are the same */
static CORE::CString
EnumValue( const CORE::CString& text )
{
    const Int32 at = text.HasSubstr( "::" );
    return at >= 0 ? text.SubstrFromRange( (UInt32) at + 2, text.Length() ) : text;
}

/*-------------------------------------------------------------------------*/

/** The object path an object property refers to, empty when it is null */
static CORE::CString
ObjectPathOf( const CORE::CDataNode* node )
{
    return GUCEF_NULL == node ? CORE::CString() : node->GetAttributeValue( "path" ).AsString();
}

/*-------------------------------------------------------------------------*/

/** The name of the parameter of an entry of one of the parameter arrays of an instance */
static CORE::CString
ParameterNameOf( const CORE::CDataNode& entry )
{
    const CORE::CDataNode* info = entry.FindChild( "ParameterInfo" );
    const CORE::CDataNode* name = GUCEF_NULL != info ? info->FindChild( "Name" ) : GUCEF_NULL;
    if ( GUCEF_NULL == name )
        name = entry.FindChild( "ParameterName" );      // older assets
    return GUCEF_NULL == name ? CORE::CString() : name->GetValue().AsString();
}

/*-------------------------------------------------------------------------*/

static CUnrealResolvedMaterial::SColour
ColourOf( const CORE::CDataNode* node )
{
    CUnrealResolvedMaterial::SColour colour = { 0.0, 0.0, 0.0, 1.0 };
    if ( GUCEF_NULL != node )
    {
        colour.r = node->GetAttributeValue( "R" ).AsFloat64( 0.0 );
        colour.g = node->GetAttributeValue( "G" ).AsFloat64( 0.0 );
        colour.b = node->GetAttributeValue( "B" ).AsFloat64( 0.0 );
        colour.a = node->GetAttributeValue( "A" ).AsFloat64( 1.0 );
    }
    return colour;
}

/*-------------------------------------------------------------------------*/

/** Applies the parameter arrays and base property overrides of an instance: what is set stays */
static void
ApplyInstance( const CORE::CDataNode& root, CUnrealResolvedMaterial& out )
{
    const CORE::CDataNode* scalars = root.FindChild( "ScalarParameterValues" );
    for ( UInt32 i=0; GUCEF_NULL != scalars && i<scalars->GetNrOfDirectChildNodes(); ++i )
    {
        const CORE::CDataNode* entry = scalars->GetChildAtIndex( i );
        const CORE::CString name = ParameterNameOf( *entry );
        const CORE::CDataNode* value = entry->FindChild( "ParameterValue" );
        if ( !name.IsNULLOrEmpty() && GUCEF_NULL != value && !out.HasScalar( name ) )
            out.scalars[ name ] = value->GetValue().AsFloat64( 0.0 );
    }
    const CORE::CDataNode* vectors = root.FindChild( "VectorParameterValues" );
    for ( UInt32 i=0; GUCEF_NULL != vectors && i<vectors->GetNrOfDirectChildNodes(); ++i )
    {
        const CORE::CDataNode* entry = vectors->GetChildAtIndex( i );
        const CORE::CString name = ParameterNameOf( *entry );
        if ( !name.IsNULLOrEmpty() && !out.HasColour( name ) )
            out.colours[ name ] = ColourOf( entry->FindChild( "ParameterValue" ) );
    }
    const CORE::CDataNode* textures = root.FindChild( "TextureParameterValues" );
    for ( UInt32 i=0; GUCEF_NULL != textures && i<textures->GetNrOfDirectChildNodes(); ++i )
    {
        const CORE::CDataNode* entry = textures->GetChildAtIndex( i );
        const CORE::CString name = ParameterNameOf( *entry );
        const CORE::CString path = ObjectPathOf( entry->FindChild( "ParameterValue" ) );
        if ( !name.IsNULLOrEmpty() && !out.HasTexture( name ) )
            out.textures[ name ] = path;
    }
    // the static switches: the runtime form of the static parameters, or the older direct array
    const CORE::CDataNode* runtime = root.FindChild( "StaticParametersRuntime" );
    const CORE::CDataNode* switches = GUCEF_NULL != runtime ? runtime->FindChild( "StaticSwitchParameters" ) : root.FindChild( "StaticSwitchParameters" );
    for ( UInt32 i=0; GUCEF_NULL != switches && i<switches->GetNrOfDirectChildNodes(); ++i )
    {
        const CORE::CDataNode* entry = switches->GetChildAtIndex( i );
        const CORE::CString name = ParameterNameOf( *entry );
        const CORE::CDataNode* value = entry->FindChild( "Value" );
        if ( !name.IsNULLOrEmpty() && GUCEF_NULL != value && !out.HasSwitch( name ) )
            out.switches[ name ] = value->GetValue().AsBool();
    }
    // base property overrides: a property that is there is overridden
    const CORE::CDataNode* base = root.FindChild( "BasePropertyOverrides" );
    if ( GUCEF_NULL != base )
    {
        const CORE::CDataNode* blend = base->FindChild( "BlendMode" );
        if ( GUCEF_NULL != blend && out.blendMode.IsNULLOrEmpty() )
            out.blendMode = EnumValue( blend->GetValue().AsString() );
        const CORE::CDataNode* shading = base->FindChild( "ShadingModel" );
        if ( GUCEF_NULL != shading && out.shadingModel.IsNULLOrEmpty() )
            out.shadingModel = EnumValue( shading->GetValue().AsString() );
        const CORE::CDataNode* twoSided = base->FindChild( "TwoSided" );
        if ( GUCEF_NULL != twoSided && !out.hasTwoSided )
        {
            out.twoSided = twoSided->GetValue().AsBool();
            out.hasTwoSided = true;
        }
        const CORE::CDataNode* clip = base->FindChild( "OpacityMaskClipValue" );
        if ( GUCEF_NULL != clip && !out.hasClipValue )
        {
            out.opacityMaskClipValue = clip->GetValue().AsFloat64( 0.3333 );
            out.hasClipValue = true;
        }
    }
}

/*-------------------------------------------------------------------------*/

/** The master material: base properties, then the defaults of the parameter expressions for the parameters nobody set */
static void
ApplyMaster( CUnrealPackage& package, UInt32 masterExport, const CORE::CDataNode& root, CUnrealResolvedMaterial& out )
{
    const CORE::CDataNode* blend = root.FindChild( "BlendMode" );
    if ( out.blendMode.IsNULLOrEmpty() )
        out.blendMode = GUCEF_NULL != blend ? EnumValue( blend->GetValue().AsString() ) : CORE::CString( "BLEND_Opaque" );
    const CORE::CDataNode* shading = root.FindChild( "ShadingModel" );
    if ( out.shadingModel.IsNULLOrEmpty() )
        out.shadingModel = GUCEF_NULL != shading ? EnumValue( shading->GetValue().AsString() ) : CORE::CString( "MSM_DefaultLit" );
    if ( !out.hasTwoSided )
    {
        const CORE::CDataNode* twoSided = root.FindChild( "TwoSided" );
        out.twoSided = GUCEF_NULL != twoSided && twoSided->GetValue().AsBool();
        out.hasTwoSided = true;
    }
    if ( !out.hasClipValue )
    {
        const CORE::CDataNode* clip = root.FindChild( "OpacityMaskClipValue" );
        out.opacityMaskClipValue = GUCEF_NULL != clip ? clip->GetValue().AsFloat64( 0.3333 ) : 0.3333;
        out.hasClipValue = true;
    }

    // the expressions that are parameters; a value that equals the default of its property is not stored, so it is 0 or false
    const CUnrealPackage::TExportVector& exports = package.GetExports();
    for ( size_t e=0; e<exports.size(); ++e )
    {
        if ( e == masterExport )
            continue;
        const CORE::CString className = package.GetClassName( exports[ e ] );
        const bool isScalar = "MaterialExpressionScalarParameter" == className;
        const bool isVector = "MaterialExpressionVectorParameter" == className;
        const bool isTexture = "MaterialExpressionTextureSampleParameter2D" == className;
        const bool isSwitch = "MaterialExpressionStaticSwitchParameter" == className || "MaterialExpressionStaticBoolParameter" == className;
        if ( !isScalar && !isVector && !isTexture && !isSwitch )
            continue;
        CUnrealArchiveReader reader;
        CORE::CDataNode expression( "root" );
        CUnrealPropertyReader properties( package );
        if ( !package.OpenExportRange( (UInt32) e, reader ) || !properties.ReadExport( reader, expression, className ) )
        {
            out.problems.push_back( "The parameter expression " + exports[ e ].objectName + " cannot be read" );
            continue;
        }
        const CORE::CDataNode* nameNode = expression.FindChild( "ParameterName" );
        if ( GUCEF_NULL == nameNode )
            continue;
        const CORE::CString name = nameNode->GetValue().AsString();
        if ( isScalar && !out.HasScalar( name ) )
        {
            const CORE::CDataNode* value = expression.FindChild( "DefaultValue" );
            out.scalars[ name ] = GUCEF_NULL != value ? value->GetValue().AsFloat64( 0.0 ) : 0.0;
        }
        else
        if ( isVector && !out.HasColour( name ) )
            out.colours[ name ] = ColourOf( expression.FindChild( "DefaultValue" ) );
        else
        if ( isTexture && !out.HasTexture( name ) )
            out.textures[ name ] = ObjectPathOf( expression.FindChild( "Texture" ) );
        else
        if ( isSwitch && !out.HasSwitch( name ) )
        {
            const CORE::CDataNode* value = expression.FindChild( "DefaultValue" );
            out.switches[ name ] = GUCEF_NULL != value && value->GetValue().AsBool();
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CUnrealMaterialInstanceReader::Resolve( const CORE::CString& materialPath, const CUnrealObjectPathResolver& resolver, CUnrealResolvedMaterial& out )
{GUCEF_TRACE;

    CORE::CString::StringSet visited;
    CORE::CString path = materialPath;
    for ( UInt32 depth=0; depth<MaxChainDepth; ++depth )
    {
        if ( !visited.insert( path ).second )
        {
            out.problems.push_back( "The parent chain of the material loops at " + path );
            return depth > 0;
        }
        CUnrealPackage package;
        if ( !package.Open( path ) )
        {
            out.problems.push_back( "The package " + path + " cannot be read: " + package.GetError() );
            return depth > 0;
        }
        out.chain.push_back( package.GetSummary().packageName );

        // the material object of the package: a top level export that is an instance or a material
        Int32 materialExport = -1;
        CORE::CString className;
        for ( size_t e=0; e<package.GetExports().size() && materialExport < 0; ++e )
        {
            if ( !package.GetExports()[ e ].outerIndex.IsNull() )
                continue;
            className = package.GetClassName( package.GetExports()[ e ] );
            if ( "MaterialInstanceConstant" == className || "Material" == className )
                materialExport = (Int32) e;
        }
        if ( materialExport < 0 )
        {
            out.problems.push_back( "The package " + package.GetSummary().packageName + " holds no material or material instance" );
            return depth > 0;
        }
        if ( 0 == depth )
            out.name = package.GetExports()[ (size_t) materialExport ].objectName;

        CUnrealArchiveReader reader;
        CORE::CDataNode root( "root" );
        CUnrealPropertyReader properties( package );
        if ( !package.OpenExportRange( (UInt32) materialExport, reader ) || !properties.ReadExport( reader, root, className ) )
        {
            out.problems.push_back( "The properties of " + package.GetExports()[ (size_t) materialExport ].objectName + " cannot be read: " + reader.GetError() );
            return depth > 0;
        }
        if ( "Material" == className )
        {
            out.masterPath = package.GetSummary().packageName + "." + package.GetExports()[ (size_t) materialExport ].objectName;
            ApplyMaster( package, (UInt32) materialExport, root, out );
            return true;
        }

        ApplyInstance( root, out );
        const CORE::CString parentPath = ObjectPathOf( root.FindChild( "Parent" ) );
        if ( parentPath.IsNULLOrEmpty() )
        {
            out.problems.push_back( "The material instance " + out.chain.back() + " has no parent" );
            return true;
        }
        if ( !resolver.ResolveReference( parentPath, ASSETCONVERTER::CAssetReference::KindUsesMaterial, path ) )
        {
            out.problems.push_back( "The parent " + parentPath + " is not in the pack (engine or plugin content): its parameters are not known" );
            return true;
        }
    }
    out.problems.push_back( "The parent chain is deeper than " + CORE::UInt32ToString( MaxChainDepth ) + " levels" );
    return true;
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
