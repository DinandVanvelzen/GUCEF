/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#ifndef GUCEF_COM_CISERVICEADMINHANDLER_H
#define GUCEF_COM_CISERVICEADMINHANDLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** How a service answered an admin command */
enum EServiceAdminStatus
{
    SERVICEADMIN_OK = 0 ,                   /**< done; the result node holds the answer */
    SERVICEADMIN_INVALID_ARGUMENT ,         /**< an argument is missing or wrong; the message says which */
    SERVICEADMIN_NOT_IMPLEMENTED ,          /**< a command of the service's design that it does not have yet */
    SERVICEADMIN_FAILED                     /**< understood and did not succeed; the message says why */
};
typedef enum EServiceAdminStatus TServiceAdminStatus;

/** "OK", "INVALID_ARGUMENT", "NOT_IMPLEMENTED", "FAILED" */
GUCEF_COM_PUBLIC_CPP const char* ServiceAdminStatusToString( TServiceAdminStatus status );

/*-------------------------------------------------------------------------*/

/** What carries out the admin commands of a service: the endpoint authenticates and parses, the handler decides */
class GUCEF_COM_PUBLIC_CPP CIServiceAdminHandler
{
    public:

    typedef CORE::CString::StringMap TArgumentMap;

    CIServiceAdminHandler( void );
    virtual ~CIServiceAdminHandler();

    /**
     *  Carries out the command, on the pulse generator the endpoint was given. 'result' receives the answer for
     *  SERVICEADMIN_OK; 'message' says what is wrong otherwise. Never put a secret in a result unless the command is
     *  meant to show it once (a pairing code)
     */
    virtual TServiceAdminStatus HandleAdminCommand( const CORE::CString& command       ,
                                                    const TArgumentMap& arguments      ,
                                                    CORE::CDataNode& result            ,
                                                    CORE::CString& message             ) = 0;

    private:

    CIServiceAdminHandler( const CIServiceAdminHandler& src );              /**< not implemented */
    CIServiceAdminHandler& operator=( const CIServiceAdminHandler& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CISERVICEADMINHANDLER_H ? */
