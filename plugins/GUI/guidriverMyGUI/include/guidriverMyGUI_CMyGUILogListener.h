/*
 *  guidriverMyGUI: GUI backend using MyGUI
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

#ifndef GUCEF_guidriverMyGUI_CMYGUILOGLISTENER_H
#define GUCEF_guidriverMyGUI_CMYGUILOGLISTENER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "MyGUI_ILogListener.h"

#ifndef GUCEF_guidriverMyGUI_MACROS_H
#include "guidriverMyGUI_macros.h"
#define GUCEF_guidriverMyGUI_MACROS_H
#endif /* GUCEF_guidriverMyGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Hands the messages of MyGUI to the GUCEF logging, prefixed with "MyGUI: " */
class GUCEF_guidriverMyGUI_PUBLIC_CPP CMyGUILogListener : public MyGUI::ILogListener
{
    public:

    CMyGUILogListener( void );

    virtual ~CMyGUILogListener();

    virtual void log( std::string_view section        ,
                      MyGUI::LogLevel level           ,
                      const struct tm* time           ,
                      std::string_view message        ,
                      std::string_view file           ,
                      int line                        ) override;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverMyGUI_CMYGUILOGLISTENER_H ? */
