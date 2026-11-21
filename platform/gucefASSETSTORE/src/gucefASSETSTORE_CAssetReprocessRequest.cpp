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

#include <algorithm>

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#include "gucefASSETSTORE_CAssetPipelineRequest.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefASSETSTORE_CAssetReprocessRequest.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

void
AddList( CORE::CString::StringVector& list, const CORE::CDataNode& node, const char* name )
{GUCEF_TRACE;

    CORE::CString::StringVector values;
    node.GetValuesOfChildByName( name, values );
    if ( values.empty() )
    {
        const CORE::CString text = node.GetAttributeValueOrChildValueByName( name ).AsString( CORE::CString::Empty, true );
        if ( !text.IsNULLOrEmpty() )
            values = text.ParseElements( ',', false );
    }
    for ( size_t i=0; i<values.size(); ++i )
    {
        const CORE::CString value = values[ i ].Trim( true ).Trim( false );
        if ( !value.IsNULLOrEmpty() && list.end() == std::find( list.begin(), list.end(), value ) )
            list.push_back( value );
    }
}

void
AddSet( CORE::CString::StringSet& set, const CORE::CDataNode& node, const char* name )
{GUCEF_TRACE;

    CORE::CString::StringVector list;
    AddList( list, node, name );
    for ( size_t i=0; i<list.size(); ++i )
        set.insert( list[ i ].Lowercase() );
}

CORE::CString
Joined( const CORE::CString::StringVector& list )
{GUCEF_TRACE;

    CORE::CString text;
    for ( size_t i=0; i<list.size(); ++i )
        text += ( 0 == i ? CORE::CString::Empty : CORE::CString( "," ) ) + list[ i ];
    return text;
}

