/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#include "gucefCORE_CNativeFileSystemAccess.h"
#define GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CFILEOPENDIALOGFORM_H
#include "gucefGUI_CFileOpenDialogForm.h"
#define GUCEF_GUI_CFILEOPENDIALOGFORM_H
#endif /* GUCEF_GUI_CFILEOPENDIALOGFORM_H ? */

#ifndef GUCEF_GUI_CDIRECTORYOPENDIALOGFORM_H
#include "gucefGUI_CDirectoryOpenDialogForm.h"
#define GUCEF_GUI_CDIRECTORYOPENDIALOGFORM_H
#endif /* GUCEF_GUI_CDIRECTORYOPENDIALOGFORM_H ? */

#include "gucefGUI_CPathFieldForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CPathFieldForm::PathChosenEvent = "GUCEF::GUI::CPathFieldForm::PathChosenEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CPathFieldForm::RegisterEvents( void )
{GUCEF_TRACE;

    PathChosenEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CPathFieldForm::CPathFieldForm( EPathKind kind )
    : CFormEx()
    , m_kind( kind )
    , m_dialogTitle()
    , m_roots()
    , m_fileTypes()
    , m_dialog( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CPathFieldForm::CPathFieldForm( CIGUIContext* context, EPathKind kind )
    : CFormEx( context )
    , m_kind( kind )
    , m_dialogTitle()
    , m_roots()
    , m_fileTypes()
    , m_dialog( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CPathFieldForm::~CPathFieldForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    GUCEF_DELETE m_dialog;
    m_dialog = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CPathFieldForm::Build( CWidget* placeholder )
{GUCEF_TRACE;

    if ( GUCEF_NULL == placeholder )
        return false;
    SetContext( placeholder->GetContext() );
    SetParent( placeholder );

    // the line takes what the button leaves
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    CORE::CDataNode* field = CFormLayout::AddWidgetNode( layout, "Panel", "PathField", "0,0 0,0", "1,0 1,0" );
    CFormLayout::AddWidgetNode( *field, "Editbox", "Path", "0,0 0,0", "1,-48 1,0" );
    CORE::CDataNode* browse = CFormLayout::AddWidgetNode( *field, "Button", "Browse", "1,-40 0,0", "0,40 1,0" );
    CFormLayout::SetWidgetNodeProperty( *browse, EWidgetProperty_Text, "..." );
    CFormLayout::SetWidgetNodeProperty( *browse, EWidgetProperty_Tooltip, PATHKIND_DIRECTORY == m_kind ? "Choose the folder" : "Choose the file" );
    return LoadLayout( layout );
}

/*-------------------------------------------------------------------------*/

void
CPathFieldForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();
    CWidget* browse = GetWidget( "Browse" );
    if ( GUCEF_NULL != browse )
    {
        TEventCallback activatedCallback( this, &CPathFieldForm::OnBrowseActivated );
        SubscribeTo( browse, CWidget::ActivatedEvent, activatedCallback );
    }
}

/*-------------------------------------------------------------------------*/

void
CPathFieldForm::SetPath( const CString& path )
{GUCEF_TRACE;

    CWidget* line = GetWidget( "Path" );
    if ( GUCEF_NULL != line )
        line->SetProperty( EWidgetProperty_Text, path );
}

/*-------------------------------------------------------------------------*/

CString
CPathFieldForm::GetPath( void )
{GUCEF_TRACE;

    CWidget* line = GetWidget( "Path" );
    return GUCEF_NULL != line ? line->GetProperty( EWidgetProperty_Text ) : CString::Empty;
}

/*-------------------------------------------------------------------------*/

CPathFieldForm::EPathKind
CPathFieldForm::GetPathKind( void ) const
{GUCEF_TRACE;

    return m_kind;
}

/*-------------------------------------------------------------------------*/

void
CPathFieldForm::SetDialogTitle( const CString& title )
{GUCEF_TRACE;

    m_dialogTitle = title;
}

/*-------------------------------------------------------------------------*/

bool
CPathFieldForm::AddRoot( const CString& displayName, CORE::TFileSystemAccessPtr access, const CString& initialDirectory )
{GUCEF_TRACE;

    if ( displayName.IsNULLOrEmpty() || access.IsNULL() || GUCEF_NULL != m_dialog )
        return false;
    SRoot root;
    root.name = displayName;
    root.access = access;
    root.initialDirectory = initialDirectory;
    m_roots.push_back( root );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CPathFieldForm::AddFileType( const CString& description, const CString::StringVector& patterns )
{GUCEF_TRACE;

    SFileType fileType;
    fileType.description = description;
    fileType.patterns = patterns;
    m_fileTypes.push_back( fileType );
}

/*-------------------------------------------------------------------------*/

CString
CPathFieldForm::NearestExistingFolder( const CString& path )
{GUCEF_TRACE;

    CString folder = CORE::RelativePath( path ).ReplaceChar( '\\', '/' );
    while ( !folder.IsNULLOrEmpty() )
    {
        if ( CORE::DirExists( folder ) )
            return folder;
        CString parent = CORE::StripLastSubDir( folder ).ReplaceChar( '\\', '/' );
        if ( parent == folder )
            break;
        folder = parent;
    }
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CPathFieldForm::MakeDialog( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_dialog )
        return true;
    CIGUIContext* context = GetContext();
    if ( GUCEF_NULL == context )
        return false;

    if ( PATHKIND_DIRECTORY == m_kind )
        m_dialog = GUCEF_NEW CDirectoryOpenDialogForm( context );
    else
        m_dialog = GUCEF_NEW CFileOpenDialogForm( context );
    if ( !m_dialogTitle.IsNULLOrEmpty() )
        m_dialog->GetOptions().title = m_dialogTitle;
    m_dialog->GetOptions().readOnly = true;     // a place is chosen, nothing is changed

    if ( m_roots.empty() )
    {
        CORE::TFileSystemAccessPtr native = ( GUCEF_NEW CORE::CNativeFileSystemAccess( "This computer" ) )->CreateSharedPtr();
        CString home;
        CORE::TryResolveSpecialDir( CORE::SPECIALDIR_HOME_DIR, home );
        m_dialog->AddRoot( "This computer", native, home );
    }
    for ( UInt32 i=0; i<(UInt32)m_roots.size(); ++i )
        m_dialog->AddRoot( m_roots[ i ].name, m_roots[ i ].access, m_roots[ i ].initialDirectory );
    for ( UInt32 i=0; i<(UInt32)m_fileTypes.size(); ++i )
        m_dialog->AddFileType( m_fileTypes[ i ].description, m_fileTypes[ i ].patterns );

    if ( !m_dialog->Build() )
    {
        GUCEF_DELETE m_dialog;
        m_dialog = GUCEF_NULL;
        return false;
    }
    TEventCallback acceptedCallback( this, &CPathFieldForm::OnDialogAccepted );
    SubscribeTo( m_dialog, CFileSystemDialogForm::AcceptedEvent, acceptedCallback );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPathFieldForm::OpenDialog( void )
{GUCEF_TRACE;

    if ( !MakeDialog() )
        return false;

    // in the folder of the path, as far as that is still there; the first root is the one paths of this computer are in
    CString folder = NearestExistingFolder( GetPath() );
    if ( !folder.IsNULLOrEmpty() && m_roots.empty() )
    {
        m_dialog->GetOptions().initialRoot = "This computer";
        m_dialog->GetOptions().initialDirectory = folder;
    }
    return m_dialog->Open();
}

/*-------------------------------------------------------------------------*/

CFileSystemDialogForm*
CPathFieldForm::GetDialog( void )
{GUCEF_TRACE;

    return m_dialog;
}

/*-------------------------------------------------------------------------*/

void
CPathFieldForm::OnBrowseActivated( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    OpenDialog();
}

/*-------------------------------------------------------------------------*/

void
CPathFieldForm::OnDialogAccepted( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_dialog )
        return;
    const CFileSystemDialogForm::TSelectionVector& selection = m_dialog->GetSelection();
    CWidget* line = GetWidget( "Path" );
    if ( !selection.empty() && GUCEF_NULL != line )
    {
        // chosen by the user: as if it was typed
        line->ChangeValueByUser( EWidgetProperty_Text, selection.front().path );
        NotifyObservers( PathChosenEvent );
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CPathFieldForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CPathFieldForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
