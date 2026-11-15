/*
 *  MFCPatcherGUI: GUI shell for the gucefPATCHER module
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "stdafx.h"
#include <assert.h>
#include "MFCPatcherGUI.h"

#ifndef MFCWIN32_CGUCEFAPPWIN32MFCDRIVER_H
#include "CGUCEFAppWin32MFCDriver.h"
#define MFCWIN32_CGUCEFAPPWIN32MFCDRIVER_H
#endif /* MFCWIN32_CGUCEFAPPWIN32MFCDRIVER_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#include "DVOSWRAP.h"

#include "ChildView.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CChildView::CChildView()
    : m_listBox( GUCEF_NULL )            ,
      m_transferProgress( GUCEF_NULL )   ,
      m_totalProgress( GUCEF_NULL )      ,
      m_patchEngine()              ,
      m_gucefDriver( GUCEF_NULL )        ,
      m_closeAppWhenDone( false )  ,
      m_fileReplacements()
{GUCEF_TRACE;

    // Subscribe to all patch engine events
    m_patchEngine.Subscribe( this );
    
    // Subscribe the patch engine to the GUCEF application object
    CORE::CGUCEFApplication* gucefApp = &CORE::CCoreGlobal::Instance()->GetApplication();
    m_patchEngine.SubscribeTo( gucefApp, CORE::CGUCEFApplication::AppInitEvent );
    m_patchEngine.SubscribeTo( gucefApp, CORE::CGUCEFApplication::AppShutdownEvent );
    
    // Set the application startup and shutdown events as triggers
    m_patchEngine.GetConfig().AddEngineStartTriggerEvent( CORE::CGUCEFApplication::AppInitEvent );
    m_patchEngine.GetConfig().AddEngineStopTriggerEvent( CORE::CGUCEFApplication::AppShutdownEvent );
}

/*-------------------------------------------------------------------------*/

CChildView::~CChildView()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_PAINT()
    ON_WM_CREATE()
    ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*-------------------------------------------------------------------------*/

// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{GUCEF_TRACE;

	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(GUCEF_NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), GUCEF_NULL);

	return TRUE;
}

/*-------------------------------------------------------------------------*/

CChildView::TFileReplacementMap&
CChildView::GetFileReplacementMap( void )
{GUCEF_TRACE;

    return m_fileReplacements;
}

/*-------------------------------------------------------------------------*/

void CChildView::DoDataExchange( CDataExchange* pDX )
{GUCEF_TRACE;

	CWnd::DoDataExchange( pDX );

	//{{AFX_DATA_MAP(CPatcherGUIDlg)
//	DDX_Control( pDX, IDC_LISTBOX, *m_listBox );
//	DDX_Control( pDX, IDC_TRANSFERPROGRESS, *m_transferProgress );
//	DDX_Control( pDX, IDC_TOTALPROGRESS, *m_totalProgress );
	//}}AFX_DATA_MAP
	
}

/*-------------------------------------------------------------------------*/

void CChildView::OnPaint() 
{GUCEF_TRACE;

	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages
}

/*-------------------------------------------------------------------------*/

