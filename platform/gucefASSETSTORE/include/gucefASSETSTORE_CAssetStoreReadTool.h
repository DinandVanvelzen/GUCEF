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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREADTOOL_H
#define GUCEF_ASSETSTORE_CASSETSTOREREADTOOL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H
#include "gucefASSETSTORE_CAssetSearchQuery.h"
#define GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H
#endif /* GUCEF_ASSETSTORE_CASSETSEARCHQUERY_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLBASE_H
#include "gucefASSETSTORE_CAssetToolBase.h"
#define GUCEF_ASSETSTORE_CASSETTOOLBASE_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLBASE_H ? */

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
 *  The read tools of the primary store (section 7.5): store_search (Search(), RunSavedSearch()), store_asset
 *  (GetAsset(), GetRelations(), GetPack(), GetLicence()), store_pack (GetPack(), GetAssets(), GetIngestHistory()),
 *  store_licence (GetLicence()), store_locations (GetLocations()), store_vocabulary (GetVocabulary()),
 *  store_saved_searches (GetSavedSearches()) and store_unclassified (Search() with unclassified). Results are built
 *  member by member from the records: descriptions, source categories and licence texts are untrusted text objects,
 *  the roots of locations are their VFS roots through CRedactor::ForUrl() (never the folder of the disk), and every
 *  search hit says whether the content of its pack may be shown (contentExposure).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreReadTool : public CAssetToolBase
{
    public:

    CAssetStoreReadTool( CAssetToolContext& context, const CORE::CString& name );

    virtual ~CAssetStoreReadTool() GUCEF_VIRTUAL_OVERRIDE;

    /** The query of the members of store_search (also the query of store_save_search) */
    static void BuildQuery( const CORE::CDataNode& arguments, CAssetSearchQuery& query );

    protected:

    virtual COM::EMcpExecuteResult Compute( COM::CMcpRequestContext& context      ,
                                            const CORE::CDataNode& arguments      ,
                                            COM::CMcpToolResult& result           ,
                                            COM::CMcpInputRequired& inputRequired ,
                                            SCallState& state                     ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void AddLicenceSummary( const CAssetLicence& licence, const CORE::CString& storePackId, CORE::CDataNode& object );

    COM::EMcpExecuteResult Search( const CORE::CDataNode& arguments, COM::CMcpToolResult& result, bool unclassified );
    COM::EMcpExecuteResult Asset( const CORE::CDataNode& arguments, COM::CMcpToolResult& result );
    COM::EMcpExecuteResult Pack( const CORE::CDataNode& arguments, COM::CMcpToolResult& result );
    COM::EMcpExecuteResult Licence( const CORE::CDataNode& arguments, COM::CMcpToolResult& result );
    COM::EMcpExecuteResult Locations( COM::CMcpToolResult& result );
    COM::EMcpExecuteResult Vocabulary( const CORE::CDataNode& arguments, COM::CMcpToolResult& result );
    COM::EMcpExecuteResult SavedSearches( const CORE::CDataNode& arguments, COM::CMcpToolResult& result );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREREADTOOL_H ? */
