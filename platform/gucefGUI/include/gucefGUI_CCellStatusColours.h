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

#ifndef GUCEF_GUI_CCELLSTATUSCOLOURS_H
#define GUCEF_GUI_CCELLSTATUSCOLOURS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

#ifndef GUCEF_GUI_CTABLECELLFORMATTER_H
#include "gucefGUI_CTableCellFormatter.h"
#define GUCEF_GUI_CTABLECELLFORMATTER_H
#endif /* GUCEF_GUI_CTABLECELLFORMATTER_H ? */

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
 *  The colour tokens of the statuses of grid cells (SCellContent::EStatus): one colour per status, read from the
 *  StatusColours property of a grid ("name=r,g,b,a" per entry of the string list, names as StatusToString() gives them)
 *  over the defaults. STATUS_NORMAL has no colour of its own: the text colour of the grid.
 *
 *  Defaults: muted 150,150,156; attention 240,176,72; negative 232,110,100; positive 110,200,130; estimate 150,190,240;
 *  unknown 160,160,168; link 120,170,255 (all opaque).
 */
class GUCEF_GUI_PUBLIC_CPP CCellStatusColours
{
    public:

    /** The default colours */
    CCellStatusColours( void );

    /**
     *  Sets the colours of the entries of the text (a StringList property value: entries separated by '\n'); statuses the
     *  text does not name keep their colour. False when an entry is not "name=colour" with a known name and a colour CColour
     *  reads; the good entries are applied all the same
     */
    bool Parse( const CString& text );

    /** Every status with its colour, as Parse() reads it */
    CString ToString( void ) const;

    /** The colour of a status; false for STATUS_NORMAL and for a value that is not a status */
    bool GetColour( SCellContent::EStatus status, CColour& colour ) const;

    /** Sets the colour of a status; STATUS_NORMAL and values that are not a status are ignored */
    void SetColour( SCellContent::EStatus status, const CColour& colour );

    /** Back to the default colours */
    void Reset( void );

    static bool GetDefaultColour( SCellContent::EStatus status, CColour& colour );

    /** "normal", "muted", "attention", "negative", "positive", "estimate", "unknown", "link"; empty for a value that is not a status */
    static const char* StatusToString( SCellContent::EStatus status );

    /** The status of a name of StatusToString(), case ignored */
    static bool StringToStatus( const CString& name, SCellContent::EStatus& status );

    private:

    CColour m_colours[ SCellContent::STATUS_COUNT ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CCELLSTATUSCOLOURS_H ? */
