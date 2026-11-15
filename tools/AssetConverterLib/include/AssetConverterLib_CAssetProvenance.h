/*
 *  AssetConverterLib: where an asset of the intermediate came from
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

#ifndef ASSETCONVERTERLIB_CASSETPROVENANCE_H
#define ASSETCONVERTERLIB_CASSETPROVENANCE_H

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
 *  The provenance of an asset, for the sidecar: the pack, the asset path in the pack, the source format and its
 *  version, the identity the source gives it (a Unity GUID, a Godot uid, an Unreal object path; the path when there is
 *  none), the engine the pack was made for ("unity", "godot", "unreal", empty for loose files), the importer that read
 *  it with its version, and the version of the converter library.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetProvenance
{
    public:

    CORE::CString packId;
    CORE::CString assetPath;
    CORE::CString sourceFormat;
    CORE::CString sourceFormatVersion;
    CORE::CString sourceId;
    CORE::CString sourceEngine;
    CORE::CString importerName;
    UInt32 importerVersion;
    UInt32 converterVersion;

    CAssetProvenance( void );

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

#endif /* ASSETCONVERTERLIB_CASSETPROVENANCE_H ? */
