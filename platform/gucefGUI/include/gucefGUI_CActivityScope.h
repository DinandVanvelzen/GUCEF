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

#ifndef GUCEF_GUI_CACTIVITYSCOPE_H
#define GUCEF_GUI_CACTIVITYSCOPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  An operation of the CActivityService for the length of a scope: Begin() when it is made, End() when it goes out of scope
 *  (or Finish()). Without a service (NULL) it does nothing, so code can use it whether or not an application has a service.
 *  Safe on any thread, as the service is.
 */
class GUCEF_GUI_PUBLIC_CPP CActivityScope
{
    public:

    CActivityScope( CActivityService* service                  ,
                    const CORE::CString& label                 ,
                    UInt32 expectedDurationInMs = 0            ,
                    bool determinate = false                   ,
                    bool cancellable = false                   );

    ~CActivityScope();

    UInt32 GetId( void ) const;

    void SetProgress( Float32 fraction );

    void SetLabel( const CORE::CString& label );

    bool IsCancelRequested( void ) const;

    /** Ends the operation now instead of at the end of the scope */
    void Finish( void );

    private:

    CActivityScope( const CActivityScope& src );
    CActivityScope& operator=( const CActivityScope& src );

    private:

    CActivityService* m_service;
    UInt32 m_id;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CACTIVITYSCOPE_H ? */
