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

#ifndef GUCEF_TIMEOUT_EXCEPTION_H
#include "gucef_timeout_exception.h"
#define GUCEF_TIMEOUT_EXCEPTION_H
#endif /* GUCEF_TIMEOUT_EXCEPTION_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_WEB_WEBVIEWTYPENAMES_H
#include "gucefWEB_WebViewTypeNames.h"
#define GUCEF_WEB_WEBVIEWTYPENAMES_H
#endif /* GUCEF_WEB_WEBVIEWTYPENAMES_H ? */

#ifndef GUCEF_WEB_CWEBVIEWGLOBAL_H
#include "gucefWEB_CWebViewGlobal.h"
#define GUCEF_WEB_CWEBVIEWGLOBAL_H
#endif /* GUCEF_WEB_CWEBVIEWGLOBAL_H ? */

#include "gucefWEB_CWebViewCommandResult.h"

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

CWebViewCommandResult::CWebViewCommandResult( void )
    : CORE::CICloneable()
    , error( WEBVIEWERROR_NONE )
    , text()
    , cookies()
    , frame()
    , view()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewCommandResult::CWebViewCommandResult( TWebViewError resultError, const CString& resultText )
    : CORE::CICloneable()
    , error( resultError )
    , text( resultText )
    , cookies()
    , frame()
    , view()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewCommandResult::CWebViewCommandResult( const CWebViewCommandResult& src )
    : CORE::CICloneable( src )
    , error( src.error )
    , text( src.text )
    , cookies( src.cookies )
    , frame( src.frame )
    , view( src.view )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewCommandResult::~CWebViewCommandResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewCommandResult&
CWebViewCommandResult::operator=( const CWebViewCommandResult& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        error = src.error;
        text = src.text;
        cookies = src.cookies;
        frame = src.frame;
        view = src.view;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewCommandResult::IsSuccess( void ) const
{GUCEF_TRACE;

    return WEBVIEWERROR_NONE == error;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CWebViewCommandResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CWebViewCommandResult( *this );
}

const char*
CWebViewCommandResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CWebViewCommandResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::WEB::CWebViewCommandResult";
}

/*-------------------------------------------------------------------------*/

CString
CWebViewCommandResult::Describe( void ) const
{GUCEF_TRACE;

    return CString( WebViewErrorToString( error ) ) + ", " + CORE::UInt32ToString( text.Length() ) + " characters, " +
           CORE::UInt32ToString( (UInt32) cookies.size() ) + " cookies" + ( frame.IsNULL() ? "" : ", a frame" ) + ( view.IsNULL() ? "" : ", a view" );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewCommandResult::TryGetFromFuture( const CORE::CFutureResult& future, CWebViewCommandResult& result )
{GUCEF_TRACE;

    if ( !future.IsReady() )
        return false;
    CORE::CTaskPtr task = future.GetResult( 0 );
    if ( task.IsNULL() )
    {
        result = CWebViewCommandResult( WEBVIEWERROR_OTHER, "The command has no outcome" );
        return true;
    }
    const CWebViewCommandResult* data = CORE::CICloneable::CloneableAs< CWebViewCommandResult >( task->GetTaskData() );
    if ( GUCEF_NULL != data )
    {
        result = *data;
        return true;
    }
    const CORE::TTaskStatus status = task->GetTaskStatus();
    if ( CORE::TASKSTATUS_CANCELLED == status )
        result = CWebViewCommandResult( WEBVIEWERROR_CANCELLED, "The command was cancelled" );
    else
        result = CWebViewCommandResult( WEBVIEWERROR_OTHER, task->GetTaskStatusExtraInfo() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewCommandResult::Await( const CORE::CFutureResult& future, UInt32 timeoutMs, CWebViewCommandResult& result )
{GUCEF_TRACE;

    if ( !CWebViewGlobal::Instance()->MayWaitOnCurrentThread() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebViewCommandResult: A thread that must not wait (an engine thread or the GUI thread) asked to wait for a web view command" );
        result = CWebViewCommandResult( WEBVIEWERROR_INVALID_ARGUMENT, "This thread must not wait for a web view" );
        return false;
    }
    if ( !future.IsReady() )
    {
        try
        {
            future.Await( (Int32) ( timeoutMs > 0x7FFFFFFF ? 0x7FFFFFFF : timeoutMs ) );
        }
        catch ( const timeout_exception& )
        {
            result = CWebViewCommandResult( WEBVIEWERROR_ENGINE_STALLED, "The web view did not answer in time" );
            return false;
        }
    }
    if ( !TryGetFromFuture( future, result ) )
    {
        result = CWebViewCommandResult( WEBVIEWERROR_ENGINE_STALLED, "The web view did not answer in time" );
        return false;
    }
    return result.IsSuccess();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
