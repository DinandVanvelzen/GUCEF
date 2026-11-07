/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CTABLEEDITRESULT_H
#define GUCEF_GUI_CTABLEEDITRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a table view model says about an edit of a cell: taken, refused for a reason the user can read (the
 *  message), or refused because the cell is read only. The grid shows the message with the editor.
 */
class GUCEF_GUI_PUBLIC_CPP CTableEditResult
{
    public:

    enum EStatus
    {
        STATUS_ACCEPTED = 0 ,   /**< the value is (or would be) taken */
        STATUS_REFUSED      ,   /**< the value is not valid for the cell; 'message' says why */
        STATUS_READ_ONLY        /**< the cell cannot be changed */
    };

    EStatus status;
    CString message;

    CTableEditResult( void );

    CTableEditResult( EStatus resultStatus, const CString& resultMessage );

    static CTableEditResult Accepted( void );

    static CTableEditResult Refused( const CString& reason );

    static CTableEditResult ReadOnly( void );

    bool IsAccepted( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTABLEEDITRESULT_H ? */