CORE::CString
Joined( const CORE::CString::StringSet& set )
{GUCEF_TRACE;

    return Joined( CORE::CString::StringVector( set.begin(), set.end() ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetReprocessRequest::StepRedownload = "redownload";
const CORE::CString CAssetReprocessRequest::StepIntermediate = "intermediate";
const CORE::CString CAssetReprocessRequest::StepTarget = "target";
const CORE::CString CAssetReprocessRequest::StepRollback = "rollback";
const CORE::CString CAssetReprocessRequest::TargetOgre = "ogre";

/*-------------------------------------------------------------------------*/

CAssetReprocessRequest::CAssetReprocessRequest( void )
    : CORE::CIDataNodeSerializable()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

CAssetReprocessRequest::~CAssetReprocessRequest()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetReprocessRequest::Clear( void )
{GUCEF_TRACE;

    operation.Clear();
    packId.Clear();
    steps.clear();
    assets.clear();
    storeAssetIds.clear();
    onlyStatuses.clear();
    kinds.clear();
    targets.clear();
    cook = true;
    thenConvert = true;
    repairOriginals = true;
    allowLarge = false;
    acceptErrors = false;
    caller.Clear();
    machine = false;
}

/*-------------------------------------------------------------------------*/

void
CAssetReprocessRequest::GetEffectiveSteps( CORE::CString::StringVector& effective ) const
{GUCEF_TRACE;

    typedef CAssetPipelineOperations Op;
    effective.clear();
    if ( Op::PackRedownloadOriginal == operation )
        effective.push_back( StepRedownload );
    else
    if ( Op::PackRegenerateIntermediate == operation || Op::AssetRegenerateIntermediate == operation )
        effective.push_back( StepIntermediate );
    else
    if ( Op::PackRegenerateTarget == operation || Op::AssetRegenerateTarget == operation )
        effective.push_back( StepTarget );
    else
    if ( Op::ReprocessRollback == operation )
        effective.push_back( StepRollback );
    else
    {
        // pack_reprocess and the plan: the steps given, in their order
        static const CORE::CString order[] = { StepRedownload, StepIntermediate, StepTarget };
        for ( UInt32 o=0; o<3; ++o )
            if ( steps.end() != std::find( steps.begin(), steps.end(), order[ o ] ) )
                effective.push_back( order[ o ] );
        if ( steps.end() != std::find( steps.begin(), steps.end(), StepRollback ) && effective.empty() )
            effective.push_back( StepRollback );
        if ( effective.empty() )
        {
            effective.push_back( StepIntermediate );
            effective.push_back( StepTarget );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetReprocessRequest::HasStep( const CORE::CString& step ) const
{GUCEF_TRACE;

    CORE::CString::StringVector effective;
    GetEffectiveSteps( effective );
    return effective.end() != std::find( effective.begin(), effective.end(), step );
}

/*-------------------------------------------------------------------------*/

bool
CAssetReprocessRequest::IsPartial( void ) const
{GUCEF_TRACE;

    return !assets.empty() || !storeAssetIds.empty() || !onlyStatuses.empty() || !kinds.empty();
}

/*-------------------------------------------------------------------------*/

bool
CAssetReprocessRequest::IsValid( CORE::CString& reason ) const
{GUCEF_TRACE;

    typedef CAssetPipelineOperations Op;
    if ( !Op::IsReprocess( operation ) && Op::ReprocessPlan != operation )
    {
        reason = "the operation " + operation + " is no reprocessing";
        return false;
    }
    if ( packId.IsNULLOrEmpty() && storeAssetIds.empty() )
    {
        reason = "no pack is given (packId, or storePackId)";
        return false;
    }
    if ( ( Op::AssetRegenerateIntermediate == operation || Op::AssetRegenerateTarget == operation ) && assets.empty() && storeAssetIds.empty() )
    {
        reason = "the operation " + operation + " needs assets: assetPath, assets or storeAssetIds";
        return false;
    }
    if ( ( Op::PackRedownloadOriginal == operation || Op::ReprocessRollback == operation ) && IsPartial() )
    {
        reason = "the operation " + operation + " is for a whole pack: no assets or filters";
        return false;
    }
    for ( size_t s=0; s<steps.size(); ++s )
    {
        if ( StepRedownload != steps[ s ] && StepIntermediate != steps[ s ] && StepTarget != steps[ s ] && StepRollback != steps[ s ] )
        {
            reason = "the step " + steps[ s ] + " is not known (redownload, intermediate, target, rollback)";
            return false;
        }
    }
    CORE::CString::StringVector effective;
    GetEffectiveSteps( effective );
    if ( IsPartial() && effective.end() != std::find( effective.begin(), effective.end(), StepRedownload ) )
    {
        reason = "the original is downloaded again for a whole pack: no assets or filters with the step redownload";
        return false;
    }
    for ( CORE::CString::StringSet::const_iterator i = onlyStatuses.begin(); i != onlyStatuses.end(); ++i )
    {
        if ( "failed" != (*i) && "skipped" != (*i) && "approximated" != (*i) && "refused" != (*i) && "converted" != (*i) && "cached" != (*i) )
        {
            reason = "the status " + (*i) + " is not known (failed, skipped, approximated, refused, converted, cached)";
            return false;
        }
    }
    for ( size_t t=0; t<targets.size(); ++t )
    {
        const CORE::CString target = targets[ t ].Lowercase();
        if ( TargetOgre != target && "gucef-ogre3d" != target && "all" != target )
        {
            reason = "the target " + targets[ t ] + " is not configured (ogre, also called gucef-ogre3d)";
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CAssetReprocessRequest
CAssetReprocessRequest::FromRequest( const CAssetPipelineRequest& request )
{GUCEF_TRACE;

    CAssetReprocessRequest result;
    CORE::CDataNodeSerializableSettings settings;
    result.Deserialize( request.arguments, settings );
    // the plan names the operation it plans in its arguments; a job is its own operation
    const CORE::CString planned = result.operation;
    result.operation = request.operation;
    if ( CAssetPipelineOperations::ReprocessPlan == request.operation && CAssetPipelineOperations::IsReprocess( planned ) )
        result.operation = planned;
    if ( !request.packId.IsNULLOrEmpty() )
        result.packId = request.packId;
    for ( size_t i=0; i<request.assets.size(); ++i )
        if ( result.assets.end() == std::find( result.assets.begin(), result.assets.end(), request.assets[ i ] ) )
            result.assets.push_back( request.assets[ i ] );
    for ( size_t i=0; i<request.cookTargets.size(); ++i )
        if ( result.targets.end() == std::find( result.targets.begin(), result.targets.end(), request.cookTargets[ i ] ) )
            result.targets.push_back( request.cookTargets[ i ] );
    if ( !request.assetPath.IsNULLOrEmpty() && result.assets.end() == std::find( result.assets.begin(), result.assets.end(), request.assetPath ) )
        result.assets.push_back( request.assetPath );
    if ( !request.storeAssetId.IsNULLOrEmpty() && result.storeAssetIds.end() == std::find( result.storeAssetIds.begin(), result.storeAssetIds.end(), request.storeAssetId ) )
        result.storeAssetIds.push_back( request.storeAssetId );
    // the paths may name the area: "original/a/b.fbx" is "a/b.fbx"
    for ( size_t i=0; i<result.assets.size(); ++i )
    {
        CORE::CString path = result.assets[ i ].ReplaceChar( '\\', '/' );
        if ( 0 == path.HasSubstr( "original/", true ) )
            path = path.CutChars( 9, true );
        result.assets[ i ] = path;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

void
CAssetReprocessRequest::ToRequest( CAssetPipelineRequest& request ) const
{GUCEF_TRACE;

    request.Clear();
    request.operation = operation;
    request.packId = packId;
    request.assets = assets;
    request.cookTargets = targets;
    CORE::CDataNodeSerializableSettings settings;
    request.arguments.Clear();
    request.arguments.SetName( "arguments" );
    request.arguments.SetNodeType( GUCEF_DATATYPE_OBJECT );
    Serialize( request.arguments, settings );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetReprocessRequest::DescribeForAudit( void ) const
{GUCEF_TRACE;

    CORE::CString::StringVector effective;
    GetEffectiveSteps( effective );
    CORE::CString text = "caller=" + ( caller.IsNULLOrEmpty() ? CORE::CString( "unknown" ) : caller ) + ( machine ? CORE::CString( " (machine)" ) : CORE::CString::Empty ) +
                         ";operation=" + operation + ";steps=" + Joined( effective );
    if ( IsPartial() )
        text += ";scope=" + CORE::ToString( (UInt32) ( assets.size() + storeAssetIds.size() ) ) + " asset(s)" +
                ( onlyStatuses.empty() ? CORE::CString::Empty : " status " + Joined( onlyStatuses ) ) + ( kinds.empty() ? CORE::CString::Empty : " kind " + Joined( kinds ) );
    else
        text += ";scope=pack";
    if ( !targets.empty() )
        text += ";targets=" + Joined( targets );
    if ( acceptErrors )
        text += ";acceptErrors";
    if ( HasStep( StepRedownload ) )
        text += CORE::CString( ";thenConvert=" ) + ( thenConvert ? "true" : "false" ) + ( allowLarge ? ";allowLarge" : "" );
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CAssetReprocessRequest::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    if ( !operation.IsNULLOrEmpty() )
        success = domRootNode.SetAttribute( "operation", operation ) && success;
    success = domRootNode.SetAttribute( "storePackId", packId ) && success;
    if ( !steps.empty() )
        success = domRootNode.SetValuesOfChildByName( "steps", steps ) && success;
    if ( !assets.empty() )
        success = domRootNode.SetValuesOfChildByName( "assetPaths", assets ) && success;
    if ( !storeAssetIds.empty() )
        success = domRootNode.SetValuesOfChildByName( "storeAssetIds", storeAssetIds ) && success;
    if ( !onlyStatuses.empty() )
        success = domRootNode.SetValuesOfChildByName( "onlyStatuses", CORE::CString::StringVector( onlyStatuses.begin(), onlyStatuses.end() ) ) && success;
    if ( !kinds.empty() )
        success = domRootNode.SetValuesOfChildByName( "kinds", CORE::CString::StringVector( kinds.begin(), kinds.end() ) ) && success;
    if ( !targets.empty() )
        success = domRootNode.SetValuesOfChildByName( "targets", targets ) && success;
    success = domRootNode.SetAttribute( "cook", cook ) && success;
    success = domRootNode.SetAttribute( "thenConvert", thenConvert ) && success;
    success = domRootNode.SetAttribute( "repairOriginals", repairOriginals ) && success;
    success = domRootNode.SetAttribute( "allowLarge", allowLarge ) && success;
    success = domRootNode.SetAttribute( "acceptErrors", acceptErrors ) && success;
    if ( !caller.IsNULLOrEmpty() )
        success = domRootNode.SetAttribute( "caller", caller ) && success;
    success = domRootNode.SetAttribute( "machine", machine ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetReprocessRequest::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    operation = domRootNode.GetAttributeValueOrChildValueByName( "operation" ).AsString( CORE::CString::Empty, true );
    packId = domRootNode.GetAttributeValueOrChildValueByName( "storePackId" ).AsString( CORE::CString::Empty, true );
    if ( packId.IsNULLOrEmpty() )
        packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString( CORE::CString::Empty, true );
    AddList( steps, domRootNode, "steps" );
    AddList( assets, domRootNode, "assetPaths" );
    AddList( assets, domRootNode, "assets" );
    const CORE::CString assetPath = domRootNode.GetAttributeValueOrChildValueByName( "assetPath" ).AsString( CORE::CString::Empty, true );
    if ( !assetPath.IsNULLOrEmpty() && assets.end() == std::find( assets.begin(), assets.end(), assetPath ) )
        assets.push_back( assetPath );
    AddList( storeAssetIds, domRootNode, "storeAssetIds" );
    const CORE::CString storeAssetId = domRootNode.GetAttributeValueOrChildValueByName( "storeAssetId" ).AsString( CORE::CString::Empty, true );
    if ( !storeAssetId.IsNULLOrEmpty() && storeAssetIds.end() == std::find( storeAssetIds.begin(), storeAssetIds.end(), storeAssetId ) )
        storeAssetIds.push_back( storeAssetId );
    AddSet( onlyStatuses, domRootNode, "onlyStatuses" );
    if ( domRootNode.GetAttributeValueOrChildValueByName( "onlyFailed" ).AsBool( false, true ) )
    {
        onlyStatuses.insert( "failed" );
        onlyStatuses.insert( "skipped" );
    }
    AddSet( kinds, domRootNode, "kinds" );
    AddList( targets, domRootNode, "targets" );
    for ( size_t t=0; t<targets.size(); ++t )
        targets[ t ] = targets[ t ].Lowercase();
    if ( targets.end() != std::find( targets.begin(), targets.end(), CORE::CString( "all" ) ) )
        targets.clear();
    cook = domRootNode.GetAttributeValueOrChildValueByName( "cook" ).AsBool( true, true );
    thenConvert = domRootNode.GetAttributeValueOrChildValueByName( "thenConvert" ).AsBool( true, true );
    repairOriginals = domRootNode.GetAttributeValueOrChildValueByName( "repairOriginals" ).AsBool( true, true );
    allowLarge = domRootNode.GetAttributeValueOrChildValueByName( "allowLarge" ).AsBool( false, true );
    acceptErrors = domRootNode.GetAttributeValueOrChildValueByName( "acceptErrors" ).AsBool( false, true );
    caller = domRootNode.GetAttributeValueOrChildValueByName( "caller" ).AsString( CORE::CString::Empty, true );
    machine = domRootNode.GetAttributeValueOrChildValueByName( "machine" ).AsBool( false, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetReprocessRequest::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetReprocessRequest( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetReprocessRequest::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetReprocessRequest";
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
