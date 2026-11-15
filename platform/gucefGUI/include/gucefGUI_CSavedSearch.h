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
#ifndef GUCEF_GUI_CSAVEDSEARCH_H
#define GUCEF_GUI_CSAVEDSEARCH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CSEARCHQUERY_H
#include "gucefGUI_CSearchQuery.h"
#define GUCEF_GUI_CSEARCHQUERY_H
#endif /* GUCEF_GUI_CSEARCHQUERY_H ? */

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


/** A search that is kept under a name: the query and whether it came with the application (a built-in one cannot be deleted by the user) */
class GUCEF_GUI_PUBLIC_CPP CSavedSearch
{
    public:

    CString id;             /**< what the model knows it by; empty for a search that is not saved yet */
    CString name;
    bool builtIn;
    CSearchQuery query;

    CSavedSearch( void );
};

typedef GUCEF::vector< CSavedSearch > TSavedSearchVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CSAVEDSEARCH_H ? */
