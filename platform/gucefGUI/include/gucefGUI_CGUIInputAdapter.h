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

#ifndef GUCEF_GUI_CGUIINPUTADAPTER_H
#define GUCEF_GUI_CGUIINPUTADAPTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

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

class CGUIContext;

/**
 *  Feeds what the gucefINPUT mice and keyboards report into a GUI context, so a driver does not need
 *  input code of its own: pointer positions in window pixels (mapped to logical pixels by the context),
 *  buttons, the wheel, keys with their modifiers and typed text.
 *
 *  Mice and keyboards attached later are picked up as well. With an input context id only input of
 *  that gucefINPUT context (the window the GUI context draws in) is passed on. While a GUI context on a
 *  higher layer of the same window shows something, the input is not passed on: that layer takes it.
 */
class GUCEF_GUI_PUBLIC_CPP CGUIInputAdapter : public CORE::CObserver
{
    public:

    /** Passes on the input of every input context */
    static const Int32 AnyInputContext;

    CGUIInputAdapter( CGUIContext& guiContext, Int32 inputContextId = AnyInputContext );

    virtual ~CGUIInputAdapter();

    /** Starts passing input on: subscribes to the input controller and every mouse and keyboard */
    void Attach( void );

    /** Stops passing input on */
    void Detach( void );

    bool IsAttached( void ) const;

    void SetInputContextId( Int32 inputContextId );

    Int32 GetInputContextId( void ) const;

    /** The GUI button for a gucefINPUT button index: 0 left, 1 right, 2 middle, 3 and 4 the side buttons */
    static TMouseButton MapMouseButton( UInt32 buttonIndex );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    bool IsForUs( Int32 inputContextId ) const;

    CGUIInputAdapter( const CGUIInputAdapter& src );            /**< not implemented */
    CGUIInputAdapter& operator=( const CGUIInputAdapter& src ); /**< not implemented */

    private:

    CGUIContext* m_guiContext;
    Int32 m_inputContextId;
    bool m_attached;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGUIINPUTADAPTER_H ? */
