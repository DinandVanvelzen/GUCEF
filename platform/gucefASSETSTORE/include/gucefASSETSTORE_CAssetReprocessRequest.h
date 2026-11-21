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

#ifndef GUCEF_ASSETSTORE_CASSETREPROCESSREQUEST_H
#define GUCEF_ASSETSTORE_CASSETREPROCESSREQUEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAssetPipelineRequest;

/**
 *  The arguments of the reprocessing of a pack of the primary store (the pipeline jobs of CAssetPipelineOperations::IsReprocess()
 *  and the query reprocess_plan): what to do, to which pack or assets, with which filters. It travels as the 'arguments' of a
 *  CAssetPipelineRequest (StartPipelineJob(), RunPipelineQuery()), so the reprocessing needs no new operation on the wire.
 *
 *  The steps, in their order: "redownload" (the original again from the store the pack came from, verified against the stored
 *  one; different files become a new pack, the old pack stays as superseded), "intermediate" (the intermediate resources made
 *  again from the originals, the cache ignored; with 'cook' the target resources after them in the same run) and "target" (the
 *  engine target resources made again from the intermediate); "rollback" exchanges the previous generation of the outputs and
 *  the current one. The operation gives the steps (pack_reprocess takes them from 'steps', by default intermediate and target).
 *
 *  The scope is the pack, or the assets named by 'assets' (paths below the original folder of the pack, "original/" in front is
 *  allowed, or globs) and 'storeAssetIds'; the filters 'onlyStatuses' (the status of the asset in the last report: failed,
 *  skipped, approximated, refused) and 'kinds' (kinds of the catalogue) narrow it further. The caller ("device:<id>",
 *  "mcp:<session>", "admin", ...) and whether it is a machine are set by the host of the service and by the tools, never taken
 *  from a remote client as they are.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetReprocessRequest : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString StepRedownload;      /**< "redownload" */
    static const CORE::CString StepIntermediate;    /**< "intermediate" */
    static const CORE::CString StepTarget;          /**< "target" */
    static const CORE::CString StepRollback;        /**< "rollback" */
    static const CORE::CString TargetOgre;          /**< "ogre": the cooked area gucef-ogre3d */

    CORE::CString operation;                        /**< a name of CAssetPipelineOperations::IsReprocess(), or reprocess_plan */
    CORE::CString packId;                           /**< the store pack id */
    CORE::CString::StringVector steps;              /**< the steps, in order; given by the operation except for pack_reprocess and reprocess_plan */
    CORE::CString::StringVector assets;             /**< paths below the original folder of the pack, or globs; empty: the whole pack */
    CORE::CString::StringVector storeAssetIds;      /**< asset ids of the catalogue, added to the scope */
    CORE::CString::StringSet onlyStatuses;          /**< only the assets with one of these statuses in the last report */
    CORE::CString::StringSet kinds;                 /**< only the assets of these kinds */
    CORE::CString::StringVector targets;            /**< the cook targets ("ogre"); empty: every target of the converter */
    bool cook;                                      /**< "intermediate" also makes the target resources again, default true */
    bool thenConvert;                               /**< "redownload": a new pack gets its intermediate and target resources, default true */
    bool repairOriginals;                           /**< "redownload": verified identical, missing or damaged original files are put back, default true */
    bool allowLarge;                                /**< "redownload": the item goes through the size guard of the store plugin, default false */
    bool acceptErrors;                              /**< the outputs of a run with errors are put in place too, default false */
    CORE::CString caller;                           /**< who asked: "device:<id>", "mcp:<session>", "admin", "workbench" */
    bool machine;                                   /**< the caller is a machine (the MCP role): the rate limits apply */

    CAssetReprocessRequest( void );
    virtual ~CAssetReprocessRequest() GUCEF_VIRTUAL_OVERRIDE;

    void Clear( void );

    /** The steps of an operation: the fixed ones, or those of 'steps' for pack_reprocess and reprocess_plan */
    void GetEffectiveSteps( CORE::CString::StringVector& effective ) const;

    bool HasStep( const CORE::CString& step ) const;

    /** Whether the scope is a part of the pack (assets, asset ids or filters given) */
    bool IsPartial( void ) const;

    /** Whether the request can be carried out; the reason when not */
    bool IsValid( CORE::CString& reason ) const;

    /** From a pipeline request: its operation and pack id, 'assets' and 'cookTargets', and the members of its arguments ("storePackId" stands for the pack id) */
    static CAssetReprocessRequest FromRequest( const CAssetPipelineRequest& request );

    /** A pipeline request that carries this one */
    void ToRequest( CAssetPipelineRequest& request ) const;

    /** A short text of the arguments for the audit: "steps=...;scope=...;targets=..." */
    CORE::CString DescribeForAudit( void ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETREPROCESSREQUEST_H ? */
