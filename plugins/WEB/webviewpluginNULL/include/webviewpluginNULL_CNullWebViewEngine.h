/*
 *  webviewpluginNULL: gucefWEB plugin with the web view engine "Null"
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

#ifndef GUCEF_WEBVIEWPLUGIN_NULL_CNULLWEBVIEWENGINE_H
#define GUCEF_WEBVIEWPLUGIN_NULL_CNULLWEBVIEWENGINE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CWEBVIEWENGINEBASE_H
#include "gucefWEB_CWebViewEngineBase.h"
#define GUCEF_WEB_CWEBVIEWENGINEBASE_H
#endif /* GUCEF_WEB_CWEBVIEWENGINEBASE_H ? */

#ifndef GUCEF_WEBVIEWPLUGIN_NULL_MACROS_H
#include "webviewpluginNULL_macros.h"
#define GUCEF_WEBVIEWPLUGIN_NULL_MACROS_H
#endif /* GUCEF_WEBVIEWPLUGIN_NULL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEBVIEWPLUGINNULL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The engine "Null": blank pages, no network. Always available, also without a display server. It reports what it
 *  can honestly: offscreen frames, pointer, keyboard and text input, headless; no cookies, scripts or navigation
 *  beyond about:blank.
 */
class GUCEF_WEBVIEWPLUGIN_NULL_EXPORT_CPP CNullWebViewEngine : public WEB::CWebViewEngineBase
{
    public:

    static const CORE::CString EngineName;

    CNullWebViewEngine( void );

    virtual ~CNullWebViewEngine();

    virtual CORE::CString GetEngineVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetAvailabilityDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual WEB::CWebViewCapabilities GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual WEB::CWebViewBase* CreateViewOnEngineThread( const WEB::CWebViewSettings& settings, UInt32 viewId, CORE::CString& problem ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CNullWebViewEngine( const CNullWebViewEngine& src );
    CNullWebViewEngine& operator=( const CNullWebViewEngine& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEBVIEWPLUGINNULL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEBVIEWPLUGIN_NULL_CNULLWEBVIEWENGINE_H ? */
