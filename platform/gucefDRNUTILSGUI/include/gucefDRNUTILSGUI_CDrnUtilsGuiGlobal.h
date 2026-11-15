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

#ifndef GUCEF_DRNUTILSGUI_CDRNUTILSGUIGLOBAL_H
#define GUCEF_DRNUTILSGUI_CDRNUTILSGUIGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

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
 *  The global systems of gucefDRNUTILSGUI: the modules it builds on (gucefCORE, gucefGUI, gucefDRNUTILS) are set up
 *  first, then the events of the forms are registered. All module wide state of gucefDRNUTILSGUI is a member of this
 *  class; nothing relies on CModule::Load() running.
 */
class GUCEF_DRNUTILSGUI_PUBLIC_CPP CDrnUtilsGuiGlobal
{
    public:

    static CDrnUtilsGuiGlobal* Instance( void );

    private:

    friend class CModule;

    static void Deinstance( void );

    void Initialize( void );

    CDrnUtilsGuiGlobal( void );

    ~CDrnUtilsGuiGlobal();

    CDrnUtilsGuiGlobal( const CDrnUtilsGuiGlobal& src );                /**< not implemented */
    CDrnUtilsGuiGlobal& operator=( const CDrnUtilsGuiGlobal& src );     /**< not implemented */

    private:

    static MT::CMutex g_dataLock;
    static CDrnUtilsGuiGlobal* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILSGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILSGUI_CDRNUTILSGUIGLOBAL_H ? */
