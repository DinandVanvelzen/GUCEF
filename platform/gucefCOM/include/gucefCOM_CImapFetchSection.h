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

#ifndef GUCEF_COM_CIMAPFETCHSECTION_H
#define GUCEF_COM_CIMAPFETCHSECTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CIMAPVALUE_H
#include "gucefCOM_CImapValue.h"
#define GUCEF_COM_CIMAPVALUE_H
#endif /* GUCEF_COM_CIMAPVALUE_H ? */

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

/** A part or section of a message that was fetched with BODY[...] */
class GUCEF_COM_PUBLIC_CPP CImapFetchSection
{
    public:

    CORE::CString name;                 /**< what was in the brackets: "", "HEADER", "TEXT", "1", "1.2", "HEADER.FIELDS (FROM TO)" */
    bool hasOrigin;                     /**< a partial fetch <origin> */
    UInt32 origin;
    bool isNil;                         /**< the server said NIL: there is nothing */
    CORE::CDynamicBuffer data;

    CImapFetchSection( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMAPFETCHSECTION_H ? */
