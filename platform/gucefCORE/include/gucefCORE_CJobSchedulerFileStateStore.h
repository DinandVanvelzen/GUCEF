/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *
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

#ifndef GUCEF_CORE_CJOBSCHEDULERFILESTATESTORE_H
#define GUCEF_CORE_CJOBSCHEDULERFILESTATESTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CIJOBSCHEDULERSTATESTORE_H
#include "gucefCORE_CIJobSchedulerStateStore.h"
#define GUCEF_CORE_CIJOBSCHEDULERSTATESTORE_H
#endif /* GUCEF_CORE_CIJOBSCHEDULERSTATESTORE_H ? */

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
 *  The state of a job scheduler in one file, through a data tree codec of the codec registry (default "json", which
 *  needs the JSON dstore codec plugin loaded). Written atomically (WriteFileAtomically(): a temporary file and a rename),
 *  so a crash leaves the old state or the new one. A file that is missing gives an empty state (a first start). A file
 *  that cannot be read is kept as <file>.bad and LoadState() fails with the reason: the scheduler then starts from its
 *  defaults and the next save writes a good file.
 */
class GUCEF_CORE_PUBLIC_CPP CJobSchedulerFileStateStore : public CIJobSchedulerStateStore
{
    public:

    static const CString DefaultCodecName;

    explicit CJobSchedulerFileStateStore( const CString& filePath = CString::Empty, const CString& codecName = DefaultCodecName );

    virtual ~CJobSchedulerFileStateStore() GUCEF_VIRTUAL_OVERRIDE;

    void SetFilePath( const CString& filePath );
    CString GetFilePath( void ) const;

    void SetCodecName( const CString& codecName );
    CString GetCodecName( void ) const;

    virtual bool LoadState( CDataNode& state, CString& error ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveState( const CDataNode& state, CString& error ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CJobSchedulerFileStateStore( const CJobSchedulerFileStateStore& src );              /**< not implemented */
    CJobSchedulerFileStateStore& operator=( const CJobSchedulerFileStateStore& src );   /**< not implemented */

    private:

    mutable MT::CMutex m_lock;
    CString m_filePath;
    CString m_codecName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBSCHEDULERFILESTATESTORE_H ? */
