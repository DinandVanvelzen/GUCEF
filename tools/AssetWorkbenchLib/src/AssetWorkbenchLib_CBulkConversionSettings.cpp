/*
 *  AssetWorkbenchLib: views and controllers of the asset workbench
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

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#ifndef ASSETWORKBENCHLIB_CBULKCONVERSIONSETTINGS_H
#include "AssetWorkbenchLib_CBulkConversionSettings.h"
#define ASSETWORKBENCHLIB_CBULKCONVERSIONSETTINGS_H
#endif /* ASSETWORKBENCHLIB_CBULKCONVERSIONSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CBulkConversionSettings::ScopeSelection = "selection";
const CORE::CString CBulkConversionSettings::ScopeFolder = "folder";
const CORE::CString CBulkConversionSettings::ScopePack = "pack";
const CORE::CString CBulkConversionSettings::ScopeGroup = "group";
const CORE::CString CBulkConversionSettings::ScopeSavedSearch = "savedSearch";
const CORE::CString CBulkConversionSettings::OgreTargetName = "gucef-ogre3d";

/*-------------------------------------------------------------------------*/

CBulkConversionSettings::CBulkConversionSettings( void )
    : CORE::CIDataNodeSerializable()
    , name()
    , scopeKind( ScopePack )
    , packId()
    , folder()
    , groupId()
    , savedSearchId()
    , assets()
    , intermediate( true )
    , ogreRuntime( true )
    , cookProfile()
    , force( false )
    , validateOnly( false )
    , cookOnly( false )
    , analyse( false )
    , assetFilter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBulkConversionSettings::~CBulkConversionSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CBulkConversionSettings::Clear( void )
{GUCEF_TRACE;

    *this = CBulkConversionSettings();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CBulkConversionSettings::GetOperation( void ) const
{GUCEF_TRACE;

    if ( validateOnly )
        return ASSETSTORE::CAssetPipelineOperations::ValidatePack;
    if ( cookOnly )
        return ASSETSTORE::CAssetPipelineOperations::CookPack;
    return ASSETSTORE::CAssetPipelineOperations::ConvertPack;
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionSettings::IsValid( CORE::CString& reason ) const
{GUCEF_TRACE;

    reason.Clear();
    if ( ScopeSavedSearch == scopeKind )
    {
        if ( savedSearchId.IsNULLOrEmpty() )
            reason = "Choose a saved search";
    }
    else
    if ( packId.IsNULLOrEmpty() )
        reason = "Choose the pack of the scope";
    else
    if ( ScopeFolder == scopeKind && folder.IsNULLOrEmpty() )
        reason = "Choose the folder of the pack";
    else
    if ( ( ScopeSelection == scopeKind || ScopeGroup == scopeKind ) && assets.empty() )
        reason = "The " + scopeKind + " has no assets";
    else
    if ( !validateOnly && !cookOnly && !intermediate && !ogreRuntime )
        reason = "Choose at least one target";
    else
    if ( cookOnly && !ogreRuntime )
        reason = "Cooking only needs the runtime files as a target";
    if ( reason.IsNULLOrEmpty() && validateOnly && cookOnly )
        reason = "Validate only and cook only exclude each other";
    return reason.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionSettings::GetScopeGlobs( CORE::CString::StringVector& globs ) const
{GUCEF_TRACE;

    globs.clear();
    if ( ScopeFolder == scopeKind && !folder.IsNULLOrEmpty() )
    {
        CORE::CString normal = folder.ReplaceChar( '\\', '/' );
        while ( !normal.IsNULLOrEmpty() && '/' == normal[ normal.Length() - 1 ] )
            normal = normal.SubstrToIndex( normal.Length() - 1, true );
        globs.push_back( normal + "/*" );
    }
    else
    if ( ScopeSelection == scopeKind || ScopeGroup == scopeKind )
        globs = assets;
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionSettings::MakeRequest( const CORE::CString& pack, const CORE::CString::StringVector& scopeAssets, bool forPlan, ASSETSTORE::CAssetPipelineRequest& request ) const
{GUCEF_TRACE;

    request.Clear();
    request.operation = forPlan ? ASSETSTORE::CAssetPipelineOperations::ConvertPlan : GetOperation();
    request.packId = pack;
    if ( scopeAssets.empty() )
    {
        // all of the pack, or what the filter names of it
        if ( !assetFilter.IsNULLOrEmpty() )
            request.assets.push_back( assetFilter );
    }
    else
    if ( scopeAssets[ 0 ].HasChar( '*' ) >= 0 )
    {
        // the glob of a folder: the filter is the glob inside that folder
        if ( assetFilter.IsNULLOrEmpty() )
            request.assets = scopeAssets;
        else
            request.assets.push_back( scopeAssets[ 0 ].SubstrToIndex( (UInt32) scopeAssets[ 0 ].HasChar( '*' ), true ) + assetFilter );
    }
    else
    {
        // explicit assets: the filter keeps those it matches
        for ( size_t i=0; i<scopeAssets.size(); ++i )
            if ( assetFilter.IsNULLOrEmpty() || CORE::CGlobPattern::IsMatch( assetFilter, scopeAssets[ i ], false ) )
                request.assets.push_back( scopeAssets[ i ] );
        if ( request.assets.empty() )
            return false;
    }

    if ( ASSETSTORE::CAssetPipelineOperations::ValidatePack != request.operation )
    {
        if ( ogreRuntime )
            request.cookTargets.push_back( OgreTargetName );
        if ( !cookProfile.IsNULLOrEmpty() )
            request.profiles.push_back( cookProfile );
    }
    if ( ASSETSTORE::CAssetPipelineOperations::CookPack != request.operation )
        request.reimport = force;
    else
        request.force = false;
    if ( !validateOnly && !cookOnly && !ogreRuntime )
        request.arguments.SetAttribute( "noCook", true );
    if ( analyse )
        request.arguments.SetAttribute( "analyse", true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CBulkConversionSettings::Describe( void ) const
{GUCEF_TRACE;

    CORE::CString scope;
    if ( ScopeSavedSearch == scopeKind )
        scope = "the packs of the saved search " + savedSearchId;
    else
    if ( ScopeFolder == scopeKind )
        scope = "the folder " + folder + " of " + packId;
    else
    if ( ScopeSelection == scopeKind )
        scope = CORE::UInt32ToString( (UInt32) assets.size() ) + " selected assets of " + packId;
    else
    if ( ScopeGroup == scopeKind )
        scope = "the group " + groupId + " of " + packId + " (" + CORE::UInt32ToString( (UInt32) assets.size() ) + " assets)";
    else
        scope = "the pack " + packId;
    CORE::CString what = validateOnly ? CORE::CString( "Validate" ) : ( cookOnly ? CORE::CString( "Cook" ) : CORE::CString( "Convert" ) );
    CORE::CString targets;
    if ( !validateOnly )
    {
        if ( intermediate && !cookOnly )
            targets += "intermediate";
        if ( ogreRuntime )
            targets += ( targets.IsNULLOrEmpty() ? "" : " and " ) + CORE::CString( "Ogre runtime files" );
        if ( !cookProfile.IsNULLOrEmpty() )
            targets += " (profile " + cookProfile + ")";
    }
    return what + " " + scope + ( targets.IsNULLOrEmpty() ? CORE::CString::Empty : " to " + targets ) + ( force ? ", again" : CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionSettings::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "name", name ) && success;
    success = domRootNode.SetAttribute( "scopeKind", scopeKind ) && success;
    success = domRootNode.SetAttribute( "packId", packId ) && success;
    success = domRootNode.SetAttribute( "folder", folder ) && success;
    success = domRootNode.SetAttribute( "groupId", groupId ) && success;
    success = domRootNode.SetAttribute( "savedSearchId", savedSearchId ) && success;
    success = domRootNode.SetValuesOfChildByName( "assets", assets ) && success;
    success = domRootNode.SetAttribute( "intermediate", intermediate ) && success;
    success = domRootNode.SetAttribute( "ogreRuntime", ogreRuntime ) && success;
    success = domRootNode.SetAttribute( "cookProfile", cookProfile ) && success;
    success = domRootNode.SetAttribute( "force", force ) && success;
    success = domRootNode.SetAttribute( "validateOnly", validateOnly ) && success;
    success = domRootNode.SetAttribute( "cookOnly", cookOnly ) && success;
    success = domRootNode.SetAttribute( "analyse", analyse ) && success;
    success = domRootNode.SetAttribute( "assetFilter", assetFilter ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionSettings::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString( name );
    scopeKind = domRootNode.GetAttributeValueOrChildValueByName( "scopeKind" ).AsString( scopeKind );
    packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString( packId );
    folder = domRootNode.GetAttributeValueOrChildValueByName( "folder" ).AsString( folder );
    groupId = domRootNode.GetAttributeValueOrChildValueByName( "groupId" ).AsString( groupId );
    savedSearchId = domRootNode.GetAttributeValueOrChildValueByName( "savedSearchId" ).AsString( savedSearchId );
    domRootNode.GetValuesOfChildByName( "assets", assets );
    intermediate = domRootNode.GetAttributeValueOrChildValueByName( "intermediate" ).AsBool( intermediate, true );
    ogreRuntime = domRootNode.GetAttributeValueOrChildValueByName( "ogreRuntime" ).AsBool( ogreRuntime, true );
    cookProfile = domRootNode.GetAttributeValueOrChildValueByName( "cookProfile" ).AsString( cookProfile );
    force = domRootNode.GetAttributeValueOrChildValueByName( "force" ).AsBool( force, true );
    validateOnly = domRootNode.GetAttributeValueOrChildValueByName( "validateOnly" ).AsBool( validateOnly, true );
    cookOnly = domRootNode.GetAttributeValueOrChildValueByName( "cookOnly" ).AsBool( cookOnly, true );
    analyse = domRootNode.GetAttributeValueOrChildValueByName( "analyse" ).AsBool( analyse, true );
    assetFilter = domRootNode.GetAttributeValueOrChildValueByName( "assetFilter" ).AsString( assetFilter );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CBulkConversionSettings::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CBulkConversionSettings( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CBulkConversionSettings::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CBulkConversionSettings::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETWORKBENCH::CBulkConversionSettings";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CBulkConversionSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CBulkConversionSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
