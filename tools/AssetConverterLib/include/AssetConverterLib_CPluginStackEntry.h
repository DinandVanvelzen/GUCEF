/*
 *  AssetConverterLib: a post-processor or validator named by a manifest, with its settings
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

#ifndef ASSETCONVERTERLIB_CPLUGINSTACKENTRY_H
#define ASSETCONVERTERLIB_CPLUGINSTACKENTRY_H

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
 *  An entry of a post-processor stack or of the validator list of a manifest: the name of the plugin and its settings.
 *  In JSON either the name alone ("NameHints") or an object { "name": ..., "settings": { ... } }.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPluginStackEntry
{
    public:

    typedef GUCEF::vector< CPluginStackEntry > TEntryVector;

    CORE::CString name;
    CORE::CDataNode settings;

    CPluginStackEntry( void );

    explicit CPluginStackEntry( const CORE::CString& pluginName );

    bool Serialize( CORE::CDataNode& node ) const;

    /** From a value node (the name) or an object node */
    bool Deserialize( const CORE::CDataNode& node );

    /** The entries of an array node, in order */
    static bool DeserializeList( const CORE::CDataNode& arrayNode, TEntryVector& entries );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPLUGINSTACKENTRY_H ? */
