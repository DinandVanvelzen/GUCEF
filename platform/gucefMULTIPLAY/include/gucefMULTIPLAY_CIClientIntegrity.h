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

#ifndef GUCEF_MULTIPLAY_CICLIENTINTEGRITY_H
#define GUCEF_MULTIPLAY_CICLIENTINTEGRITY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

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
 *  What a client tells about itself when it creates or joins a lobby: name -> value, for example a file of the game
 *  and its SHA-256 (CFileHashClientIntegrity). The report travels in the request; the built-in lobby server compares
 *  it with what it expects when it is told to (CGucefLobbyServerSettings::expectedIntegrity) and passes it on to the
 *  members in the launch description (CLaunchMember::integrity): the game decides what a mismatch means.
 *  A report is a claim of the client, a changed client can report what it likes: it raises the bar, it proves nothing.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CIClientIntegrity
{
    public:

    CIClientIntegrity( void );

    virtual ~CIClientIntegrity();

    /** The report: name -> value. False when there is nothing to report */
    virtual bool GetIntegrityReport( CORE::CValueList& report ) = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CICLIENTINTEGRITY_H ? */
