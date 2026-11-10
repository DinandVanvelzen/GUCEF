/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CEXTERNALTHREADSCOPE_H
#define GUCEF_CORE_CEXTERNALTHREADSCOPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Scope helper for a thread that was not created by GUCEF: while the object exists the calling thread is known to
 *  the task manager, in its external thread pool (CTaskManager::ExternalThreadPoolName).
 *
 *  The task manager is the single point of knowledge about all threads of the process. Threads made by the operating
 *  system or a driver, for example the audio callback thread of a sound device, announce themselves with this class,
 *  typically with a scope object that lives as long as the callback registration does, or a function static one in the callback.
 *
 *  The object must be created and destroyed on the thread that it registers.
 */
class GUCEF_CORE_PUBLIC_CPP CExternalThreadScope
{
    public:

    explicit CExternalThreadScope( const CString& threadName );

    ~CExternalThreadScope();

    /** True if the task manager accepted the registration */
    bool IsRegistered( void ) const;

    private:

    CExternalThreadScope( const CExternalThreadScope& src );            /**< not implemented */
    CExternalThreadScope& operator=( const CExternalThreadScope& src ); /**< not implemented */

    private:

    bool m_registered;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CEXTERNALTHREADSCOPE_H ? */
