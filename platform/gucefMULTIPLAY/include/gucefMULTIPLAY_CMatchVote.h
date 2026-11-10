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

#ifndef GUCEF_MULTIPLAY_CMATCHVOTE_H
#define GUCEF_MULTIPLAY_CMATCHVOTE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CMATCHPROPOSAL_H
#include "gucefMULTIPLAY_CMatchProposal.h"
#define GUCEF_MULTIPLAY_CMATCHPROPOSAL_H
#endif /* GUCEF_MULTIPLAY_CMATCHPROPOSAL_H ? */

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
 *  The answer of a participant to a proposal, signed. It names the digest of the proposal it answers, so a vote
 *  for one proposal is no vote for another, and the match id is the context of the signature.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchVote : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString matchId;
    CORE::CString proposalId;
    CORE::CString proposalDigest;
    CORE::CString voterUserId;
    bool yes;
    Int64 createdAt;
    CORE::CString signature;            /**< hex */

    CMatchVote( void );

    CMatchVote( const CMatchVote& src );

    virtual ~CMatchVote();

    CMatchVote& operator=( const CMatchVote& src );

    void WriteSigningBytes( CMatchBinaryWriter& writer ) const;

    /** Makes the vote of the identity on the proposal and signs it */
    bool Sign( const CPlayerIdentity& voter, const CMatchProposal& proposal, bool voteYes, Int64 now );

    bool VerifySignature( const CMatchInfo& info ) const;

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

#endif /* GUCEF_MULTIPLAY_CMATCHVOTE_H ? */
