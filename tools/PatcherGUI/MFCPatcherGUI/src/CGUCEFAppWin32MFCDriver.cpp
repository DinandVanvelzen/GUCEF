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

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"                  /* Main application object for GUCEF applications */
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#include "CGUCEFAppWin32MFCDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEFWIN32DRIVERMSG_STARTTIMER (WM_USER+164)
#define GUCEFWIN32DRIVERMSG_STOPTIMER  (WM_USER+165)
#define GUCEFWIN32DRIVERMSG_UPDATE     (WM_USER+163)

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

BEGIN_MESSAGE_MAP( CGUCEFAppWin32MFCDriver, CWnd )
	//{{AFX_MSG_MAP(CXBAppWin32Driver)
    ON_WM_TIMER()
    ON_WM_CLOSE()
    ON_MESSAGE( GUCEFWIN32DRIVERMSG_STARTTIMER, OnStartUpdateTimer )
    ON_MESSAGE( GUCEFWIN32DRIVERMSG_STOPTIMER, OnStopUpdateTimer )
    ON_MESSAGE( GUCEFWIN32DRIVERMSG_UPDATE, OnGUCEFUpdateRequestMsg )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CGUCEFAppWin32MFCDriver::CGUCEFAppWin32MFCDriver( void )
    : CWnd()                                ,
      GUCEF::CORE::CIPulseGeneratorDriver() ,
      m_nTimer( 0 )                         ,
      m_frequency( 10 )                     ,
      m_initialized( false )                ,
      m_useTimer( false )                   ,
      m_pulseGenerator( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGUCEFAppWin32MFCDriver::~CGUCEFAppWin32MFCDriver()
{GUCEF_TRACE;

    if ( GetSafeHwnd() )
    {
        OnStopUpdateTimer( 0, 0 );
        DestroyWindow();
    }
    if ( GUCEF_NULL != m_pulseGenerator )
    {
        m_pulseGenerator->SetPulseGeneratorDriver( GUCEF_NULL );
    }
}

/*-------------------------------------------------------------------------*/

bool
CGUCEFAppWin32MFCDriver::Init( CWnd* pParentWnd                             ,
                               GUCEF::CORE::CPulseGenerator& pulseGenerator )
{GUCEF_TRACE;

    if ( !m_initialized )
    {
        if ( Create( GUCEF_NULL                                     ,
                     (LPCTSTR)"CGUCEFAppWin32MFCDriverWindow" ,
                     WS_CHILD|WS_VISIBLE                      ,
                     CRect(0, 0, 0, 0)                        ,
                     pParentWnd                               ,
                     1234                                     ) )
        {    
            m_initialized = true;
            m_pulseGenerator = &pulseGenerator;
            pulseGenerator.SetPulseGeneratorDriver( this );
            return true;
        }
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGUCEFAppWin32MFCDriver::RequestPulseInterval( GUCEF::CORE::CPulseGenerator& pulseGenerator    ,
                                               const GUCEF::CORE::UInt32 pulseDeltaInMilliSecs )
{GUCEF_TRACE;

    m_frequency = pulseDeltaInMilliSecs;
    if ( pulseGenerator.IsPulsingPeriodicly() )
    {
        /*
         *  Reset the timer with the new timer interval
         */
        m_useTimer = true;
        if ( GetSafeHwnd() )
        {
            PostMessage( GUCEFWIN32DRIVERMSG_STARTTIMER, 0, (LPARAM)&pulseGenerator );
        }
    }
}

/*-------------------------------------------------------------------------*/

void 
CGUCEFAppWin32MFCDriver::RequestPulse( GUCEF::CORE::CPulseGenerator& pulseGenerator )
{GUCEF_TRACE;

    if ( GetSafeHwnd() )
    {
        PostMessage( GUCEFWIN32DRIVERMSG_UPDATE, 0, (LPARAM)&pulseGenerator );
    }
}

/*-------------------------------------------------------------------------*/

afx_msg LRESULT
CGUCEFAppWin32MFCDriver::OnGUCEFUpdateRequestMsg( WPARAM wParam , 
                                                  LPARAM lParam )
{GUCEF_TRACE;

    GUCEF::CORE::CPulseGenerator& pulseGenerator = *( (GUCEF::CORE::CPulseGenerator*) lParam );
    SendDriverPulse( pulseGenerator );
    return 0;
}

/*-------------------------------------------------------------------------*/

afx_msg LRESULT 
CGUCEFAppWin32MFCDriver::OnStartUpdateTimer( WPARAM wParam , 
                                             LPARAM lParam )
{GUCEF_TRACE;

    if ( GetSafeHwnd() )
    {
        if ( m_nTimer > 0 )
        {
            KillTimer( m_nTimer );
        }
        m_nTimer = (UINT) SetTimer( GUCEFWIN32DRIVERMSG_UPDATE, (DWORD)m_frequency, 0 );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

afx_msg LRESULT 
CGUCEFAppWin32MFCDriver::OnStopUpdateTimer( WPARAM wParam , 
                                            LPARAM lParam )
{GUCEF_TRACE;

    if ( GetSafeHwnd() )
    {    
        if ( m_nTimer )
        {
            KillTimer( m_nTimer );
            m_nTimer = 0;
        }
    }
    m_useTimer = false;
    return 0;
}

/*-------------------------------------------------------------------------*/

void
CGUCEFAppWin32MFCDriver::OnClose()
{GUCEF_TRACE;

    if ( GetSafeHwnd() )
    {
        PostMessage( GUCEFWIN32DRIVERMSG_STOPTIMER );
    }
    
    if ( GUCEF_NULL != m_pulseGenerator )
    {
        m_pulseGenerator->SetPulseGeneratorDriver( GUCEF_NULL );
    }
    CWnd::OnClose();
}

/*-------------------------------------------------------------------------*/

afx_msg void
CGUCEFAppWin32MFCDriver::OnTimer( UINT nIDEvent ) 
{GUCEF_TRACE;

    if ( nIDEvent == GUCEFWIN32DRIVERMSG_UPDATE )
    {
        if ( GetSafeHwnd() )
        {
            if ( GUCEF_NULL != m_pulseGenerator )
            {
                SendDriverPulse( *m_pulseGenerator );
            }
        }
    }
    CWnd::OnTimer( nIDEvent );
}

/*-------------------------------------------------------------------------*/

void
CGUCEFAppWin32MFCDriver::RequestStopOfPeriodicUpdates( GUCEF::CORE::CPulseGenerator& pulseGenerator )
{GUCEF_TRACE;

    if ( GetSafeHwnd() )
    {
        PostMessage( GUCEFWIN32DRIVERMSG_STOPTIMER, 0, (LPARAM)&pulseGenerator );
    }
    m_useTimer = false;    
}

/*-------------------------------------------------------------------------*/

void
CGUCEFAppWin32MFCDriver::RequestPeriodicPulses( GUCEF::CORE::CPulseGenerator& pulseGenerator    ,
                                                const GUCEF::CORE::UInt32 pulseDeltaInMilliSecs )
{GUCEF_TRACE;

    // don't start the timer twice
    if ( m_useTimer )
    {
        return;
    }
    
    // Get the update frequency desired and post the timer start message
    m_frequency = pulseGenerator.GetRequiredPulseDeltaInMilliSecs();
    if ( GetSafeHwnd() )
    {
        PostMessage( GUCEFWIN32DRIVERMSG_STARTTIMER, 0, (LPARAM)&pulseGenerator );
    }
    m_useTimer = true;    
}

/*-------------------------------------------------------------------------*/