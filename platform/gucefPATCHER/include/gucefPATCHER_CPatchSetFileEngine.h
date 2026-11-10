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

#ifndef GUCEF_PATCHER_CPATCHSETFILEENGINE_H
#define GUCEF_PATCHER_CPATCHSETFILEENGINE_H
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#include "CObservingNotifier.h"
#include "gucefCORE_CAsyncUriResourceRetriever.h"
#include "CFileAccess.h"

#include "gucefPATCHER_CPatchSetFileEngineEvents.h"
#include "gucefPATCHER_CPatchSetParser.h"
#include "gucefPATCHER_macros.h"

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

/**
 *  Engine for patching a list of files given.
 *  The files are compared with the ones in the local root that is given.
 */
class GUCEFPATCHER_EXPORT_CPP CPatchSetFileEngine : public CORE::CObservingNotifier  ,
                                                    public CPatchSetFileEngineEvents
{  
    public:
    
    CPatchSetFileEngine( void );
    
    CPatchSetFileEngine( const CORE::PulseGeneratorPtr& pulseGenerator );
    
    virtual ~CPatchSetFileEngine();
    
    public:
    
    bool Start( const TFileList& fileList            ,
                const CORE::CString& localRoot       ,
                const CORE::CString& tempStorageRoot ,
                const bool stopOnReplacementFailure  );
    
    void Stop( void );
    
    bool IsActive( void ) const;
    
    virtual const CString& GetClassTypeName( void ) const;
    
    protected:
        
    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = NULL );

    private:
    
    bool ProcessCurrentFile( void );
    bool ProceedToNextFile( void );
    bool TryNextFileLocation( void );
    TPatchSetFileEngineEventData* CreateEventStatusObj( void ) const;
    
    private:
    
    CPatchSetFileEngine( const CPatchSetFileEngine& src );            /**< not implemented */
    CPatchSetFileEngine& operator=( const CPatchSetFileEngine& src ); /**< not implemented */
    
    private:
    
    UInt32 m_curFileIndex;
    UInt32 m_curFileLocIndex;
    TFileList m_fileList;
    
    CORE::CFileAccess* m_fileAccess;                    /**< destination of the file retrieval, must outlive the retriever */
    CORE::CAsyncUriResourceRetriever m_retriever;
    
    bool m_isActive;
    bool m_stopSignalGiven;
    CORE::CString m_localRoot;
    CORE::CString m_tempStorageRoot;
    UInt64 m_totalBytesProcessed;
    UInt64 m_currentFileReceivedBytes;
    bool m_stopOnReplacementFailure;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PATCHER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PATCHER_CPATCHSETFILEENGINE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 27-12-2006 :
        - Dinand: Initial version

-----------------------------------------------------------------------------*/