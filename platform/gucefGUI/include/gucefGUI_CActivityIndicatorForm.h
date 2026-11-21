/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CACTIVITYINDICATORFORM_H
#define GUCEF_GUI_CACTIVITYINDICATORFORM_H

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

#ifndef GUCEF_GUI_CACTIVITYSERVICE_H
#include "gucefGUI_CActivityService.h"
#define GUCEF_GUI_CACTIVITYSERVICE_H
#endif /* GUCEF_GUI_CACTIVITYSERVICE_H ? */

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
 *  The visible side of the CActivityService for the status area of an application: it takes the place of a widget (Build(
 *  placeholder )), wide and about a line high, and shows nothing while no operation is shown by the service. Else it shows
 *  the newest operation that is shown: its label, a progress bar (the fraction of a determinate operation, else a bar that
 *  runs to and fro while the operation is busy), how many operations there are when it is more than one, and a Cancel button
 *  when the operation can be cancelled (it asks the service, RequestCancel(), and the button is disabled after).
 *  The form follows the service by its events, and animates on the frames of the context. From the left: the label and the
 *  bar share the room that the count (90 px) and the button (80 px) leave; the label is elided when it is too long.
 */
class GUCEF_GUI_PUBLIC_CPP CActivityIndicatorForm : public CFormEx
{
    public:

    enum EDefaults
    {
        IndicatorRowHeight = 30     ,   /**< the height the form is made for, logical pixels */
        IndeterminatePeriodInMs = 1600  /**< one run of the bar of an indeterminate operation, to and fro */
    };

    CActivityIndicatorForm( void );

    CActivityIndicatorForm( CIGUIContext* context );

    virtual ~CActivityIndicatorForm();

    bool Build( CWidget* placeholder );

    /** The service whose operations are shown; NULL: nothing. The service must outlive the form or be destroyed while set (the form notices) */
    void SetService( CActivityService* service );

    CActivityService* GetService( void ) const;

    /** Reads the service again: done on its events, public for a test and for an application that wants it at once */
    void Refresh( void );

    /** Whether the indicator is on screen: the service shows an operation */
    bool IsIndicatorShown( void ) const;

    /** The operation the indicator shows, 0 for none */
    UInt32 GetDisplayedOperation( void ) const;

    static UInt32 GetPreferredHeight( void ) { return (UInt32) IndicatorRowHeight; }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** The destruction of the service: the form lets go of it (the standard events reach only this) */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CActivityIndicatorForm > TEventCallback;

    void OnServiceChanged( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );

    void OnCancelActivated( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    void OnFrameStarted( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void ShowIndeterminatePhase( void );

    CActivityIndicatorForm( const CActivityIndicatorForm& src );
    CActivityIndicatorForm& operator=( const CActivityIndicatorForm& src );

    private:

    CActivityService* m_service;
    UInt32 m_displayed;
    bool m_indeterminate;
    UInt32 m_phaseInMs;
    bool m_frameSubscribed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CACTIVITYINDICATORFORM_H ? */
