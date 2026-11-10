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

#ifndef GUCEF_MULTIPLAY_CMATCHINFO_H
#define GUCEF_MULTIPLAY_CMATCHINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CMATCHPARTICIPANT_H
#include "gucefMULTIPLAY_CMatchParticipant.h"
#define GUCEF_MULTIPLAY_CMATCHPARTICIPANT_H
#endif /* GUCEF_MULTIPLAY_CMATCHPARTICIPANT_H ? */

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
 *  What a match is, the same for all participants and all providers: which game and ruleset it plays, who takes
 *  part with which pinned key, how decisions are taken and the settings of the match.
 *
 *  settings are key/value pairs that the provider carrying the match defines; the generic layer stores them
 *  and signs them without knowing what they mean (for peer mail, for example, how a turn is transferred and
 *  to whom it is sent). They are part of the signed match record.
 *
 *  Times are milliseconds since the Unix epoch (UTC).
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchInfo : public CORE::CIDataNodeSerializable
{
    public:

    static const UInt32 MaxParticipants = 16;
    static const UInt32 MaxTextLength = 128;            /**< title, game id, game version, ruleset id, backend */
    static const UInt32 MaxContentHashLength = 128;
    static const UInt32 MaxSettings = 32;
    static const UInt32 MaxSettingKeyLength = 64;
    static const UInt32 MaxSettingValueLength = 256;

    typedef CORE::CString::StringMap TSettingsMap;

    CORE::CString matchId;
    CORE::CString title;
    CORE::CString gameId;
    CORE::CString gameVersion;
    CORE::CString rulesetId;
    CORE::CString contentHash;                          /**< of the content packs: a mismatch is found before play */
    CORE::CString backend;                              /**< the name of the provider */
    TMatchMode mode;
    TMatchStatus status;
    TMatchDecisionPolicy decisionPolicy;
    CORE::CString creatorUserId;
    CORE::CString closedByUserId;                       /**< who cancelled the match, empty when it was not cancelled; not signed */
    Int64 createdAt;
    TMatchParticipantVector participants;
    TSettingsMap settings;

    CMatchInfo( void );

    CMatchInfo( const CMatchInfo& src );

    virtual ~CMatchInfo();

    CMatchInfo& operator=( const CMatchInfo& src );

    CMatchParticipant* FindParticipant( const CORE::CString& userId );
    const CMatchParticipant* FindParticipant( const CORE::CString& userId ) const;

    /** The participant with the public key (hex, any case), GUCEF_NULL when there is none */
    const CMatchParticipant* FindParticipantByKey( const CORE::CString& publicKeyHex ) const;

    /** The participants that vote (see CMatchParticipant::IsEligibleVoter()) */
    UInt32 GetEligibleVoterCount( void ) const;

    CORE::CString GetSetting( const CORE::CString& key, const CORE::CString& defaultValue = CORE::CString::Empty ) const;

    void SetSetting( const CORE::CString& key, const CORE::CString& value );

    /**
     *  Appends the fields in the fixed layout that is signed and hashed. Settings go in the order of their keys.
     *  The status is not part of it, it changes during the match
     */
    void WriteSigningBytes( CMatchBinaryWriter& writer ) const;

    void GetSigningBytes( CORE::CDynamicBuffer& bytes ) const;

    /** SHA-256 of the signing bytes in hex: what an acceptance binds itself to */
    CORE::CString GetDigest( void ) const;

    /**
     *  Whether the info can be trusted as a start: a valid match id, the fields within their limits, one to
     *  MaxParticipants participants with distinct user ids and slots, each with the user id of its key, and a
     *  creator who is one of them
     */
    bool IsValid( CORE::CString& reason ) const;

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

#endif /* GUCEF_MULTIPLAY_CMATCHINFO_H ? */
