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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREFILE_H
#define GUCEF_ASSETSTORE_CASSETSTOREFILE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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

/**
 *  One file of a format of a store item (section 4.2 of the design): what the store says about it before it is
 *  downloaded. The name is the name the store gives, which can hold folders ("Textures/T_Wall_D.png"); it is checked
 *  with CAssetStoreSafeNames before anything is written.
 *
 *  Serialized as a DataNode object: "fileId", "name", "size", "digest", "digestAlgorithm" ("sha1", "sha256" or empty
 *  when the store gives none) and "isArchive".
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreFile : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString fileId;
    CORE::CString name;
    UInt64 size;                        /**< in bytes, 0 when not known */
    CORE::CString digest;               /**< lower case hexadecimal, empty when not known */
    CORE::CString digestAlgorithm;      /**< "sha1" or "sha256" */
    bool isArchive;                     /**< zip, 7z, rar, .unitypackage: kept as downloaded, the VFS mounts it */

    CAssetStoreFile( void );
    CAssetStoreFile( const CAssetStoreFile& src );
    virtual ~CAssetStoreFile() GUCEF_VIRTUAL_OVERRIDE;
    CAssetStoreFile& operator=( const CAssetStoreFile& src );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

typedef GUCEF::vector< CAssetStoreFile > TAssetStoreFileVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREFILE_H ? */
