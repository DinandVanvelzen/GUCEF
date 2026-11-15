/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CDRNUTILSGLOBAL_H
#define GUCEF_DRNUTILS_CDRNUTILSGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The global systems of gucefDRNUTILS: the modules it builds on (gucefCORE, gucefCOMCORE, gucefDRN) are set up first,
 *  then the events of the hosts and clients are registered and the contracts gucef.drnutils.scheduler 1.0 and
 *  gucef.drnutils.tasks 1.0 are registered in the contract registry of gucefCOMCORE.
 *  All module wide state of gucefDRNUTILS is a member of this class; nothing relies on CModule::Load() running.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CDrnUtilsGlobal
{
    public:

    static CDrnUtilsGlobal* Instance( void );

    private:

    friend class CModule;

    static void Deinstance( void );

    void Initialize( void );

    CDrnUtilsGlobal( void );

    ~CDrnUtilsGlobal();

    CDrnUtilsGlobal( const CDrnUtilsGlobal& src );              /**< not implemented */
    CDrnUtilsGlobal& operator=( const CDrnUtilsGlobal& src );   /**< not implemented */

    private:

    static MT::CMutex g_dataLock;
    static CDrnUtilsGlobal* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDRNUTILSGLOBAL_H ? */
