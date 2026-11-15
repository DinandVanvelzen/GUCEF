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

#ifndef GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H
#define GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef GUCEF_WEB_IWEBVIEW_H
#include "gucefWEB_IWebView.h"
#define GUCEF_WEB_IWEBVIEW_H
#endif /* GUCEF_WEB_IWEBVIEW_H ? */

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
 *  The result data of the future of a web view command or of IWebViewEngine::CreateViewAsync(). Every command ends with
 *  one: 'error' is WEBVIEWERROR_NONE for success. What else it holds depends on the command: the final address of a
 *  navigation, the JSON of a script result or the message of its exception, cookies, a frame, a created view.
 *  Cookie values are secrets: a result is never logged as a whole (Describe() leaves values and text out).
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewCommandResult : public CORE::CICloneable
{
    public:

    TWebViewError error;
    CString text;                       /**< the final address, the JSON of a result, the message of an error */
    TWebViewCookieVector cookies;
    IMAGE::TPixelMapPtr frame;
    TWebViewPtr view;                   /**< CreateViewAsync(): the view */

    CWebViewCommandResult( void );

    CWebViewCommandResult( TWebViewError resultError, const CString& resultText );

    CWebViewCommandResult( const CWebViewCommandResult& src );

    virtual ~CWebViewCommandResult();

    CWebViewCommandResult& operator=( const CWebViewCommandResult& src );

    bool IsSuccess( void ) const;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name GetCloneableTypeName() says, see MT::CICloneable::CloneableAs() */
    static const char* CloneableTypeName( void );

    /** The outcome without values: the error, the length of the text, the number of cookies */
    CString Describe( void ) const;

    /**
     *  The result of a future that has ended: true with the result data. A future that ended without data (cancelled,
     *  or failed by the task system) gives a result with an error that says so. False while it has not ended.
     */
    static bool TryGetFromFuture( const CORE::CFutureResult& future, CWebViewCommandResult& result );

    /**
     *  Waits for the future up to 'timeoutMs' and gives its result: for a thread that may block (a sign-in task).
     *  Refused (false, WEBVIEWERROR_INVALID_ARGUMENT) on an engine thread and on a thread registered as one that must
     *  not wait (CWebViewGlobal::SetThreadMustNotWait(), the GUI thread). A time out gives false with
     *  WEBVIEWERROR_ENGINE_STALLED.
     */
    static bool Await( const CORE::CFutureResult& future, UInt32 timeoutMs, CWebViewCommandResult& result );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H ? */
