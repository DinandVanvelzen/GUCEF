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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefWEB_CWebViewCommand.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebViewCommand::CWebViewCommand( void )
    : type( CMD_NONE )
    , viewId( 0 )
    , id( 0 )
    , text()
    , text2()
    , number1( 0 )
    , number2( 0 )
    , scale( 1.0f )
    , flag( false )
    , input()
    , cookie()
    , postedAtMs( 0 )
    , m_completion( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewCommand::CWebViewCommand( TCommandType commandType, UInt32 forViewId )
    : type( commandType )
    , viewId( forViewId )
    , id( 0 )
    , text()
    , text2()
    , number1( 0 )
    , number2( 0 )
    , scale( 1.0f )
    , flag( false )
    , input()
    , cookie()
    , postedAtMs( 0 )
    , m_completion( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewCommand::CWebViewCommand( const CWebViewCommand& src )
    : type( src.type )
    , viewId( src.viewId )
    , id( src.id )
    , text( src.text )
    , text2( src.text2 )
    , number1( src.number1 )
    , number2( src.number2 )
    , scale( src.scale )
    , flag( src.flag )
    , input( src.input )
    , cookie( src.cookie )
    , postedAtMs( src.postedAtMs )
    , m_completion( GUCEF_NULL != src.m_completion ? GUCEF_NEW CORE::CFutureResultSource( *src.m_completion ) : GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewCommand::~CWebViewCommand()
{GUCEF_TRACE;

    GUCEF_DELETE m_completion;
    m_completion = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CWebViewCommand&
CWebViewCommand::operator=( const CWebViewCommand& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        type = src.type;
        viewId = src.viewId;
        id = src.id;
        text = src.text;
        text2 = src.text2;
        number1 = src.number1;
        number2 = src.number2;
        scale = src.scale;
        flag = src.flag;
        input = src.input;
        cookie = src.cookie;
        postedAtMs = src.postedAtMs;
        GUCEF_DELETE m_completion;
        m_completion = GUCEF_NULL != src.m_completion ? GUCEF_NEW CORE::CFutureResultSource( *src.m_completion ) : GUCEF_NULL;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewCommand::CreateCompletion( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_completion )
        m_completion = GUCEF_NEW CORE::CFutureResultSource();
    return m_completion->GetFuture();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewCommand::HasCompletion( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_completion;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResultSource*
CWebViewCommand::GetCompletion( void ) const
{GUCEF_TRACE;

    return m_completion;
}

/*-------------------------------------------------------------------------*/

const char*
CWebViewCommand::TypeToString( TCommandType commandType )
{GUCEF_TRACE;

    static const char* names[] = { "none", "createView", "closeAllViews", "navigate", "reload", "stop", "goBack", "goForward",
                                   "evaluateScript", "getCookies", "clearCookies", "setCookie", "setSize", "setVisible", "setFocus",
                                   "injectInput", "setZoom", "resolveDecision", "editCommand", "captureFrame", "close" };
    const UInt32 count = (UInt32) ( sizeof( names ) / sizeof( names[ 0 ] ) );
    return (UInt32) commandType < count ? names[ commandType ] : "unknown";
}

/*-------------------------------------------------------------------------*/

CString
CWebViewCommand::Describe( void ) const
{GUCEF_TRACE;

    return CString( TypeToString( type ) ) + " for view " + CORE::UInt32ToString( viewId );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
