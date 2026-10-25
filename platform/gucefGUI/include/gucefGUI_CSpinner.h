/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#ifndef GUCEF_GUI_CSPINNER_H
#define GUCEF_GUI_CSPINNER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

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
 *  A number the user steps up or down: a click on the right part (the right quarter) adds Step, on the
 *  left part subtracts it; the wheel and Up / Down do the same. Properties: Value, Min, Max, Step,
 *  Decimals, Font, TextColour, BackgroundColour.
 */
class GUCEF_GUI_PUBLIC_CPP CSpinner : public CWidget
{
    public:

    CSpinner( void );

    virtual ~CSpinner();

    virtual bool SetValueUInt32( const UInt32 value );

    virtual bool SetValueInt32( const Int32 value );

    virtual bool SetValueFloat32( const Float32 value );

    virtual UInt32 GetValueUInt32( void ) const;

    virtual Int32 GetValueInt32( void ) const;

    virtual Float32 GetValueFloat32( void ) const;

    /** Adds 'steps' times Step, clamped to Min and Max, as the user would */
    bool StepValue( Int32 steps );

    /** The value as shown, with Decimals digits after the point */
    CString GetValueText( void ) const;

    virtual bool GetNaturalSize( Int32& width, Int32& height ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "Spinner"; }

    static CWidget* Create( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    Float32 Clamp( Float32 value ) const;

    CSpinner( const CSpinner& src );
    CSpinner& operator=( const CSpinner& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CSPINNER_H ? */
