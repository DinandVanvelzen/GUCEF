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

#ifndef GUCEF_GUI_CSECRETINPUTSETTINGS_H
#define GUCEF_GUI_CSECRETINPUTSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

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
 *  How a CSecretInput treats the secret. The defaults are the most secure choice for every setting: a
 *  setting that makes the input less strict has to be asked for.
 */
class GUCEF_GUI_PUBLIC_CPP CSecretInputSettings : public CORE::CIConfigurable
{
    public:

    /** The name of the config section: SaveConfig() writes it in the node it gets, LoadConfig() reads it from there */
    static const CString ConfigSectionName;

    /** The most code points a value can have whatever is asked for: the storage of the secret is a fixed block of at most 4 bytes per code point */
    static const UInt32 AbsoluteMaxLength = 4096;

    static const UInt32 DefaultMaxLength = 256;
    static const UInt32 DefaultMaskCodePoint = 0x2022;     /**< a bullet */
    static const UInt32 DefaultFixedMaskLength = 12;
    static const UInt32 DefaultRevealTimeoutInMs = 5000;

    /** Most code points the value can have (4 bytes of storage for each, allocated once); 1 to AbsoluteMaxLength */
    UInt32 maxLength;

    /** What is drawn for every code point of the value; a printable code point, else the bullet */
    UInt32 maskCodePoint;

    /** Draws 'fixedMaskLength' mask characters whatever the length of the value, so the length cannot be read from the screen (an empty value draws none) */
    bool hideLength;

    UInt32 fixedMaskLength;

    /** Copy and cut put the value on the clipboard; off: the keys do nothing (a cut does not remove anything either) */
    bool allowCopy;

    /** Paste (Ctrl+V, Shift+Insert) and text that arrives in one piece (several code points at once) are taken; off: only what is typed code point by code point */
    bool allowPaste;

    /** The value can be shown (SetRevealed()); off: it cannot, whatever calls for it */
    bool allowReveal;

    /** A revealed value is hidden again after this long; 0 for the default */
    UInt32 revealTimeoutInMs;

    /** The value is wiped when the input loses the keyboard focus */
    bool clearOnFocusLoss;

    /** The value is wiped when the input stops being visible (it or an ancestor is hidden, the form is closed) */
    bool clearOnHide;

    /** CapsLockStateChangedEvent is notified when the state of Caps Lock changes while typing */
    bool notifyCapsLock;

    /** The code point typed last is shown for this long before it becomes a mask character; 0: never */
    UInt32 lastCharRevealInMs;

    CSecretInputSettings( void );

    CSecretInputSettings( const CSecretInputSettings& src );

    virtual ~CSecretInputSettings();

    CSecretInputSettings& operator=( const CSecretInputSettings& src );

    /** Brings every value inside what the input can work with: the length bounds, a printable mask character, a reveal time */
    void Sanitize( void );

    /** Whether two settings are the same */
    bool operator==( const CSecretInputSettings& other ) const;

    /** Writes the settings as the section ConfigSectionName in 'tree' */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the section ConfigSectionName of 'treeroot', or 'treeroot' itself when it is that section; what it lacks keeps its (secure) default */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CSECRETINPUTSETTINGS_H ? */
