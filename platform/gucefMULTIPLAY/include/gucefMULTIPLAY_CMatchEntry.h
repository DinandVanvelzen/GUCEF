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

#ifndef GUCEF_MULTIPLAY_CMATCHENTRY_H
#define GUCEF_MULTIPLAY_CMATCHENTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CMATCHPREVIEW_H
#include "gucefMULTIPLAY_CMatchPreview.h"
#define GUCEF_MULTIPLAY_CMATCHPREVIEW_H
#endif /* GUCEF_MULTIPLAY_CMATCHPREVIEW_H ? */

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
 *  What a match book knows about one match on this machine: the newest preview and the info, and the local
 *  state of the player that is never shared (what was seen, muted, pinned, archived, since when it is the
 *  player's turn, and the position of the provider in its source). Stored as one data node per match.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchEntry : public CORE::CIDataNodeSerializable
{
    public:

    CMatchPreview preview;
    CMatchInfo info;
    bool hasInfo;                           /**< whether the info is known; a match announced by a preview only has none yet */
    CORE::CString providerName;             /**< the provider that carries the match */
    UInt32 lastSeenRevision;                /**< the newest revision the player has opened */
    bool unseen;                            /**< a revision arrived that the player has not opened */
    bool muted;
    bool pinned;
    bool archived;
    CORE::CString cachedPackagePath;
    Int64 yourTurnSince;                    /**< ms since the Unix epoch when the turn became the player's, 0 when it is not */
    UInt32 notifiedRevision;                /**< the newest revision a your turn event was raised for */
    CORE::CString cursor;                   /**< opaque to the book: the position of the provider (IMAP UID state, ETag) */
    CORE::CString localTitle;               /**< the name the player gave the game; never shared, empty when the player gave none */

    CMatchEntry( void );

    CMatchEntry( const CMatchEntry& src );

    virtual ~CMatchEntry();

    CMatchEntry& operator=( const CMatchEntry& src );

    const CORE::CString& GetMatchId( void ) const;

    /** What a list shows as the name of the game: the player's own name, else the shared title when the info is known; may be empty */
    const CORE::CString& GetDisplayTitle( void ) const;

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

typedef GUCEF::vector< CMatchEntry > TMatchEntryVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHENTRY_H ? */
