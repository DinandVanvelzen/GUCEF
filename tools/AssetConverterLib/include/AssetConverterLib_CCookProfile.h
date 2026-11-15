/*
 *  AssetConverterLib: a named set of output settings of the cooking
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

#ifndef ASSETCONVERTERLIB_CCOOKPROFILE_H
#define ASSETCONVERTERLIB_CCOOKPROFILE_H

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
 *  A cook profile: a named set of output settings, such as "desktop", "android" or "web". Per texture usage the
 *  texture format ("png" until the compressor plugins exist; later "BC7", "BC5", "ASTC_6x6" and the like), the largest
 *  texture size (0 for no limit), whether mip levels are made, and the LOD bias. Each profile cooks into its own
 *  folder: <out>/<profile>.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CCookProfile
{
    public:

    CORE::CString name;
    CORE::CString textureFormats[ TEXTUREUSAGE_COUNT ];
    UInt32 maxTextureSize;
    bool generateMips;
    Float32 lodBias;

    /** The default profile: "desktop", PNG for every usage, mips, no size limit */
    CCookProfile( void );

    explicit CCookProfile( const CORE::CString& profileName );

    const CORE::CString& GetTextureFormat( TTextureUsage usage ) const;

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

#endif /* ASSETCONVERTERLIB_CCOOKPROFILE_H ? */
