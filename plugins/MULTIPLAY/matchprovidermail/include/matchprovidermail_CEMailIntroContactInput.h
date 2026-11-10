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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACTINPUT_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACTINPUT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H
#include "matchprovidermail_macros.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H ? */

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

/** What a creator enters for one remote player */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailIntroContactInput
{
    public:

    CORE::CString name;
    CORE::CString address;
    CORE::CString playerCode;           /**< the public key in hex when the creator has it already, empty otherwise */

    CEMailIntroContactInput( void );
    CEMailIntroContactInput( const CORE::CString& contactName, const CORE::CString& contactAddress, const CORE::CString& code = CORE::CString::Empty );
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

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILINTROCONTACTINPUT_H ? */
