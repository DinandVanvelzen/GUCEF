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

#ifndef GUCEF_GUI_CWIDGETPROPERTYVALUE_H
#define GUCEF_GUI_CWIDGETPROPERTYVALUE_H

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
 *  The value of one property as text (the form layouts, themes and drivers know) that remembers what it
 *  read as a boolean, a number or a colour, so a painter that asks for the same colour every frame
 *  parses it once. Widgets, the compiled theme and the type descriptors keep their values in these.
 *  The readings are the ones CWidget::GetBoolProperty() and its siblings always gave.
 */
class GUCEF_GUI_PUBLIC_CPP CWidgetPropertyValue
{
    public:

    CWidgetPropertyValue( void );

    explicit CWidgetPropertyValue( const CString& text );

    /** The text as given; changing it forgets what was read from it */
    const CString& GetText( void ) const;

    void SetText( const CString& text );

    bool AsBool( void ) const;

    Int32 AsInt( void ) const;

    Float32 AsFloat( void ) const;

    const CColour& AsColour( void ) const;

    /** False when the text is no colour (and empty text is none): 'colour' is then left alone */
    bool TryAsColour( CColour& colour ) const;

    /**
     *  Reads every kind now. For a value that many widgets share (a default of a descriptor, a value of the
     *  theme): it is then never changed by being read, so the widgets of different threads may read it.
     */
    void ReadAll( void ) const;

    private:

    enum EReadFlags
    {
        READ_BOOL   = 1 ,
        READ_INT    = 2 ,
        READ_FLOAT  = 4 ,
        READ_COLOUR = 8
    };

    CString m_text;
    mutable UInt32 m_read;
    mutable bool m_bool;
    mutable Int32 m_int;
    mutable Float32 m_float;
    mutable CColour m_colour;
    mutable bool m_colourValid;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CWIDGETPROPERTYVALUE_H ? */
