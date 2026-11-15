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

#ifndef GUCEF_WEB_CWEBVIEWCOMMAND_H
#define GUCEF_WEB_CWEBVIEWCOMMAND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_WEB_CWEBVIEWINPUTEVENT_H
#include "gucefWEB_CWebViewInputEvent.h"
#define GUCEF_WEB_CWEBVIEWINPUTEVENT_H
#endif /* GUCEF_WEB_CWEBVIEWINPUTEVENT_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCOOKIE_H
#include "gucefWEB_CWebViewCookie.h"
#define GUCEF_WEB_CWEBVIEWCOOKIE_H
#endif /* GUCEF_WEB_CWEBVIEWCOOKIE_H ? */

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
 *  A command for the engine thread: what a call of IWebView or IWebViewEngine becomes, put in the mailbox of the
 *  engine host and carried out on the engine thread in the order of posting. A command that has a result carries the
 *  source of its future (a copy refers to the same future); fire and forget commands (input) carry none, so they
 *  cost no task object.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewCommand
{
    public:

    enum ECommandType
    {
        CMD_NONE                = 0 ,
        CMD_CREATE_VIEW             ,   /**< engine: 'id' is the creation request */
        CMD_CLOSE_ALL_VIEWS         ,   /**< engine: shutting down */
        CMD_NAVIGATE                ,
        CMD_RELOAD                  ,
        CMD_STOP                    ,
        CMD_GO_BACK                 ,
        CMD_GO_FORWARD              ,
        CMD_EVALUATE_SCRIPT         ,   /**< text: the script, text2: the world */
        CMD_GET_COOKIES             ,   /**< text: the address filter */
        CMD_CLEAR_COOKIES           ,
        CMD_SET_COOKIE              ,
        CMD_SET_SIZE                ,   /**< number1, number2: logical size, scale: the device scale */
        CMD_SET_VISIBLE             ,   /**< flag */
        CMD_SET_FOCUS               ,   /**< flag */
        CMD_INJECT_INPUT            ,
        CMD_SET_ZOOM                ,   /**< scale */
        CMD_RESOLVE_DECISION        ,   /**< id: the decision, flag: allow, text: the text of a prompt */
        CMD_EDIT_COMMAND            ,   /**< number1: TWebViewEditCommand */
        CMD_CAPTURE_FRAME           ,
        CMD_CLOSE
    };
    typedef enum ECommandType TCommandType;

    TCommandType type;
    UInt32 viewId;                  /**< the view; 0 for commands of the engine */
    UInt32 id;                      /**< a creation request or a decision */
    CString text;
    CString text2;
    UInt32 number1;
    UInt32 number2;
    Float32 scale;
    bool flag;
    CWebViewInputEvent input;
    CWebViewCookie cookie;
    UInt64 postedAtMs;              /**< MT::PrecisionTickCount() based milliseconds, set by the host */

    CWebViewCommand( void );

    explicit CWebViewCommand( TCommandType commandType, UInt32 forViewId = 0 );

    CWebViewCommand( const CWebViewCommand& src );

    ~CWebViewCommand();

    CWebViewCommand& operator=( const CWebViewCommand& src );

    /** Gives the command a future and returns it; the engine completes it */
    CORE::CFutureResult CreateCompletion( void );

    bool HasCompletion( void ) const;

    /** The source of the future, NULL for a command without one */
    CORE::CFutureResultSource* GetCompletion( void ) const;

    /** For logs: the type and the view, never the text, the cookie or the input */
    CString Describe( void ) const;

    static const char* TypeToString( TCommandType type );

    private:

    CORE::CFutureResultSource* m_completion;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWCOMMAND_H ? */
