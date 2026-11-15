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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREFORMAT_H
#define GUCEF_ASSETSTORE_CASSETSTOREFORMAT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETSTOREFILE_H
#include "gucefASSETSTORE_CAssetStoreFile.h"
#define GUCEF_ASSETSTORE_CASSETSTOREFILE_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREFILE_H ? */
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
 *  One format in which a store offers an item (section 4.2): glTF, FBX, OBJ, USD, a version of Unreal Engine, Unity,
 *  with the group it belongs to ("3d-exchange", "engine", "source", ...) as the store names it, and its files.
 *
 *  Serialized as a DataNode object: "formatId", "name", "group" and "files" (an array of file objects).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreFormat : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString formatId;
    CORE::CString name;
    CORE::CString group;
    TAssetStoreFileVector files;

    CAssetStoreFormat( void );
    virtual ~CAssetStoreFormat() GUCEF_VIRTUAL_OVERRIDE;

    /** The file with the id, GUCEF_NULL when the format has none */
    const CAssetStoreFile* FindFile( const CORE::CString& fileId ) const;

    /** The sum of the known file sizes */
    UInt64 GetTotalSize( void ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

typedef GUCEF::vector< CAssetStoreFormat > TAssetStoreFormatVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREFORMAT_H ? */
