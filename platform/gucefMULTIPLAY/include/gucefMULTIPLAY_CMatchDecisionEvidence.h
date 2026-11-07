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

#ifndef GUCEF_MULTIPLAY_CMATCHDECISIONEVIDENCE_H
#define GUCEF_MULTIPLAY_CMATCHDECISIONEVIDENCE_H

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
 *  What proves a decision to someone who was not there: the signed proposal and the signed votes that were cast
 *  on it. The evidence travels with the package that applies the decision. In a match with the BOSS decision
 *  policy the evidence is the signed command of the boss alone (a proposal without votes).
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchDecisionEvidence : public CORE::CIDataNodeSerializable
{
    public:

    typedef GUCEF::vector< CMatchVote > TVoteVector;

    static const UInt32 MaxVotes = CMatchInfo::MaxParticipants;

    CMatchProposal proposal;
    TVoteVector votes;

    CMatchDecisionEvidence( void );

    CMatchDecisionEvidence( const CMatchDecisionEvidence& src );

    virtual ~CMatchDecisionEvidence();

    CMatchDecisionEvidence& operator=( const CMatchDecisionEvidence& src );

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

#endif /* GUCEF_MULTIPLAY_CMATCHDECISIONEVIDENCE_H ? */
