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

#ifndef GUCEF_ASSETSTORE_CASSETSTORETRANSFER_H
#define GUCEF_ASSETSTORE_CASSETSTORETRANSFER_H

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
 *  One concrete transfer of a download plan (section 4.1): where the bytes come from, how many there are, the digest
 *  they must have and the name the file gets in the destination of the job. The address may be signed and short
 *  lived: 'expiresAtMs' tells when it stops working when the store says so. A transfer is never persisted with the
 *  queue of the download manager, a new plan is made when a job starts again.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreTransfer : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString fileId;               /**< the store file the transfer delivers */
    CORE::CString url;
    UInt64 size;                        /**< in bytes, 0 when not known */
    CORE::CString digest;               /**< lower case hexadecimal, empty when not known */
    CORE::CString digestAlgorithm;      /**< "sha1" or "sha256" */
    CORE::CString targetName;           /**< relative name in the destination, may hold folders; checked before use */
    Int64 expiresAtMs;                  /**< UTC milliseconds since the Unix epoch when the address stops working, 0 when not known */

    CAssetStoreTransfer( void );
    virtual ~CAssetStoreTransfer() GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the address stopped working by the given time, as far as known */
    bool IsExpired( Int64 nowUtcMs ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

typedef GUCEF::vector< CAssetStoreTransfer > TAssetStoreTransferVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTORETRANSFER_H ? */
