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

#ifndef GUCEF_PATCHER_CPATCHSETDIRENGINE_H
#define GUCEF_PATCHER_CPATCHSETDIRENGINE_H
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CFORWARDINGNOTIFIER_H
#include "gucefCORE_CForwardingNotifier.h"
#define GUCEF_CORE_CFORWARDINGNOTIFIER_H
#endif /* GUCEF_CORE_CFORWARDINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_PATCHER_CPATCHSETFILEENGINEEVENTS_H
#include "gucefPATCHER_CPatchSetFileEngineEvents.h"
#define GUCEF_PATCHER_CPATCHSETFILEENGINEEVENTS_H
#endif /* GUCEF_PATCHER_CPATCHSETFILEENGINEEVENTS_H ? */

#ifndef GUCEF_PATCHER_CPATCHSETDIRENGINEEVENTS_H
#include "gucefPATCHER_CPatchSetDirEngineEvents.h"
#define GUCEF_PATCHER_CPATCHSETDIRENGINEEVENTS_H
#endif /* GUCEF_PATCHER_CPATCHSETDIRENGINEEVENTS_H ? */

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

class CPatchSetFileEngine;

/*-------------------------------------------------------------------------*/

/**
 *  Engine for patching a directory tree and all the files it contains.
 *  The tree is mirrored to the one starting at the given local root.
 */
class GUCEFPATCHER_EXPORT_CPP CPatchSetDirEngine : public CORE::CForwardingNotifier ,
                                                   public CPatchSetFileEngineEvents ,
                                                   public CPatchSetDirEngineEvents
{    
    public:
    
    CPatchSetDirEngine( void );
    
    CPatchSetDirEngine( const CORE::PulseGeneratorPtr& pulseGenerator );
    
    virtual ~CPatchSetDirEngine();
    
    public:
    
    bool Start( const TDirEntry& startingDir            ,
                const CORE::CString& localRoot          ,
                const CORE::CString& tempStorageRoot    ,
                const bool stopOnFileReplacementFailure );
    
    void Stop( void );
    
    bool IsActive( void ) const;
    
    virtual const CString& GetClassTypeName( void ) const;

    protected:
        
    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = GUCEF_NULL );    
    
    private:
    
    bool ProcessCurSubDir( void );
    bool ProcessNextSubDir( void );
    bool ProcessFilesInDir( void );
    void Initialize( void );
    
    TPatchSetDirEngineEventData* CreateEventStatusObj( void ) const;
    
    private:
        
    UInt32 m_curSubDirIndex;
    CPatchSetDirEngine* m_subDirPatchEngine;
    CPatchSetFileEngine* m_filePatchEngine;
    
    TDirEntry m_dir;
    bool m_isActive;
    bool m_stopSignalGiven;
    CORE::CString m_localRoot;
    CORE::CString m_tempStorageRoot;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    UInt64 m_processedDataSizeInBytes;
    UInt64 m_processedSubDirDataSizeInBytes;
    bool m_stopOnFileReplacementFailure;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PATCHER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PATCHER_CPATCHSETDIRENGINE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 27-12-2006 :
        - Dinand: Initial version

-----------------------------------------------------------------------------*/