/*
 *  gucefPRODMAN: Product management module
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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
//      INCLUDE                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#include "gucefPRODMAN_CDownloadsManager.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PRODMAN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const GUCEF::CORE::CEvent CDownloadsManager::DownloadStartedEvent = "GUCEF::PRODMAN::CDownloadsManager::DownloadStartedEvent";
const GUCEF::CORE::CEvent CDownloadsManager::DownloadStoppedEvent = "GUCEF::PRODMAN::CDownloadsManager::DownloadStartedEvent";
const GUCEF::CORE::CEvent CDownloadsManager::DownloadStopRequestedEvent = "GUCEF::PRODMAN::CDownloadsManager::DownloadStartedEvent";
const GUCEF::CORE::CEvent CDownloadsManager::DownloadFinishedEvent = "GUCEF::PRODMAN::CDownloadsManager::DownloadStartedEvent";
const GUCEF::CORE::CEvent CDownloadsManager::DownloadPausedEvent = "GUCEF::PRODMAN::CDownloadsManager::DownloadStartedEvent";
const GUCEF::CORE::CEvent CDownloadsManager::DownloadResumedEvent = "GUCEF::PRODMAN::CDownloadsManager::DownloadStartedEvent";
const GUCEF::CORE::CEvent CDownloadsManager::PatchTaskEventReceivedEvent = "GUCEF::PRODMAN::CDownloadsManager::PatchTaskEventReceivedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CDownloadsManager::RegisterEvents( void )
{GUCEF_TRACE;

    DownloadStartedEvent.Initialize();
    DownloadStoppedEvent.Initialize();
    DownloadStopRequestedEvent.Initialize();
    DownloadFinishedEvent.Initialize();
    DownloadPausedEvent.Initialize();
    DownloadResumedEvent.Initialize();
    PatchTaskEventReceivedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDownloadsManager::CDownloadsManager( void )
    : m_productMap()                                                        ,
      m_patchEngineConfigTemplatePath( "Products/PatchConfigTemplate.xml" ) ,
      m_patchEngineConfigTemplateCodec( "xml" )                             ,
      m_patchManager()                                                      ,
      m_adListUrl()                                                         ,
      m_adListBuffer( true )                                                ,
      m_adListRetriever()                                                   ,
      m_resumeDownloadsOnStartup( false )
{GUCEF_TRACE;

    RegisterEvents();

    TEventCallback callback( this, &CDownloadsManager::OnADListRetrievalEvent );
    SubscribeTo( &m_adListRetriever                                           ,
                 GUCEF::CORE::CAsyncUriResourceRetriever::RetrievalCompletedEvent ,
                 callback                                                         );
    TEventCallback callback2( this, &CDownloadsManager::OnADListRetrievalEvent );
    SubscribeTo( &m_adListRetriever                                        ,
                 GUCEF::CORE::CAsyncUriResourceRetriever::RetrievalFailedEvent ,
                 callback2                                                     );
}

/*-------------------------------------------------------------------------*/

CDownloadsManager::~CDownloadsManager()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDownloadsManager::ObtainAvailableDownloadsList( const CString& listUrl )
{GUCEF_TRACE;

    if ( m_adListRetriever.IsBusy() )
        return false;

    m_adListUrl = listUrl;
    m_adListBuffer.Clear();
    return m_adListRetriever.Retrieve( GUCEF::CORE::CUri( m_adListUrl ), m_adListBuffer ).HasAFuture();
}

/*-------------------------------------------------------------------------*/

void
CDownloadsManager::RefreshAvailableDownloadsList( void )
{GUCEF_TRACE;

    if ( m_adListRetriever.IsBusy() || m_adListUrl.IsNULLOrEmpty() )
        return;

    m_adListBuffer.Clear();
    m_adListRetriever.Retrieve( GUCEF::CORE::CUri( m_adListUrl ), m_adListBuffer );
}

/*-------------------------------------------------------------------------*/

