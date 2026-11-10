/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_CLINKRECORD_H
#define GUCEF_AUTH_CLINKRECORD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_AUTH_CACCOUNTREF_H
#include "gucefAUTH_CAccountRef.h"
#define GUCEF_AUTH_CACCOUNTREF_H
#endif /* GUCEF_AUTH_CACCOUNTREF_H ? */

#ifndef GUCEF_AUTH_ILINKSIGNER_H
#include "gucefAUTH_ILinkSigner.h"
#define GUCEF_AUTH_ILINKSIGNER_H
#endif /* GUCEF_AUTH_ILINKSIGNER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The statement "this account belongs to this profile", signed by the key of the profile. Possession of the account
 *  was proven by the provider at link time (proofDigest is the digest of that proof), possession of the profile by
 *  the signature. A record that is changed after it was signed no longer verifies.
 *
 *  The signed bytes are the text "gucefAUTH-link-v1" and the fields accountRef, profileKeyId, linkedAtMs, proofDigest,
 *  publicKey and mergedFrom, one per line. The version is the format of the record: a record of a version this code
 *  does not know is kept as it is by the stores but is not used.
 */
class GUCEF_AUTH_EXPORT_CPP CLinkRecord : public CORE::CIDataNodeSerializable
{
    public:

    static const UInt32 CurrentVersion = 1;

    UInt32 version;
    CAccountRef accountRef;
    CORE::CString profileKeyId;
    UInt64 linkedAtMs;              /**< milliseconds since 1970-01-01 UTC */
    CORE::CString proofDigest;      /**< SHA-256 of the provider proof as 64 hex digits, empty when the login had no proof */
    CORE::CString publicKey;        /**< of the profile key, 64 hex digits */
    CORE::CString signature;        /**< Ed25519 over the signed bytes, 128 hex digits */
    CORE::CString mergedFrom;       /**< the profile key id the account was moved from by a merge, empty otherwise */

    CLinkRecord( void );

    CLinkRecord( const CLinkRecord& src );

    virtual ~CLinkRecord();

    CLinkRecord& operator=( const CLinkRecord& src );

    /** A signed record for the account: the profile is the one of the signer */
    static bool Create( ILinkSigner& signer             ,
                        const CAccountRef& accountRef   ,
                        const CORE::CString& proofDigest,
                        UInt64 linkedAtMs               ,
                        CLinkRecord& record             ,
                        const CORE::CString& mergedFrom = CORE::CString::Empty );

    bool IsKnownVersion( void ) const;

    /** The fields have values of the allowed form: a valid account, an id, hex digests and keys of the right length */
    bool IsWellFormed( void ) const;

    /** The bytes the signature is over, for a well formed record */
    bool GetSignedBytes( CORE::CDynamicBuffer& signedBytes ) const;

    /** Signs the record with the signer: profileKeyId and publicKey become those of the signer */
    bool Sign( ILinkSigner& signer );

    /** Well formed, of a known version, and the signature matches the fields under the public key of the record */
    bool VerifySignature( void ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CLinkRecord > TLinkRecordVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CLINKRECORD_H ? */
