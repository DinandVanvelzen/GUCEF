/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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


#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXREPORT_H
#define GUCEF_ASSETSTORE_CASSETPACKINDEXREPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */
#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a run of the pack index over many packs did (CPrimaryAssetStore::IndexPacks() and ReindexFromStorage()): per pack
 *  the number of packs that were loaded from a document, found unchanged (the catalogue held the document already), built by
 *  the converter hook, derived from the catalogue, skipped (a location that is not reachable, a pack that cannot be indexed
 *  here) or failed. 'documentsWithoutPack' counts the documents found in the storage locations that no pack of the
 *  catalogue owns: they are reported and not loaded, as the design says (a rebuild never invents a pack).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetPackIndexReport : public CORE::CICloneable
{
    public:

    TAssetStoreStatus status;
    CORE::CString message;
    UInt32 packsSeen;
    UInt32 packsLoaded;
    UInt32 packsUnchanged;
    UInt32 packsBuilt;
    UInt32 packsDerived;
    UInt32 packsSkipped;
    UInt32 packsFailed;
    UInt32 documentsWithoutPack;
    CORE::CString::StringVector problems;

    CAssetPackIndexReport( void );
    virtual ~CAssetPackIndexReport() GUCEF_VIRTUAL_OVERRIDE;

    /** The counts in one line for people */
    CORE::CString ToSummaryText( void ) const;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXREPORT_H ? */
