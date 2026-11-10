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

#ifndef GUCEF_GUI_CPATHFIELDFORM_H
#define GUCEF_GUI_CPATHFIELDFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CFILESYSTEMDIALOGFORM_H
#include "gucefGUI_CFileSystemDialogForm.h"
#define GUCEF_GUI_CFILESYSTEMDIALOGFORM_H
#endif /* GUCEF_GUI_CFILESYSTEMDIALOGFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A path to fill in: a line of text with a button next to it that opens a file system dialog to choose the folder or
 *  file, which then goes in the line. It takes the place of a widget of another form (Build( placeholder ) fills that
 *  widget), so a settings page can have a path where it would have an edit box.
 *
 *  A path chosen in the dialog is a change made by the user: the edit box routes a ValueChangedEvent, as when the path is
 *  typed, and PathChosenEvent is notified. Without roots of its own the dialog browses the file system of this computer.
 *  The dialog is shown in the context of the field; the form goes before that context, as forms do.
 */
class GUCEF_GUI_PUBLIC_CPP CPathFieldForm : public CFormEx
{
    public:

    static const CORE::CEvent PathChosenEvent;

    static void RegisterEvents( void );

    enum EPathKind
    {
        PATHKIND_DIRECTORY = 0,     /**< a folder, chosen with the directory open dialog */
        PATHKIND_FILE               /**< a file, chosen with the file open dialog */
    };

    explicit CPathFieldForm( EPathKind kind = PATHKIND_DIRECTORY );

    CPathFieldForm( CIGUIContext* context, EPathKind kind = PATHKIND_DIRECTORY );

    virtual ~CPathFieldForm();

    /** Makes the field in 'placeholder', which it fills (as a settings page is built in its pane) */
    bool Build( CWidget* placeholder );

    void SetPath( const CString& path );

    CString GetPath( void );

    EPathKind GetPathKind( void ) const;

    /** The title of the dialog; empty for the one of the dialog */
    void SetDialogTitle( const CString& title );

    /** A file system the dialog browses (see CFileSystemDialogForm::AddRoot()); none: this computer */
    bool AddRoot( const CString& displayName, CORE::TFileSystemAccessPtr access, const CString& initialDirectory = CString::Empty );

    /** File fields: a kind of file the dialog offers (see CFileSystemDialogForm::AddFileType()) */
    void AddFileType( const CString& description, const CString::StringVector& patterns );

    /** Opens the dialog as the button does, in the folder of the path when it is there */
    bool OpenDialog( void );

    /** The dialog, once it was opened */
    CFileSystemDialogForm* GetDialog( void );

    /** The nearest folder of a path that exists (the path when it is a folder), empty when there is none */
    static CString NearestExistingFolder( const CString& path );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CPathFieldForm > TEventCallback;

    /** The button that opens the dialog was activated */
    void OnBrowseActivated( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    /** The user accepted a path in the dialog */
    void OnDialogAccepted( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );

    struct SRoot
    {
        CString name;
        CORE::TFileSystemAccessPtr access;
        CString initialDirectory;
    };
    typedef GUCEF::vector< SRoot > TRootVector;

    struct SFileType
    {
        CString description;
        CString::StringVector patterns;
    };
    typedef GUCEF::vector< SFileType > TFileTypeVector;

    bool MakeDialog( void );

    CPathFieldForm( const CPathFieldForm& src );
    CPathFieldForm& operator=( const CPathFieldForm& src );

    private:

    EPathKind m_kind;
    CString m_dialogTitle;
    TRootVector m_roots;
    TFileTypeVector m_fileTypes;
    CFileSystemDialogForm* m_dialog;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CPATHFIELDFORM_H ? */
