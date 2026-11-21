/*
 *  AssetConverterLib: which tools and versions produced a pack index
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXPRODUCER_H
#define ASSETCONVERTERLIB_CPACKINDEXPRODUCER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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
 *  { tool, toolVersion, schemaWriter, indexKey, settingsHash, plugins, kindRegistryVersion, kindRegistryHash, groupRulesVersion,
 *    groupRulesHash, outputRoot, derivedFromCatalogue? }. These are the inputs of the indexKey except the pack itself; the gate
 *  compares them with what the running build would produce to name the first thing that differs.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexProducer
{
    public:

    typedef GUCEF::map< CORE::CString, UInt32 > TPluginVersionMap;

    CORE::CString tool;
    CORE::CString toolVersion;
    UInt32 schemaWriter;                /**< the indexer version: raised whenever the output for the same input changes */
    CORE::CString indexKey;
    CORE::CString settingsHash;
    TPluginVersionMap plugins;          /**< "recogniser:<name>" and so on, in the form of CAssetCacheEntry::plugins */
    UInt32 kindRegistryVersion;         /**< the number of declarations of the extension table */
    CORE::CString kindRegistryHash;     /**< the hash of the extension table: the content version */
    UInt32 groupRulesVersion;           /**< the version of the built-in rules */
    CORE::CString groupRulesHash;       /**< the hash of the built-in rule data and the profile rules */
    CORE::CString outputRoot;           /**< "pack" or "intermediate": what the paths of the outputs are relative to */
    bool derivedFromCatalogue;

    CPackIndexProducer( void );

    bool operator==( const CPackIndexProducer& other ) const;
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

#endif /* ASSETCONVERTERLIB_CPACKINDEXPRODUCER_H ? */

