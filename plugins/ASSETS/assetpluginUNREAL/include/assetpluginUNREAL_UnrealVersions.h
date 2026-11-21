/*
 *  assetpluginUNREAL: Named version values and flags of the Unreal package format
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#define GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#include "assetpluginUNREAL_macros.h"
#define GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/**
 *  Version values of the package format. Every branch of the readers is a comparison against one of these named
 *  values; a value is the file version that introduced the layout difference it names.
 */
enum EUnrealUe4Version
{
    UE4_ASSET_REGISTRY_TAGS                             = 112,
    UE4_ADDED_CHUNKID_TO_ASSETDATA_AND_UPACKAGE         = 278,
    UE4_PROPERTY_GUID_IN_PROPERTY_TAG                   = 282,
    UE4_ARRAY_PROPERTY_INNER_TAGS                       = 282,
    UE4_SUMMARY_HAS_BULKDATA_OFFSET                     = 326,
    UE4_CHANGED_CHUNKID_TO_BE_AN_ARRAY_OF_CHUNKIDS      = 326,
    UE4_ENGINE_VERSION_OBJECT                           = 336,
    UE4_ADD_STRING_ASSET_REFERENCES_MAP                 = 384,
    UE4_STRUCT_GUID_IN_PROPERTY_TAG                     = 441,
    UE4_SERIALIZE_TEXT_IN_PACKAGES                      = 459,
    UE4_PACKAGE_SUMMARY_HAS_COMPATIBLE_ENGINE_VERSION  = 444,
    UE4_COOKED_ASSETS_IN_EDITOR_SUPPORT                 = 485,
    UE4_NAME_HASHES_SERIALIZED                          = 504,
    UE4_PRELOAD_DEPENDENCIES_IN_COOKED_EXPORTS         = 507,
    UE4_TEMPLATE_INDEX_IN_COOKED_EXPORTS                = 508,
    UE4_PROPERTY_TAG_SET_MAP_SUPPORT                    = 509,
    UE4_ADDED_SEARCHABLE_NAMES                          = 510,
    UE4_64BIT_EXPORTMAP_SERIALSIZES                     = 511,
    UE4_ADDED_PACKAGE_SUMMARY_LOCALIZATION_ID           = 516,
    UE4_ADDED_PACKAGE_OWNER                             = 518,
    UE4_NON_OUTER_PACKAGE_IMPORT                        = 520,
    UE4_NEWEST_KNOWN                                    = 522,

    UE4_MINIMUM_SUPPORTED                               = 508     /**< the oldest engine release read: 4.14 */
};

/*-------------------------------------------------------------------------*/

enum EUnrealUe5Version
{
    UE5_NONE                                            = 0,
    UE5_INITIAL_VERSION                                 = 1000,
    UE5_NAMES_REFERENCED_FROM_EXPORT_DATA               = 1001,
    UE5_PAYLOAD_TOC                                     = 1002,
    UE5_OPTIONAL_RESOURCES                              = 1003,
    UE5_LARGE_WORLD_COORDINATES                         = 1004,
    UE5_REMOVE_OBJECT_EXPORT_PACKAGE_GUID               = 1005,
    UE5_TRACK_OBJECT_EXPORT_IS_INHERITED                = 1006,
    UE5_SOFTOBJECTPATH_REMOVE_ASSET_PATH_FNAMES         = 1007,
    UE5_ADD_SOFTOBJECTPATH_LIST                         = 1008,
    UE5_DATA_RESOURCES                                  = 1009,
    UE5_SCRIPT_SERIALIZATION_OFFSET                     = 1010,
    UE5_PROPERTY_TAG_EXTENSION_AND_OVERRIDABLE_SERIALIZATION = 1011,
    UE5_PROPERTY_TAG_COMPLETE_TYPE_NAME                 = 1012,
    UE5_ASSETREGISTRY_PACKAGEBUILDDEPENDENCIES          = 1013,
    UE5_METADATA_SERIALIZATION_OFFSET                   = 1014,
    UE5_VERSE_CELLS                                     = 1015,
    UE5_PACKAGE_SAVED_HASH                              = 1016,
    UE5_OS_SUB_OBJECT_SHADOW_SERIALIZATION              = 1017,
    UE5_IMPORT_TYPE_HIERARCHIES                         = 1018,

    UE5_NEWEST_SUPPORTED                                = 1018
};

/*-------------------------------------------------------------------------*/

/** The legacy file version of the summary: 7, 8 and 9 are the values read (stored negative in the file) */
enum EUnrealLegacyFileVersion
{
    UE_LEGACY_NEWEST_SUPPORTED = -9,
    UE_LEGACY_OLDEST_SUPPORTED = -7
};

/*-------------------------------------------------------------------------*/

enum EUnrealPackageFlags
{
    UE_PKG_COOKED                   = 0x00000200,
    UE_PKG_UNVERSIONED_PROPERTIES   = 0x00002000,
    UE_PKG_FILTER_EDITOR_ONLY       = 0x80000000
};

/*-------------------------------------------------------------------------*/

/** Flags of a tag in the complete type name layout */
enum EUnrealPropertyTagFlags
{
    UE_TAGFLAG_HAS_ARRAY_INDEX          = 0x01,
    UE_TAGFLAG_HAS_PROPERTY_GUID        = 0x02,
    UE_TAGFLAG_HAS_PROPERTY_EXTENSIONS  = 0x04,
    UE_TAGFLAG_HAS_BINARY_OR_NATIVE     = 0x08,
    UE_TAGFLAG_BOOL_TRUE                = 0x10,
    UE_TAGFLAG_SKIPPED_SERIALIZE        = 0x20
};

/*-------------------------------------------------------------------------*/

enum EUnrealPropertyTagExtension
{
    UE_TAGEXT_RESERVED_FOR_FUTURE_USE   = 0x01,
    UE_TAGEXT_OVERRIDABLE_INFORMATION   = 0x02
};

/*-------------------------------------------------------------------------*/

/** The magic number at the start and at the end of a package */
static const UInt32 UE_PACKAGE_FILE_TAG = 0x9E2A83C1;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H ? */
