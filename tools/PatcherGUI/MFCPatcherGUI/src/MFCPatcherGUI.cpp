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

#include "stdafx.h"

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CCOM_H
#include "CCom.h"
#define GUCEF_COMCORE_CCOM_H
#endif /* GUCEF_COMCORE_CCOM_H ? */

#ifndef GUCEF_COM_CHTTPCLIENT_H
#include "CHTTPClient.h"
#define GUCEF_COM_CHTTPCLIENT_H
#endif /* GUCEF_COM_CHTTPCLIENT_H ? */

#include "MFCPatcherGUI.h"
#include "MainFrm.h"
#include "CMFCCommandLineInfo.h"

// CMFCPatcherGUIApp

BEGIN_MESSAGE_MAP(CMFCPatcherGUIApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CMFCPatcherGUIApp::OnAppAbout)
END_MESSAGE_MAP()


// CMFCPatcherGUIApp construction

CMFCPatcherGUIApp::CMFCPatcherGUIApp()
    : m_cmdLineParamList()
{

    // Make sure that logging is up and running before we do anything else
    GUCEF::CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
    GUCEF::CORE::AppendToPath( logFilename, "MFCPatcherGUI_Log.txt" );
    GUCEF::CORE::CFileAccess logFileAccess( logFilename, "w" );
    
    GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &m_logger );
    
    #if defined( GUCEF_MSWIN_BUILD ) && defined( GUCEF_PATCHER_DEBUG_MODE )
    GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &m_consoleOut );
    #endif /* GUCEF_MSWIN_BUILD && GUCEF_PATCHER_DEBUG_MODE ? */    
    
    // We have to call some code in several modules as a hacky method for those modules to actually
    // be linked and loaded. Those modules can auto-register functionality.
    // Some compilers/linkers attempt to optimize by removing dependencies on modules that have no 
    // direct calls from this app.
    GUCEF::VFS::CVfsGlobal::Instance();
    GUCEF::COMCORE::CComCoreGlobal::Instance();
    GUCEF::COM::CHTTPClient::RegisterEvents();
        
	// Place all significant initialization in InitInstance
}


// The one and only CMFCPatcherGUIApp object

CMFCPatcherGUIApp theApp;

CFrameWnd*
CMFCPatcherGUIApp::GetMainFrame( void )
{GUCEF_TRACE;

    return static_cast< CFrameWnd* >( m_pMainWnd );
}

// CMFCPatcherGUIApp initialization

BOOL CMFCPatcherGUIApp::InitInstance()
{
    // Parse the command line params	
    CMFCCommandLineInfo oInfo;
    ParseCommandLine( oInfo );
    m_cmdLineParamList.SetMultiple( oInfo.GetParamString(), '\'' );

    // Obtain the plugin dir path
    GUCEF::CORE::CString pluginDir = m_cmdLineParamList.GetValueAlways( "PluginDir" );    
    if ( pluginDir.Length() == 0 )
    {
        pluginDir = "$MODULEDIR$\\plugins";
        pluginDir = GUCEF::CORE::RelativePath( pluginDir );
        if ( !GUCEF::CORE::IsPathValid( pluginDir ) )
        {
            pluginDir = "$CURWORKDIR$\\plugins";
            pluginDir = GUCEF::CORE::RelativePath( pluginDir );    
        }
    }
    
    // Load all plugins
    GUCEF::CORE::CPluginControl* pluginControl = &GUCEF::CORE::CCoreGlobal::Instance()->GetPluginControl();
    pluginControl->AddPluginDir( pluginDir );
    pluginControl->LoadAll();

    // Check for mandatory command line params
    // If we do not find any params we will use defaults, this makes it easy to use the tool 
    // for rapid deployment while keeping your options open.
    if ( !m_cmdLineParamList.HasKey( "ConfigFile" ) )
    {
        m_cmdLineParamList.Set( "ConfigFile", "$CURWORKDIR$\\MFCPatcherConfig.xml" );
    }
    if ( !m_cmdLineParamList.HasKey( "ConfigFileCodec" ) )
    {
        m_cmdLineParamList.Set( "ConfigFileCodec", "xml" );
    }

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored	
	SetRegistryKey(_T("VanvelzenSoftware"));
	
	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	
	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, GUCEF_NULL,
		GUCEF_NULL);

	// The one and only window has been initialized, so show and update it
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
}

const GUCEF::CORE::CValueList&
CMFCPatcherGUIApp::GetCommandLineParams( void ) const
{GUCEF_TRACE;
    
    return m_cmdLineParamList;
}

// CMFCPatcherGUIApp message handlers




// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CMFCPatcherGUIApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CMFCPatcherGUIApp message handlers