bool
CChildView::LoadPatchEngineConfig( void )
{GUCEF_TRACE;

    // Check for mandatory command line params
    if ( !theApp.GetCommandLineParams().HasKey( "ConfigFile" ) )
    {
        // we need command line params
        return false;
    }
    if ( !theApp.GetCommandLineParams().HasKey( "ConfigFileCodec" ) )
    {
        // we need command line params
        return false;
    }    
    
    m_closeAppWhenDone = GUCEF::CORE::StringToBool( theApp.GetCommandLineParams().GetValueAlways( "CloseWhenDone" ) );
    
    GUCEF::CORE::CString configFile = theApp.GetCommandLineParams().GetValueAlways( "ConfigFile" );    
    configFile = GUCEF::CORE::RelativePath( configFile );
    GUCEF::CORE::CString configFileCodec = theApp.GetCommandLineParams().GetValueAlways( "ConfigFileCodec" );
    if ( ( configFile.Length() == 0 )     ||
         ( configFileCodec.Length() == 0 ) )
    {
        // We need actual values
        return false;
    }
    
    // Load the config for the patch engine
    GUCEF::CORE::CDStoreCodecRegistry* codecRegistry = &GUCEF::CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry();
    if ( codecRegistry->IsRegistered( configFileCodec ) )
    {
        GUCEF::CORE::CDataNode configData;
        GUCEF::CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec = codecRegistry->Lookup( configFileCodec );
        if ( codec->BuildDataTree( &configData ,
                                   configFile  ) )
        {
            return m_patchEngine.GetConfig().LoadConfig( configData );
        }
        else
        {
            // The configuration data was not as required by the patch engine
            return false;
        }
    }
    else
    {   
        // The requested codec is not available, this is a fatal error
        return false;
    }
}

/*-------------------------------------------------------------------------*/

int CChildView::OnCreate(LPCREATESTRUCT lpcs)
{GUCEF_TRACE;

    if ( !LoadPatchEngineConfig() )
    {
        GUCEF::CORE::ShowErrorMessage( "Init error", "Failed to load patch engine config" );
        PostQuitMessage( 0 );
        return 1;
    }

    // Hook the GUCEF application driver into the window message loop
    m_gucefDriver = new CGUCEFAppWin32MFCDriver();
    assert( m_gucefDriver );
    if ( m_gucefDriver->Init( this                                                                ,
                              CORE::CCoreGlobal::Instance()->GetApplication().GetPulseGenerator() ) )
    {
        // Create our listbox
        m_listBox = new CListBox();
        assert( m_listBox );
        m_listBox->Create( WS_CHILD|WS_VISIBLE|LBS_STANDARD|WS_HSCROLL , 
                           CRect( 10, 40, 460, 200 )                   ,
                           this                                        , 
                           1                                           );
   
      /*  CFont listFont;
        if ( TRUE == listFont.CreatePointFont( 60, "Arial" ) )
        {
            m_listBox->SetFont( &listFont, TRUE );
        } */
        
        m_listBox->SetFont( GetFont() );
        
        m_transferProgress = new CProgressCtrl();
        assert( m_transferProgress );
        m_transferProgress->Create( WS_CHILD|WS_VISIBLE      , 
                                    CRect( 10, 10, 460, 20 ) ,
                                    this                     , 
                                    1                        );
        m_transferProgress->SetRange( 0, 100 );
                                            
        m_totalProgress = new CProgressCtrl();
        assert( m_totalProgress );
        m_totalProgress->Create( WS_CHILD|WS_VISIBLE      , 
                                 CRect( 10, 30, 460, 40 ) ,
                                 this                     , 
                                 1                        );
        m_totalProgress->SetRange( 0, 100 );
        
        // Start the application from the GUCEF point of view
        CORE::CCoreGlobal::Instance()->GetApplication().Main( AfxGetInstanceHandle() ,
                                                              GUCEF_NULL                   ,
                                                              0                      ,
                                                              false                  );
        return 0;                                                  
    }
    
    GUCEF::CORE::ShowErrorMessage( "Init error", "Failed to initialize the GUCEF application driver" );
    PostQuitMessage( 0 );
    return 1;
}

/*-------------------------------------------------------------------------*/

void CChildView::OnSize(UINT nType, int cx, int cy)
{GUCEF_TRACE;

  //  MoveWindow( 0, 0, 480, 160 );
  //  m_listBox->MoveWindow(0, 0, cx, 100);
}

/*-------------------------------------------------------------------------*/

void
CChildView::PrintOutput( const GUCEF::CORE::CString& output )
{GUCEF_TRACE;

    if ( m_listBox->GetCount() == 0 )
    {
        m_listBox->AddString( output.C_String() );
    }  
    else
    {
        m_listBox->InsertString( m_listBox->GetCount(), output.C_String() );
    }
}

