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

#ifndef GUCEF_MULTIPLAY_CIMATCHPACKAGEVERIFIER_H
#define GUCEF_MULTIPLAY_CIMATCHPACKAGEVERIFIER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHINFO_H
#include "gucefMULTIPLAY_CMatchInfo.h"
#define GUCEF_MULTIPLAY_CMATCHINFO_H
#endif /* GUCEF_MULTIPLAY_CMATCHINFO_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHPACKAGE_H
#include "gucefMULTIPLAY_CMatchPackage.h"
#define GUCEF_MULTIPLAY_CMATCHPACKAGE_H
#endif /* GUCEF_MULTIPLAY_CMATCHPACKAGE_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHDECISIONEVIDENCE_H
#include "gucefMULTIPLAY_CMatchDecisionEvidence.h"
#define GUCEF_MULTIPLAY_CMATCHDECISIONEVIDENCE_H
#endif /* GUCEF_MULTIPLAY_CMATCHDECISIONEVIDENCE_H ? */

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

enum EMatchPackageVerdict
{
    MATCHPACKAGE_VERIFIED = 0 ,     /**< the package is the correct result of the turn on the trusted base */
    MATCHPACKAGE_NO_BASE  = 1 ,     /**< it cannot be judged: the package it follows is not at hand */
    MATCHPACKAGE_REJECTED = 2       /**< it is not a correct result: reason says why */
};
typedef enum EMatchPackageVerdict TMatchPackageVerdict;

GUCEF_MULTIPLAY_EXPORT_CPP const char* MatchPackageVerdictToString( TMatchPackageVerdict verdict );

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The part of judging a turn that only the game can do: replaying the commands of the turn on the package it
 *  follows and checking that the result is exactly the game the new package carries. A provider that carries
 *  matches between participants who sign what they send (peer to peer mail) first checks the signatures and the
 *  chain of revisions (CMatchChainVerifier) and then asks the verifier of the game before it applies a package.
 *
 *  The verifier is implemented by the game and handed to the provider, which does not own it. It is called on the
 *  thread that runs the provider and gets bytes that came from outside: it has to be as hostile-input safe as the
 *  game's own loader.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CIMatchPackageVerifier
{
    public:

    typedef GUCEF::vector< CMatchDecisionEvidence > TEvidenceVector;

    CIMatchPackageVerifier( void );

    virtual ~CIMatchPackageVerifier();

    /**
     *  Judges the package of the next revision
     *
     *  @param info the match as it is known now (participants, settings)
     *  @param trusted the package of the newest trusted revision, GUCEF_NULL when the next package is the first one
     *  @param next the package to judge; its signer, revision, base and SHA-256 were checked already
     *  @param reason receives why the package is rejected, or why the base is missing
     */
    virtual TMatchPackageVerdict Verify( const CMatchInfo& info         ,
                                         const CMatchPackage* trusted   ,
                                         const CMatchPackage& next      ,
                                         CORE::CString& reason          ) = 0;

    /**
     *  The evidence of the decisions (hand a participant to the computer, drop a participant, ...) that the commands
     *  in the package apply. The provider checks each one with CMatchDecisionVerifier and carries out what passed, so
     *  that the participants of the match stay the same on every machine. The default has none.
     *
     *  @return the number of evidence elements added
     */
    virtual UInt32 GetDecisionEvidence( const CMatchInfo& info, const CMatchPackage& package, TEvidenceVector& evidence );

    /**
     *  A short upper case word (letters, digits and underscores, at most 32) that says why the last Verify() rejected a package: the settings do
     *  not make the game, the version is not the one of this game, the game cannot be reproduced. It is the machine readable part of the notice that a
     *  participant sends when it cannot play the first turn. The default is NOT_PLAYABLE
     */
    virtual CORE::CString GetRefusalCode( void ) const;

    private:

    CIMatchPackageVerifier( const CIMatchPackageVerifier& src );
    CIMatchPackageVerifier& operator=( const CIMatchPackageVerifier& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CIMATCHPACKAGEVERIFIER_H ? */
