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

#ifndef GUCEF_GUI_CCHARTPALETTE_H
#define GUCEF_GUI_CCHARTPALETTE_H

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
 *  The colours of chart series: a categorical palette of eight hues in a fixed order, stepped for dark and
 *  for light surfaces, and the check that a palette keeps neighbouring series apart for readers with and
 *  without a colour vision deficiency. Series take the slots in order; the order is part of the palette,
 *  since the separation is measured between neighbours.
 */
class GUCEF_GUI_PUBLIC_CPP CChartPalette
{
    public:

    typedef GUCEF::vector< CColour > TColourVector;
    typedef GUCEF::vector< CString > TStringVector;

    /** What Validate() measured */
    struct SValidation
    {
        bool passed;
        Float64 worstAdjacentCvdDeltaE;     /**< OKLab Delta E x 100 between neighbours, the worst of simulated protanopia and deuteranopia */
        Float64 worstAdjacentNormalDeltaE;  /**< the same without simulation */
        Float64 minLightness;               /**< OKLCH L */
        Float64 maxLightness;
        Float64 minChroma;                  /**< OKLCH C */
    };
    typedef struct SValidation TValidation;

    /** Separation of neighbours under simulated colour vision deficiency, OKLab Delta E x 100 */
    static const Float64 CvdTarget;

    /** Separation of neighbours under normal vision */
    static const Float64 NormalVisionFloor;

    /** Chroma below which a hue reads as grey */
    static const Float64 ChromaFloor;

    /** The eight default series colours for a dark surface (the default of the SeriesColours property) */
    static const TColourVector& GetDarkPalette( void );

    /** The eight default series colours for a light surface */
    static const TColourVector& GetLightPalette( void );

    /** As the text of the SeriesColours property: one "r,g,b,a" entry per colour */
    static TStringVector ToStringList( const TColourVector& colours );

    /** The entries as colours; false when one is no colour (the valid ones are still added) */
    static bool FromStringList( const TStringVector& entries, TColourVector& colours );

    /**
     *  Measures a palette for a surface: every colour within the lightness band of the surface (OKLCH L 0.48 to
     *  0.67 on dark surfaces, 0.43 to 0.77 on light ones) and at or above the chroma floor; neighbouring slots at
     *  least CvdTarget apart under simulated protanopia and deuteranopia (Machado, Oliveira and Fernandes 2009,
     *  severity 1) and at least NormalVisionFloor apart under normal vision. 'passed' when all of them hold.
     */
    static TValidation Validate( const TColourVector& colours, bool darkSurface );

    /** OKLab Delta E x 100 between two colours; 'simulation' 0 for normal vision, 1 protanopia, 2 deuteranopia, 3 tritanopia */
    static Float64 DeltaE( const CColour& a, const CColour& b, Int32 simulation );

    /** OKLCH lightness and chroma of a colour */
    static void LightnessAndChroma( const CColour& colour, Float64& lightness, Float64& chroma );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CCHARTPALETTE_H ? */
