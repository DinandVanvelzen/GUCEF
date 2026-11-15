/*
 *  AssetConverterLib: what a run would do with one asset of a pack
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

#ifndef ASSETCONVERTERLIB_CASSETPLANENTRY_H
#define ASSETCONVERTERLIB_CASSETPLANENTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One asset of a plan (CAssetPackPlan, CAssetPipeline::PlanPack()): its path in the pack, the importer that would
 *  take it, what the run would do (the action) and why (the reason of the cache comparison, as the activity log of
 *  the run would say it, with its detail), and the effective settings of the asset with the layers they came from.
 *  An import whose reason is "unchanged" does not occur; a cached asset has the reason "unchanged".
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetPlanEntry
{
    public:

    CORE::CString assetPath;
    CORE::CString importer;
    TPlanAction action;
    TActivityReason reason;
    CORE::CString detail;
    CORE::CDataNode settings;                   /**< the effective settings (importer defaults, rules, presets, override) */
    CORE::CString::StringVector layers;         /**< where the settings came from, in order */

    CAssetPlanEntry( void );

    /** The activity action a run that does what the plan says records for the asset */
    TActivityAction GetExpectedActivityAction( void ) const;

    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETPLANENTRY_H ? */
