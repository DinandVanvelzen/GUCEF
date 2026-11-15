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
#include "MFCPatcherGUI.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(GUCEF_NULL, GUCEF_NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 480, 160), this, AFX_IDW_PANE_FIRST, GUCEF_NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}
	
	//ModifyStyle(WS_SYSMENU, NULL);
	
    // Remove positions of menu
    CMenu* pTopMenu = GetSystemMenu(FALSE);
    if( pTopMenu != GUCEF_NULL)
    {   
       // pTopMenu->DestroyMenu();
        pTopMenu->RemoveMenu(SC_SIZE, MF_BYCOMMAND);      // no resize
       // pTopMenu->RemoveMenu(SC_MOVE, MF_BYCOMMAND);      //non-Moveable window
        pTopMenu->RemoveMenu(SC_MAXIMIZE, MF_BYCOMMAND);  //no Maximize
    }	
	return 0;
}

/*-------------------------------------------------------------------------*/

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{GUCEF_TRACE;

   // MoveWindow( 0, 0, 220, 160 );
  //  m_listBox->MoveWindow(0, 0, cx, 100);
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.cx = 480;
	cs.cy = 300;
	
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs


    // Create a window without max button and sizable border
    cs.style &= ~(WS_MAXIMIZEBOX|WS_THICKFRAME);

	//cs.style = WS_OVERLAPPED | WS_CAPTION 
	//	 | WS_MINIMIZEBOX;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


