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

#ifndef GUCEF_GUI_CKEYBOARDLAYOUTREGISTRY_H
#define GUCEF_GUI_CKEYBOARDLAYOUTREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_GUI_CKEYBOARDLAYOUT_H
#include "gucefGUI_CKeyboardLayout.h"
#define GUCEF_GUI_CKEYBOARDLAYOUT_H
#endif /* GUCEF_GUI_CKEYBOARDLAYOUT_H ? */

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
 *  The keyboard layouts of the on screen keyboard by name, "US" built in. Applications and plugins register
 *  their own (a layout of a language, a number pad, a keyboard for a game); a registered layout is a copy,
 *  so the registry does not depend on the lifetime of the registrar.
 */
class GUCEF_GUI_PUBLIC_CPP CKeyboardLayoutRegistry
{
    public:

    typedef GUCEF::vector< CString > TStringVector;

    /** The name of the layout that is built in */
    static const CString DefaultLayoutName;

    CKeyboardLayoutRegistry( void );

    ~CKeyboardLayoutRegistry();

    /**
     *  Registers a layout under its name, replacing a layout of that name (also the built in one)
     *
     *  @return false (and nothing changes) when the layout has no name or is not valid (CKeyboardLayout::IsValid())
     */
    bool Register( const CKeyboardLayout& layout );

    /** Removes a layout; false when there is none of that name. The built in layout comes back with Reset() only */
    bool Unregister( const CString& name );

    bool IsRegistered( const CString& name ) const;

    /** A copy of the layout; false when there is none of that name */
    bool Get( const CString& name, CKeyboardLayout& layout ) const;

    /** The names, in alphabetical order */
    TStringVector GetNames( void ) const;

    /** Back to the layout that is built in */
    void Reset( void );

    private:

    CKeyboardLayoutRegistry( const CKeyboardLayoutRegistry& src );
    CKeyboardLayoutRegistry& operator=( const CKeyboardLayoutRegistry& src );

    private:

    typedef GUCEF::map< CString, CKeyboardLayout > TLayoutMap;

    TLayoutMap m_layouts;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CKEYBOARDLAYOUTREGISTRY_H ? */
