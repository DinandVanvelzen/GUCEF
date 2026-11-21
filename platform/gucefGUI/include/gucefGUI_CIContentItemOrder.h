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

#ifndef GUCEF_GUI_CICONTENTITEMORDER_H
#define GUCEF_GUI_CICONTENTITEMORDER_H

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
 *  An order that a root of a CContentBrowserForm has of its own: the names of its items do not sort the way a person expects (the
 *  function groups of a pack come in a fixed order, not alphabetically). The form asks the rank of each item it lists in that root;
 *  items of a lower rank come first, items of the same rank by the name (or the column that was clicked). Folders stay before
 *  files. The rank is used when the list is sorted by name, in the list and in the tree. Not owned by the form.
 */
class GUCEF_GUI_PUBLIC_CPP CIContentItemOrder
{
    public:

    CIContentItemOrder( void ) {}
    virtual ~CIContentItemOrder() {}

    /** The rank of an item of the root: 0 for no opinion; a lower rank is listed first */
    virtual Int32 GetRank( const CString& path, bool isDirectory ) const = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CICONTENTITEMORDER_H ? */