/*-------------------------------------------------------------------------*/

void
CChildView::PrintPatchListEngineStatus( const GUCEF::CORE::CString& summary ,
                                        GUCEF::CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const GUCEF::PATCHER::CPatchListEngineEvents::TPatchListEngineEventData* eventDataWrapper = static_cast< GUCEF::PATCHER::CPatchListEngineEvents::TPatchListEngineEventData* >( eventData );
    const GUCEF::PATCHER::CPatchListEngineEvents::TPatchListEngineEventDataStorage& storage = eventDataWrapper->GetData();
    
    PrintOutput( "Summary: " + summary );
    PrintOutput( "Processed patch list data size: " + GUCEF::CORE::UInt64ToString( storage.processedDataSizeInBytes ) );
    PrintOutput( "Total patch list data size: " + GUCEF::CORE::UInt64ToString( storage.totalDataSizeInBytes ) );
    PrintOutput( "-------------------------------------" );
    
    if ( storage.totalDataSizeInBytes > 0 )
    {
        m_totalProgress->SetPos( (int)( ( storage.processedDataSizeInBytes / storage.totalDataSizeInBytes ) * 100.0 ) );
    }
    else
    {
        m_totalProgress->SetPos( 0 );
    }
}

/*-------------------------------------------------------------------------*/

void
CChildView::PrintPatchSetEngineStatus( const GUCEF::CORE::CString& summary ,
                                       CORE::CICloneable* eventData        )
{GUCEF_TRACE;

    const GUCEF::PATCHER::CPatchSetEngineEvents::TPatchSetEngineEventData* eventDataWrapper = static_cast< GUCEF::PATCHER::CPatchSetEngineEvents::TPatchSetEngineEventData* >( eventData );
    const GUCEF::PATCHER::CPatchSetEngineEvents::TPatchSetEngineEventDataStorage& storage = eventDataWrapper->GetData();

    PrintOutput( "Summary: " + summary );
    PrintOutput( "Processed patch set data size: " + GUCEF::CORE::UInt64ToString( storage.processedDataSizeInBytes ) );
    PrintOutput( "Total patch set data size: " + GUCEF::CORE::UInt64ToString( storage.totalDataSizeInBytes ) );
    PrintOutput( "-------------------------------------" );
}

/*-------------------------------------------------------------------------*/

void
CChildView::PrintPatchSetDirEngineStatus( const GUCEF::CORE::CString& summary ,
                                          CORE::CICloneable* eventData        )
{GUCEF_TRACE;

    const GUCEF::PATCHER::CPatchSetDirEngineEvents::TPatchSetDirEngineEventData* eventDataWrapper = static_cast< GUCEF::PATCHER::CPatchSetDirEngineEvents::TPatchSetDirEngineEventData* >( eventData );
    const GUCEF::PATCHER::CPatchSetDirEngineEvents::TPatchSetDirEngineEventDataStorage& storage = eventDataWrapper->GetData();
    
    PrintOutput( "Summary: " + summary );
    
    PrintOutput( "Current Directory: " + storage.dirName );
    PrintOutput( "Total directory size: " + GUCEF::CORE::UInt64ToString( storage.dirSizeInBytes ) );
    PrintOutput( "Directory hash: " + storage.dirHash );
    PrintOutput( "-------------------------------------" );
}

/*-------------------------------------------------------------------------*/

