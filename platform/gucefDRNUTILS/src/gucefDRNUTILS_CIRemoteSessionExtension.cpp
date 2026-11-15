/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CIRemoteSessionExtension.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIRemoteSessionExtension::CIRemoteSessionExtension( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIRemoteSessionExtension::~CIRemoteSessionExtension()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void CIRemoteSessionExtension::OnSessionOpened( const CRemoteSessionPtr& ) {}
void CIRemoteSessionExtension::OnSessionSuspended( const CRemoteSessionPtr& ) {}
void CIRemoteSessionExtension::OnSessionResumed( const CRemoteSessionPtr& ) {}
void CIRemoteSessionExtension::OnSessionClosed( const CRemoteSessionPtr&, const CORE::CString& ) {}
void CIRemoteSessionExtension::FillResume( const CRemoteSessionPtr&, CORE::CDataNode& ) {}
void CIRemoteSessionExtension::FillResync( const CRemoteSessionPtr&, CORE::CDataNode& ) {}
UInt64 CIRemoteSessionExtension::GetSessionBytes( const CRemoteSessionPtr& ) const { return 0; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
