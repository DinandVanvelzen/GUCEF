/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGELASTRESULTS_H
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGELASTRESULTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGERESULT_H
#include "vfspluginDRN_CDrnBridgeResult.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGERESULT_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGERESULT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The result of the last call per thread: the bool, size and empty string answers of the archive interface say
 *  nothing about why, so the archive records the outcome of each call here, for the thread that made it (by
 *  MT::GetCurrentThreadID(), as the data tables do). A success clears the entry of the thread, so what Get()
 *  gives is the result of the latest call of the calling thread, a success when that call went well or none was made.
 *  The number of threads that have an entry is bounded: past the bound the table starts over.
 */
class GUCEF_VFSPLUGIN_DRN_EXPORT_CPP CDrnBridgeLastResults
{
    public:

    static const UInt32 MaxThreads = 256;

    CDrnBridgeLastResults( void );

    ~CDrnBridgeLastResults();

    /** Records the result of the call the calling thread just made; returns the same result for chaining */
    const CDrnBridgeResult& Set( const CDrnBridgeResult& result ) const;

    /** The result of the latest call of the calling thread */
    CDrnBridgeResult Get( void ) const;

    private:

    CDrnBridgeLastResults( const CDrnBridgeLastResults& src );                  /**< not implemented */
    CDrnBridgeLastResults& operator=( const CDrnBridgeLastResults& src );       /**< not implemented */

    typedef GUCEF::map< TThreadId, CDrnBridgeResult > TResultMap;

    mutable MT::CMutex m_lock;
    mutable TResultMap m_results;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGELASTRESULTS_H ? */