void
CChildView::PrintPatchSetFileEngineStatus( const GUCEF::CORE::CString& summary ,
                                           CORE::CICloneable* eventData        )
{GUCEF_TRACE;

    const GUCEF::PATCHER::CPatchSetFileEngineEvents::TPatchSetFileEngineEventData* eventDataWrapper = static_cast< GUCEF::PATCHER::CPatchSetFileEngineEvents::TPatchSetFileEngineEventData* >( eventData );
    const GUCEF::PATCHER::CPatchSetFileEngineEvents::TPatchSetFileEngineEventDataStorage& storage = eventDataWrapper->GetData();

    PrintOutput( "Summary: " + summary );
    
    PrintOutput( "Current file: " + storage.currentFileEntry.name );
    PrintOutput( "Total file size: " + GUCEF::CORE::UInt64ToString( storage.currentFileEntry.sizeInBytes ) );
    PrintOutput( "Total bytes processed: " + GUCEF::CORE::UInt64ToString( storage.totalBytesProcessed ) );
    PrintOutput( "Local root: " + storage.localRoot );
    PrintOutput( "Temp. storage root: " + storage.tempStorageRoot );
    PrintOutput( "File hash: " + storage.currentFileEntry.hash );
    
    if ( storage.currentFileEntry.fileLocations.size() > storage.currentRemoteLocationIndex )
    {
        PrintOutput( "Remote source: " + storage.currentFileEntry.fileLocations[ storage.currentRemoteLocationIndex ].URL );
    }
    PrintOutput( "-------------------------------------" );
    
    if ( storage.currentFileEntry.sizeInBytes > 0 )
    {
        m_transferProgress->SetPos( (int)( ( storage.totalBytesProcessed / storage.currentFileEntry.sizeInBytes ) * 100.0 ) );
    }
    else
    {
        m_transferProgress->SetPos( 0 );
    }
}

/*-------------------------------------------------------------------------*/

