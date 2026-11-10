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

#ifndef GUCEF_MULTIPLAY_CMATCHALERT_H
#define GUCEF_MULTIPLAY_CMATCHALERT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

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

enum EMatchAlertKind
{
    MATCHALERTKIND_YOUR_TURN  = 0 ,   /**< a turn for the player in one match */
    MATCHALERTKIND_COALESCED  = 1 ,   /**< several matches are waiting: count says how many ("3 games are waiting for you") */
    MATCHALERTKIND_DIGEST     = 2 ,   /**< the daily summary of the matches that are waiting */
    MATCHALERTKIND_INVITE     = 3 ,   /**< an invitation: it never carries a title or a name */
    MATCHALERTKIND_FINISHED   = 4
};
typedef enum EMatchAlertKind TMatchAlertKind;

GUCEF_MULTIPLAY_EXPORT_CPP const char* MatchAlertKindToString( TMatchAlertKind kind );

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a sink shows to the player. It names the match title and the previous mover only, never what is in the
 *  match. The text is for the user interface layer to make (translated, with the count): this carries the facts.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchAlert : public CORE::CIDataNodeSerializable
{
    public:

    TMatchAlertKind kind;
    CORE::CString matchId;          /**< empty for a match alert about several matches */
    UInt32 revision;
    CORE::CString title;            /**< of the match */
    CORE::CString previousMover;    /**< display name of the participant who played the turn before */
    UInt32 count;                   /**< the matches a COALESCED or DIGEST match alert is about, 1 otherwise */
    Int64 createdAt;                /**< ms since the Unix epoch */

    CMatchAlert( void );

    CMatchAlert( const CMatchAlert& src );

    virtual ~CMatchAlert();

    CMatchAlert& operator=( const CMatchAlert& src );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CMatchAlert > TMatchAlertVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHALERT_H ? */
