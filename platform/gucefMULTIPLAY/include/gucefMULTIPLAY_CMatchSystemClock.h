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

#ifndef GUCEF_MULTIPLAY_CMATCHSYSTEMCLOCK_H
#define GUCEF_MULTIPLAY_CMATCHSYSTEMCLOCK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CIMATCHCLOCK_H
#include "gucefMULTIPLAY_CIMatchClock.h"
#define GUCEF_MULTIPLAY_CIMATCHCLOCK_H
#endif /* GUCEF_MULTIPLAY_CIMATCHCLOCK_H ? */

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

/** The clock of the system */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchSystemClock : public CIMatchClock
{
    public:

    CMatchSystemClock( void );

    virtual ~CMatchSystemClock();

    virtual Int64 GetNowMs( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 GetLocalMinuteOfDay( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** One clock of the system for everything that has no clock of its own */
    static CMatchSystemClock& Instance( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHSYSTEMCLOCK_H ? */
