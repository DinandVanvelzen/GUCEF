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

#ifndef GUCEF_GUI_CIGRIDHELPPROVIDER_H
#define GUCEF_GUI_CIGRIDHELPPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

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

class CGridView;

/**
 *  The help topics of the parts of a grid, asked only when a tooltip or a help key wants them (CGridView::SetHelpProvider()):
 *  the topic of a column title and the topic of a cell. A topic is an id of the help source of the application (a document,
 *  or a brief that may name one). An empty answer: the part has no topic of its own.
 */
class GUCEF_GUI_PUBLIC_CPP CIGridHelpProvider
{
    public:

    CIGridHelpProvider( void );

    virtual ~CIGridHelpProvider();

    /** The topic of the title of the column of the data 'column' */
    virtual CString GetColumnHelpTopic( const CGridView& grid, UInt32 column ) const = 0;

    /** The topic of a cell: 'row' in the order of the grid, 'column' of the data */
    virtual CString GetCellHelpTopic( const CGridView& grid, UInt32 row, UInt32 column ) const = 0;

    private:

    CIGridHelpProvider( const CIGridHelpProvider& src );
    CIGridHelpProvider& operator=( const CIGridHelpProvider& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CIGRIDHELPPROVIDER_H ? */
