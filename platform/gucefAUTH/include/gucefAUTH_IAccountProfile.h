/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_IACCOUNTPROFILE_H
#define GUCEF_AUTH_IACCOUNTPROFILE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CEVENT_H
#include "CEvent.h"
#define GUCEF_CORE_CEVENT_H
#endif /* GUCEF_CORE_CEVENT_H ? */

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The profile of the person that logs in, as the user interface of an account page needs it: the name shown for the
 *  person, the id of the profile (the profile key id of the manager when the tool has one) and the way to follow changes.
 *  The module does not know where the profile lives: gucefMULTIPLAY implements this on its identity providers, and
 *  CAuthSettingsProfile keeps the name in the settings of the manager for tools that have no profile of their own.
 */
class GUCEF_AUTH_EXPORT_CPP IAccountProfile
{
    public:

    IAccountProfile( void );

    virtual ~IAccountProfile();

    /** The name shown for the person; empty when there is none */
    virtual CORE::CString GetDisplayName( void ) = 0;

    /** Changes the name and keeps it; false when the name is not good (empty) or could not be kept */
    virtual bool SetDisplayName( const CORE::CString& displayName ) = 0;

    /** The id of the profile; empty when there is none */
    virtual CORE::CString GetProfileId( void ) = 0;

    /** The notifier that raises GetProfileChangedEvent() when the name or the id changed; NULL for a profile that never changes by itself */
    virtual CORE::CNotifier* GetProfileNotifier( void ) = 0;

    /** The event of GetProfileNotifier() that says the profile changed */
    virtual const CORE::CEvent& GetProfileChangedEvent( void ) const = 0;

    private:

    IAccountProfile( const IAccountProfile& src );
    IAccountProfile& operator=( const IAccountProfile& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_IACCOUNTPROFILE_H ? */
