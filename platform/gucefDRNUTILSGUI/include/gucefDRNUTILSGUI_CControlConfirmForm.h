/*
 *  gucefDRNUTILSGUI: GUCEF module providing the forms that view and control job schedulers and the task manager
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUCEF_DRNUTILSGUI_CCONTROLCONFIRMFORM_H
#define GUCEF_DRNUTILSGUI_CCONTROLCONFIRMFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_DRNUTILSGUI_MACROS_H
#include "gucefDRNUTILSGUI_macros.h"
#define GUCEF_DRNUTILSGUI_MACROS_H
#endif /* GUCEF_DRNUTILSGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILSGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Confirmation of a control command: the target, the action and a reason field that goes into the request
 *  (behaviour of a message box form). ConfirmedEvent carries the reason the operator typed.
 */
class GUCEF_DRNUTILSGUI_PUBLIC_CPP CControlConfirmForm : public GUI::CFormEx
{
    public:

    static const CORE::CString ClassTypeName;

    static const CORE::CEvent ConfirmedEvent;       /**< TConfirmedEventData: the reason */
    static const CORE::CEvent CancelledEvent;       /**< no data */

    typedef CORE::TCloneableString TConfirmedEventData;

    static void RegisterEvents( void );

    CControlConfirmForm( void );

    CControlConfirmForm( GUI::CIGUIContext* context );

    virtual ~CControlConfirmForm() GUCEF_VIRTUAL_OVERRIDE;

    bool Build( void );

    /** Shows the question modally over the other forms; the answer comes as ConfirmedEvent or CancelledEvent. The reason field starts empty */
    bool Ask( const CORE::CString& title, const CORE::CString& actionText, const CORE::CString& targetText );

    bool IsOpen( void ) const;

    /** As the buttons do: Confirm() takes the given reason (the Confirm button takes the text of the reason field) */
    void Confirm( const CORE::CString& reason );
    void Cancel( void );

    /** The reason of the last confirmation */
    const CORE::CString& GetReason( void ) const;

    /** The texts the question shows, for tests and for a status line */
    const CORE::CString& GetActionText( void ) const;
    const CORE::CString& GetTargetText( void ) const;

    /** What the operator typed in the reason field so far */
    CORE::CString GetTypedReason( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CControlConfirmForm > TEventCallback;

    void OnConfirmActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCancelActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout );
    void SetLabelText( const char* widgetName, const CORE::CString& text );

    CControlConfirmForm( const CControlConfirmForm& src );                /**< not implemented */
    CControlConfirmForm& operator=( const CControlConfirmForm& src );     /**< not implemented */

    CORE::CString m_reason;
    CORE::CString m_actionText;
    CORE::CString m_targetText;
    bool m_open;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILSGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILSGUI_CCONTROLCONFIRMFORM_H ? */
