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

#ifndef GUCEF_CORE_CGENERICCALLBACKTASKCONSUMER_H
#define GUCEF_CORE_CGENERICCALLBACKTASKCONSUMER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CIASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

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
 *  TaskConsumer specialization for delegating the task logic to a callback function.
 *  Intended to be used for tasks that are not too complex and can be handled by a single callback function
 *  In such a case a lower development effort is desired to hook things up and get the task running.
 */
class GUCEF_CORE_PUBLIC_CPP CGenericCallbackTaskConsumer : public CTaskConsumer
{
    public:

    static const CString TaskType;

    CGenericCallbackTaskConsumer( void );

    virtual ~CGenericCallbackTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Startup routine for the task. You should return true if startup succeeded and the task can commence
     *  cycling.
     */
    virtual bool OnTaskStart( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Perform all your main task work in this function.
     *  It will be called repeatedly until true is returned indicating that the task has been completed.
     *  Thus for ongoing tasks you can write this function to take care of a single interation of the task.
     */
    virtual bool OnTaskCycle( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  This is where all the cleanup should be done for task data
     *  Note that this member function will be called from within the spawned thread when ending gracefully
     *  but in the case of a forceful termination of the spawned thread this member function will be called
     *  from the thread that triggered the forceful termination.
     */
    virtual void OnTaskEnded( CTaskPtr task  ,
                              bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;
        
    private:

    CGenericCallbackTaskConsumer( const CGenericCallbackTaskConsumer& src  );
    CGenericCallbackTaskConsumer& operator=( const CGenericCallbackTaskConsumer& src );

    private:

};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CGenericCallbackTaskConsumer, MT::CMutex >         CGenericCallbackTaskConsumerPtr;
typedef CTBasicSharedPtr< CGenericCallbackTaskConsumer, MT::CMutex >    CCGenericCallbackTaskConsumerBasicPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CGENERICCALLBACKTASKCONSUMER_H ? */

