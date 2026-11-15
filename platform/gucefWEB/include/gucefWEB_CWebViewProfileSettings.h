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

#ifndef GUCEF_WEB_CWEBVIEWPROFILESETTINGS_H
#define GUCEF_WEB_CWEBVIEWPROFILESETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

#ifndef GUCEF_WEB_WEBVIEWTYPES_H
#include "gucefWEB_WebViewTypes.h"
#define GUCEF_WEB_WEBVIEWTYPES_H
#endif /* GUCEF_WEB_WEBVIEWTYPES_H ? */

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
 *  The profile of a view: where its cookies, storage and cache live. One profile per use (a sign-in uses
 *  "signin-<store id>"), never the person's own browser profile. A profile folder is per engine, because the formats
 *  of the engines differ: by default $APPDATADIR$/<application>/WebView/<engine>/<profileId>. Nothing in it is secret.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewProfileSettings : public CORE::CIConfigurable
{
    public:

    static const UInt32 MaxProfileIdLength = 64;

    CString profileId;                          /**< the folder name: [A-Za-z0-9._-], at most 64, not "." or ".." */
    bool ephemeral;                             /**< nothing is kept: no folder, everything in memory */
    CString dataDirectory;                      /**< empty: the default folder (ResolveDataDirectory()) */
    TWebViewProxyMode proxyMode;
    CString proxyHost;                          /**< WEBVIEWPROXY_EXPLICIT: the proxy */
    UInt16 proxyPort;
    CString::StringVector proxyIgnoreHosts;     /**< WEBVIEWPROXY_EXPLICIT: hosts reached without the proxy (the loopback in tests) */
    TWebViewThirdPartyCookies thirdPartyCookies;
    bool trackingPrevention;

    CWebViewProfileSettings( void );

    virtual ~CWebViewProfileSettings();

    /** [A-Za-z0-9._-]{1,64}, not "." or ".." */
    static bool IsValidProfileId( const CString& profileId );

    /** Makes the settings valid: an invalid profile id becomes "default", an explicit proxy without host becomes none. True when nothing had to change */
    bool Sanitize( void );

    /** The folder of the profile for an engine and an application, resolved ($APPDATADIR$ and the like); empty for an ephemeral profile */
    CString ResolveDataDirectory( const CString& engineName, const CString& applicationName ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWPROFILESETTINGS_H ? */
