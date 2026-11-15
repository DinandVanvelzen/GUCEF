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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLCATALOGUE_H
#define GUCEF_ASSETSTORE_CASSETTOOLCATALOGUE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMCPTOOLDESCRIPTOR_H
#include "gucefCOM_CMcpToolDescriptor.h"
#define GUCEF_COM_CMCPTOOLDESCRIPTOR_H
#endif /* GUCEF_COM_CMCPTOOLDESCRIPTOR_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETVOCABULARYINFO_H
#include "gucefASSETSTORE_CAssetVocabularyInfo.h"
#define GUCEF_ASSETSTORE_CASSETVOCABULARYINFO_H
#endif /* GUCEF_ASSETSTORE_CASSETVOCABULARYINFO_H ? */

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
 *  The tools of the asset tool set (section 7 of the asset tools design): for every tool its name, the group that
 *  enables it (section 7.7: "read" always on; "convert", "author", "store" from the exposure file), its effect, the
 *  family of classes that carries it out, its description for the model, and its input and output schemas
 *  (COM::CMcpSchemaBuilder: inputs refuse undeclared members and carry their bounds; outputs describe the members a
 *  client can rely on and allow more). The order of the table is the order of tools/list.
 *
 *  The schema of store_search takes its enumerations (kinds, themes, styles, licence classes, restriction flags) from
 *  the vocabulary of the store when one is given, so a model cannot invent values; a change of the vocabulary
 *  registers the tool again.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolCatalogue
{
    public:

    enum EFamily
    {
        FAMILY_PIPELINE_QUERY = 0   ,   /**< RunPipelineQuery() of the operation of the same name */
        FAMILY_PIPELINE_CHANGE      ,   /**< preview with RunPipelineQuery(), apply with ApplyPipelineChange() */
        FAMILY_JOB_START            ,   /**< a job of the job bridge over a job of the service */
        FAMILY_JOB                  ,   /**< job_status, job_result, job_list, job_cancel */
        FAMILY_STORE_READ           ,   /**< the catalogue of the primary store */
        FAMILY_STORE_WRITE          ,   /**< saved searches, classification, rules */
        FAMILY_ONLINE               ,   /**< online stores and the transfer queue */
        FAMILY_RENDER                   /**< render_sheet */
    };

    struct GUCEF_ASSETSTORE_PUBLIC_CPP SEntry
    {
        const char* name;
        const char* group;
        EFamily family;
        COM::EMcpToolEffect effect;
        bool idempotent;
        bool openWorld;
        const char* title;
        const char* description;
    };

    static UInt32 GetCount( void );

    static const SEntry* GetEntry( UInt32 index );

    /** GUCEF_NULL when the name is not a tool of the set */
    static const SEntry* Find( const CORE::CString& name );

    /** The group of a tool; empty when the name is not a tool of the set */
    static CORE::CString GetGroup( const CORE::CString& name );

    /** The group that controls a job of the kind (the name of the tool that started it) */
    static CORE::CString GetGroupOfJobKind( const CORE::CString& kind );

    /** Whether the tool may change something (its effect is not read-only) */
    static bool IsWriteTool( const CORE::CString& name );

    /** The descriptor of the tool; 'vocabulary' (may be GUCEF_NULL) feeds the enumerations of store_search */
    static bool BuildDescriptor( const CORE::CString& name, const CAssetVocabularyInfo* vocabulary, COM::CMcpToolDescriptor& descriptor );

    static CORE::CDataNode BuildInputSchema( const CORE::CString& name, const CAssetVocabularyInfo* vocabulary );

    static CORE::CDataNode BuildOutputSchema( const CORE::CString& name );

    /** The output schema of a job object, with the members the asset tools add (packId, resultUri, summary, hint) */
    static CORE::CDataNode BuildJobSchema( void );

    private:

    CAssetToolCatalogue( void );    /**< not implemented: static functions only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTOOLCATALOGUE_H ? */
