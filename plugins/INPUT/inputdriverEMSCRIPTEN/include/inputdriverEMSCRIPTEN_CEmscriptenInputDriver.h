/*
 *  inputdriverEMSCRIPTEN: GUCEF input driver for web browsers: mouse, wheel, keys and text of a page
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

#ifndef GUCEF_INPUTDRIVEREMSCRIPTEN_CEMSCRIPTENINPUTDRIVER_H
#define GUCEF_INPUTDRIVEREMSCRIPTEN_CEMSCRIPTENINPUTDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_INPUT_CINPUTDRIVER_H
#include "gucefINPUT_CInputDriver.h"
#define GUCEF_INPUT_CINPUTDRIVER_H
#endif /* GUCEF_INPUT_CINPUTDRIVER_H ? */

#ifndef GUCEF_INPUTDRIVEREMSCRIPTEN_MACROS_H
#include "inputdriverEMSCRIPTEN_macros.h"
#define GUCEF_INPUTDRIVEREMSCRIPTEN_MACROS_H
#endif /* GUCEF_INPUTDRIVEREMSCRIPTEN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUTDRIVEREMSCRIPTEN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CEmscriptenInputContext;

/**
 *  Input driver "EMSCRIPTEN": the input of a web page as gucefINPUT devices, a mouse and a keyboard with id 0.
 *
 *  A context reads the input of one canvas, named by the context parameter "WINDOW" (a CSS selector, "#canvas"
 *  when not given; the window driver "Emscripten" gives it as its WINDOW property):
 *  - mouse buttons and movement over the canvas, in pixels of its drawing buffer (also when the page shows it
 *    larger or smaller); a button let go outside the canvas is let go as well
 *  - the wheel in notches, up and right positive
 *  - keys of the whole page by their position on the keyboard (the "code" of the browser), and the text they type
 *    as UTF-8
 *  The browser does not get to act on keys the driver reads (scrolling, the browser's own full screen ...), except
 *  for keys with Ctrl or Meta held and F5 and F12. When the page loses the focus, held keys and buttons are let go.
 */
class INPUTDRIVEREMSCRIPTEN_PUBLIC_CPP CEmscriptenInputDriver : public INPUT::CInputDriver
{
    public:

    static const CORE::CString DriverName;

    CEmscriptenInputDriver( void );

    virtual ~CEmscriptenInputDriver();

    virtual const CORE::TVersion* GetVersion( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The key code of a key by the "code" the browser gives it ("KeyA", "ArrowUp" ...); KEYCODE_UNASSIGNED when unknown */
    static INPUT::KeyCode KeyCodeFromDomCode( const char* domCode );

    /** The gucefINPUT button index of a mouse button by the index the browser gives it (0 left, 1 middle, 2 right ...) */
    static CORE::UInt32 ButtonFromDomButton( const CORE::UInt32 domButton );

    protected:

    virtual bool OnUpdate( const CORE::UInt64 tickcount               ,
                           const CORE::Float64 updateDeltaInMilliSecs ,
                           INPUT::CInputContext* context              ) GUCEF_VIRTUAL_OVERRIDE;

    virtual INPUT::CInputContext* CreateContext( const CORE::CValueList& params ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void DeleteContext( INPUT::CInputContext* context ) GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CEmscriptenInputContext;

    void OnMouseButton( CEmscriptenInputContext& context, const CORE::UInt32 button, const bool pressed );

    void OnMouseMove( CEmscriptenInputContext& context, const CORE::Int32 x, const CORE::Int32 y );

    void OnWheel( CEmscriptenInputContext& context, const CORE::UInt32 variable, const CORE::Int32 notches );

    void OnKey( CEmscriptenInputContext& context, const INPUT::KeyCode key, const bool pressed );

    void OnText( CEmscriptenInputContext& context, const CORE::CString& utf8Text );

    CEmscriptenInputDriver( const CEmscriptenInputDriver& src );
    CEmscriptenInputDriver& operator=( const CEmscriptenInputDriver& src );

    private:

    CORE::UInt32 m_contextCount;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CEmscriptenInputDriver, MT::CMutex > TEmscriptenInputDriverPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUTDRIVEREMSCRIPTEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_INPUTDRIVEREMSCRIPTEN_CEMSCRIPTENINPUTDRIVER_H ? */
