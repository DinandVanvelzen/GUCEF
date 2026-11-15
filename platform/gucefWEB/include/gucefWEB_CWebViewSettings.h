/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_CWEBVIEWSETTINGS_H
#define GUCEF_WEB_CWEBVIEWSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CWEBVIEWPROFILESETTINGS_H
#include "gucefWEB_CWebViewProfileSettings.h"
#define GUCEF_WEB_CWEBVIEWPROFILESETTINGS_H
#endif /* GUCEF_WEB_CWEBVIEWPROFILESETTINGS_H ? */

#ifndef GUCEF_WEB_CWEBVIEWPOLICY_H
#include "gucefWEB_CWebViewPolicy.h"
#define GUCEF_WEB_CWEBVIEWPOLICY_H
#endif /* GUCEF_WEB_CWEBVIEWPOLICY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings of one view, given when it is created and fixed for its life. The user agent override is a
 *  compatibility measure per use, never a disguise of the engine for a provider that refuses embedded agents.
 *  The inspector stays off for sign-in views. Nothing in it is secret.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewSettings : public CORE::CIConfigurable
{
    public:

    static const UInt32 DefaultWidth = 1024;
    static const UInt32 DefaultHeight = 768;
    static const UInt32 MaxSize = 16384;
    static const UInt32 DefaultMaxScriptResultBytes = 65536;
    static const UInt32 DefaultDecisionTimeoutMs = 30000;

    CWebViewProfileSettings profile;
    TWebViewMode mode;
    UInt32 width;                                   /**< logical pixels of the view */
    UInt32 height;
    Float32 deviceScale;                            /**< device pixels per logical pixel */
    CString userAgent;                              /**< empty: the engine's own */
    TWebViewHardwareAcceleration hardwareAcceleration;
    bool transparentBackground;
    bool consoleMessages;                           /**< ConsoleMessageEvent; off by default (page text may hold anything) */
    bool inspector;                                 /**< the engine's developer tools; never for sign-in views */
    UInt32 maxScriptResultBytes;                    /**< the largest script result; 0 for no limit */
    UInt32 decisionTimeoutMs;                       /**< an unanswered deferred decision is refused after this; 0 never */
    CWebViewPolicy policy;

    CWebViewSettings( void );

    virtual ~CWebViewSettings();

    /**
     *  Makes the settings valid: sizes 1..16384, a device scale of 0.25..8, a valid profile. With 'forSignIn' also
     *  safe for a sign-in: the inspector and console messages off and the policy sanitised for sign-in.
     *  True when nothing had to change.
     */
    bool Sanitize( bool forSignIn );

    /** The size of a frame in device pixels: the logical size times the scale, rounded up */
    void GetDeviceSize( UInt32& deviceWidth, UInt32& deviceHeight ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The device size of a logical size at a scale, as the views use it */
    static void ToDeviceSize( UInt32 logicalWidth, UInt32 logicalHeight, Float32 scale, UInt32& deviceWidth, UInt32& deviceHeight );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWSETTINGS_H ? */
