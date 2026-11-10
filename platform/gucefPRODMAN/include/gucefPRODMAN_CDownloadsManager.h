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

#ifndef GU_CORE_CDOWNLOADSMANAGER_H
#define GU_CORE_CDOWNLOADSMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDE                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H
#include "gucefCORE_CAsyncUriResourceRetriever.h"
#define GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H
#endif /* GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H ? */

#ifndef GUCEF_PATCHER_CPATCHMANAGER_H
#include "gucefPATCHER_CPatchManager.h"
#define GUCEF_PATCHER_CPATCHMANAGER_H
#endif /* GUCEF_PATCHER_CPATCHMANAGER_H ? */

#ifndef GUCEF_PRODMAN_CPRODUCTINFO_H
#include "gucefPRODMAN_CProductInfo.h"
#define GUCEF_PRODMAN_CPRODUCTINFO_H
#endif /* GUCEF_PRODMAN_CPRODUCTINFO_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PRODMAN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/
 
/**
 *  Manager for all content downloads
 */
class GUCEF_PRODMAN_PUBLIC_CPP CDownloadsManager : public CORE::CObservingNotifier
{
    public:
    
    static const CORE::CEvent DownloadStartedEvent;
    static const CORE::CEvent DownloadStoppedEvent;
    static const CORE::CEvent DownloadStopRequestedEvent;
    static const CORE::CEvent DownloadFinishedEvent;
    static const CORE::CEvent DownloadPausedEvent;
    static const CORE::CEvent DownloadResumedEvent;
    static const CORE::CEvent PatchTaskEventReceivedEvent;
    
    typedef GUCEF::CORE::CTCloneableObj< CProductInfo > TDownloadEventData;
    typedef TDownloadEventData  TDownloadStartedEventData;
    typedef TDownloadEventData  TDownloadStoppedEventData;
    typedef TDownloadEventData  TDownloadStopRequestedEventData;
    typedef TDownloadEventData  TDownloadFinishedEventData;
    typedef TDownloadEventData  TDownloadPausedEventData;
    typedef TDownloadEventData  TDownloadResumedEventData;
    
    struct SProductPatchEventData
    {
        CProductInfo productInfo;        /** product for which a patch event was emitted */ 
        PATCHER::CPatchEngine& notifier; /** patch engine that emitted the event */
        CORE::CEvent patchEvent;         /** patch event emitted for the given product */
        CORE::CICloneable* eventData;    /** event data for the given patch event */
    };
    typedef struct SProductPatchEventData TProductPatchEventData;
    typedef CORE::CTCloneableObj< TProductPatchEventData > TPatchTaskEventReceivedEventData;
    
    static void RegisterEvents( void );
    
    public:
    
    typedef std::map< CString, CProductInfo > TProductMap;
    
    CDownloadsManager( void );
    
    virtual ~CDownloadsManager();
    
    bool StartDownload( const CProductInfo& productInfo );

    bool PauseDownload( const CProductInfo& productInfo );
    
    bool ResumeDownload( const CProductInfo& productInfo );
    
    bool RequestDownloadToStop( const CProductInfo& productInfo );
        
    void GetDownloadList( TProductMap& list ) const;

    bool ObtainAvailableDownloadsList( const CString& listUrl );
    
    void RefreshAvailableDownloadsList( void );
    
    bool SaveConfig( CORE::CDataNode& node ) const;
                                
    bool LoadConfig( const CORE::CDataNode& node );    
    
    private:
    
    typedef CORE::CTEventHandlerFunctor< CDownloadsManager > TEventCallback;
    
    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = NULL );

    virtual void OnADListRetrievalEvent( CORE::CNotifier* notifier           ,
                                         const CORE::CEvent& eventid         ,
                                         CORE::CICloneable* eventdata = NULL );
    
    private:
    
    CDownloadsManager( const CDownloadsManager& src );
    CDownloadsManager& operator=( const CDownloadsManager& src );
        
    private:    
     
    TProductMap m_productMap;
    CString m_patchEngineConfigTemplatePath;
    CString m_patchEngineConfigTemplateCodec;
    PATCHER::CPatchManager m_patchManager;
    CString m_adListUrl;
    CORE::CDynamicBuffer m_adListBuffer;                    /**< destination of the list retrieval, must outlive the retriever */
    CORE::CAsyncUriResourceRetriever m_adListRetriever;
    bool m_resumeDownloadsOnStartup;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PRODMAN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PRODMAN_CDOWNLOADSMANAGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 11-02-2008 :
        - Initial implementation
          
---------------------------------------------------------------------------*/
