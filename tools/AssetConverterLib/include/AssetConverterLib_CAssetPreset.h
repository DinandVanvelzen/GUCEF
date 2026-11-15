/*
 *  AssetConverterLib: default settings for the assets a glob matches
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

#ifndef ASSETCONVERTERLIB_CASSETPRESET_H
#define ASSETCONVERTERLIB_CASSETPRESET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

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
 *  A preset of a manifest: settings for the assets whose path in the pack matches the glob ('match', CGlobPattern:
 *  '*' within a folder, '**' across folders, letter case ignored), optionally only for one importer. Presets apply top
 *  to bottom and a later one wins per key, as Unity's Preset Manager does.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetPreset
{
    public:

    typedef GUCEF::vector< CAssetPreset > TPresetVector;

    CORE::CString match;
    CORE::CString importer;
    CORE::CDataNode settings;

    CAssetPreset( void );

    bool Matches( const CORE::CString& assetPath, const CORE::CString& importerName ) const;

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

#endif /* ASSETCONVERTERLIB_CASSETPRESET_H ? */