void
CChildView::OnNotify( CORE::CNotifier* notifier                 ,
                      const CORE::CEvent& eventid               ,
                      CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( eventid == PATCHER::CPatchEngine::PatchProcessStartedEvent )
    {
        PrintOutput( "Patch process starting..." );
        PrintOutput( "-------------------------------------" );
    }
    else
    if ( eventid == PATCHER::CPatchEngine::PatchProcessFailedEvent )
    {
        PrintOutput( "ERROR: Patch process failed" );
        PrintOutput( "-------------------------------------" );
    }
    else
    if ( eventid == PATCHER::CPatchEngine::PatchProcessCompletedEvent )
    {
        PrintOutput( "Completed the patch process" );
        PrintOutput( "-------------------------------------" );
        
        if ( m_closeAppWhenDone )
        {
            PostQuitMessage( 0 );
        }
    }        
    else
    if ( eventid == PATCHER::CPatchEngine::PatchProcessAbortedEvent )
    {
        PrintOutput( "The patching process has been aborted" );
        PrintOutput( "-------------------------------------" );
    }
    else        
    if ( eventid == PATCHER::CPatchEngine::PatchListRetrievalStartedEvent )
    {
        PrintOutput( "Retrieving patch list..." );
        PrintOutput( "-------------------------------------" );
    }
    else        
    if ( eventid == PATCHER::CPatchEngine::PatchListRetrievalFailedEvent )
    {
        PrintOutput( "ERROR: Failed to retrieve the patch list" );
        PrintOutput( "-------------------------------------" );
    }
    else
    if ( eventid == PATCHER::CPatchEngine::PatchListRetrievalCompletedEvent )
    {
        PrintOutput( "Completed patch list retrieval" );
        PrintOutput( "-------------------------------------" );
    }
    else        
    if ( eventid == PATCHER::CPatchEngine::PatchListDataReceivedEvent )
    {
        PrintOutput( "Received patch list data" );
        PrintOutput( "-------------------------------------" );
    }
    else
    if ( eventid == PATCHER::CPatchEngine::PatchListRetrievalAbortedEvent )
    {
        PrintOutput( "Aborted patch list retrieval" );
        PrintOutput( "-------------------------------------" );
    } 
    else
    if ( eventid == PATCHER::CPatchEngine::PatchListDecodingFailedEvent )
    {
        PrintOutput( "ERROR: Failed to decode the patch list" );
        PrintOutput( "-------------------------------------" );
    }
    else
    if ( eventid == PATCHER::CPatchEngine::PatchListProcessingStartedEvent )
    {
        PrintPatchListEngineStatus( "Patch list processing started...", eventdata );
    }
    else
    if ( eventid == PATCHER::CPatchEngine::PatchListProcessingProgressEvent )
    {
        PrintPatchListEngineStatus( "Progress processing a patch list", eventdata );
    }
    else        
    if ( eventid == PATCHER::CPatchEngine::PatchListProcessingCompletedEvent )
    {
        PrintPatchListEngineStatus( "Completed processing the patch list", eventdata );
    }        
    else
    if ( eventid == PATCHER::CPatchListEngineEvents::PatchSetRetrievalFailedEvent )
    {
        PrintPatchListEngineStatus( "ERROR: Failed to retrieve the patch set", eventdata );
    }
    else
    if ( eventid == PATCHER::CPatchListEngineEvents::PatchSetDataRecievedEvent )
    {
        PrintPatchListEngineStatus( "Received patch set data", eventdata );
    }
    else        
    if ( eventid == PATCHER::CPatchListEngineEvents::PatchSetRetrievalAbortedEvent )
    {
        PrintPatchListEngineStatus( "Aborted patch set retrieval", eventdata );
    }  
    else        
    if ( eventid == PATCHER::CPatchListEngineEvents::PatchSetRetrievalStartedEvent )
    {
        PrintPatchListEngineStatus( "Retrieving patch set...", eventdata );
    }  
    else        
    if ( eventid == PATCHER::CPatchListEngineEvents::PatchSetRetrievalCompletedEvent )
    {
        PrintPatchListEngineStatus( "Completed patch set retrieval", eventdata );
    }  
    else         
    if ( eventid == PATCHER::CPatchSetEngineEvents::PatchSetProcessingStartedEvent )
    {
        PrintPatchSetEngineStatus( "Started processing patch set...", eventdata );
    }        
    else
    if ( eventid == PATCHER::CPatchSetEngineEvents::PatchSetProcessingStartedEvent )
    {
        PrintPatchSetEngineStatus( "Started processing of a patch set", eventdata );
    }
    else
    if ( eventid == PATCHER::CPatchSetEngineEvents::PatchSetProcessingProgressEvent )
    {
        PrintPatchSetEngineStatus( "Progress processing a patch set", eventdata );
    }
    else
    if ( eventid == PATCHER::CPatchSetEngineEvents::PatchSetProcessingAbortedEvent )
    {
        PrintPatchSetEngineStatus( "Aborted processing of a patch set", eventdata );
    }
    else 
    if ( eventid == PATCHER::CPatchSetEngineEvents::PatchSetProcessingFailedEvent )
    {
        PrintPatchSetEngineStatus( "Failed to process the complete patch set", eventdata );
    }
    else
    if ( eventid == PATCHER::CPatchSetEngineEvents::PatchSetProcessingCompletedEvent )
    {
        PrintPatchSetEngineStatus( "Completed processing a patch set", eventdata );
    }        
    else
    if ( eventid == PATCHER::CPatchListEngineEvents::PatchSetDecodingFailedEvent )
    {
        PrintPatchListEngineStatus( "ERROR: Failed to decode a patch set", eventdata );
    }
    else
    if ( eventid == PATCHER::CPatchSetDirEngineEvents::DirProcessingStartedEvent )
    {
        PrintPatchSetDirEngineStatus( "Started processing directory...", eventdata );
    }
    else
    if ( eventid == PATCHER::CPatchSetDirEngineEvents::DirProcessingCompletedEvent )
    {
        PrintPatchSetDirEngineStatus( "Completed processing directory", eventdata );
    }        
    else
    if ( eventid == PATCHER::CPatchSetDirEngineEvents::SubDirProcessingCompletedEvent )
    {
        PrintPatchSetDirEngineStatus( "Completed processing sub directories", eventdata );
    }        
    else        
    if ( eventid == PATCHER::CPatchSetFileEngineEvents::FileRetrievalStartedEvent )
    {
        PrintPatchSetFileEngineStatus( "Retrieving file...", eventdata );
    }                 
    else        
    if ( eventid == PATCHER::CPatchSetFileEngineEvents::FileListProcessingStartedEvent )
    {
        PrintPatchSetFileEngineStatus( "Started processing file list...", eventdata );
    }
    else        
    if ( eventid == PATCHER::CPatchSetFileEngineEvents::FileRetrievalProgressEvent )
    {
        PrintPatchSetFileEngineStatus( "Progress retrieving a file", eventdata );
    }    
    else        
    if ( eventid == PATCHER::CPatchSetFileEngineEvents::FileListProcessingCompleteEvent )
    {
        PrintPatchSetFileEngineStatus( "Completed file list processing", eventdata );
    }                 
    else
    if ( eventid == PATCHER::CPatchSetFileEngineEvents::FileListProcessingAbortedEvent )
    {
        PrintPatchSetFileEngineStatus( "Aborted file list processing", eventdata );
    }
    else
    if ( eventid == PATCHER::CPatchSetFileEngineEvents::LocalFileIsOKEvent )
    {
        PrintPatchSetFileEngineStatus( "Local file is Ok", eventdata );
    }         
    else
    if ( eventid == PATCHER::CPatchSetFileEngineEvents::LocalFileSizeMismatchEvent )
    {
        PrintPatchSetFileEngineStatus( "Local file size mismatch", eventdata );
    }         
    else
    if ( eventid == PATCHER::CPatchSetFileEngineEvents::LocalFileHashMismatchEvent )
    {
        PrintPatchSetFileEngineStatus( "Local file hash mismatch", eventdata );
    }        
    else
    if ( eventid == PATCHER::CPatchSetFileEngineEvents::LocalFileNotFoundEvent )
    {
        PrintPatchSetFileEngineStatus( "Local file not found", eventdata );
    }
    else
    if ( eventid == PATCHER::CPatchSetFileEngineEvents::LocalFileReplacedEvent )
    {
        PrintPatchSetFileEngineStatus( "Local file replaced", eventdata );
    }
    else
    if ( eventid == PATCHER::CPatchSetFileEngineEvents::LocalFileReplacementFailure )
    {
        PrintPatchSetFileEngineStatus( "Local file replacement failed", eventdata );

        // Add the file combo to the list of file we will try to replace after patcher shutdown        
        const GUCEF::PATCHER::CPatchSetFileEngineEvents::TPatchSetFileEngineEventData* eventDataWrapper = static_cast< GUCEF::PATCHER::CPatchSetFileEngineEvents::TPatchSetFileEngineEventData* >( eventdata );
        const GUCEF::PATCHER::CPatchSetFileEngineEvents::TPatchSetFileEngineEventDataStorage& storage = eventDataWrapper->GetData();
        
        GUCEF::CORE::CString oldFile = storage.localRoot;
        GUCEF::CORE::AppendToPath( oldFile, storage.currentFileEntry.name );
        
        GUCEF::CORE::CString newFile = storage.tempStorageRoot;
        GUCEF::CORE::AppendToPath( newFile, storage.currentFileEntry.name );
                
        m_fileReplacements[ oldFile ] = newFile;
    }    
    else
    {
        if ( ( GUCEF_NULL != m_listBox )                            &&
             ( eventid != CORE::CNotifier::DestructionEvent ) &&
             ( eventid != CORE::CNotifier::UnsubscribeEvent )  )
        {
            PrintOutput( "Event: " + eventid.GetName() );
            PrintOutput( "-------------------------------------" );
        }
    }
}

/*-------------------------------------------------------------------------*/
