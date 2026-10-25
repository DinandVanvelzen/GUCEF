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

#ifndef GUCEF_GUI_CMESSAGEBOXFORM_H
#define GUCEF_GUI_CMESSAGEBOXFORM_H

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
 *  A small modal window with a message and buttons: OK, OK and Cancel, Yes and No, or Yes, No and Cancel. It can also ask for a
 *  line of text (OpenInput()). Enter in the text chooses the first button, Escape the last one. When it closes it notifies
 *  ClosedEvent with the button (TCloneableInt32 of EMessageBoxResult); GetResult() and GetInputText() give the answer after.
 *  The layout is built in code and made for BuildDarkTheme().
 */
class GUCEF_GUI_PUBLIC_CPP CMessageBoxForm : public CFormEx
{
    public:

    static const CORE::CEvent ClosedEvent;

    static void RegisterEvents( void );

    enum EMessageBoxButtons
    {
        MESSAGEBOXBUTTONS_OK = 0,
        MESSAGEBOXBUTTONS_OK_CANCEL,
        MESSAGEBOXBUTTONS_YES_NO,
        MESSAGEBOXBUTTONS_YES_NO_CANCEL
    };

    enum EMessageBoxResult
    {
        MESSAGEBOXRESULT_NONE = 0,  /**< still open, or never opened */
        MESSAGEBOXRESULT_OK,
        MESSAGEBOXRESULT_CANCEL,
        MESSAGEBOXRESULT_YES,
        MESSAGEBOXRESULT_NO
    };

    CMessageBoxForm( void );

    CMessageBoxForm( CIGUIContext* context );

    virtual ~CMessageBoxForm();

    bool Build( void );

    /** Shows the message, modal on top of what is shown */
    bool Open( const CString& title, const CString& text, EMessageBoxButtons buttons );

    /** Shows the message with a line of text to fill in */
    bool OpenInput( const CString& title, const CString& text, const CString& initialInput, EMessageBoxButtons buttons );

    /** Closes as if the button was pressed */
    void Choose( EMessageBoxResult result );

    EMessageBoxResult GetResult( void ) const;

    CString GetInputText( void );

    bool IsOpen( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CMessageBoxForm > TEventCallback;

    /** A button of the slot was activated: chooses what the slot means */
    void OnButton1Activated( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData );

    void OnButton2Activated( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData );

    void OnButton3Activated( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData );

    /** Enter in the text chooses the first button that is there */
    void OnInputTextEntered( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData );

    /** Escape chooses the last button */
    void OnNavigation( CORE::CNotifier* notifier    ,
                       const CORE::CEvent& eventId  ,
                       CORE::CICloneable* eventData );

    void ChooseSlot( UInt32 slot );

    void BuildLayout( CORE::CDataNode& layout );

    bool Show( const CString& title, const CString& text, const CString* initialInput, EMessageBoxButtons buttons );

    CMessageBoxForm( const CMessageBoxForm& src );
    CMessageBoxForm& operator=( const CMessageBoxForm& src );

    private:

    static const UInt32 MaxButtons = 3;

    EMessageBoxResult m_buttonResults[ MaxButtons ];    /**< what each button slot means, NONE for an unused slot */
    EMessageBoxResult m_result;
    bool m_open;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CMESSAGEBOXFORM_H ? */
