/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CEMAILERROREVENTDATA_H
#define GUCEF_COM_CEMAILERROREVENTDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CIEMAILTRANSPORT_H
#include "gucefCOM_CIEMailTransport.h"
#define GUCEF_COM_CIEMAILTRANSPORT_H
#endif /* GUCEF_COM_CIEMAILTRANSPORT_H ? */

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

/** What went wrong with a connection or a command. The description never has credentials or message content in it */
class GUCEF_COM_PUBLIC_CPP CEMailErrorEventData : public CORE::CICloneable
{
    public:

    EMailErrorClass errorClass;
    CORE::CString description;
    UInt32 serverCode;                  /**< the numeric reply of the server (SMTP, 0 when there was none) */
    CORE::CString enhancedCode;         /**< the enhanced status code of an SMTP reply, for example 5.1.1 */

    CEMailErrorEventData( void );
    CEMailErrorEventData( EMailErrorClass errorClass, const CORE::CString& description );
    CEMailErrorEventData( const CEMailErrorEventData& src );
    virtual ~CEMailErrorEventData() GUCEF_VIRTUAL_OVERRIDE;
    CEMailErrorEventData& operator=( const CEMailErrorEventData& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CEMAILERROREVENTDATA_H ? */
