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

#ifndef GUCEF_GUI_CGRIDCOLUMNFITTER_H
#define GUCEF_GUI_CGRIDCOLUMNFITTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CGRIDCOLUMNFIT_H
#include "gucefGUI_CGridColumnFit.h"
#define GUCEF_GUI_CGRIDCOLUMNFIT_H
#endif /* GUCEF_GUI_CGRIDCOLUMNFIT_H ? */

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
 *  The widths of the shown columns of a grid that fits its columns by policy (FitMode Fill or Content). Integer
 *  arithmetic on the numbers given, no widget involved, so the results can be computed by hand:
 *
 *  1. per column the minimum and the nominal width by its CGridColumnFit (see there); the nominal width is held within
 *     the minimum and the maximum (a maximum below the minimum is not applied);
 *  2. room to spare and 'fill': the growers (the MODE_FILL columns; when there are none the MODE_AUTO text columns;
 *     when there are none every MODE_AUTO column) get the room in proportion to their nominal widths, rounded down and
 *     capped at their maximum; what is left goes to the growers in column order, each up to its maximum. Room no grower
 *     can take stays free;
 *  3. too little room: the columns shrink by priority, lowest first; within a priority in proportion to the room each
 *     has above its minimum (rounded down), the rest taken in column order; never below the minimum;
 *  4. the total can be larger than the room: the grid then scrolls sideways.
 */
class GUCEF_GUI_PUBLIC_CPP CGridColumnFitter
{
    public:

    /** The smallest width of a column, and the width of a column that has neither an explicit nor a content width */
    enum EDefaults
    {
        MIN_WIDTH = 24      ,
        DEFAULT_WIDTH = 100
    };

    /** What the computation knows of a column */
    struct GUCEF_GUI_PUBLIC_CPP SColumn
    {
        Int32 nominal;          /**< the explicit width of the column (a property or the model), 0 for none */
        Int32 content;          /**< the width its content needs, padding included; 0 when not measured */
        bool numeric;           /**< numbers or dates: in MODE_AUTO never below their content */
        CGridColumnFit fit;

        SColumn( void );
    };
    typedef struct SColumn TColumn;
    typedef GUCEF::vector< TColumn > TColumnVector;
    typedef GUCEF::vector< Int32 > TInt32Vector;

    /** The widths, one per column in the order given; returns their total, which exceeds 'available' when the grid has to scroll */
    static Int32 Fit( const TColumnVector& columns, Int32 available, bool fill, TInt32Vector& widths );

    /** The width the column never goes below */
    static Int32 GetMinimumWidth( const TColumn& column );

    /** The width the column starts from, within its minimum and maximum */
    static Int32 GetNominalWidth( const TColumn& column );

    /** The width the column may grow to; 0 for no limit */
    static Int32 GetMaximumWidth( const TColumn& column );

    /** Whether the column takes room to spare in Fill, given whether any column has MODE_FILL and whether any is a MODE_AUTO text column */
    static bool IsGrower( const TColumn& column, bool anyFill, bool anyAutoText );

    private:

    CGridColumnFitter( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDCOLUMNFITTER_H ? */
