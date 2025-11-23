/*
 *  gucefMULTIPLAY: GUCE module providing multiplayer RAD functionality
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#include "gucefMULTIPLAY_CPassiveURLServerListProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CPassiveURLServerListProvider::CPassiveURLServerListProvider( void )
    : CServerListProvider() ,
      m_url()               ,
      m_isBusy( false )     ,
      m_buffer( true )      ,
      m_codecType( "xml" )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPassiveURLServerListProvider::~CPassiveURLServerListProvider()
{GUCEF_TRACE;

    m_url.Deactivate();
}

/*-------------------------------------------------------------------------*/

bool
CPassiveURLServerListProvider::Start( const CORE::CValueList& params )
{GUCEF_TRACE;

    if ( !m_isBusy )
    {
        // See if we have a URL param which will override any URL that
        // may have already been set
        if ( params.HasKey( "URL" ) )
        {
            m_url.SetURL( params.GetValue( CORE::CString( "URL" ) ) );
        }
        if ( params.HasKey( "CodecType" ) )
        {
            m_codecType = params.GetValue( CORE::CString( "CodecType" ) );
        }

        // Active the URL mechanism
        m_isBusy = m_url.Activate();
        return m_isBusy;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPassiveURLServerListProvider::IsBusy( void )
{GUCEF_TRACE;

    return m_isBusy;
}

/*-------------------------------------------------------------------------*/

void
CPassiveURLServerListProvider::Stop( void )
{GUCEF_TRACE;

    if ( m_isBusy )
    {
        m_url.Deactivate();
        m_isBusy = false;

        NotifyObservers( ServerListProviderAbortEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CPassiveURLServerListProvider::OnNotify( CORE::CNotifier* notifier                 ,
                                         const CORE::CEvent& eventid               ,
                                         CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( notifier == &m_url )
    {
        if ( CORE::CURL::URLActivateEvent == eventid )
        {
            m_buffer.Clear( true );
        }
        else
        if ( CORE::CURL::URLDataRecievedEvent == eventid )
        {
            // append the received data to the buffer
            const CORE::CURL::TURLDataRecievedEventData* data = static_cast< CORE::CURL::TURLDataRecievedEventData* >( eventdata );
            m_buffer.Append( data->GetData().GetConstBufferPtr(), data->GetData().GetDataSize() );
        }
        else
        if ( CORE::CURL::URLAllDataRecievedEvent == eventid )
        {
            ProcessRetrievedList();
        }
        else
        if ( CORE::CURL::URLDataRetrievalErrorEvent == eventid )
        {
            m_buffer.Clear( true );
            m_isBusy = false;
            NotifyObservers( ServerListProviderErrorEvent );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CPassiveURLServerListProvider::ParseList( const CORE::CDataNode& listRoot )
{GUCEF_TRACE;

    const CORE::CDataNode* root = listRoot.Find( "ServerList" );
    if ( NULL != root )
    {
        TServerInfo serverInfo;
        const CORE::CDataNode::TKeyValuePair* att = NULL;
        CORE::CDataNode::const_iterator i = root->ConstBegin();
        const CORE::CDataNode* entryNode = NULL;
        CORE::CValueList valueList;

        while ( i != root->ConstEnd() )
        {
            entryNode = (*i);
            if ( "ServerEntry" == entryNode->GetName() )
            {
                att = entryNode->GetAttribute( "Name" );
                if ( NULL != att )
                {
                    serverInfo.name = att->second.value;
                    att = entryNode->GetAttribute( "Address" );
                    if ( NULL != att )
                    {
                        serverInfo.hostAddress.SetHostname( att->second.value );
                        att = entryNode->GetAttribute( "Port" );
                        if ( NULL != att )
                        {
                            serverInfo.hostAddress.SetPortInHostByteOrder( CORE::StringToUInt16( att->second.value ) );

                            // We now have enough info to consider this a valid entry
                            // We will add the rest to the optional parameter list
                            valueList.DeleteAll();
                            for ( UInt32 n=0; n<entryNode->GetAttCount(); ++n )
                            {
                                att = entryNode->GetAttribute( n );
                                if ( ( "Name" != att->first )    &&
                                     ( "Address" != att->first ) &&
                                     ( "Port" != att->first )     )
                                {
                                    valueList.Set( att->first        ,
                                                   att->second.value );
                                }
                            }

                            ServerInfoEventData eData( serverInfo );
                            NotifyObservers( ServerInfoEvent, &eData );
                        }
                    }
                }
            }
            ++i;
        }
    }

    NotifyObservers( ServerListProviderFinisedEvent );
}

/*-------------------------------------------------------------------------*/

void
CPassiveURLServerListProvider::ProcessRetrievedList( void )
{GUCEF_TRACE;

    // Try to obtain the list codec
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec = CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().Lookup( m_codecType );
    if ( !codec.IsNULL() )
    {
        CORE::CDynamicBufferAccess bufferAccess( &m_buffer, false );
        CORE::CDataNode serverList;
        if ( codec->BuildDataTree( &serverList   ,
                                   &bufferAccess ) )
        {
            ParseList( serverList );
            return;
        }
    }

    // No codec of the given type was found
    NotifyObservers( ServerListProviderErrorEvent );
}

/*-------------------------------------------------------------------------*/

bool
CPassiveURLServerListProvider::SetListCodecType( const CORE::CString& codecType )
{GUCEF_TRACE;

    if ( !m_isBusy )
    {
        m_codecType = codecType;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPassiveURLServerListProvider::GetListCodecType( void ) const
{GUCEF_TRACE;

    return m_codecType;
}

/*-------------------------------------------------------------------------*/

bool
CPassiveURLServerListProvider::SetURL( const CORE::CString& url )
{GUCEF_TRACE;

    return m_url.SetURL( url );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPassiveURLServerListProvider::GetURL( void ) const
{GUCEF_TRACE;

    return m_url.GetURL();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
