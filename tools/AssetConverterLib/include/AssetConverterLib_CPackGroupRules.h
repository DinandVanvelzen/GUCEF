/*
 *  AssetConverterLib: assigns the function group, sub-group and kind of the resources of a pack
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

#ifndef ASSETCONVERTERLIB_CPACKGROUPRULES_H
#define ASSETCONVERTERLIB_CPACKGROUPRULES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETKINDREGISTRY_H
#include "AssetConverterLib_CAssetKindRegistry.h"
#define ASSETCONVERTERLIB_CASSETKINDREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETKINDREGISTRY_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXPROFILE_H
#include "AssetConverterLib_CPackIndexProfile.h"
#define ASSETCONVERTERLIB_CPACKINDEXPROFILE_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXPROFILE_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXRESOURCE_H
#include "AssetConverterLib_CPackIndexResource.h"
#define ASSETCONVERTERLIB_CPACKINDEXRESOURCE_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXRESOURCE_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

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
 *  The group assignment engine of the pack index (section 2.5 of the design). Every file of a pack goes through the stages:
 *
 *      N  noise: whether the file is listed at all (profile noise patterns, the kind registry's metadata files, hidden and OS files)
 *      P  the profile rules (CPackGroupRule), evaluated first because they are explicit: group and sub-group
 *      K  the kind registry: the kind and the default group
 *      R  refinement from analysis (a skinned model is a character); applied by Refine() when statistics are known
 *      F  folder rules: the sub-group from the folders of the path that are not structural
 *      T  name token rules: the sub-group from the domain tokens of the file name (CPackNamingConventions)
 *
 *  A stage assigns only what an earlier stage left open (F and T only when there is no sub-group, T also when F gave a generic one).
 *  Structural folders are those that repeat the function (Models, Textures, ...), the engine roots (Assets, Resources, ...), the
 *  folders of the profile's grouping hints and the folders that hold (nearly) the whole pack (the folder of the pack itself).
 *  The rules are data and tables; Version is raised whenever the built-in tables change, because it is part of the index key.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackGroupRules
{
    public:

    static const UInt32 Version = 2;                /**< the version of the built-in tables and stages */
    static const UInt32 MaxSubGroupDepth = 2;       /**< folder levels in a sub-group id */

    explicit CPackGroupRules( const CAssetKindRegistry& kinds );

    /** The profile of the pack: its noise patterns, grouping hints and explicit rules */
    void SetProfile( const CPackIndexProfile& profile );

    /** Looks at the paths of the pack (the files that are resources) and finds the folders that hold nearly all of it (the pack's own folder), which are no sub-groups */
    void Prepare( const CORE::CString::StringVector& paths );

    /** Stage N for the generic and the profile patterns; metadata files are the registry's business (CAssetKindRegistry::IsMetadataPath) */
    bool IsNoise( const CORE::CString& path ) const;

    /** Stages P, K, F and T for the record: path must be set; sets name, format, kind, title, storeKind, group, subGroup and groupRule */
    void Assign( CPackIndexResource& resource ) const;

    /** Stage R: with statistics a skinned model is a character; only for records the kind registry placed */
    void Refine( CPackIndexResource& resource ) const;

    /** The hash of the rules that are in effect: the version of the built-in tables and the explicit rules of the profile */
    CORE::CString GetRulesHash( void ) const;

    /** A folder or name as a sub-group segment: lower case, camel case and separators as '-' ("PolygonShops" is "polygon-shops") */
    static CORE::CString NormaliseSegment( const CORE::CString& segment );

    /** Whether the segment (normalised) names the function, an engine root or a source folder: it says nothing about what is in it */
    static bool IsStructuralSegment( const CORE::CString& normalisedSegment );

    private:

    CPackGroupRules( const CPackGroupRules& src );              /**< not implemented */
    CPackGroupRules& operator=( const CPackGroupRules& src );   /**< not implemented */

    typedef GUCEF::vector< CORE::CString::StringVector > TSegmentLists;

    void GetFolderSegments( const CORE::CString& path, CORE::CString::StringVector& segments ) const;
    bool RuleMatches( const CPackGroupRule& rule, const CORE::CString& lowerPath, const CORE::CString::StringVector& tokens, const CORE::CString& kindId ) const;

    const CAssetKindRegistry& m_kinds;
    CPackIndexProfile m_profile;
    GUCEF::vector< size_t > m_ruleOrder;                        /**< indices into the profile rules, the highest priority first */
    CORE::CString::StringSet m_hintSegments;                    /**< normalised segments of the grouping hints */
    CORE::CString::StringVector m_rootSegments;                 /**< the folders that hold nearly the whole pack, outermost first */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKGROUPRULES_H ? */

