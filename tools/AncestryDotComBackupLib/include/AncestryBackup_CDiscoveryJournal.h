/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef ANCESTRYBACKUP_CDISCOVERYJOURNAL_H
#define ANCESTRYBACKUP_CDISCOVERYJOURNAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef ANCESTRYBACKUP_CENDPOINTCATALOG_H
#include "AncestryBackup_CEndpointCatalog.h"
#define ANCESTRYBACKUP_CENDPOINTCATALOG_H
#endif /* ANCESTRYBACKUP_CENDPOINTCATALOG_H ? */

#ifndef ANCESTRYBACKUP_CDATALAYOUT_H
#include "AncestryBackup_CDataLayout.h"
#define ANCESTRYBACKUP_CDATALAYOUT_H
#endif /* ANCESTRYBACKUP_CDATALAYOUT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The journal of discovery: an append-only file with one JSON document per observed request.
 *  The catalog, and with it the endpoint report, can be rebuilt from it at any time without a browser.
 */
class CDiscoveryJournal
{
    public:

    CDiscoveryJournal( void );

    ~CDiscoveryJournal();

    /** Opens the journal of the layout for appending, creating it if needed */
    bool Open( const CDataLayout& layout );

    void Close( void );

    bool IsOpen( void ) const;

    bool Append( const SObservedRequest& observation );

    /** The sequence number to give the next observation: one past the highest in the journal */
    UInt32 GetNextSequence( void ) const;

    /**
     *  Rebuilds a catalog from the journal. Response bodies that were saved next to the journal are analyzed
     *  for their structure, which needs the json codec plugin. Without it the catalog is built without shapes.
     *  @return the number of observations loaded
     */
    static UInt32 Load( const CDataLayout& layout, CEndpointCatalog& catalog, UInt32* nextSequence = GUCEF_NULL );

    /** The sequence number after the highest in the journal file, from its last part only. 1 if there is no journal. */
    static UInt32 GetNextSequenceOf( const CORE::CString& path );

    private:

    CDiscoveryJournal( const CDiscoveryJournal& src );
    CDiscoveryJournal& operator=( const CDiscoveryJournal& src );

    private:

    CORE::CFileAccess m_file;
    UInt32 m_nextSequence;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CDISCOVERYJOURNAL_H ? */
