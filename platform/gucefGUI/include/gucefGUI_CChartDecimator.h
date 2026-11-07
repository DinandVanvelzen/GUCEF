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

#ifndef GUCEF_GUI_CCHARTDECIMATOR_H
#define GUCEF_GUI_CCHARTDECIMATOR_H

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
 *  Reduces the points of a series to what a line drawing at a given width can show: per pixel column the
 *  first, the lowest, the highest and the last point, in their original order (the M4 reduction). A polyline
 *  is continuous, so inside one column it covers exactly the span from its lowest to its highest point and it
 *  enters and leaves the column through the first and the last point: a polyline through the kept points
 *  covers the same pixels as one through all points.
 */
class GUCEF_GUI_PUBLIC_CPP CChartDecimator
{
    public:

    /** Indexes of the points to keep, in ascending order */
    typedef GUCEF::vector< UInt32 > TIndexVector;

    /**
     *  Reduces points [first, last) to at most four per pixel column. The column of a point is
     *  ColumnOf( x, xMin, xMax, pixelColumns ), the same rounding the chart uses to place it; points outside
     *  [xMin, xMax] get columns outside [0, pixelColumns) and are reduced the same way. NaN values are kept
     *  and split the columns they are in, so gaps stay gaps. 'keep' is cleared first and keeps its capacity,
     *  so a steady caller does not allocate.
     */
    static void Decimate( const Int64* x, const Float64* values, UInt32 first, UInt32 last,
                          Float64 xMin, Float64 xMax, Int32 pixelColumns, TIndexVector& keep );

    /** The pixel column of x: 0 at xMin, pixelColumns - 1 at xMax, rounded to the nearest column */
    static Int32 ColumnOf( Int64 x, Float64 xMin, Float64 xMax, Int32 pixelColumns );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CCHARTDECIMATOR_H ? */
