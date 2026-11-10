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

#ifndef GUCEF_MULTIPLAY_CMATCHTURNEVENTDATA_H
#define GUCEF_MULTIPLAY_CMATCHTURNEVENTDATA_H

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

/** The data of YourTurnEvent: the match and the revision that made it the player's turn */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchTurnEventData : public CORE::CICloneable
{
    public:

    CORE::CString matchId;
    UInt32 revision;

    CMatchTurnEventData( void );

    CMatchTurnEventData( const CORE::CString& id, UInt32 turnRevision );

    CMatchTurnEventData( const CMatchTurnEventData& src );

    virtual ~CMatchTurnEventData();

    CMatchTurnEventData& operator=( const CMatchTurnEventData& src );

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

#endif /* GUCEF_MULTIPLAY_CMATCHTURNEVENTDATA_H ? */
