/*
 *  gucefPATCHER: GUCEF RAD module providing a patch delivery system
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#ifndef GUCEF_PATCHER_CPATCHMANAGER_H
#define GUCEF_PATCHER_CPATCHMANAGER_H
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_PATCHER_CPATCHENGINE_H
#include "gucefPATCHER_CPatchEngine.h"
#define GUCEF_PATCHER_CPATCHENGINE_H
#endif /* GUCEF_PATCHER_CPATCHENGINE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PATCHER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CPatchTaskConsumer;

/*-------------------------------------------------------------------------*/

class GUCEF_PATCHER_EXPORT_CPP CPatchManager : public CORE::CTSGNotifier
{
    public:
    
    static const CORE::CEvent PatchTaskStartedEvent;
    static const CORE::CEvent PatchTaskStoppedEvent;
    static const CORE::CEvent PatchTaskStopRequestedEvent;
    static const CORE::CEvent PatchTaskFinishedEvent;
    static const CORE::CEvent PatchTaskPausedEvent;
    static const CORE::CEvent PatchTaskResumedEvent;    
    static const CORE::CEvent PatchTaskEventReceivedEvent;

    typedef CORE::TCloneableString  TPatchTaskStartedEventData;
    typedef CORE::TCloneableString  TPatchTaskStoppedEventData;
    typedef CORE::TCloneableString  TPatchTaskStopRequestedEventData;
    typedef CORE::TCloneableString  TPatchTaskFinishedEventData;
    typedef CORE::TCloneableString  TPatchTaskPausedEventData;
    typedef CORE::TCloneableString  TPatchTaskResumedEventData;

    static void RegisterEvents( void );
    
    public:
    
    typedef std::vector< CString > TStringVector;
    
    CPatchManager( void );
    
    CPatchManager( const CORE::PulseGeneratorPtr& pulseGenerator );
    
    virtual ~CPatchManager();
    
    bool StartTask( const CString& taskName                  ,
                    const CORE::CDataNode& patchEngineConfig );

    bool PauseTask( const CString& taskName );
    
    bool ResumeTask( const CString& taskName );
    
    bool RequestTaskToStop( const CString& taskName );
        
    void GetTaskList( TStringVector& list ) const;
    
    virtual const CString& GetClassTypeName( void ) const;
    
    protected:
    
    virtual void OnPumpedNotify( CORE::CNotifier* notifier           ,
                                 const CORE::CEvent& eventid         ,
                                 CORE::CICloneable* eventdata = GUCEF_NULL );
    
    private:
    friend class CPatchTaskConsumer;
    
    void RegisterTask( CPatchTaskConsumer* task );
    
    void UnregisterTask( CPatchTaskConsumer* task );
        
    private:
    
    CPatchManager( const CPatchManager& src );
    CPatchManager& operator=( const CPatchManager& src );
    
    private:
    
    typedef std::map< CString, CPatchTaskConsumer* > TTaskMap;
    
    TTaskMap m_taskMap;
    CORE::CTaskManager* m_taskManager;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PATCHER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PATCHER_CPATCHMANAGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 29-12-2006 :
        - Dinand: Initial version

----------------------------------------------------------------------------*/
