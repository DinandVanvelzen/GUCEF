/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#define GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A game or lobby as a listing shows it, whichever provider found it.
 *  The endpoints are URI strings such as tcp://host:port or ws://host:port/path, so listings need no networking
 *  module to be passed around.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CGameListingEntry : public CORE::CIDataNodeSerializable
{
    public:

    enum EKind
    {
        KIND_LOBBY            = 0 ,
        KIND_GAME_IN_PROGRESS = 1
    };
    typedef enum EKind TKind;

    CORE::CString listingId;
    TKind kind;
    CORE::CString name;
    CORE::CString gameId;
    CORE::CString gameVersion;
    CORE::CString mapName;
    CORE::CString modeName;
    UInt32 players;
    UInt32 maxPlayers;
    UInt32 spectators;
    bool passworded;
    CORE::CString hostName;
    CORE::CString::StringVector endpoints;
    CORE::CString backendName;                  /**< the provider that found it */
    CORE::CValueList extra;
    Int32 pingMs;                               /**< -1 when unknown */
    CORE::CDateTime lastSeen;

    CGameListingEntry( void );

    CGameListingEntry( const CGameListingEntry& src );

    virtual ~CGameListingEntry();

    CGameListingEntry& operator=( const CGameListingEntry& src );

    /** Whether the advertised content is the same: everything except the ping and when it was last seen */
    bool HasSameContent( const CGameListingEntry& other ) const;

    /** Whether the two share an endpoint */
    bool SharesEndpointWith( const CGameListingEntry& other ) const;

    bool IsFull( void ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* KindToString( TKind kind );

    static TKind StringToKind( const CORE::CString& kind );

    /**
     *  Takes apart an endpoint URI: scheme://host:port/path
     *
     *  @return false when there is no host and port in it
     */
    static bool ParseEndpoint( const CORE::CString& endpoint ,
                               CORE::CString& scheme         ,
                               CORE::CString& host           ,
                               UInt16& port                  ,
                               CORE::CString& path           );

    static CORE::CString MakeEndpoint( const CORE::CString& scheme, const CORE::CString& host, UInt16 port );
};

/*-------------------------------------------------------------------------*/

/**
 *  A list of listings, the result data of CILobbyService::Browse()
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CGameListingList : public CORE::CICloneable
{
    public:

    typedef GUCEF::vector< CGameListingEntry > TGameListingEntryVector;

    TGameListingEntryVector listings;

    CGameListingList( void );

    CGameListingList( const CGameListingList& src );

    virtual ~CGameListingList();

    CGameListingList& operator=( const CGameListingList& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Gets the list out of a ready future that ended well */
    static bool TryGetFromFuture( const CORE::CFutureResult& future, CGameListingList& list );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H ? */
