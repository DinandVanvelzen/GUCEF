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

#ifndef GUCEF_MULTIPLAY_CMATCHRECORD_H
#define GUCEF_MULTIPLAY_CMATCHRECORD_H

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

#ifndef GUCEF_MULTIPLAY_CMATCHACCEPTANCE_H
#include "gucefMULTIPLAY_CMatchAcceptance.h"
#define GUCEF_MULTIPLAY_CMATCHACCEPTANCE_H
#endif /* GUCEF_MULTIPLAY_CMATCHACCEPTANCE_H ? */

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
 *  The match record: the info of a match as the creator signed it, with the participants and their pinned
 *  keys and the settings. It is sent to every invitee. The signature is made with the key of the creator
 *  and has the match id as context.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchRecord : public CORE::CIDataNodeSerializable
{
    public:

    CMatchInfo info;
    CORE::CString signature;            /**< hex */

    CMatchRecord( void );

    explicit CMatchRecord( const CMatchInfo& matchInfo );

    CMatchRecord( const CMatchRecord& src );

    virtual ~CMatchRecord();

    CMatchRecord& operator=( const CMatchRecord& src );

    /** Signs the info; the identity has to be the creator named in the info, with the key that is pinned for it */
    bool Sign( const CPlayerIdentity& creator );

    /** Whether the info is valid (CMatchInfo::IsValid()) and signed by the pinned key of its creator, for this match */
    bool Verify( CORE::CString& reason ) const;

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

#endif /* GUCEF_MULTIPLAY_CMATCHRECORD_H ? */
