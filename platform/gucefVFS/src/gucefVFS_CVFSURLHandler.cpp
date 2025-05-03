/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CURLHANDLERREGISTRY_H
#include "CURLHandlerRegistry.h"      /* gucefCORE registry for URL handlers */
#define GUCEF_CORE_CURLHANDLERREGISTRY_H
#endif /* GUCEF_CORE_CURLHANDLERREGISTRY_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_VFS_MACROS_H
#include "gucefVFS_macros.h"          /* often used gucefVFS macros */
#define GUCEF_VFS_MACROS_H
#endif /* GUCEF_VFS_MACROS_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#include "gucefVFS_CVFSURLHandler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CVFSURLHandler::CVFSURLHandler( void )
        : m_vfs( &CVfsGlobal::Instance()->GetVfs() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVFSURLHandler::CVFSURLHandler( const CVFSURLHandler& src )
        : m_vfs( &CVfsGlobal::Instance()->GetVfs() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVFSURLHandler::~CVFSURLHandler()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CVFSURLHandler&
CVFSURLHandler::operator=( const CVFSURLHandler& src )
{GUCEF_TRACE;
        return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CVFSURLHandler::Clone( void ) const
{GUCEF_TRACE;
        return GUCEF_NEW CVFSURLHandler( *this );
}

/*-------------------------------------------------------------------------*/

void
CVFSURLHandler::Register( void )
{GUCEF_TRACE;

    CORE::CURLHandlerRegistry* registry = &CORE::CCoreGlobal::Instance()->GetUrlHandlerRegistry();
    if ( !registry->IsRegistered( "vfs" ) )
    {
        registry->Register( "vfs", CORE::CURLHandlerRegistry::TRegisteredObjTypedPtr( GUCEF_NEW CVFSURLHandler() ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CVFSURLHandler::Unregister( void )
{GUCEF_TRACE;

    CORE::CCoreGlobal::Instance()->GetUrlHandlerRegistry().Unregister( "vfs" );
}

/*-------------------------------------------------------------------------*/

bool
CVFSURLHandler::Activate( CORE::CURL& url )
{GUCEF_TRACE;

    assert( &url );

    // Tell the data handlers we have begun our activation sequence
    NotifyObservers( URLActivateEvent );

    // Obtain the file
    TBasicVfsResourcePtr resource = m_vfs->GetFile( url.GetURL().SubstrToSubstr( "://" ), "rb", false );
    if ( !resource.IsNULL() )
    {
            CORE::IOAccessPtr access = resource->GetAccess();
            if ( access.IsNULL() )
            {
                NotifyObservers( URLDataRetrievalErrorEvent );
                return false;
            }
            else
            {
                // Pass the file data on to the data handlers
                NotifyObservers( URLDataRecievedEvent, &access );

                NotifyObservers( URLAllDataRecievedEvent );
            }

            return true;
    }
    else
    {
        NotifyObservers( URLDataRetrievalErrorEvent );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CVFSURLHandler::Deactivate( CORE::CURL& url )
{GUCEF_TRACE;
        /*
         *      This URL handler works instantly and need not be deactivated
         */
}

/*-------------------------------------------------------------------------*/

bool
CVFSURLHandler::IsActive( const CORE::CURL& url ) const
{GUCEF_TRACE;
        return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
