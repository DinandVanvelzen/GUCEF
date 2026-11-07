/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CMATCHPREVIEW_H
#define GUCEF_MULTIPLAY_CMATCHPREVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CMATCHINFO_H
#include "gucefMULTIPLAY_CMatchInfo.h"
#define GUCEF_MULTIPLAY_CMATCHINFO_H
#endif /* GUCEF_MULTIPLAY_CMATCHINFO_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#include "gucefMULTIPLAY_CPlayerIdentity.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The small signed statement about the newest state of a match: the only thing that is polled. It says which
 *  revision it is, whose turn it is, the SHA-256 of the package that belongs to the revision and of the package it follows, and is signed
 *  by the issuer: the participant who played the turn (peer to peer) or a service that is trusted for the match.
 *  The signature has the match id as context. The binary form (ToBinary()) is under 1 KB.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchPreview : public CORE::CIDataNodeSerializable
{
    public:

    static const UInt32 MaxSummaryLength = 128;     /**< opaque text for a list row */
    static const UInt32 MaxCurrentUserIds = 8;
    static const UInt32 MaxBinarySize = 1024;

    CORE::CString matchId;
    UInt32 revision;                                /**< counts hand overs, 1 is the start */
    UInt32 turn;
    CORE::CString::StringSet currentUserIds;        /**< a set, so a simultaneous mode can exist */
    TMatchStatus status;
    Int64 deadline;                                 /**< ms since the Unix epoch on a clock the issuer is trusted for, 0 for none */
    CORE::CString packageSha256;                    /**< hex, of the package that belongs to the revision */
    CORE::CString baseSha256;                       /**< hex, of the package this revision follows (the preceding revision), empty for revision 1 */
    CORE::CString summary;
    Int64 lastActivity;                             /**< ms since the Unix epoch */
    CORE::CString issuerKey;                        /**< hex public key of the issuer */
    CORE::CString signature;                        /**< hex */

    CMatchPreview( void );

    CMatchPreview( const CMatchPreview& src );

    virtual ~CMatchPreview();

    CMatchPreview& operator=( const CMatchPreview& src );

    bool IsCurrent( const CORE::CString& userId ) const;

    /** The user id that follows from the issuer key, empty when there is no valid key */
    CORE::CString GetIssuerUserId( void ) const;

    /** Appends the signed fields in the fixed layout (everything but the signature) */
    void WriteSigningBytes( CMatchBinaryWriter& writer ) const;

    void GetSigningBytes( CORE::CDynamicBuffer& bytes ) const;

    /** Sets the issuer key to the one of the identity and signs */
    bool Sign( const CPlayerIdentity& issuer );

    /** Whether the signature is valid for the issuer key given in the preview and this match */
    bool VerifySignature( void ) const;

    /** Whether the signature is valid and the issuer key is the given pinned key */
    bool VerifyWithKey( const CORE::CString& pinnedPublicKeyHex ) const;

    /** Whether the fields are within their limits */
    bool IsWithinLimits( void ) const;

    /** The binary form: the signed bytes and the 64 byte signature */
    bool ToBinary( CORE::CDynamicBuffer& bytes ) const;

    /** Strict reader of the binary form, for bytes from outside; false for anything that is not exactly one preview */
    bool FromBinary( const void* data, UInt32 size );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHPREVIEW_H ? */
