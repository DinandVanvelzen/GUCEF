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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLBASE_H
#define GUCEF_ASSETSTORE_CASSETTOOLBASE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_COM_CIMCPTOOL_H
#include "gucefCOM_CIMcpTool.h"
#define GUCEF_COM_CIMCPTOOL_H
#endif /* GUCEF_COM_CIMCPTOOL_H ? */

#ifndef GUCEF_COM_CMCPINPUTREQUIRED_H
#include "gucefCOM_CMcpInputRequired.h"
#define GUCEF_COM_CMCPINPUTREQUIRED_H
#endif /* GUCEF_COM_CMCPINPUTREQUIRED_H ? */

#ifndef GUCEF_COM_CMCPREQUESTCONTEXT_H
#include "gucefCOM_CMcpRequestContext.h"
#define GUCEF_COM_CMCPREQUESTCONTEXT_H
#endif /* GUCEF_COM_CMCPREQUESTCONTEXT_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H
#include "gucefASSETSTORE_CAssetToolContext.h"
#define GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H ? */

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
 *  The base of the tools of the asset tool set: the descriptor comes from CAssetToolCatalogue, Execute() is fixed
 *  (section 10.5 of the asset tools design) and runs on the pool of the server:
 *
 *  1. the call enters the call gate of the tool set (a tool set that is taken away lets no call in and cancels the
 *     calls inside);
 *  2. Compute() of the tool does the work, on the service only, and tags every content block it adds with the pack it
 *     shows;
 *  3. CAssetToolContext::FinishResult() drops untagged or withheld content, redacts every text and bounds the size.
 *
 *  A tool that hands its result to the job bridge (DEFERRED, or a result its result builder already finished) says
 *  so with SCallState::finished.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolBase : public COM::CIMcpTool
{
    public:

    /** What a call collects besides its result */
    struct GUCEF_ASSETSTORE_PUBLIC_CPP SCallState
    {
        CAssetToolContext::TContentTagVector tags;
        bool finished;

        SCallState( void );
    };

    CAssetToolBase( CAssetToolContext& context, const CORE::CString& name );

    virtual ~CAssetToolBase() GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CString& GetName( void ) const;

    /** The vocabulary that feeds the enumerations of the schemas (store_search); copied */
    void SetVocabulary( const CAssetVocabularyInfo& vocabulary );

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context      ,
                                            const CORE::CDataNode& arguments      ,
                                            COM::CMcpToolResult& result           ,
                                            COM::CMcpInputRequired& inputRequired ) GUCEF_VIRTUAL_OVERRIDE;

    // argument helpers, for the tools and their tests
    static CORE::CString GetStringArgument( const CORE::CDataNode& arguments, const CORE::CString& name );
    static bool GetBoolArgument( const CORE::CDataNode& arguments, const CORE::CString& name, bool defaultValue );
    static UInt32 GetUInt32Argument( const CORE::CDataNode& arguments, const CORE::CString& name, UInt32 defaultValue );
    static Float64 GetFloat64Argument( const CORE::CDataNode& arguments, const CORE::CString& name, Float64 defaultValue );
    static void GetStringListArgument( const CORE::CDataNode& arguments, const CORE::CString& name, CORE::CString::StringVector& values );
    static void GetStringSetArgument( const CORE::CDataNode& arguments, const CORE::CString& name, CORE::CString::StringSet& values );
    static bool HasArgument( const CORE::CDataNode& arguments, const CORE::CString& name );

    /** Adds a JSON null member */
    static void SetNull( CORE::CDataNode& object, const CORE::CString& name );

    /** Sets a text member, or null when it is empty */
    static void SetTextOrNull( CORE::CDataNode& object, const CORE::CString& name, const CORE::CString& text );

    protected:

    /** The work of the tool; arguments are validated against the input schema already */
    virtual COM::EMcpExecuteResult Compute( COM::CMcpRequestContext& context      ,
                                            const CORE::CDataNode& arguments      ,
                                            COM::CMcpToolResult& result           ,
                                            COM::CMcpInputRequired& inputRequired ,
                                            SCallState& state                     ) = 0;

    /** Sets the structured result (the JSON text block is added when the result is finished) */
    static void SetStructured( COM::CMcpToolResult& result, const CORE::CDataNode& document );

    /** resource_link blocks for the "links" of a document of the service: { uri, name, mimeType } */
    static void AddResourceLinks( const CORE::CDataNode& document, COM::CMcpToolResult& result );

    /** An untrusted text object of the design (5.4), redacted and capped */
    CORE::CDataNode MakeUntrusted( const CORE::CString& source, const CORE::CString& text ) const;

    CAssetToolContext& m_context;
    CORE::CString m_name;
    bool m_hasVocabulary;
    CAssetVocabularyInfo m_vocabulary;
    mutable MT::CMutex m_vocabularyLock;     /**< the vocabulary changes while other threads ask the descriptor */

    private:

    CAssetToolBase( const CAssetToolBase& src );              /**< not implemented */
    CAssetToolBase& operator=( const CAssetToolBase& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTOOLBASE_H ? */
