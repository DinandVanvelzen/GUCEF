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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#include "AssetWorkbenchLib_CWorkbenchReprocessActions.h"

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

namespace {

typedef ASSETSTORE::CAssetPipelineOperations Op;

const CWorkbenchReprocessActions::SReprocessInfo g_infos[] =
{
    { "redownload", "Download original again", "pack_redownload_original", GUCEF_NULL,
      "the original is downloaded again from its store and verified; the same files change nothing, other files become a new pack (the old one is kept, superseded)" },
    { "intermediate", "Regenerate intermediate", "pack_regenerate_intermediate", "asset_regenerate_intermediate",
      "the intermediate resources are made again from the originals with the importers as they are now, and cooked; the replaced outputs are kept for a rollback" },
    { "target", "Regenerate target", "pack_regenerate_target", "asset_regenerate_target",
      "the engine target resources (gucef-ogre3d) are cooked again from the intermediate; the replaced outputs are kept for a rollback" },
    { "reprocess", "Reprocess", "pack_reprocess", GUCEF_NULL,
      "the steps of the options are done in a row (by default the intermediate and the target resources)" },
    { "rollback", "Roll back regeneration", "reprocess_rollback", GUCEF_NULL,
      "the previous generation of the outputs becomes current again" },
    { "plan", "Plan reprocessing", "reprocess_plan", GUCEF_NULL,
      "what a reprocessing would do now is told; nothing is done" }
};

CORE::CString::StringVector
SplitList( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString::StringVector values;
    const CORE::CString::StringVector parts = text.ParseElements( ',', false );
    for ( size_t i=0; i<parts.size(); ++i )
    {
        const CORE::CString value = parts[ i ].Trim( true ).Trim( false );
        if ( !value.IsNULLOrEmpty() )
            values.push_back( value );
    }
    return values;
}

}

/*-------------------------------------------------------------------------*/

const CWorkbenchReprocessActions::SReprocessInfo&
CWorkbenchReprocessActions::GetInfo( EReprocessAction action )
{GUCEF_TRACE;

    return g_infos[ (UInt32) action < (UInt32) REPROCESS_COUNT ? action : REPROCESS_INTERMEDIATE ];
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchReprocessActions::Parse( const CORE::CString& id, EReprocessAction& action )
{GUCEF_TRACE;

    if ( id.Equals( "all", false ) )
    {
        action = REPROCESS_ALL;
        return true;
    }
    for ( UInt32 i=0; i<(UInt32) REPROCESS_COUNT; ++i )
    {
        if ( id.Equals( g_infos[ i ].id, false ) )
        {
            action = (EReprocessAction) i;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchReprocessActions::GetActionId( EReprocessAction action )
{GUCEF_TRACE;

    return CORE::CString( "Reprocess:" ) + GetInfo( action ).id;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchReprocessActions::ParseActionId( const CORE::CString& actionId, EReprocessAction& action )
{GUCEF_TRACE;

    return 0 == actionId.HasSubstr( "Reprocess:" ) && Parse( actionId.CutChars( 10, true ), action );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchReprocessActions::ParseOptions( const CORE::CString::StringVector& words, ASSETSTORE::CAssetReprocessRequest& options, CORE::CString& problem )
{GUCEF_TRACE;

    for ( size_t i=0; i<words.size(); ++i )
    {
        const CORE::CString& word = words[ i ];
        if ( word.IsNULLOrEmpty() )
            continue;
        if ( 0 != word.HasSubstr( "--", true ) )
        {
            options.assets.push_back( word );
            continue;
        }
        const bool hasValue = i + 1 < words.size();
        if ( "--failed" == word )
        {
            options.onlyStatuses.insert( "failed" );
            options.onlyStatuses.insert( "skipped" );
        }
        else
        if ( "--no-cook" == word )
            options.cook = false;
        else
        if ( "--no-convert" == word )
            options.thenConvert = false;
        else
        if ( "--no-repair" == word )
            options.repairOriginals = false;
        else
        if ( "--allow-large" == word )
            options.allowLarge = true;
        else
        if ( "--accept-errors" == word )
            options.acceptErrors = true;
        else
        if ( ( "--statuses" == word || "--kinds" == word || "--targets" == word || "--steps" == word ) && hasValue )
        {
            const CORE::CString::StringVector values = SplitList( words[ ++i ] );
            for ( size_t v=0; v<values.size(); ++v )
            {
                if ( "--statuses" == word )
                    options.onlyStatuses.insert( values[ v ].Lowercase() );
                else
                if ( "--kinds" == word )
                    options.kinds.insert( values[ v ].Lowercase() );
                else
                if ( "--targets" == word )
                    options.targets.push_back( values[ v ].Lowercase() );
                else
                    options.steps.push_back( values[ v ].Lowercase() );
            }
        }
        else
        {
            problem = "the option " + word + " is not known or has no value (--failed, --statuses a,b, --kinds a,b, --targets ogre, --steps redownload,intermediate,target, "
                      "--no-cook, --no-convert, --no-repair, --allow-large, --accept-errors)";
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchReprocessActions::MakePipelineRequest( EReprocessAction action, const CORE::CString& packId, const ASSETSTORE::CAssetReprocessRequest& options,
                                                 ASSETSTORE::CAssetPipelineRequest& request )
{GUCEF_TRACE;

    const SReprocessInfo& info = GetInfo( action );
    ASSETSTORE::CAssetReprocessRequest spec( options );
    spec.packId = packId;
    spec.caller = "workbench";
    spec.machine = false;
    const bool assets = !spec.assets.empty() || !spec.storeAssetIds.empty();
    spec.operation = assets && GUCEF_NULL != info.assetOperation ? info.assetOperation : info.packOperation;
    spec.ToRequest( request );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchReprocessActions::MakeConfirmation( EReprocessAction action, const CORE::CString& packId, UInt32 assetCount )
{GUCEF_TRACE;

    const SReprocessInfo& info = GetInfo( action );
    return CORE::CString( info.menuText ) + " of pack " + packId + ( assetCount > 0 ? " (" + CORE::UInt32ToString( assetCount ) + " asset(s))" : CORE::CString::Empty ) +
           ": " + info.explanation + ". The work is a job, see the jobs.";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
