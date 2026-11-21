/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#ifndef GUCEF_WEB_CREACTFLIGHTROW_H
#define GUCEF_WEB_CREACTFLIGHTROW_H

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

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One row of a React Server Components payload ("flight" data, see CReactFlightPayload): "<id>:<tag><payload>".
 *
 *  - id: the hexadecimal id of the row as written ("0", "1a"); other rows refer to it as "$<id>", "$L<id>", "$@<id>".
 *  - tag: the letters before the payload: empty for a plain model row (JSON), "I" a module reference, "HL" a hint
 *    (H and its hint code), "E" an error, "T" a text, "D" debug information, "W" a console call, and so on.
 *  - text: the payload without the tag: the JSON text of a model row, the characters of a text row.
 *  - json: the payload parsed when it is JSON (a model, module, hint, error or debug row) and the JSON codec is
 *    loaded; hasJson says whether it is.
 */
class GUCEF_WEB_PUBLIC_CPP CReactFlightRow
{
    public:

    CORE::CString id;
    CORE::CString tag;
    CORE::CString text;
    CORE::CDataNode json;
    bool hasJson;

    CReactFlightRow( void );

    /** A model row: no tag, the payload is a JSON value (an element tree, an object, a list, a string) */
    bool IsModel( void ) const;

    /** A text row: tag "T", the payload is the text itself (it is not JSON) */
    bool IsText( void ) const;
};

typedef GUCEF::vector< CReactFlightRow > TReactFlightRowVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CREACTFLIGHTROW_H ? */
