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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHREDOACTIONS_H
#include "AssetWorkbenchLib_CWorkbenchRedoActions.h"
#define ASSETWORKBENCHLIB_CWORKBENCHREDOACTIONS_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHREDOACTIONS_H ? */

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

const CWorkbenchRedoActions::SRedoInfo g_infos[] =
{
    { "reindex", "Re-index", "--reindex", "the structure and grouping of the pack index", "the analysis with valid keys, the conversion outputs and the thumbnails" },
    { "reanalyse", "Re-analyse", "--reanalyse", "the analysis records of the scope (statistics, bounds, references) and with them the index of the scope", "the conversion outputs and the thumbnails" },
    { "reimport", "Re-import", "--reimport", "the cache entries of the scope, its intermediate and its cooked files", "the structure of the index and the analysis of the source" },
    { "reconvert", "Re-convert", "--cook-only", "the cooked files of the scope", "the intermediate, the index and the analysis" },
    { "rethumbnail", "Re-thumbnail", "--rethumbnail", "the thumbnails of the scope", "everything else" }
};

}

/*-------------------------------------------------------------------------*/

const CWorkbenchRedoActions::SRedoInfo&
CWorkbenchRedoActions::GetInfo( ERedoAction action )
{GUCEF_TRACE;

    return g_infos[ (UInt32) action < (UInt32) REDO_COUNT ? action : REDO_REINDEX ];
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchRedoActions::Parse( const CORE::CString& id, ERedoAction& action )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) REDO_COUNT; ++i )
    {
        if ( id.Equals( g_infos[ i ].id, false ) )
        {
            action = (ERedoAction) i;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchRedoActions::GetActionId( ERedoAction action )
{GUCEF_TRACE;

    return CORE::CString( "Redo:" ) + GetInfo( action ).id;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchRedoActions::ParseActionId( const CORE::CString& actionId, ERedoAction& action )
{GUCEF_TRACE;

    return 0 == actionId.HasSubstr( "Redo:" ) && Parse( actionId.CutChars( 5, true ), action );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchRedoActions::IsLocal( ERedoAction action )
{GUCEF_TRACE;

    return REDO_REINDEX == action || REDO_REANALYSE == action || REDO_RETHUMBNAIL == action;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchRedoActions::MakeLocalRedo( ERedoAction action, ASSETCONVERTER::CPackRedoRequest& redo )
{GUCEF_TRACE;

    redo = ASSETCONVERTER::CPackRedoRequest();
    switch ( action )
    {
        case REDO_REINDEX:      redo.reindex = true; return true;
        case REDO_REANALYSE:    redo.reanalyse = true; return true;
        case REDO_RETHUMBNAIL:  redo.rethumbnail = true; return true;
        default:                return false;
    }
}

/*-------------------------------------------------------------------------*/

const char*
CWorkbenchRedoActions::GetJobSubKind( ERedoAction action )
{GUCEF_TRACE;

    switch ( action )
    {
        case REDO_REINDEX:      return "index";
        case REDO_REANALYSE:    return "analyse";
        case REDO_RETHUMBNAIL:  return "thumbnail";
        case REDO_RECONVERT:    return "cook";
        default:                return "convert";
    }
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchRedoActions::NeedsAnalysis( ERedoAction action )
{GUCEF_TRACE;

    return REDO_REANALYSE == action;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchRedoActions::MakePipelineRequest( ERedoAction action, const CORE::CString& packId, const CORE::CString::StringVector& assetGlobs, ASSETSTORE::CAssetPipelineRequest& request )
{GUCEF_TRACE;

    request.Clear();
    request.packId = packId;
    request.assets = assetGlobs;
    request.operation = REDO_RECONVERT == action ? ASSETSTORE::CAssetPipelineOperations::CookPack : ASSETSTORE::CAssetPipelineOperations::ConvertPack;
    switch ( action )
    {
        case REDO_REINDEX:
            request.arguments.SetAttribute( "indexOnly", true );
            request.arguments.SetAttribute( "reindex", true );
            break;
        case REDO_REANALYSE:
            request.arguments.SetAttribute( "indexOnly", true );
            request.arguments.SetAttribute( "analyse", true );
            request.arguments.SetAttribute( "reanalyse", true );
            break;
        case REDO_RETHUMBNAIL:
            request.arguments.SetAttribute( "indexOnly", true );
            request.arguments.SetAttribute( "rethumbnail", true );
            break;
        case REDO_REIMPORT:
            request.reimport = true;
            break;
        default:
            break;      // the cook of the intermediate that is there
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchRedoActions::MakeConfirmation( ERedoAction action, const CORE::CString& scopeText, UInt32 assetCount )
{GUCEF_TRACE;

    const SRedoInfo& info = GetInfo( action );
    return CORE::CString( info.menuText ) + " " + scopeText + ( assetCount > 0 ? " (" + CORE::UInt32ToString( assetCount ) + " assets)" : CORE::CString::Empty ) +
           ": " + info.invalidates + " are made again. Kept: " + info.keeps + ". The work is a job, see the jobs.";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
