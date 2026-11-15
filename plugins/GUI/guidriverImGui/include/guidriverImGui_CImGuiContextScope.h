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

#ifndef GUIDRIVERIMGUI_CIMGUICONTEXTSCOPE_H
#define GUIDRIVERIMGUI_CIMGUICONTEXTSCOPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUIDRIVERIMGUI_MACROS_H
#include "guidriverImGui_macros.h"
#define GUIDRIVERIMGUI_MACROS_H
#endif /* GUIDRIVERIMGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

struct ImGuiContext;

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/**
 *  Makes a Dear ImGui context the current one for as long as the scope exists and makes the context that was current
 *  before current again when it ends. Dear ImGui has one current context per process (GImGui): every ImGui call of
 *  this library is made within such a scope.
 */
class GUIDRIVERIMGUI_PUBLIC_CPP CImGuiContextScope
{
    public:

    explicit CImGuiContextScope( ::ImGuiContext* context );

    ~CImGuiContextScope();

    /** The context that was current when the scope began */
    ::ImGuiContext* GetPreviousContext( void ) const;

    private:

    CImGuiContextScope( const CImGuiContextScope& src );
    CImGuiContextScope& operator=( const CImGuiContextScope& src );

    private:

    ::ImGuiContext* m_previous;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUI_CIMGUICONTEXTSCOPE_H ? */
