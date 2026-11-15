/*
 *  AssetConverterLib: the size report of the cooked files of a profile
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

#ifndef ASSETCONVERTERLIB_CSIZEREPORT_H
#define ASSETCONVERTERLIB_CSIZEREPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_CREFERENCEGRAPH_H
#include "AssetConverterLib_CReferenceGraph.h"
#define ASSETCONVERTERLIB_CREFERENCEGRAPH_H
#endif /* ASSETCONVERTERLIB_CREFERENCEGRAPH_H ? */


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
 *  sizes.json of a cook profile: the size of every cooked file, and per asset its inclusive size. The files of an
 *  asset are those cooked for it with everything they reference; a file that no other asset has counts once in its
 *  exclusive size, a file that other assets have too counts in its shared size; the inclusive size is both together.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CSizeReport
{
    public:

    struct SAsset
    {
        UInt64 exclusiveSize;
        UInt64 sharedSize;
        UInt32 fileCount;
    };
    typedef GUCEF::map< CORE::CString, UInt64 > TFileSizeMap;
    typedef GUCEF::map< CORE::CString, SAsset > TAssetMap;

    CSizeReport( void );

    /** From the graph, with the sizes of the files in the folder of the profile (VFS) */
    void Build( const CReferenceGraph& graph, const CORE::CString& profileDir );

    /** From the graph and known sizes (files without a size count as 0) */
    void Build( const CReferenceGraph& graph, const TFileSizeMap& fileSizes );

    const TFileSizeMap& GetFileSizes( void ) const;
    const TAssetMap& GetAssets( void ) const;

    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );

    bool Load( const CORE::CString& vfsPath );
    bool Save( const CORE::CString& vfsPath ) const;

    private:

    CORE::CString m_profile;
    TFileSizeMap m_fileSizes;
    TAssetMap m_assets;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CSIZEREPORT_H ? */
