/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <vector>

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"            /* OS wrapping functionality */
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"          /* data storage node class,.. this is what it's all about */
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"        /* file recource access */
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CPLUGINFACTORYADAPTERFORDATADRIVENDSTORECODEC_H
#include "gucefCORE_CPluginFactoryAdapterForDataDrivenDStoreCodec.h"
#define GUCEF_CORE_CPLUGINFACTORYADAPTERFORDATADRIVENDSTORECODEC_H
#endif /* GUCEF_CORE_CPLUGINFACTORYADAPTERFORDATADRIVENDSTORECODEC_H ? */

#include "CDStoreCodecPlugin.h"  /* definition of the class implemented here */

#ifndef GUCEF_CORE_C_API_H
#include "gucefCORE_c_api.h"
#define GUCEF_CORE_C_API_H
#endif /* GUCEF_CORE_C_API_H ? */

#ifndef GUCEF_CORE_C_API_IMP_H
#include "gucefCORE_c_api_impl.h"
#define GUCEF_CORE_C_API_IMP_H
#endif /* GUCEF_CORE_C_API_IMP_H ? */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SParserData
{
    CDataNode* root;
    CDataNode* curnode;
    CString error;
    Int32 errorcode;
};

typedef struct SParserData TParserData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
OnTreeBeginHandler( void* privdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    TParserData* pd = static_cast<TParserData*>(privdata);
    pd->errorcode = 0;
    pd->error = GUCEF_NULL;
    pd->curnode = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
OnTreeEndHandler( void* privdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;
        /* currently not used */
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
OnNodeBeginHandler( void* privdata       ,
                    const char* nodename ,
                    int nodeType         ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;
    TParserData* pd = static_cast<TParserData*>(privdata);
    if ( GUCEF_NULL != pd )
    {
        if ( GUCEF_NULL != pd->curnode )
        {
            pd->curnode = pd->curnode->AddChild( nodename, nodeType );
            return;
        }

        /*
        *      First node
        */
        pd->curnode = pd->root;
        pd->curnode->SetName( nodename );
        pd->curnode->SetNodeType( nodeType );
    }
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
OnNodeEndHandler( void* privdata       ,
                  const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    TParserData* pd = static_cast<TParserData*>(privdata);
    if ( GUCEF_NULL != pd )
    {
        if ( GUCEF_NULL != pd->curnode )
        {
            pd->curnode = pd->curnode->GetParent();
        }
    }
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
OnNodeAttHandler( void* privdata               ,
                  const char* nodename         ,
                  const TVariantData* attname  ,
                  const TVariantData* attvalue ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    TParserData* pd = static_cast<TParserData*>(privdata);
    if ( GUCEF_NULL != pd && GUCEF_NULL != pd->curnode )
    {
        if ( GUCEF_NULL != attvalue )
        {
            Int32 nodeType = pd->curnode->GetNodeType();
            if ( GUCEF_DATATYPE_ARRAY == nodeType ||
                 GUCEF_DATATYPE_SET == nodeType    )
            {
                pd->curnode->AddValueAsChild( *attvalue );
            }
            else
            {
                if ( GUCEF_NULL != attname )
                {
                    pd->curnode->SetAttribute( *attname, *attvalue );
                }
                else
                {
                    pd->curnode->SetValue( *attvalue );
                }
            }   
        }
    }
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
OnNodeValueHandler( void* privdata               ,
                    const char* nodename         ,
                    const TVariantData* attvalue ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    TParserData* pd = static_cast<TParserData*>(privdata);
    if ( GUCEF_NULL != pd && GUCEF_NULL != pd->curnode )
    {
        if ( GUCEF_NULL != attvalue )
        {
            Int32 nodeType = pd->curnode->GetNodeType();
            if ( GUCEF_DATATYPE_ARRAY == nodeType ||
                 GUCEF_DATATYPE_SET == nodeType    )
            {
                pd->curnode->AddValueAsChild( *attvalue );
            }
            else
            {
                pd->curnode->SetValue( *attvalue );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
OnNodeChildrenBeginHandler( void* privdata       ,
                            const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;
        /* currently not used */
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
OnNodeChildrenEndHandler( void* privdata       ,
                          const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;
        /* currently not used */
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
OnParserErrorHandler( void* privdata          ,
                      Int32 errorCode         ,
                      const char* description ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL == privdata )
        return;

    TParserData* pd = static_cast<TParserData*>(privdata);
    pd->error = description;
    pd->errorcode = errorCode;

    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DStore Codec error with code " + ToString( errorCode ) + " and description : " + pd->error );
}

/*-------------------------------------------------------------------------*/

/**
 *  Forwards the read events of a plugin to a CIDocumentParserCallbacks implementation.
 *  Values held by array and set nodes are reported as node values, matching how BuildDataTree() interprets them.
 */
struct SParseForwardData
{
    CIDocumentParserCallbacks* callbacks;
    std::vector< int > nodeTypes;
    bool hadError;
};

typedef struct SParseForwardData TParseForwardData;

/*-------------------------------------------------------------------------*/

static inline CVariant
NodeIdFromName( const char* nodename )
{GUCEF_TRACE;

    return CVariant( GUCEF_NULL != nodename ? nodename : "" );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
ForwardTreeBeginHandler( void* privdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    TParseForwardData* fd = static_cast< TParseForwardData* >( privdata );
    fd->callbacks->OnDocumentBegin();
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
ForwardTreeEndHandler( void* privdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    TParseForwardData* fd = static_cast< TParseForwardData* >( privdata );
    fd->callbacks->OnDocumentEnd();
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
ForwardNodeBeginHandler( void* privdata       ,
                         const char* nodename ,
                         int nodeType         ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    TParseForwardData* fd = static_cast< TParseForwardData* >( privdata );
    fd->nodeTypes.push_back( nodeType );
    fd->callbacks->OnNodeBegin( NodeIdFromName( nodename ), nodeType );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
ForwardNodeEndHandler( void* privdata       ,
                       const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    TParseForwardData* fd = static_cast< TParseForwardData* >( privdata );
    if ( !fd->nodeTypes.empty() )
        fd->nodeTypes.pop_back();
    fd->callbacks->OnNodeEnd( NodeIdFromName( nodename ) );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
ForwardNodeAttHandler( void* privdata               ,
                       const char* nodename         ,
                       const TVariantData* attname  ,
                       const TVariantData* attvalue ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL == attvalue )
        return;

    TParseForwardData* fd = static_cast< TParseForwardData* >( privdata );
    CVariant value( attvalue, false );

    int nodeType = fd->nodeTypes.empty() ? GUCEF_DATATYPE_OBJECT : fd->nodeTypes.back();
    if ( GUCEF_NULL == attname || GUCEF_DATATYPE_ARRAY == nodeType || GUCEF_DATATYPE_SET == nodeType )
    {
        fd->callbacks->OnNodeValue( NodeIdFromName( nodename ), value );
    }
    else
    {
        fd->callbacks->OnNodeAttribute( NodeIdFromName( nodename ), CVariant( attname, false ), value );
    }
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
ForwardNodeValueHandler( void* privdata               ,
                         const char* nodename         ,
                         const TVariantData* attvalue ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL == attvalue )
        return;

    TParseForwardData* fd = static_cast< TParseForwardData* >( privdata );
    fd->callbacks->OnNodeValue( NodeIdFromName( nodename ), CVariant( attvalue, false ) );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
ForwardNodeChildrenBeginHandler( void* privdata       ,
                                 const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    TParseForwardData* fd = static_cast< TParseForwardData* >( privdata );
    fd->callbacks->OnNodeChildrenBegin( NodeIdFromName( nodename ) );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
ForwardNodeChildrenEndHandler( void* privdata       ,
                               const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    TParseForwardData* fd = static_cast< TParseForwardData* >( privdata );
    fd->callbacks->OnNodeChildrenEnd( NodeIdFromName( nodename ) );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
ForwardParserErrorHandler( void* privdata          ,
                           Int32 errorCode         ,
                           const char* description ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    TParseForwardData* fd = static_cast< TParseForwardData* >( privdata );
    fd->hadError = true;
    CString errorText( GUCEF_NULL != description ? description : "" );
    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DStore Codec error with code " + ToString( errorCode ) + " and description : " + errorText );
    fd->callbacks->OnParseError( errorCode, errorText );
}

/*-------------------------------------------------------------------------*/

/**
 *  Passes the nodes written to it straight on to a dstore codec plugin
 */
class CDStoreCodecPluginDocumentWriter : public CDocumentStreamWriter
{
    public:

    CDStoreCodecPluginDocumentWriter( CDStoreCodecPlugin& plugin, void** codecData, void* filedata )
        : CDocumentStreamWriter()
        , m_plugin( plugin )
        , m_codecData( codecData )
        , m_filedata( filedata )
        , m_openNodes()
    {GUCEF_TRACE;
    }

    virtual ~CDStoreCodecPluginDocumentWriter() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        Close();
    }

    protected:

    virtual bool OnBeginNode( const CDataNode& node, bool willHaveChildren ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_plugin.BeginNodeStore( m_codecData, &m_filedata, node, willHaveChildren );

        SOpenNode openNode;
        openNode.name = node.GetName();
        openNode.attCount = node.GetAttCount();
        openNode.hasChildren = willHaveChildren;
        m_openNodes.push_back( openNode );
        return true;
    }

    virtual bool OnEndNode( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( m_openNodes.empty() )
            return false;

        const SOpenNode& openNode = m_openNodes.back();
        m_plugin.EndNodeStore( m_codecData, &m_filedata, openNode.name, openNode.attCount, openNode.hasChildren );
        m_openNodes.pop_back();
        return true;
    }

    virtual bool OnWriteNode( const CDataNode& node ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_plugin.StoreNode( m_codecData, &node, &m_filedata );
        return true;
    }

    virtual bool OnClose( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_plugin.m_api.Dest_File_Close( &m_plugin.m_plugdata, m_codecData, &m_filedata );
        return true;
    }

    private:

    struct SOpenNode
    {
        CString name;
        UInt32 attCount;
        bool hasChildren;
    };

    CDStoreCodecPlugin& m_plugin;
    void** m_codecData;
    void* m_filedata;
    std::vector< SOpenNode > m_openNodes;
};

/*-------------------------------------------------------------------------*/

TPluginMetaDataPtr
CDStoreCodecPlugin::GetMetaData( void ) const
{GUCEF_TRACE;

    return m_metaData;
}

/*-------------------------------------------------------------------------*/

void*
CDStoreCodecPlugin::GetModulePointer( void )
{GUCEF_TRACE;

    return m_sohandle;
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::Link( void* modulePtr                   ,
                          TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    if ( IsLoaded() ) 
        return false;

    m_sohandle = modulePtr;
    if ( GUCEF_NULL != m_sohandle )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "DStoreCodecPlugin: Linking API using module pointer: " + PointerToString( modulePtr ) );

        m_api.Init = (TDSTOREPLUGFPTR_Init) GetFunctionAddress( m_sohandle         ,
                                                                "DSTOREPLUG_Init" ,
                                                                1*sizeof(void*)   ).funcPtr;
        m_api.Shutdown = (TDSTOREPLUGFPTR_Shutdown) GetFunctionAddress( m_sohandle             ,
                                                                        "DSTOREPLUG_Shutdown" ,
                                                                        1*sizeof(void*)       ).funcPtr;
        m_api.Name = (TDSTOREPLUGFPTR_Name) GetFunctionAddress( m_sohandle         ,
                                                                "DSTOREPLUG_Name" ,
                                                                1*sizeof(void*)   ).funcPtr;
        m_api.Copyright = (TDSTOREPLUGFPTR_Copyright) GetFunctionAddress( m_sohandle              ,
                                                                          "DSTOREPLUG_Copyright" ,
                                                                          1*sizeof(void*)        ).funcPtr;
        m_api.Version = (TDSTOREPLUGFPTR_Version) GetFunctionAddress( m_sohandle            ,
                                                                      "DSTOREPLUG_Version" ,
                                                                      1*sizeof(void*)      ).funcPtr;
        m_api.Type = (TDSTOREPLUGFPTR_Type) GetFunctionAddress( m_sohandle         ,
                                                                "DSTOREPLUG_Type" ,
                                                                1*sizeof(void*)   ).funcPtr;
        m_api.Type_Is_Data_Driven = (TDSTOREPLUGFPTR_Type_Is_Data_Driven) GetFunctionAddress( m_sohandle                        ,
                                                                                              "DSTOREPLUG_Type_Is_Data_Driven" ,
                                                                                              1*sizeof(void*)                  ).funcPtr;
        m_api.Create_Data_Driven_Codec = (TDSTOREPLUGFPTR_Create_Data_Driven_Codec) GetFunctionAddress( m_sohandle                             ,
                                                                                                        "DSTOREPLUG_Create_Data_Driven_Codec" ,
                                                                                                        4*sizeof(void*)                       ).funcPtr;
        m_api.Destroy_Data_Driven_Codec = (TDSTOREPLUGFPTR_Destroy_Data_Driven_Codec) GetFunctionAddress( m_sohandle                              ,
                                                                                                          "DSTOREPLUG_Destroy_Data_Driven_Codec" ,
                                                                                                          3*sizeof(void*)                        ).funcPtr;
        m_api.Dest_File_Open = (TDSTOREPLUGFPTR_Dest_File_Open) GetFunctionAddress( m_sohandle                   ,
                                                                                    "DSTOREPLUG_Dest_File_Open" ,
                                                                                    4*sizeof(void*)             ).funcPtr;
        m_api.Dest_File_Close = (TDSTOREPLUGFPTR_Dest_File_Close) GetFunctionAddress( m_sohandle                    ,
                                                                                      "DSTOREPLUG_Dest_File_Close" ,
                                                                                      3*sizeof(void*)              ).funcPtr;
        m_api.Begin_Node_Store = (TDSTOREPLUGFPTR_Begin_Node_Store) GetFunctionAddress( m_sohandle                    ,
                                                                                        "DSTOREPLUG_Begin_Node_Store" ,
                                                                                        4*sizeof(void*)+12            ).funcPtr;
        m_api.End_Node_Store = (TDSTOREPLUGFPTR_End_Node_Store) GetFunctionAddress( m_sohandle                   ,
                                                                                    "DSTOREPLUG_End_Node_Store" ,
                                                                                    4*sizeof(void*)+8           ).funcPtr;
        m_api.Store_Node_Att = (TDSTOREPLUGFPTR_Store_Node_Att) GetFunctionAddress( m_sohandle                   ,
                                                                                    "DSTOREPLUG_Store_Node_Att" ,
                                                                                    6*sizeof(void*)+12          ).funcPtr;
        m_api.Begin_Node_Children = (TDSTOREPLUGFPTR_Begin_Node_Children) GetFunctionAddress( m_sohandle                        ,
                                                                                              "DSTOREPLUG_Begin_Node_Children" ,
                                                                                              4*sizeof(void*)                  ).funcPtr;
        m_api.End_Node_Children = (TDSTOREPLUGFPTR_End_Node_Children) GetFunctionAddress( m_sohandle                      ,
                                                                                          "DSTOREPLUG_End_Node_Children" ,
                                                                                          4*sizeof(void*)                ).funcPtr;
        m_api.Src_File_Open = (TDSTOREPLUGFPTR_Src_File_Open) GetFunctionAddress( m_sohandle                  ,
                                                                                  "DSTOREPLUG_Src_File_Open" ,
                                                                                  3*sizeof(void*)            ).funcPtr;
        m_api.Src_File_Close = (TDSTOREPLUGFPTR_Src_File_Close) GetFunctionAddress( m_sohandle                   ,
                                                                                    "DSTOREPLUG_Src_File_Close" ,
                                                                                    3*sizeof(void*)             ).funcPtr;
        m_api.Set_Read_Handlers = (TDSTOREPLUGFPTR_Set_Read_Handlers) GetFunctionAddress( m_sohandle                      ,
                                                                                          "DSTOREPLUG_Set_Read_Handlers" ,
                                                                                          5*sizeof(void*)                ).funcPtr;
        m_api.Start_Reading = (TDSTOREPLUGFPTR_Start_Reading) GetFunctionAddress( m_sohandle                  ,
                                                                                  "DSTOREPLUG_Start_Reading" ,
                                                                                  3*sizeof(void*)            ).funcPtr;


        // Check to make sure the must-have function pointers are valid
        if ( ( m_api.Init == GUCEF_NULL ) ||
             ( m_api.Shutdown == GUCEF_NULL ) ||
             ( m_api.Name == GUCEF_NULL ) ||
             ( m_api.Copyright == GUCEF_NULL ) ||
             ( m_api.Version == GUCEF_NULL ) ||
             ( m_api.Type == GUCEF_NULL ) ||
             ( m_api.Dest_File_Open == GUCEF_NULL ) ||
             ( m_api.Dest_File_Close == GUCEF_NULL ) ||
             ( m_api.Begin_Node_Store == GUCEF_NULL ) ||
             ( m_api.End_Node_Store == GUCEF_NULL ) ||
             ( m_api.Store_Node_Att == GUCEF_NULL ) ||
             ( m_api.Begin_Node_Children == GUCEF_NULL ) ||
             ( m_api.End_Node_Children == GUCEF_NULL ) ||
             ( m_api.Src_File_Open == GUCEF_NULL ) ||
             ( m_api.Src_File_Close == GUCEF_NULL ) ||
             ( m_api.Set_Read_Handlers == GUCEF_NULL ) ||
             ( m_api.Start_Reading == GUCEF_NULL ) )
        {
            memset( &m_api, 0, sizeof(m_api) );
            m_sohandle = GUCEF_NULL;

            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DStoreCodecPlugin: Invalid codec module: One or more functions could not be located in the module " + PointerToString( modulePtr ) );
            return false;
        }

        /*
         *      Intialize the plugin module
         */
        TGucefCoreCApi libraryCApi;
        GUCEF_CORE_LinkCApi( &libraryCApi );
        UInt32 statusCode = m_api.Init( &m_plugdata, &libraryCApi );
        if ( 0 == statusCode )
        {
            memset( &m_api, 0, sizeof(m_api) );
            m_sohandle = GUCEF_NULL;
            m_plugdata = GUCEF_NULL;

            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DStoreCodecPlugin: Initialization routine reported an error for module " + PointerToString( modulePtr ) );
            return false;
        }

        if ( IsCodecTypeDataDriven() )
        {
            if ( GUCEF_NULL == m_ddCodecFactory )
            {
                m_ddCodecFactory = GUCEF_NEW CPluginFactoryAdapterForDataDrivenDStoreCodec( this );
                if ( GUCEF_NULL == m_ddCodecFactory )
                {
                    Unlink();                    
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DStoreCodecPlugin: Failed to create factory for data driven codec" );
                    return false;
                }
            }
        }

        // We have loaded & linked our plugin module
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "DStoreCodecPlugin: Successfully loaded module and invoked Init() which returned status " +
                Int32ToString( statusCode  ) + " using module: " + PointerToString( modulePtr ) );
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "  - Name: " + GetName() );
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "  - Copyright/EULA: " + GetCopyright() );

        // Copy the given metadata and update it with info from the actual module
        m_metaData = TPluginMetaDataStoragePtr( GUCEF_NEW CPluginMetaData( *pluginMetaData ) );
        m_metaData->SetDescription( GetDescription() );
        m_metaData->SetCopyright( GetCopyright() );
        m_metaData->SetVersion( GetVersion() );

        return true;
    }               

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::Unlink( void )
{GUCEF_TRACE;

    if ( IsLoaded() )
    {
        // The data driven codecs made through the plugin can outlive it: the plugin destroys their data while it can
        if ( GUCEF_NULL != m_ddCodecFactory )
            m_ddCodecFactory->DetachAllLinks();

        // Call the module's shutdown()
        if ( GUCEF_NULL != m_api.Shutdown )
            m_api.Shutdown( &m_plugdata );
        
        // Cleanup recources
        GUCEF_DELETE m_ddCodecFactory;
        m_ddCodecFactory = GUCEF_NULL;
        memset( &m_api, 0, sizeof(m_api) );
        m_metaData.Unlink();
        m_sohandle = GUCEF_NULL;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

CDStoreCodecPlugin::CDStoreCodecPlugin( void )
    : CDStoreCodec()    
    , CIPlugin()        
    , _id( 0 )          
    , m_api()
    , m_sohandle( GUCEF_NULL ) 
    , m_codecData( GUCEF_NULL )
    , m_plugdata( GUCEF_NULL ) 
    , m_metaData()
    , m_ddCodecFactory( GUCEF_NULL )
{GUCEF_TRACE;

    memset( &m_api, 0, sizeof(m_api) );
}

/*-------------------------------------------------------------------------*/

CDStoreCodecPlugin::~CDStoreCodecPlugin()
{GUCEF_TRACE;

    Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::IsLoaded( void ) const
{GUCEF_TRACE;

    return m_sohandle != GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CString
CDStoreCodecPlugin::GetDescription( void ) const
{GUCEF_TRACE;

    return GetName();
}

/*-------------------------------------------------------------------------*/

void
CDStoreCodecPlugin::BeginNodeStore( void** codecData   ,
                                    void** filedata    ,
                                    const CDataNode& n ,
                                    bool hasChildren   )
{GUCEF_TRACE;

    int nodeType( n.GetNodeType() );
    UInt32 count( n.GetAttCount() );
    const char* name( n.GetName().C_String() );
    const CDataNode::TKeyValuePair* att = GUCEF_NULL;
    UInt32 hasChildrenFlag = hasChildren ? 1 : 0;

    /*
     *  Begin storing the node
     */
    UInt32 valueAsAtt = n.HasValue() ? 1 : 0;
    m_api.Begin_Node_Store( &m_plugdata        ,
                            codecData          ,
                            filedata           ,
                            name               ,
                            nodeType           ,
                            count + valueAsAtt ,
                            hasChildrenFlag    );

    /*
     *  Store all node attributes
     */
    for ( UInt32 i=0; i<count; ++i )
    {
            att = n.GetAttribute( i );
            m_api.Store_Node_Att( &m_plugdata                ,
                                  codecData                  ,
                                  filedata                   ,
                                  name                       ,
                                  count                      ,
                                  i                          ,
                                  att->first.C_String()      ,
                                  att->second.CStyleAccess() ,
                                  hasChildrenFlag            );
    }

    /*
     *  Store node value
     */
    if ( n.HasValue() )
    {
        m_api.Store_Node_Att( &m_plugdata                 ,
                              codecData                   ,
                              filedata                    ,
                              name                        ,
                              count + valueAsAtt          ,
                              count                       ,
                              GUCEF_NULL                  ,
                              n.GetValue().CStyleAccess() ,
                              hasChildrenFlag             );
    }

    if ( hasChildren )
    {
        m_api.Begin_Node_Children( &m_plugdata ,
                                   codecData   ,
                                   filedata    ,
                                   name        );
    }
}

/*-------------------------------------------------------------------------*/

void
CDStoreCodecPlugin::EndNodeStore( void** codecData    ,
                                  void** filedata     ,
                                  const CString& name ,
                                  UInt32 attCount     ,
                                  bool hasChildren    )
{GUCEF_TRACE;

    if ( hasChildren )
    {
        m_api.End_Node_Children( &m_plugdata     ,
                                 codecData       ,
                                 filedata        ,
                                 name.C_String() );
    }

    /*
     *      We are finished with this node
     */
    m_api.End_Node_Store( &m_plugdata         ,
                          codecData           ,
                          filedata            ,
                          name.C_String()     ,
                          attCount            ,
                          hasChildren ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

void
CDStoreCodecPlugin::StoreNode( void** codecData   ,
                               const CDataNode* n ,
                               void** filedata    )
{GUCEF_TRACE;

    bool hasChildren = n->HasChildren();
    BeginNodeStore( codecData, filedata, *n, hasChildren );

    /*
     *      Iterate the child node recursively storing each node level
     */
    if ( hasChildren )
    {
        CDataNode::const_iterator i = n->ConstBegin();
        while ( i != n->ConstEnd() )
        {
            StoreNode( codecData, (*i), filedata );
            ++i;
        }
    }

    EndNodeStore( codecData, filedata, n->GetName(), n->GetAttCount(), hasChildren );
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::StoreDataTree( const CDataNode* tree   ,
                                   const CString& filename )
{GUCEF_TRACE;

    return StoreDataTree( &m_codecData, tree, filename );
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::StoreDataTree( void** codecData        ,
                                   const CDataNode* tree   ,
                                   const CString& filename )
{GUCEF_TRACE;

    // Note: opening the file for writing creates any missing parent directories
    // Calling CreateDirs() on the file path itself would create a directory named after the file
    {
        CFileAccess access( filename, "wb" );
        if ( access.IsValid() )
        {
            return StoreDataTree( codecData, tree, &access );
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::StoreDataTree( const CDataNode* tree   ,
                                   CIOAccess* file         )
{GUCEF_TRACE;

    return StoreDataTree( &m_codecData, tree, file );
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::StoreDataTree( void** codecData        ,
                                   const CDataNode* tree   ,
                                   CIOAccess* file         )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_sohandle )
    {
        return false;
    }

    /*
     *      Open the destination file
     */
    void* filedata;
    if ( !m_api.Dest_File_Open( &m_plugdata, codecData, &filedata, file->CStyleAccess() ) )
    {
        return false;
    }

    /*
     *      Recursively traverse the tree storing each node and sub-triers as we go
     */
    StoreNode( codecData, tree, &filedata );

    /*
     *      We are finished,.. close the file
     */
    m_api.Dest_File_Close( &m_plugdata, codecData, &filedata );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::BuildDataTree( CDataNode* treeroot     ,
                                   const CString& filename )

{GUCEF_TRACE;

    return BuildDataTree( &m_codecData, treeroot, filename );
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::BuildDataTree( void** codecData         ,
                                   CDataNode* treeroot     ,
                                   const CString& filename )

{GUCEF_TRACE;

    CFileAccess access( filename );
    if ( access.IsValid() )
    {
        return BuildDataTree( codecData, treeroot, &access );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::BuildDataTree( CDataNode* treeroot ,
                                   CIOAccess* file     )
{GUCEF_TRACE;

    return BuildDataTree( &m_codecData, treeroot, file );
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::BuildDataTree( void** codecData    ,
                                   CDataNode* treeroot ,
                                   CIOAccess* file     )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_sohandle )
    {
        return false;
    }

    /*
     *  Open the source file
     */
    void* filedata;
    if ( !m_api.Src_File_Open( &m_plugdata, codecData, &filedata, file->CStyleAccess() ) )
    {
        return false;
    }

    /*
     *      Hook up our read event handlers
     */
    void* privdata = GUCEF_NEW TParserData;
    static_cast<TParserData*>(privdata)->root = treeroot;
    TReadHandlers rhandlers;
    rhandlers.OnTreeBegin = OnTreeBeginHandler;
    rhandlers.OnTreeEnd = OnTreeEndHandler;
    rhandlers.OnNodeBegin = OnNodeBeginHandler;
    rhandlers.OnNodeEnd = OnNodeEndHandler;
    rhandlers.OnNodeAtt = OnNodeAttHandler;
    rhandlers.OnNodeValue = OnNodeValueHandler;
    rhandlers.OnNodeChildrenBegin = OnNodeChildrenBeginHandler;
    rhandlers.OnNodeChildrenEnd = OnNodeChildrenEndHandler;
    rhandlers.OnError = OnParserErrorHandler;
    m_api.Set_Read_Handlers( &m_plugdata, codecData, &filedata, &rhandlers, privdata );

    /*
     *      So far so good,.. delete whatever children the treeroot may alreay have
     *      and start reading the data. This should cause the handlers to be called when
     *      needed and they in turn construct our tree based on the events.
     */
    treeroot->DelSubTree();
    UInt32 errorCode = m_api.Start_Reading( &m_plugdata, codecData, &filedata );
    GUCEF_DELETE static_cast<TParserData*>(privdata);

    /*
     *      We are finished,.. close the file
     */
    m_api.Src_File_Close( &m_plugdata, codecData, &filedata );
    return 0 == errorCode;
}

/*-------------------------------------------------------------------------*/

CDocumentStreamWriterPtr
CDStoreCodecPlugin::CreateDocumentWriter( CIOAccess& output )
{GUCEF_TRACE;

    return CreateDocumentWriter( &m_codecData, output );
}

/*-------------------------------------------------------------------------*/

CDocumentStreamWriterPtr
CDStoreCodecPlugin::CreateDocumentWriter( void** codecData, CIOAccess& output )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_sohandle )
        return CDocumentStreamWriterPtr();

    void* filedata = GUCEF_NULL;
    if ( !m_api.Dest_File_Open( &m_plugdata, codecData, &filedata, output.CStyleAccess() ) )
        return CDocumentStreamWriterPtr();

    return CDocumentStreamWriterPtr( GUCEF_NEW CDStoreCodecPluginDocumentWriter( *this, codecData, filedata ) );
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::ParseDocument( CIOAccess& input                     ,
                                   CIDocumentParserCallbacks& callbacks )
{GUCEF_TRACE;

    return ParseDocument( &m_codecData, input, callbacks );
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::ParseDocument( void** codecData                     ,
                                   CIOAccess& input                     ,
                                   CIDocumentParserCallbacks& callbacks )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_sohandle )
        return false;

    void* filedata = GUCEF_NULL;
    if ( !m_api.Src_File_Open( &m_plugdata, codecData, &filedata, input.CStyleAccess() ) )
        return false;

    TParseForwardData forwardData;
    forwardData.callbacks = &callbacks;
    forwardData.hadError = false;

    TReadHandlers rhandlers;
    rhandlers.OnTreeBegin = ForwardTreeBeginHandler;
    rhandlers.OnTreeEnd = ForwardTreeEndHandler;
    rhandlers.OnNodeBegin = ForwardNodeBeginHandler;
    rhandlers.OnNodeEnd = ForwardNodeEndHandler;
    rhandlers.OnNodeAtt = ForwardNodeAttHandler;
    rhandlers.OnNodeValue = ForwardNodeValueHandler;
    rhandlers.OnNodeChildrenBegin = ForwardNodeChildrenBeginHandler;
    rhandlers.OnNodeChildrenEnd = ForwardNodeChildrenEndHandler;
    rhandlers.OnError = ForwardParserErrorHandler;
    m_api.Set_Read_Handlers( &m_plugdata, codecData, &filedata, &rhandlers, &forwardData );

    UInt32 errorCode = m_api.Start_Reading( &m_plugdata, codecData, &filedata );

    m_api.Src_File_Close( &m_plugdata, codecData, &filedata );
    return 0 == errorCode && !forwardData.hadError;
}

/*-------------------------------------------------------------------------*/

CString
CDStoreCodecPlugin::GetTypeName( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_api.Type )
        return m_api.Type( m_plugdata );
    return CString::Empty; 
}

/*-------------------------------------------------------------------------*/

bool 
CDStoreCodecPlugin::IsCodecTypeDataDriven( void ) const
{GUCEF_TRACE;

    // Check if the plugin has a function to determine if it is data driven
    // This function is optional and may not be present in all plugins
    // not having the function means the plugin is not data driven, same as having it and returning 0
    if ( GUCEF_NULL != m_api.Type_Is_Data_Driven )
        return m_api.Type_Is_Data_Driven( m_plugdata ) != 0;
    return false; 
}

/*-------------------------------------------------------------------------*/

CPluginFactoryAdapterForDataDrivenDStoreCodec* 
CDStoreCodecPlugin::GetDataDrivenCodecFactory( void ) const
{GUCEF_TRACE;

    return m_ddCodecFactory;
}

/*-------------------------------------------------------------------------*/

CString
CDStoreCodecPlugin::GetName( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_api.Name )
        return m_api.Name( m_plugdata );
    return CString::Empty; 
}

/*-------------------------------------------------------------------------*/

CString
CDStoreCodecPlugin::GetCopyright( void ) const
{GUCEF_TRACE;
    
    if ( GUCEF_NULL != m_api.Copyright )
        return m_api.Copyright( m_plugdata );
    return CString::Empty; 
}

/*-------------------------------------------------------------------------*/

TVersion
CDStoreCodecPlugin::GetVersion( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_api.Version )
    {
        const TVersion* versionPtr = m_api.Version( m_plugdata );
        if ( GUCEF_NULL != versionPtr )
            return *versionPtr;
    }

    // If the plugin does not have a version function we return a default version
    TVersion version = { 0, 0, 0, 0 }; // major, minor, patch, release
    return version; 
}

/*-------------------------------------------------------------------------*/

CICloneable*
CDStoreCodecPlugin::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
