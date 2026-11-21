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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#include "vfspluginDRN_CDrnBridgeGlobal.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDrnBridgeGlobal::ArchiveTypeName = "drn";
MT::CMutex CDrnBridgeGlobal::g_dataLock;
CDrnBridgeGlobal* CDrnBridgeGlobal::g_instance = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDrnBridgeGlobal*
CDrnBridgeGlobal::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CDrnBridgeGlobal();
            g_instance->Initialize();
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_dataLock );
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CDrnBridgeGlobal::CDrnBridgeGlobal( void )
    : m_lock()
    , m_connections()
    , m_memberLinks()
    , m_archiveFactory( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDrnBridgeGlobal::~CDrnBridgeGlobal()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_archiveFactory )
    {
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        vfs.UnregisterArchiveFactory( ArchiveTypeName );
        GUCEF_DELETE m_archiveFactory;
        m_archiveFactory = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeGlobal::Initialize( void )
{GUCEF_TRACE;

    // the modules this one builds on are initialized first
    CORE::CCoreGlobal::Instance();
    DRN::CDRNGlobal::Instance();

    // created here and destroyed with the global: as a static object it could be destroyed at process exit while still registered with the VFS
    m_archiveFactory = GUCEF_NEW TArchiveFactory();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    vfs.RegisterArchiveFactory( ArchiveTypeName, *m_archiveFactory );
}

/*-------------------------------------------------------------------------*/

CDrnBridgeConnectionPtr
CDrnBridgeGlobal::RegisterConnection( const CORE::CString& id, DRN::CDRNSession& session, UInt32 peerId )
{GUCEF_TRACE;

    CDrnBridgeConnectionPtr connection( GUCEF_NEW CDrnBridgeConnection( session, peerId ) );
    if ( !RegisterConnection( id, connection ) )
        return CDrnBridgeConnectionPtr();
    return connection;
}

/*-------------------------------------------------------------------------*/

CDrnBridgeConnectionPtr
CDrnBridgeGlobal::RegisterConnection( const CORE::CString& id, DRN::CDRNSession& session, const CORE::CString& member )
{GUCEF_TRACE;

    CDrnBridgeConnectionPtr connection( GUCEF_NEW CDrnBridgeConnection( session, member ) );
    if ( !RegisterConnection( id, connection ) )
        return CDrnBridgeConnectionPtr();
    return connection;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeGlobal::RegisterConnection( const CORE::CString& id, CDrnBridgeConnectionPtr connection )
{GUCEF_TRACE;

    // a '|' in the id could not be told from the end of the id in "<id>|<share>"
    if ( id.IsNULLOrEmpty() || connection.IsNULL() || id.HasChar( '|' ) >= 0 )
        return false;

    MT::CScopeMutex lock( m_lock );
    if ( m_connections.find( id ) != m_connections.end() )
        return false;
    connection->SetId( id );
    m_connections[ id ] = connection;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeGlobal::UnregisterConnection( const CORE::CString& id )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TConnectionMap::iterator i = m_connections.find( id );
    if ( i != m_connections.end() )
    {
        (*i).second->SetId( CORE::CString::Empty );
        m_connections.erase( i );
    }
}

/*-------------------------------------------------------------------------*/

CDrnBridgeConnectionPtr
CDrnBridgeGlobal::FindConnection( const CORE::CString& id ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TConnectionMap::const_iterator i = m_connections.find( id );
    return i != m_connections.end() ? (*i).second : CDrnBridgeConnectionPtr();
}

CDrnBridgeMemberLinkPtr
CDrnBridgeGlobal::AcquireMemberLink( DRN::CDRNSession& session, const CORE::CString& memberKey )
{GUCEF_TRACE;

    // what is let go of is released after the lock: the destruction of a link takes the lock again
    CDrnBridgeMemberLinkPtr stale;
    CDrnBridgeMemberLinkPtr link;
    {
        MT::CScopeMutex lock( m_lock );

        // the entries of links that are gone are dropped here, not by the links: a weak reference is not let go of from inside the
        // destruction of the object it refers to
        for ( TMemberLinkMap::iterator e = m_memberLinks.begin(); e != m_memberLinks.end(); )
        {
            if ( (*e).second.IsExpired() )
                m_memberLinks.erase( e++ );
            else
                ++e;
        }
        const CORE::CString key = CORE::PointerToString( &session ) + "|" + memberKey;
        TMemberLinkMap::iterator i = m_memberLinks.find( key );
        if ( i != m_memberLinks.end() )
        {
            link = (*i).second.GetSharedPtr();
            if ( !link.IsNULL() && link->GetSession() != &session )
            {
                // the session of that name is another one now (made at the address of one that is gone)
                stale = link;
                link.Unlink();
            }
        }
        if ( link.IsNULL() )
        {
            link = CDrnBridgeMemberLinkPtr( GUCEF_NEW CDrnBridgeMemberLink( session, memberKey ) );
            m_memberLinks[ key ] = TMemberLinkWeakPtr( link );
        }
    }
    return link;
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnBridgeGlobal::GetMemberLinkCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt32 count = 0;
    for ( TMemberLinkMap::const_iterator i = m_memberLinks.begin(); i != m_memberLinks.end(); ++i )
        if ( !(*i).second.IsExpired() )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
