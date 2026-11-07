/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHNOTICE_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHNOTICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHPROVIDERSETTINGS_H
#include "matchprovidermail_CEMailMatchProviderSettings.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHPROVIDERSETTINGS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHPROVIDERSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the provider tells besides the events of CIMatchProvider: a turn that was refused, a participant who signed two
 *  successors of a revision, a reminder, a proposal or a vote that arrived. No addresses and no contents of packages.
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailMatchNotice : public CORE::CICloneable
{
    public:

    CORE::CString matchId;
    UInt32 revision;
    CORE::CString code;                 /**< see CEMailRefusal::code, or PROPOSAL, VOTE, NUDGE */
    CORE::CString description;
    CORE::CString userId;               /**< who it is about, when that is known */
    CORE::CString proposalId;           /**< the proposal a PROPOSAL or VOTE notice is about */

    CEMailMatchNotice( void );
    CEMailMatchNotice( const CEMailMatchNotice& src );
    virtual ~CEMailMatchNotice() GUCEF_VIRTUAL_OVERRIDE;
    CEMailMatchNotice& operator=( const CEMailMatchNotice& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHNOTICE_H ? */
