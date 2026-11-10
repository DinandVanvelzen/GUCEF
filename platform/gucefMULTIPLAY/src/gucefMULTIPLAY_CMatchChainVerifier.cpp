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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefMULTIPLAY_CMatchChainVerifier.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

TMatchChainResult
CMatchChainVerifier::VerifySuccessor( const CMatchInfo& info        ,
                                      const CMatchPreview* trusted  ,
                                      const CMatchPreview& next     ,
                                      const CMatchPackage* package  ,
                                      CORE::CString& reason         ,
                                      bool anyParticipantMaySign    )
{GUCEF_TRACE;

    if ( next.matchId != info.matchId || ( GUCEF_NULL != trusted && trusted->matchId != info.matchId ) )
    {
        reason = "The record is of another match";
        return MATCHCHAIN_WRONG_MATCH;
    }
    if ( !next.VerifySignature() )
    {
        reason = "The preview is outside its limits or not correctly signed";
        return MATCHCHAIN_BAD_PREVIEW;
    }
    const CMatchParticipant* signer = info.FindParticipantByKey( next.issuerKey );
    if ( GUCEF_NULL == signer )
    {
        reason = "The preview is signed by a key that is not pinned for the match";
        return MATCHCHAIN_UNKNOWN_SIGNER;
    }

    if ( signer->IsSpectator() )
    {
        reason = "A spectator holds no turn";
        return MATCHCHAIN_NOT_ON_TURN;
    }
    for ( CORE::CString::StringSet::const_iterator holder = next.currentUserIds.begin(); holder != next.currentUserIds.end(); ++holder )
    {
        const CMatchParticipant* named = info.FindParticipant( *holder );
        if ( GUCEF_NULL != named && named->IsSpectator() )
        {
            reason = "The preview gives the turn to a spectator";
            return MATCHCHAIN_BAD_PREVIEW;
        }
    }

    if ( GUCEF_NULL == trusted )
    {
        if ( signer->userId != info.creatorUserId )
        {
            reason = "The first revision is not by the creator";
            return MATCHCHAIN_NOT_ON_TURN;
        }
        if ( 1 != next.revision || !next.baseSha256.IsNULLOrEmpty() )
        {
            reason = "The first trusted revision has to be revision 1 and has no base";
            return MATCHCHAIN_BROKEN_CHAIN;
        }
    }
    else
    {
        if ( next.revision <= trusted->revision )
        {
            reason = "The revision is not newer than the trusted one";
            return MATCHCHAIN_OLD_REVISION;
        }
        if ( anyParticipantMaySign ? !signer->IsEligibleVoter() : !trusted->IsCurrent( signer->userId ) )
        {
            reason = "The signer was not the participant whose turn it was";
            return MATCHCHAIN_NOT_ON_TURN;
        }
        if ( next.revision != trusted->revision + 1 || next.baseSha256 != trusted->packageSha256 )
        {
            reason = "The revision does not follow the trusted one";
            return MATCHCHAIN_BROKEN_CHAIN;
        }
    }

    if ( GUCEF_NULL != package )
    {
        if ( !package->IsConsistent() || package->sha256 != next.packageSha256 || package->revision != next.revision )
        {
            reason = "The package is not the one the preview names";
            return MATCHCHAIN_BAD_PACKAGE;
        }
        if ( GUCEF_NULL == trusted )
        {
            if ( 0 != package->baseRevision || !package->baseSha256.IsNULLOrEmpty() )
            {
                reason = "The first package has no base";
                return MATCHCHAIN_BROKEN_CHAIN;
            }
        }
        else
        if ( package->baseRevision != trusted->revision || package->baseSha256 != trusted->packageSha256 )
        {
            reason = "The package does not follow the trusted package";
            return MATCHCHAIN_BROKEN_CHAIN;
        }
    }
    return MATCHCHAIN_OK;
}

/*-------------------------------------------------------------------------*/

bool
CMatchChainVerifier::IsEquivocation( const CMatchPreview& a, const CMatchPreview& b )
{GUCEF_TRACE;

    return a.matchId == b.matchId && a.revision == b.revision && a.issuerKey.Lowercase() == b.issuerKey.Lowercase() &&
           a.packageSha256 != b.packageSha256 && a.VerifySignature() && b.VerifySignature();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
