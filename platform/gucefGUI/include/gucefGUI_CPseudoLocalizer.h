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

#ifndef GUCEF_GUI_CPSEUDOLOCALIZER_H
#define GUCEF_GUI_CPSEUDOLOCALIZER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CIDISPLAYTEXTTRANSFORM_H
#include "gucefGUI_CIDisplayTextTransform.h"
#define GUCEF_GUI_CIDISPLAYTEXTTRANSFORM_H
#endif /* GUCEF_GUI_CIDISPLAYTEXTTRANSFORM_H ? */

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
 *  Pseudo-localisation: shows text the way a translation stresses a layout, without a translation. A string is
 *  lengthened by at least the expansion (40 % by default, 100 % for strings of 10 characters or fewer, as text grows
 *  most in short labels), its letters get Latin-1 accents (a -> U+00E1) so that tall glyphs show, and it is put in
 *  brackets with padding (U+00B7 dots; Latin-1 only, because the rasterised fonts are Latin-1), so a cut shows
 *  as a missing "]". Digits and the characters around them are kept: a string without letters (a number) is not
 *  changed at all. The line spacing stress multiplies the line height of every font (1.5 tests WCAG 1.4.12).
 *
 *  Results are kept per source string (at most MaxCacheEntries, then the cache starts again); a change of a setting
 *  clears it.
 */
class GUCEF_GUI_PUBLIC_CPP CPseudoLocalizer : public CIDisplayTextTransform
{
    public:

    enum EDefaults
    {
        DefaultExpansionPercent = 40            ,
        DefaultShortStringExpansionPercent = 100 ,
        DefaultShortStringLength = 10           ,
        MaxCacheEntries = 4096
    };

    CPseudoLocalizer( void );

    virtual ~CPseudoLocalizer();

    /** CIDisplayTextTransform: the pseudo-localised text */
    virtual CString TransformText( const CString& text ) GUCEF_VIRTUAL_OVERRIDE;

    virtual Float32 GetLineSpacing( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The transform without the cache, for one string */
    CString Transform( const CString& text ) const;

    void SetExpansionPercent( UInt32 percent );
    UInt32 GetExpansionPercent( void ) const;

    void SetShortStringExpansionPercent( UInt32 percent );
    UInt32 GetShortStringExpansionPercent( void ) const;

    /** Strings of at most this many characters get the short string expansion */
    void SetShortStringLength( UInt32 length );
    UInt32 GetShortStringLength( void ) const;

    void SetAccents( bool accents );
    bool GetAccents( void ) const;

    void SetBrackets( bool brackets );
    bool GetBrackets( void ) const;

    /** The line spacing stress, 1 (none) by default; values below 1 are taken as 1 */
    void SetLineSpacing( Float32 lineSpacing );

    void ClearCache( void );

    private:

    typedef GUCEF::map< CString, CString > TTextMap;

    UInt32 m_expansionPercent;
    UInt32 m_shortStringExpansionPercent;
    UInt32 m_shortStringLength;
    bool m_accents;
    bool m_brackets;
    Float32 m_lineSpacing;
    TTextMap m_cache;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CPSEUDOLOCALIZER_H ? */
