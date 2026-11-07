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

#ifndef GUCEF_MULTIPLAY_CMATCHACCEPTANCE_H
#define GUCEF_MULTIPLAY_CMATCHACCEPTANCE_H

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
 *  The answer of an invitee to a match record, signed with the key the record pins for it. Joining by a signed
 *  acceptance proves that the invitee holds the key and binds the contact of the record to it. The acceptance
 *  names the digest of the record it answers, so it is no answer to a changed record.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchAcceptance : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString matchId;
    CORE::CString userId;
    bool accepted;                      /**< false for a decline */
    CORE::CString recordDigest;         /**< CMatchInfo::GetDigest() of the record that is answered */
    CORE::CString signature;            /**< hex */

    CMatchAcceptance( void );

    CMatchAcceptance( const CMatchAcceptance& src );

    virtual ~CMatchAcceptance();

    CMatchAcceptance& operator=( const CMatchAcceptance& src );

    /** Makes the acceptance of the record by the identity and signs it */
    bool Sign( const CPlayerIdentity& identity, const CMatchInfo& record, bool accept );

    /** Whether it answers exactly this record and is signed by the pinned key of the participant it names */
    bool Verify( const CMatchInfo& record, CORE::CString& reason ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    private:

    void GetSigningBytes( CORE::CDynamicBuffer& bytes ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHACCEPTANCE_H ? */
