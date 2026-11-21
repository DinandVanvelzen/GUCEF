/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CTWEAKPTR_H
#include "gucefCORE_CTWeakPtr.h"
#define GUCEF_CORE_CTWEAKPTR_H
#endif /* GUCEF_CORE_CTWEAKPTR_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMEMBERLINK_H
#include "vfspluginDRN_CDrnBridgeMemberLink.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMEMBERLINK_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMEMBERLINK_H ? */

#ifndef GUCEF_VFS_CARCHIVE_H
#include "gucefVFS_CArchive.h"
#define GUCEF_VFS_CARCHIVE_H
#endif /* GUCEF_VFS_CARCHIVE_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEARCHIVE_H
#include "vfspluginDRN_CDrnBridgeArchive.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEARCHIVE_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEARCHIVE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The global state of the plugin, as every module has its Global class: made on first use (Instance()), which registers
 *  the archive type "drn" with the VFS, and gone with Deinstance() (the plugin's unload, or an application that linked the
 *  plugin and wants the type gone). It holds the connections that applications registered by id.
 *
 *  An application that links the plugin makes the global by asking for the Instance() before it mounts a share of type "drn";
 *  an application that loads the plugin (the plugin's load function does the same) needs no code. Connections are registered
 *  in code, with the session and the peer id (or the name of the member) that the application has: the id is what the
 *  actual path of the archive names ("<connection id>|<share>").
 */
class GUCEF_VFSPLUGIN_DRN_EXPORT_CPP CDrnBridgeGlobal
{
    public:

    typedef CORE::CTFactory< VFS::CArchive, CDrnBridgeArchive, MT::CMutex > TArchiveFactory;

    /** The archive type: "drn" */
    static const CORE::CString ArchiveTypeName;

    static CDrnBridgeGlobal* Instance( void );

    static void Deinstance( void );

    /**
     *  Registers a connection to the member of a session: a fixed peer id, or the node id of a member, or "sequencer" for the host
     *  of a client-server session (resolved each time it is used). The connection is NULL when the id is taken or empty.
     *  The session has to outlive the registration: unregister before the session goes.
     */
    CDrnBridgeConnectionPtr RegisterConnection( const CORE::CString& id, DRN::CDRNSession& session, UInt32 peerId );
    CDrnBridgeConnectionPtr RegisterConnection( const CORE::CString& id, DRN::CDRNSession& session, const CORE::CString& member );

    /** Registers a connection the application made; false when the id is empty or taken */
    bool RegisterConnection( const CORE::CString& id, CDrnBridgeConnectionPtr connection );

    void UnregisterConnection( const CORE::CString& id );

    /** The connection registered under the id, NULL when there is none */
    CDrnBridgeConnectionPtr FindConnection( const CORE::CString& id ) const;

    /**
     *  The one link (namespace client with its request budget, handle budget) of a member of a session, shared by every mount
     *  of that pair: made for the first mount, the same for the others, gone when the last one let go. 'memberKey' is the member as
     *  the mounts name it: a node id, "sequencer", or a peer id as text; the same member named in two ways is two links.
     */
    CDrnBridgeMemberLinkPtr AcquireMemberLink( DRN::CDRNSession& session, const CORE::CString& memberKey );

    /** Links alive now, for tests and diagnostics */
    UInt32 GetMemberLinkCount( void ) const;


    private:

    typedef GUCEF::map< CORE::CString, CDrnBridgeConnectionPtr > TConnectionMap;
    typedef CORE::CTWeakPtr< CDrnBridgeMemberLink, MT::CMutex > TMemberLinkWeakPtr;
    typedef GUCEF::map< CORE::CString, TMemberLinkWeakPtr > TMemberLinkMap;

    CDrnBridgeGlobal( void );

    ~CDrnBridgeGlobal();

    CDrnBridgeGlobal( const CDrnBridgeGlobal& src );                /**< not implemented */
    CDrnBridgeGlobal& operator=( const CDrnBridgeGlobal& src );     /**< not implemented */

    void Initialize( void );

    static MT::CMutex g_dataLock;
    static CDrnBridgeGlobal* g_instance;

    mutable MT::CMutex m_lock;
    TConnectionMap m_connections;
    TMemberLinkMap m_memberLinks;       /**< weak: the mounts that use a link keep it alive */
    TArchiveFactory* m_archiveFactory;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H ? */
