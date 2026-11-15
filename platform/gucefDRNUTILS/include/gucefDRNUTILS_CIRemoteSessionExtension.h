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

#ifndef GUCEF_DRNUTILS_CIREMOTESESSIONEXTENSION_H
#define GUCEF_DRNUTILS_CIREMOTESESSIONEXTENSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESESSION_H
#include "gucefDRNUTILS_CRemoteSession.h"
#define GUCEF_DRNUTILS_CREMOTESESSION_H
#endif /* GUCEF_DRNUTILS_CREMOTESESSION_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a component that keeps state per client session (uploads, prompts, open views) plugs into a remote service host
 *  (CRemoteServiceHost::AddSessionExtension()). Every member is called on the host's pulse generator (the thread its RPC
 *  handlers run on); the defaults do nothing.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CIRemoteSessionExtension
{
    public:

    CIRemoteSessionExtension( void );
    virtual ~CIRemoteSessionExtension();

    /** The name of the extension's child in the "extensions" documents of Resume and Resync */
    virtual CORE::CString GetExtensionName( void ) const = 0;

    /** Session/Hello opened the session */
    virtual void OnSessionOpened( const CRemoteSessionPtr& session );

    /** The control channel went without Goodbye; the state stays for the resume grace */
    virtual void OnSessionSuspended( const CRemoteSessionPtr& session );

    /** The session is bound to a control channel again (an RPC resume or Session/Resume) */
    virtual void OnSessionResumed( const CRemoteSessionPtr& session );

    /** The session ended (Goodbye, grace over, evicted, revoked, closed by an admin, the host stops): release its state */
    virtual void OnSessionClosed( const CRemoteSessionPtr& session, const CORE::CString& reason );

    /** Fills the extension's child of the "extensions" document of a Resume answer */
    virtual void FillResume( const CRemoteSessionPtr& session, CORE::CDataNode& extensionNode );

    /** Fills the extension's child of the "extensions" document of a Resync answer */
    virtual void FillResync( const CRemoteSessionPtr& session, CORE::CDataNode& extensionNode );

    /** What the extension holds for the session: the budget of the session cache */
    virtual UInt64 GetSessionBytes( const CRemoteSessionPtr& session ) const;

    private:

    CIRemoteSessionExtension( const CIRemoteSessionExtension& src );                /**< not implemented */
    CIRemoteSessionExtension& operator=( const CIRemoteSessionExtension& src );     /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CIREMOTESESSIONEXTENSION_H ? */
