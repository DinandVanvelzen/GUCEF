/*
 *  AssetWorkbenchLib: the arrangement of the views of a stage: one view or several, each with its own camera and display
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHVIEWLAYOUT_H
#define ASSETWORKBENCHLIB_CWORKBENCHVIEWLAYOUT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How the surface of a CWorkbenchStage is divided into views: a list of cells, each a rectangle of the surface (as fractions of
 *  its width and height, the origin at the top left) with a camera and what it displays. It is plain data: the stage makes a
 *  viewport and a camera for every cell. A cell is either a free perspective view (the camera is the orbit controller of the
 *  application) or an orthographic view from one of the three fixed directions, which looks at the point the controller looks at
 *  and has an extent that follows the bounds only (so the orthographic views stay put while the perspective view zooms).
 *
 *  Made here: a single view and the four view layout (one perspective view with the front, the side and the top view in a grid of
 *  two by two). Any arrangement can be built with AddCell(), for a layout of two views or one that the user composes.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchViewLayout
{
    public:

    enum EProjection
    {
        PROJECTION_PERSPECTIVE = 0  ,
        PROJECTION_ORTHOGRAPHIC
    };

    /** The side an orthographic cell looks from; a perspective cell is free: its camera is the controller */
    enum EDirection
    {
        DIRECTION_FREE = 0  ,
        DIRECTION_FRONT     ,
        DIRECTION_SIDE      ,
        DIRECTION_TOP
    };

    struct SCell
    {
        CORE::CString name;
        Float32 left;           /**< the part of the width of the surface where the cell starts, 0 to 1 */
        Float32 top;
        Float32 width;          /**< the part of the width the cell takes */
        Float32 height;
        EProjection projection;
        EDirection direction;
        bool showGrid;          /**< what the view draws of the grid, the axes and the bounding box of the model */
        bool showAxes;
        bool showBounds;

        SCell( void );
    };

    typedef GUCEF::vector< SCell > TCellVector;

    CWorkbenchViewLayout( void );

    /** The name of the layout as the view list of the application shows it */
    const CORE::CString& GetName( void ) const;
    void SetName( const CORE::CString& name );

    void Clear( void );

    /** Adds a cell; the index of it */
    UInt32 AddCell( const SCell& cell );

    UInt32 GetCellCount( void ) const;
    const SCell& GetCell( UInt32 index ) const;

    /** What the view draws; false for a cell that is not there */
    bool SetCellDisplay( UInt32 index, bool showGrid, bool showAxes, bool showBounds );

    /** One perspective view that takes the whole surface */
    static CWorkbenchViewLayout MakeSingle( void );

    /** The perspective view top left, the front view top right, the side view bottom left and the top view bottom right */
    static CWorkbenchViewLayout MakeFourViews( void );

    bool operator==( const CWorkbenchViewLayout& other ) const;
    bool operator!=( const CWorkbenchViewLayout& other ) const;

    private:

    CORE::CString m_name;
    TCellVector m_cells;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHVIEWLAYOUT_H ? */
