/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
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

#ifndef GUCEF_GAMEGUI_CMATCHACTIONEVENTDATA_H
#define GUCEF_GAMEGUI_CMATCHACTIONEVENTDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GAMEGUI_MACROS_H
#include "gucefGAMEGUI_macros.h"
#define GUCEF_GAMEGUI_MACROS_H
#endif /* GUCEF_GAMEGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What the player did to a match in the row menu of CMatchListForm */
enum EMatchAction
{
    MATCHACTION_MUTE      = 0 ,
    MATCHACTION_UNMUTE    = 1 ,
    MATCHACTION_RENAME    = 2 ,
    MATCHACTION_ARCHIVE   = 3 ,
    MATCHACTION_UNARCHIVE = 4 ,
    MATCHACTION_REMOVE    = 5 ,
    MATCHACTION_NUDGE     = 6 ,
    MATCHACTION_RESIGN    = 7 ,
    MATCHACTION_REMATCH   = 8 ,
    MATCHACTION_DETAILS   = 9           /**< the action of the application, see CMatchListForm::SetDetailsAction() */
};
typedef enum EMatchAction TMatchAction;

/*-------------------------------------------------------------------------*/

/** The data of CMatchListForm::MatchActionEvent */
class GUCEF_GAMEGUI_EXPORT_CPP CMatchActionEventData : public CORE::CICloneable
{
    public:

    CORE::CString matchId;
    TMatchAction action;

    CMatchActionEventData( void );

    CMatchActionEventData( const CORE::CString& id, TMatchAction matchAction );

    CMatchActionEventData( const CMatchActionEventData& src );

    virtual ~CMatchActionEventData();

    CMatchActionEventData& operator=( const CMatchActionEventData& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_CMATCHACTIONEVENTDATA_H ? */
