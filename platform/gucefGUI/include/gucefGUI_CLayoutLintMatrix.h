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

#ifndef GUCEF_GUI_CLAYOUTLINTMATRIX_H
#define GUCEF_GUI_CLAYOUTLINTMATRIX_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CPSEUDOLOCALIZER_H
#include "gucefGUI_CPseudoLocalizer.h"
#define GUCEF_GUI_CPSEUDOLOCALIZER_H
#endif /* GUCEF_GUI_CPSEUDOLOCALIZER_H ? */

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

class CGUIContext;

/**
 *  The conditions a layout is checked under, as an index iteration: window sizes (the form's minimum up to ultrawide),
 *  the render scale, the text scale and pseudo-localised text with a line spacing stress. A test loops over the
 *  combinations: ApplyCombination(), then lets the form fill its data if it wants, then CLayoutLint::Run(); Restore()
 *  puts the context back.
 *
 *  ApplyCombination() sets the logical resolution, the render scale and the display text transform (the matrix's own
 *  CPseudoLocalizer) of the context. The text scale cannot be applied by the matrix, because fonts are registered by the
 *  application at a pixel size: a test that can register its fonts again at a size times GetTextScale() says so with
 *  SetTextScaleSupported(), and only then has combinations with a text scale.
 *
 *  The default combinations: 640x480, 1024x768, 1280x720, 1600x900, 1920x1080 and 3440x1440, each plain and pseudo-localised;
 *  1920x1080 at render scale 2; 1920x1080 pseudo-localised with a line spacing of 1.5; 1920x1080 at text scale 2 when
 *  supported. That is 14 (15) combinations.
 */
class GUCEF_GUI_PUBLIC_CPP CLayoutLintMatrix
{
    public:

    struct SCombination
    {
        UInt32 width;
        UInt32 height;
        UInt32 renderScale;
        Float32 textScale;
        bool pseudoLocalized;
        Float32 lineSpacing;

        SCombination( void );
    };
    typedef struct SCombination TCombination;

    CLayoutLintMatrix( void );

    /** The sizes of the plain and pseudo-localised combinations; the defaults when none are added */
    void ClearSizes( void );
    void AddSize( UInt32 width, UInt32 height );

    /** Leaves out the sizes below the minimum of the form and adds the minimum itself (0: no minimum) */
    void SetMinimumSize( UInt32 width, UInt32 height );

    void SetTextScaleSupported( bool supported );
    bool IsTextScaleSupported( void ) const;

    UInt32 GetCombinationCount( void ) const;

    /** The combination; the first one for an index past the end */
    const TCombination& GetCombination( UInt32 index ) const;

    /** "1920x1080", "1920x1080 pseudo", "1920x1080 render scale 2", "1920x1080 pseudo line spacing 1.5", "1920x1080 text scale 2" */
    CString DescribeCombination( UInt32 index ) const;

    /** Sets the context up for the combination; the first call remembers what to restore */
    void ApplyCombination( UInt32 index, CGUIContext& context );

    /** The logical resolution, render scale and display text transform the context had before the first ApplyCombination() */
    void Restore( CGUIContext& context );

    CPseudoLocalizer& GetPseudoLocalizer( void );

    private:

    void Build( void );

    typedef GUCEF::vector< TCombination > TCombinationVector;
    typedef GUCEF::vector< UInt32 > TUInt32Vector;

    TUInt32Vector m_sizes;          /**< width and height pairs */
    bool m_textScaleSupported;
    TCombinationVector m_combinations;
    CPseudoLocalizer m_pseudoLocalizer;
    bool m_saved;
    UInt32 m_savedWidth;
    UInt32 m_savedHeight;
    UInt32 m_savedRenderScale;
    CIDisplayTextTransform* m_savedTransform;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CLAYOUTLINTMATRIX_H ? */
