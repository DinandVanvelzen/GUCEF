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

#ifndef GUCEF_MULTIPLAY_CGAMELISTINGFILTER_H
#define GUCEF_MULTIPLAY_CGAMELISTINGFILTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#include "gucefMULTIPLAY_CGameListingEntry.h"
#define GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#endif /* GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H ? */

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
 *  Which listings a player wants to see. Empty or default fields do not filter.
 *  An unknown ping passes the ping limit: nothing is hidden because it could not be measured.
 *
 *  Config: text, kind (any, lobby, game), gameId, gameVersion, notFull, noPassword, maxPingMs
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CGameListingFilter : public CORE::CIConfigurable
{
    public:

    enum EKindFilter
    {
        KINDFILTER_ANY             = 0 ,
        KINDFILTER_LOBBY           = 1 ,
        KINDFILTER_GAME_IN_PROGRESS = 2
    };
    typedef enum EKindFilter TKindFilter;

    CORE::CString text;             /**< found in the name, map, mode or host name, without regard to case */
    TKindFilter kind;
    CORE::CString gameId;           /**< compatible games only: the same game id */
    CORE::CString gameVersion;      /**< and the same version */
    bool notFull;
    bool noPassword;
    Int32 maxPingMs;                /**< -1 for no limit */

    CGameListingFilter( void );

    CGameListingFilter( const CGameListingFilter& src );

    virtual ~CGameListingFilter();

    CGameListingFilter& operator=( const CGameListingFilter& src );

    bool Matches( const CGameListingEntry& entry ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CGAMELISTINGFILTER_H ? */
