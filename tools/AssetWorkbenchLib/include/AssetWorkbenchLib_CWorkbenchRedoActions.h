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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHREDOACTIONS_H
#define ASSETWORKBENCHLIB_CWORKBENCHREDOACTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#include "gucefASSETSTORE_CAssetPipelineRequest.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H ? */

#ifndef ASSETCONVERTERLIB_CPACKREDOREQUEST_H
#include "AssetConverterLib_CPackRedoRequest.h"
#define ASSETCONVERTERLIB_CPACKREDOREQUEST_H
#endif /* ASSETCONVERTERLIB_CPACKREDOREQUEST_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The redo actions of the pack index design (section 5): work is done once and the stored result is used until the user asks for it
 *  again. Each action names the step that is done again, the flag of the converter command line and of the job that carries it out,
 *  and what it invalidates and keeps:
 *
 *    - Re-index:      --reindex      the structure and grouping of the index (the analysis with valid keys, the outputs and thumbnails stay)
 *    - Re-analyse:    --reanalyse    the analysis records of the scope and with them the statistics of the index (outputs and thumbnails stay)
 *    - Re-import:     --reimport     the cache entries, the intermediate and the cooked files of the scope (the index structure and the analysis stay)
 *    - Re-convert:    --cook-only    the cooked files of the scope, from the intermediate that stays (index and analysis stay)
 *    - Re-thumbnail:  --rethumbnail  the thumbnails of the scope (everything else stays)
 *
 *  The action is a job: for a pack the workbench has open in the resource view the index steps run in this process (registered in the
 *  jobs list as local jobs); for a pack of the store, and for the import and convert steps, a job of the asset store service (a
 *  pipeline job, which runs on the server in client mode). MakePipelineRequest() makes that request, MakeLocalRedo() the redo request
 *  of the local index job.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchRedoActions
{
    public:

    enum ERedoAction
    {
        REDO_REINDEX = 0    ,
        REDO_REANALYSE      ,
        REDO_REIMPORT       ,
        REDO_RECONVERT      ,
        REDO_RETHUMBNAIL    ,
        REDO_COUNT
    };

    struct SRedoInfo
    {
        const char* id;             /**< "reindex": the console word and the menu action id suffix */
        const char* menuText;       /**< "Re-index" */
        const char* flag;           /**< "--reindex" */
        const char* invalidates;
        const char* keeps;
    };

    static const SRedoInfo& GetInfo( ERedoAction action );

    static bool Parse( const CORE::CString& id, ERedoAction& action );

    /** The id of the menu item that does the action ("Redo:reindex") */
    static CORE::CString GetActionId( ERedoAction action );

    static bool ParseActionId( const CORE::CString& actionId, ERedoAction& action );

    /** Whether the step can run on a folder of the VFS in this process (the index steps); the others need a pack of the store */
    static bool IsLocal( ERedoAction action );

    /** The redo request of the index job of the converter library for the local actions; false for the others */
    static bool MakeLocalRedo( ERedoAction action, ASSETCONVERTER::CPackRedoRequest& redo );

    /** What the job of the action does within the kind of the jobs list: "index", "analyse", "thumbnail", "convert" or "cook" */
    static const char* GetJobSubKind( ERedoAction action );

    /** Whether the local index job has to analyse (the analysis level) for the action */
    static bool NeedsAnalysis( ERedoAction action );

    /** The request of the pipeline for a pack of the store: the operation, the globs of the scope and the flags of the action */
    static void MakePipelineRequest( ERedoAction action, const CORE::CString& packId, const CORE::CString::StringVector& assetGlobs, ASSETSTORE::CAssetPipelineRequest& request );

    /** The question before the action runs: what is done again for which scope, and what that invalidates and keeps */
    static CORE::CString MakeConfirmation( ERedoAction action, const CORE::CString& scopeText, UInt32 assetCount );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHREDOACTIONS_H ? */
