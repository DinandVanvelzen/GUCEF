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

#ifndef GUCEF_PATCHER_CPATCHLISTENGINE_H
#define GUCEF_PATCHER_CPATCHLISTENGINE_H
 
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

#ifndef GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H
#include "gucefCORE_CAsyncUriResourceRetriever.h"
#define GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H
#endif /* GUCEF_CORE_CASYNCURIRESOURCERETRIEVER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_PATCHER_CPATCHLISTPARSER_H
#include "gucefPATCHER_CPatchListParser.h"
#define GUCEF_PATCHER_CPATCHLISTPARSER_H
#endif /* GUCEF_PATCHER_CPATCHLISTPARSER_H ? */

#ifndef GUCEF_PATCHER_CPATCHSETPARSER_H
#include "gucefPATCHER_CPatchSetParser.h"
#define GUCEF_PATCHER_CPATCHSETPARSER_H
#endif /* GUCEF_PATCHER_CPATCHSETPARSER_H ? */

#ifndef GUCEF_PATCHER_CPATCHSETFILEENGINEEVENTS_H
#include "gucefPATCHER_CPatchSetFileEngineEvents.h"
#define GUCEF_PATCHER_CPATCHSETFILEENGINEEVENTS_H
#endif /* GUCEF_PATCHER_CPATCHSETFILEENGINEEVENTS_H ? */

#ifndef GUCEF_PATCHER_CPATCHSETDIRENGINEEVENTS_H
#include "gucefPATCHER_CPatchSetDirEngineEvents.h"
#define GUCEF_PATCHER_CPATCHSETDIRENGINEEVENTS_H
#endif /* GUCEF_PATCHER_CPATCHSETDIRENGINEEVENTS_H ? */

#ifndef GUCEF_PATCHER_CPATCHSETENGINEEVENTS_H
#include "gucefPATCHER_CPatchSetEngineEvents.h"
#define GUCEF_PATCHER_CPATCHSETENGINEEVENTS_H
#endif /* GUCEF_PATCHER_CPATCHSETENGINEEVENTS_H ? */

#ifndef GUCEF_PATCHER_CPATCHLISTENGINEEVENTS_H
#include "gucefPATCHER_CPatchListEngineEvents.h"
#define GUCEF_PATCHER_CPATCHLISTENGINEEVENTS_H
#endif /* GUCEF_PATCHER_CPATCHLISTENGINEEVENTS_H ? */

#ifndef GUCEF_PATCHER_MACROS_H
#include "gucefPATCHER_macros.h"
#define GUCEF_PATCHER_MACROS_H
#endif /* GUCEF_PATCHER_MACROS_H ? */

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

class CPatchSetEngine;

/*-------------------------------------------------------------------------*/

/**
 *  Engine for processing a patch list.
 */
class GUCEF_PATCHER_EXPORT_CPP CPatchListEngine : public CORE::CForwardingNotifier ,
                                                  public CPatchSetDirEngineEvents  , /* event interface */
                                                  public CPatchSetFileEngineEvents , /* event interface */
                                                  public CPatchSetEngineEvents     , /* event interface */
                                                  public CPatchListEngineEvents      /* event interface */
{
    public:

    typedef CPatchListParser::TPatchSetLocation TPatchSetLocation;
    typedef CPatchListParser::TPatchSetLocations TPatchSetLocations;
    typedef CPatchListParser::TPatchList TPatchList;

    typedef CPatchSetParser::TFileLocation TFileLocation;
    typedef CPatchSetParser::TFileEntry TFileEntry;
    typedef CPatchSetParser::TDirEntry TDirEntry;
    typedef CPatchSetParser::TPatchSet TPatchSet;
    
    public:
    
    CPatchListEngine( void );
    
    CPatchListEngine( const CORE::PulseGeneratorPtr& pulseGenerator );
    
    virtual ~CPatchListEngine();
    
    bool Start( const TPatchList& patchList             ,
                const CORE::CString& localRoot          ,
                const CORE::CString& tempStorageRoot    ,
                const bool stopOnFileReplacementFailure );
    
    void Stop( void );
    
    bool IsActive( void ) const;
    
    virtual const CString& GetClassTypeName( void ) const;
    
    protected:
        
    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = NULL );

    private:
    
    CPatchListEngine( const CPatchListEngine& src ); /**< not implemented */
    CPatchListEngine& operator=( const CPatchListEngine& src ); /**< not implemented */
    bool ObtainCurrentPatchSet( void );
    bool ProcessRecievedPatchSet( void );
    bool GetCurrentPatchSetLocation( TPatchSetLocation** location );
    void Initialize( void );
    TPatchListEngineEventData* CreateEventStatusObj( void ) const;
    
    private:
    
    typedef std::vector< TPatchSet >    TPatchSetVector;
    
    TPatchList m_patchList;
    TPatchSetVector m_patchSets;
    UInt32 m_setIndex;
    UInt32 m_setLocIndex;
    CPatchSetEngine* m_patchSetEngine;
    CORE::CDynamicBuffer m_setDataBuffer;           /**< destination of the patch set retrieval, must outlive the retriever */
    CORE::CAsyncUriResourceRetriever m_retriever;
    bool m_isActive;
    bool m_stopSignalGiven;
    CString m_localRoot;
    CString m_tempStorageRoot;
    UInt64 m_totalDataSizeInBytes;
    UInt64 m_processedDataSizeInBytes;
    UInt64 m_processedCurrentSetDataSizeInBytes;
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

#endif /* GUCEF_PATCHER_CPATCHLISTENGINE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 29-12-2006 :
        - Dinand: Initial version

-----------------------------------------------------------------------------*/