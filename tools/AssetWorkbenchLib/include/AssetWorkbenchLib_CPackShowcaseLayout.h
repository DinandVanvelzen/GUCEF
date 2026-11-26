/*
 *  AssetWorkbenchLib: the arrangement of the models of a generated showcase: a grid grouped by folder with spacing from the bounds
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

#ifndef ASSETWORKBENCHLIB_CPACKSHOWCASELAYOUT_H
#define ASSETWORKBENCHLIB_CPACKSHOWCASELAYOUT_H

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
 *  The arrangement of a generated showcase, as numbers only. The items are the models with the size of their bounds. They are put in
 *  a deterministic order (by folder, then by name, letter case ignored), every folder starts a new row, a row holds at most
 *  'maxColumns' models, and the distance between the middles of two models is the footprint of the biggest model in the arrangement
 *  with a gap. A model much bigger than the typical one (more than 'maxScaleRatio' times the median footprint) is scaled down so that
 *  one outlier does not spread the others. Positions are in the plane X and Z, the models stand on the ground (Y 0), and the whole
 *  arrangement is centred on the origin.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CPackShowcaseLayout
{
    public:

    struct ASSETWORKBENCHLIB_PUBLIC_CPP SItem
    {
        CORE::CString path;         /**< the path of the model in the pack */
        CORE::CString group;        /**< the folder it is grouped under */
        Float32 sizeX, sizeY, sizeZ;/**< the extents of its bounds */
        SItem( void );
    };
    typedef GUCEF::vector< SItem > TItemVector;

    struct SCell
    {
        UInt32 item;                /**< the index in the sorted items */
        Float32 x, z;               /**< where the middle of the model stands */
        Float32 scale;              /**< 1, less for an outlier, more for a small model when minShare asks for it */
        UInt32 row, column;
        UInt32 groupIndex;
        SCell( void );
    };
    typedef GUCEF::vector< SCell > TCellVector;

    struct ASSETWORKBENCHLIB_PUBLIC_CPP SOptions
    {
        UInt32 maxColumns;          /**< 8 */
        Float32 gap;                /**< the gap between two footprints as part of the biggest footprint, 0.35 */
        Float32 maxScaleRatio;      /**< 2.5 */
        Float32 minShare;           /**< 0: a model smaller than this part of the biggest footprint is scaled up to it (at most maxUpscale times), so that small models do not stand lost in big cells */
        Float32 maxUpscale;         /**< 4 */
        SOptions( void );
    };

    CPackShowcaseLayout( void );

    /** Sorts the items into the order of the arrangement; the same items always give the same order */
    static void SortItems( TItemVector& items );
    /** The folder of a path of the pack: everything before the last '/', empty for a file in the root */
    static CORE::CString GetGroupOfPath( const CORE::CString& path );

    /** Arranges the items; they are sorted first, the cells refer to the sorted order. False without items */
    bool Compute( TItemVector& items, const SOptions& options );

    const TCellVector& GetCells( void ) const;
    UInt32 GetRowCount( void ) const;
    UInt32 GetGroupCount( void ) const;
    /** The distance between the middles of two neighbours in a row, and between two rows */
    Float32 GetPitch( void ) const;
    /** The sphere that holds the whole arrangement (the models standing on the ground): the camera frames it */
    void GetBoundingSphere( Float32& centerX, Float32& centerY, Float32& centerZ, Float32& radius ) const;
    /** The footprint, middle and height of the arrangement, for the ground and the camera */
    void GetExtent( Float32& minX, Float32& maxX, Float32& minZ, Float32& maxZ, Float32& maxHeight ) const;

    private:

    TCellVector m_cells;
    UInt32 m_rows;
    UInt32 m_groups;
    Float32 m_pitch;
    Float32 m_minX, m_maxX, m_minZ, m_maxZ, m_maxHeight;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CPACKSHOWCASELAYOUT_H ? */
