/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#define GUCEF_CORE_CGUCEFAPPLICATION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>

#ifndef GUCEF_MT_CILOCKABLE_H
#include "gucefMT_CILockable.h"
#define GUCEF_MT_CILOCKABLE_H
#endif /* GUCEF_MT_CILOCKABLE_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CISYSCONSOLECMDHANDLER_H
#include "CISysConsoleCmdHandler.h"
#define GUCEF_CORE_CISYSCONSOLECMDHANDLER_H
#endif /* GUCEF_CORE_CISYSCONSOLECMDHANDLER_H ? */

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H
#include "gucefCORE_CBusyWaitPulseGeneratorDriver.h"
#define GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H
#endif /* GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H ? */

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
 *
 *      Dispatches the following events:
 *       - AppInitEvent:
 *              Indicates that the application is now initializing
 *       - AppShutdownEvent
 *              Signals that the application is shutting down at the
 *              GUCEF level. All higher level code should shutdown as well
 */
class GUCEF_CORE_PUBLIC_CPP CGUCEFApplication : public CTSGNotifier                   ,
                                                public CGloballyConfigurable          ,
                                                public virtual CISysConsoleCmdHandler
{
    public:

    static const CEvent AppInitEvent;
    static const CEvent AppShutdownEvent;
    static const CEvent AppShutdownAbortedEvent;
    static const CEvent AppShutdownCompleteEvent;
    static const CEvent FirstCycleEvent;

    static void RegisterEvents( void );

    struct SAppInitEventData
    {
        #ifdef GUCEF_MSWIN_BUILD
        HINSTANCE hinstance;
        LPSTR lpcmdline;
        int ncmdshow;
        #endif

        int argc;
        char** argv;
    };
    typedef CTCloneableObj< struct SAppInitEventData > TAppInitEventData;

    public:

    #ifdef GUCEF_MSWIN_BUILD
    int Main( HINSTANCE hinstance     ,
              LPSTR lpcmdline         ,
              int ncmdshow            ,
              bool run                );
    #endif

    int main( int argc    ,
              char** argv ,
              bool run    );


    void Update( void );

    bool Stop( bool wait = false );

    CString GetApplicationDir( void ) const;

    /**
     *      Attempts to store the given tree in the file
     *      given according to the method of the codec meta data
     *
     *      @param tree the data tree you wish to store
     *      @return whether storing the tree was successful
     */
    virtual bool SaveConfig( CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Attempts to load data from the given file to the
     *      root node given. The root data will be replaced
     *      and any children the node may already have will be deleted.
     *
     *      @param treeroot pointer to the node that is to act as root of the data tree
     *      @return whether building the tree from the given file was successful
     */
    virtual bool LoadConfig( const CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetForcedMinimalCycleDeltaInMilliSecs( UInt32 cycleDelta );

    UInt32 GetForcedMinimalCycleDeltaInMilliSecs( void ) const;

    void SetDesiredMaximumCycleDeltaInMilliSecs( UInt32 cycleDelta );

    UInt32 GetDesiredMaximumCycleDeltaInMilliSecs( void ) const;

    virtual void SetPulseGenerator( PulseGeneratorPtr newPulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnSysConsoleCommand( const CString& path               ,
                                      const CString& command            ,
                                      const CString::StringVector& args ,
                                      CString::StringVector& resultdata ) GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CCoreGlobal;

    CGUCEFApplication( void );

    virtual ~CGUCEFApplication();

    bool LinkPulseGeneratorDriver( void );
    
    private:

    CGUCEFApplication( const CGUCEFApplication& src );            /**< not implemented */
    
    CGUCEFApplication& operator=( const CGUCEFApplication& src ); /**< not implemented */

    int MainLoop( void );

    private:

    bool _initialized;
    bool m_shutdownRequested;
    CString _appdir;
    CBusyWaitPulseGeneratorDriver m_busyWaitPulseDriver;
    UInt32 m_forcedMinimalCycleDeltaInMilliSecs; 
    UInt32 m_desiredMaximumCycleDeltaInMilliSecs;
};


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* CGUCEFAPPLICATION_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 27-08-2005 :
        - Added GetCycleDelay()
- 12-11-2004 :
        - Designed and implemented this class.

-----------------------------------------------------------------------------*/
