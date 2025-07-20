/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CFUTURERESULT_H
#define GUCEF_CORE_CFUTURERESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

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
 *  Class which provides syntactically convenient access to the outcome of async task processing.
 *  This class implements a flavor of the 'futures' concept.
 */
class GUCEF_CORE_PUBLIC_CPP CFutureResult
{
    public:

    CFutureResult( CTaskPtr task );

    CFutureResult( const CFutureResult& src );

    /**
     *  You can use Await to block the calling thread until the task completes
     *  If a timeout occurs before the task completes a timeout_exception will be thrown
     */
    const CFutureResult& Await( Int32 timeoutInMs = GUCEF_MT_ULTRA_LONG_LOCK_TIMEOUT ) const;

    CTaskPtr GetResult( Int32 timeoutInMs = GUCEF_MT_ULTRA_LONG_LOCK_TIMEOUT ) const;

    bool IsReady( void ) const;

    /**
     *  If there won't be any further future results on this 'future' its considered as having no future
     *  This can happen due to an fatal error along the init and/or initialization of the task or
     *  perhaps the task has already concluded with a failed state
     */
    bool HasNoFuture( void ) const;

    /**
     *  Simply the opposite of HasNoFuture()
     *  Provided as a distinct function for code clarity
     */
    bool HasAFuture( void ) const;

    private:

    CFutureResult( void ); /**< not implemented, you need a task reference */

    private:

    CTaskPtr m_task;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CFUTURERESULT_H ? */
