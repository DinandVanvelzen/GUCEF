/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CIDRNNAMESPACEENUMERATOR_H
#define GUCEF_DRN_CIDRNNAMESPACEENUMERATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACEENTRY_H
#include "gucefDRN_CDRNNamespaceEntry.h"
#define GUCEF_DRN_CDRNNAMESPACEENTRY_H
#endif /* GUCEF_DRN_CDRNNAMESPACEENTRY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The streaming listing of one directory of a provider (CIDRNResourceProvider::OpenEnumerator()): the serving side keeps
 *  it between the pages of one listing, so a page costs what it holds and not the directory again, and memory per
 *  listing stays what the enumerator itself needs.
 *
 *  Called from one pool thread at a time, never concurrently. It describes what is there: the serving side leaves out
 *  and counts the names it will not send, applies the shares and the access policy.
 */
class GUCEF_DRN_EXPORT_CPP CIDRNNamespaceEnumerator
{
    public:

    CIDRNNamespaceEnumerator( void );

    virtual ~CIDRNNamespaceEnumerator();

    /**
     *  Appends at most maxEntries entries to 'entries' (entry names are single names of the directory) and sets atEnd
     *  when there are no more. A directory that can no longer be read answers IO_ERROR or NOT_FOUND.
     */
    virtual TDRNResourceStatus ReadEntries( UInt32 maxEntries, GUCEF::vector< CDRNNamespaceEntry >& entries, bool& atEnd ) = 0;

    private:

    CIDRNNamespaceEnumerator( const CIDRNNamespaceEnumerator& src );                /**< not implemented */
    CIDRNNamespaceEnumerator& operator=( const CIDRNNamespaceEnumerator& src );     /**< not implemented */
};

typedef CORE::CTSharedPtr< CIDRNNamespaceEnumerator, MT::CMutex > CIDRNNamespaceEnumeratorPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CIDRNNAMESPACEENUMERATOR_H ? */
