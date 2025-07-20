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

#ifndef GUCEF_CORE_CSINGLETASKDELEGATOR_H
#define GUCEF_CORE_CSINGLETASKDELEGATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKDELEGATOR_H
#include "gucefCORE_CTaskDelegator.h"
#define GUCEF_CORE_CTASKDELEGATOR_H
#endif /* GUCEF_CORE_CTASKDELEGATOR_H ? */

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

class CThreadPool;
class CTaskConsumer;

/*-------------------------------------------------------------------------*/

/**
 *  This is an internally used class for the CTaskManager class.
 *  It is not meant to be used beyond that use-case.
 */
class GUCEF_CORE_PRIVATE_CPP CSingleTaskDelegator : public CTaskDelegator
{
    private:
    friend class CThreadPool;

    CSingleTaskDelegator( const ThreadPoolPtr& threadPool ,
                          const CTaskPtr& task            );

    virtual ~CSingleTaskDelegator();

    bool ExecuteTaskFromCallingThread( void );

    protected:

    /**
     *  Overloads the cycle of the regular task delegator to only
     *  execute the already known task within a thread and then exit.
     */
    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CSingleTaskDelegator( void );                                       /**< not implemented, makes no sense */
    CSingleTaskDelegator( const CSingleTaskDelegator& src );            /**< not implemented, makes no sense */
    CSingleTaskDelegator& operator=( const CSingleTaskDelegator& src ); /**< not implemented, makes no sense */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSINGLETASKDELEGATOR_H ? */
