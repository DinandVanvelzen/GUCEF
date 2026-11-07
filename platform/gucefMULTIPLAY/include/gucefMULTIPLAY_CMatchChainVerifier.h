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

#ifndef GUCEF_MULTIPLAY_CMATCHCHAINVERIFIER_H
#define GUCEF_MULTIPLAY_CMATCHCHAINVERIFIER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CMATCHPREVIEW_H
#include "gucefMULTIPLAY_CMatchPreview.h"
#define GUCEF_MULTIPLAY_CMATCHPREVIEW_H
#endif /* GUCEF_MULTIPLAY_CMATCHPREVIEW_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHPACKAGE_H
#include "gucefMULTIPLAY_CMatchPackage.h"
#define GUCEF_MULTIPLAY_CMATCHPACKAGE_H
#endif /* GUCEF_MULTIPLAY_CMATCHPACKAGE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

enum EMatchChainResult
{
    MATCHCHAIN_OK                = 0 ,
    MATCHCHAIN_WRONG_MATCH       = 1 ,  /**< the record is of another match than the info */
    MATCHCHAIN_BAD_PREVIEW       = 2 ,  /**< outside its limits or not correctly signed */
    MATCHCHAIN_UNKNOWN_SIGNER    = 3 ,  /**< the issuer key is not the pinned key of a participant */
    MATCHCHAIN_NOT_ON_TURN       = 4 ,  /**< the signer was not the participant whose turn it was */
    MATCHCHAIN_OLD_REVISION      = 5 ,  /**< not newer than what is trusted already */
    MATCHCHAIN_BROKEN_CHAIN      = 6 ,  /**< the revision is not the next one, or the base is not the trusted package */
    MATCHCHAIN_BAD_PACKAGE       = 7    /**< the package is not the one the preview names, or is not consistent */
};
typedef enum EMatchChainResult TMatchChainResult;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Decides whether a signed preview (and the package it names) is the next link of the chain of a match, for
 *  matches where every participant signs what they send and nobody hosts. The preview is accepted when
 *  (a) it is signed by the pinned key of the participant whose turn it was in the newest trusted preview
 *      (the creator for the first revision),
 *  (b) it is the next revision, and it follows the newest trusted package: the signed preview names the SHA-256 of
 *      the trusted package as its base, and the package it is checked with has that base and the preceding revision,
 *  (c) the package is consistent and is the one the signed preview names (sha256).
 *  Whether the game in the package is the result of replaying the turn is for the game to check.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchChainVerifier
{
    public:

    /**
     *  @param trusted the newest preview that is trusted, GUCEF_NULL for the first revision
     *  @param package the package of the preview, GUCEF_NULL to check the preview alone
     *  @param anyParticipantMaySign the signer has to be a participant who takes part as a person, not the one whose turn it was: for the turn
     *         that applies the decision to hand the participant who is up to the computer (the caller has checked the evidence of the decision)
     */
    static TMatchChainResult VerifySuccessor( const CMatchInfo& info        ,
                                              const CMatchPreview* trusted  ,
                                              const CMatchPreview& next     ,
                                              const CMatchPackage* package  ,
                                              CORE::CString& reason         ,
                                              bool anyParticipantMaySign = false );

    /**
     *  Whether the two previews are proof that their issuer signed two different successors of one revision:
     *  the same match, revision and issuer key, both correctly signed, different packages
     */
    static bool IsEquivocation( const CMatchPreview& a, const CMatchPreview& b );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHCHAINVERIFIER_H ? */
