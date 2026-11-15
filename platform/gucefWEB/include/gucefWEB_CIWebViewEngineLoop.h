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

#ifndef GUCEF_WEB_CIWEBVIEWENGINELOOP_H
#define GUCEF_WEB_CIWEBVIEWENGINELOOP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  The foreign event loop of an engine as the engine host drives it on the engine thread, and the engine's own way to
 *  wake it from another thread: g_main_context_iteration() and g_main_context_wakeup() for GTK and WPE, GetMessage()
 *  and PostMessage() to a message-only window for WebView2. The thread sleeps inside RunOnce() until there is work:
 *  an idle view costs no CPU. CWebViewSemaphoreEngineLoop is the loop of engines without a foreign loop.
 */
class GUCEF_WEB_PUBLIC_CPP CIWebViewEngineLoop
{
    public:

    /** RunOnce() without a time limit: until Wake() */
    static const UInt32 WaitForever = 0xFFFFFFFF;

    CIWebViewEngineLoop( void );

    virtual ~CIWebViewEngineLoop();

    /** Engine thread, first: initialises the loop on this thread (gtk_init(), a thread default main context, COM) */
    virtual bool OnLoopStart( void ) = 0;

    /** Engine thread: sleeps until Wake() or 'maxWaitMs' and dispatches what the foreign loop has to do meanwhile */
    virtual void RunOnce( UInt32 maxWaitMs ) = 0;

    /** Any thread: makes a RunOnce() in progress, or the next one, return soon. Cheap when called often */
    virtual void Wake( void ) = 0;

    /** Engine thread, last: the loop is left; foreign objects of the thread are released */
    virtual void OnLoopStop( void ) = 0;

    private:

    CIWebViewEngineLoop( const CIWebViewEngineLoop& src );              /**< not implemented */
    CIWebViewEngineLoop& operator=( const CIWebViewEngineLoop& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CIWEBVIEWENGINELOOP_H ? */
