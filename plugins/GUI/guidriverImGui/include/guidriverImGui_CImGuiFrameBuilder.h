/*
 *  guidriverImGui: the gucefGUI driver library of Dear ImGui, shared by its renderer drivers
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

#ifndef GUIDRIVERIMGUI_CIMGUIFRAMEBUILDER_H
#define GUIDRIVERIMGUI_CIMGUIFRAMEBUILDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTWEAKPTR_H
#include "gucefCORE_CTWeakPtr.h"
#define GUCEF_CORE_CTWEAKPTR_H
#endif /* GUCEF_CORE_CTWEAKPTR_H ? */

#ifndef GUCEF_GUI_CIGUICONTEXT_H
#include "gucefGUI_CIGUIContext.h"
#define GUCEF_GUI_CIGUICONTEXT_H
#endif /* GUCEF_GUI_CIGUICONTEXT_H ? */

#ifndef GUIDRIVERIMGUI_MACROS_H
#include "guidriverImGui_macros.h"
#define GUIDRIVERIMGUI_MACROS_H
#endif /* GUIDRIVERIMGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CImGuiContext;

/**
 *  The base of an immediate-mode tool: it builds its part of every frame of an ImGui context in OnBuildFrame(), called
 *  from the context's BuildFrameEvent while the tool is visible, with the Dear ImGui context current (ImGui::Begin() and
 *  the rest are called directly there and only there).
 *
 *  The context is kept weakly: its owners decide when it goes. A context that goes ends the subscription by itself and
 *  IsAttached() then answers false; a builder that goes is not called again. Handlers of BuildFrameEvent are called in
 *  no particular order, so a builder does not depend on the others.
 */
class GUIDRIVERIMGUI_PUBLIC_CPP CImGuiFrameBuilder : public CORE::CObservingNotifier
{
    public:

    typedef CORE::CTWeakPtr< GUI::CIGUIContext, MT::CMutex > TGuiContextWeakPtr;

    CImGuiFrameBuilder( void );

    virtual ~CImGuiFrameBuilder();

    /** Builds in the frames of the context from now on; false when the context is no ImGui context */
    bool Attach( GUI::TGuiContextPtr context );

    void Detach( void );

    /** Whether it is attached to a context that still exists */
    bool IsAttached( void ) const;

    /** The context it builds in, NULL when it is not attached or the context went */
    GUI::TGuiContextPtr GetContext( void ) const;

    /** Only a visible builder is called; visible by default */
    void SetVisible( bool visible );

    bool IsVisible( void ) const;

    /** How often OnBuildFrame() was called */
    UInt32 GetBuildCount( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** Builds the part of the frame of the tool; 'deltaInMs' is the frame time */
    virtual void OnBuildFrame( CImGuiContext& context, UInt32 deltaInMs ) = 0;

    private:

    typedef CORE::CTEventHandlerFunctor< CImGuiFrameBuilder > TEventCallback;

    CImGuiFrameBuilder( const CImGuiFrameBuilder& src );
    CImGuiFrameBuilder& operator=( const CImGuiFrameBuilder& src );

    void OnBuildFrameEvent( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    private:

    TGuiContextWeakPtr m_context;
    CImGuiContext* m_imguiContext;
    bool m_visible;
    UInt32 m_builds;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUI_CIMGUIFRAMEBUILDER_H ? */