bool
CDownloadsManager::StartDownload( const CProductInfo& productInfo )
{GUCEF_TRACE;

    //GUCEF::CORE::CDataNode configTemplate;
    //CString productString = productInfo.GetCombinedProductString();
    //
    //if ( GUCE::CORE::VFSLoadDataTree( m_patchEngineConfigTemplatePath  ,
    //                                  m_patchEngineConfigTemplateCodec ,
    //                                  &configTemplate                  ) )
    //{
    //    GUCEF::CORE::CDataNode* node = configTemplate.Find( "CPatchEngine" );
    //    node->SetAttribute( "PatchListURL", productInfo.GetPatchListURL() );
    //
    //    m_productMap[ productString ] = productInfo;
    //
    //    return m_patchManager.StartTask( productString  ,
    //                                     configTemplate );
    //}
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDownloadsManager::PauseDownload( const CProductInfo& productInfo )
{GUCEF_TRACE;

    return m_patchManager.PauseTask( productInfo.GetCombinedProductString() );
}

/*-------------------------------------------------------------------------*/

bool
CDownloadsManager::ResumeDownload( const CProductInfo& productInfo )
{GUCEF_TRACE;

    return m_patchManager.ResumeTask( productInfo.GetCombinedProductString() );
}

/*-------------------------------------------------------------------------*/

bool
CDownloadsManager::RequestDownloadToStop( const CProductInfo& productInfo )
{GUCEF_TRACE;

    return m_patchManager.RequestTaskToStop( productInfo.GetCombinedProductString() );
}

/*-------------------------------------------------------------------------*/

void
CDownloadsManager::GetDownloadList( TProductMap& list ) const
{GUCEF_TRACE;

    list = m_productMap;
}

/*-------------------------------------------------------------------------*/

void
CDownloadsManager::OnADListRetrievalEvent( GUCEF::CORE::CNotifier* notifier    ,
                                           const GUCEF::CORE::CEvent& eventid  ,
                                           GUCEF::CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( GUCEF::CORE::CAsyncUriResourceRetriever::RetrievalCompletedEvent == eventid )
    {
        // The list is in m_adListBuffer
    //
    //    GUCEF::CORE::CDataNode dataTree;
    //    if ( GUCE::CORE::LoadDataTree( *access   ,
    //                                   "xml"     ,
    //                                   &dataTree ) )
    //    {
    //        const GUCEF::CORE::CDataNode* rootNode = dataTree.Find( "ProductList" );
    //        GUCEF::CORE::CDataNode::const_iterator i = dataTree.ConstBegin();
    //        while ( i != dataTree.ConstEnd() )
    //        {
    //
    //            ++i;
    //        }
    //    }
    }
    else
    if ( GUCEF::CORE::CAsyncUriResourceRetriever::RetrievalFailedEvent == eventid )
    {

    }
}

/*-------------------------------------------------------------------------*/

bool
CDownloadsManager::SaveConfig( GUCEF::CORE::CDataNode& node ) const
{GUCEF_TRACE;

    GUCEF::CORE::CDataNode* n = node.Structure( "GU%CORE%CDownloadsManager", '%' );
    n->SetAttribute( "AvailableDownloadsListURL", m_adListUrl );
    n->SetAttribute( "ResumeDownloadsOnStartup", GUCEF::CORE::BoolToString( m_resumeDownloadsOnStartup ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDownloadsManager::LoadConfig( const GUCEF::CORE::CDataNode& node )
{GUCEF_TRACE;

    GUCEF::CORE::CDataNode* n = node.Search( "GU%CORE%CDownloadsManager", '%', false );
    if ( NULL != n )
    {
        GUCEF::CORE::CDataNode::TKeyValuePair* att = NULL;
        att = n->GetAttribute( "AvailableDownloadsListURL" );
        if ( NULL != att )
        {
            m_adListUrl = att->second;
        }
        att = n->GetAttribute( "ResumeDownloadsOnStartup" );
        if ( NULL != att )
        {
            m_resumeDownloadsOnStartup = att->second.AsBool();
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CDownloadsManager::OnNotify( GUCEF::CORE::CNotifier* notifier    ,
                             const GUCEF::CORE::CEvent& eventid  ,
                             GUCEF::CORE::CICloneable* eventdata )

{GUCEF_TRACE;

    if ( GUCEF::PATCHER::CPatchManager::PatchTaskStartedEvent == eventid )
    {
       // const CString& taskName = static_cast< GUCEF::PATCHER::CPatchManager::TPatchTaskStartedEventData* >( eventdata )->GetData();
       // TProductMap::iterator i = m_productMap.find( taskName );
       // if ( i != m_productMap.end() )
        {
       //     TDownloadStartedEventData eData( (*i).second );
         //   NotifyObservers( DownloadStartedEvent, &eData );
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GU */

/*-------------------------------------------------------------------------*/
