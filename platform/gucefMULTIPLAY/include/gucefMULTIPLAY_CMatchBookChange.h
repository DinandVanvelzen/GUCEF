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

#ifndef GUCEF_MULTIPLAY_CMATCHBOOKCHANGE_H
#define GUCEF_MULTIPLAY_CMATCHBOOKCHANGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CIMATCHPROVIDER_H
#include "gucefMULTIPLAY_CIMatchProvider.h"
#define GUCEF_MULTIPLAY_CIMATCHPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CIMATCHPROVIDER_H ? */

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

enum EMatchBookChangeKind
{
    MATCHBOOKCHANGE_ADDED   = 0 ,
    MATCHBOOKCHANGE_UPDATED = 1 ,   /**< a newer state arrived */
    MATCHBOOKCHANGE_REMOVED = 2 ,
    MATCHBOOKCHANGE_OPENED  = 3 ,   /**< the player opened the match: match alerts for it are cleared */
    MATCHBOOKCHANGE_FLAGS   = 4     /**< seen, muted, pinned or archived changed */
};
typedef enum EMatchBookChangeKind TMatchBookChangeKind;

/*-------------------------------------------------------------------------*/

/** The data of MatchBookChangedEvent */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchBookChange : public CORE::CICloneable
{
    public:

    CORE::CString matchId;
    TMatchBookChangeKind kind;

    CMatchBookChange( void );

    CMatchBookChange( const CORE::CString& id, TMatchBookChangeKind changeKind );

    CMatchBookChange( const CMatchBookChange& src );

    virtual ~CMatchBookChange();

    CMatchBookChange& operator=( const CMatchBookChange& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* GUCEF_MULTIPLAY_CMATCHBOOKCHANGE_H ? */
