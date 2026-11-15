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

#ifndef GUCEF_GUI_CGRIDCOLUMNFIT_H
#define GUCEF_GUI_CGRIDCOLUMNFIT_H

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

/**
 *  How the width of one column of a grid is chosen when the grid fits its columns by policy (the FitMode Fill or
 *  Content of CGridView; see CGridColumnFitter for the computation). The defaults let numbers and dates keep the width
 *  of their content and let text columns shrink and grow.
 *
 *  - MODE_AUTO: numeric and temporal columns never go below their content width; text columns go down to the minimum
 *    and take the room that is left in Fill.
 *  - MODE_CONTENT: always the width of the content (within the minimum and maximum); never grows, never shrinks below it.
 *  - MODE_FIXED: the nominal width (the explicit width of the column, else the default); never grows, never shrinks.
 *  - MODE_FILL: as MODE_AUTO, and the room that is left goes to the MODE_FILL columns only.
 *
 *  minWidth raises the minimum of every mode (0: the smallest width of a grid column); maxWidth caps the nominal width
 *  and the growth (0: no maximum) but never pushes a column below its minimum, so a number wider than maxWidth keeps
 *  its width. When the room is short the columns of the lowest priority shrink first.
 */
class GUCEF_GUI_PUBLIC_CPP CGridColumnFit
{
    public:

    enum EMode
    {
        MODE_AUTO = 0   ,
        MODE_CONTENT    ,
        MODE_FIXED      ,
        MODE_FILL
    };

    EMode mode;             /**< default MODE_AUTO */
    Int32 minWidth;         /**< pixels; 0 for the smallest width of a grid column */
    Int32 maxWidth;         /**< pixels; 0 for no maximum */
    Int32 priority;         /**< the columns of the lowest priority shrink first; default 0 */

    CGridColumnFit( void );

    CGridColumnFit( EMode fitMode, Int32 minimumWidth = 0, Int32 maximumWidth = 0, Int32 fitPriority = 0 );

    bool operator==( const CGridColumnFit& other ) const;

    bool operator!=( const CGridColumnFit& other ) const;

    /** "Auto", "Content", "Fixed" or "Fill" */
    static CString ModeToString( EMode mode );

    /** The mode of a name of ModeToString(), case ignored; false for another text */
    static bool StringToMode( const CString& text, EMode& mode );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDCOLUMNFIT_H ? */
