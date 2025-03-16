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
                  const char* attname          ,
                  const TVariantData* attvalue ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    TParserData* pd = static_cast<TParserData*>(privdata);
    if ( GUCEF_NULL != pd && GUCEF_NULL != pd->curnode )
    {
        if ( GUCEF_NULL != attvalue )
        {
            if ( GUCEF_NULL != attname )
            {
                pd->curnode->SetAttribute( attname, *attvalue );
            }
            else        
            {
                if ( GUCEF_DATATYPE_ARRAY == pd->curnode->GetNodeType() )
                    pd->curnode->AddValueAsChild( *attvalue );
                else
                    pd->curnode->SetValue( *attvalue );
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
            if ( GUCEF_DATATYPE_ARRAY == pd->curnode->GetNodeType() )
                pd->curnode->AddValueAsChild( *attvalue );
            else
                pd->curnode->SetValue( *attvalue );
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

TPluginMetaDataPtr
CDStoreCodecPlugin::GetMetaData( void ) const
{GUCEF_TRACE;

    return m_metaData;
}

/*-------------------------------------------------------------------------*/

void*
CDStoreCodecPlugin::GetModulePointer( void )
{GUCEF_TRACE;

    return _sohandle;
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::Link( void* modulePtr                   ,
                          TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    if ( IsLoaded() ) 
        return false;

    _sohandle = modulePtr;
    if ( GUCEF_NULL != _sohandle )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "DStoreCodecPlugin: Linking API using module pointer: " + PointerToString( modulePtr ) );

        m_api.Init = (TDSTOREPLUGFPTR_Init) GetFunctionAddress( _sohandle         ,
                                                                "DSTOREPLUG_Init" ,
                                                                1*sizeof(void*)   ).funcPtr;
        m_api.Shutdown = (TDSTOREPLUGFPTR_Shutdown) GetFunctionAddress( _sohandle             ,
                                                                        "DSTOREPLUG_Shutdown" ,
                                                                        1*sizeof(void*)       ).funcPtr;
        m_api.Name = (TDSTOREPLUGFPTR_Name) GetFunctionAddress( _sohandle         ,
                                                                "DSTOREPLUG_Name" ,
                                                                1*sizeof(void*)   ).funcPtr;
        m_api.Copyright = (TDSTOREPLUGFPTR_Copyright) GetFunctionAddress( _sohandle              ,
                                                                          "DSTOREPLUG_Copyright" ,
                                                                          1*sizeof(void*)        ).funcPtr;
        m_api.Version = (TDSTOREPLUGFPTR_Version) GetFunctionAddress( _sohandle            ,
                                                                      "DSTOREPLUG_Version" ,
                                                                      1*sizeof(void*)      ).funcPtr;
        m_api.Type = (TDSTOREPLUGFPTR_Type) GetFunctionAddress( _sohandle         ,
                                                                "DSTOREPLUG_Type" ,
                                                                1*sizeof(void*)   ).funcPtr;
        m_api.Type_Is_Data_Driven = (TDSTOREPLUGFPTR_Type_Is_Data_Driven) GetFunctionAddress( _sohandle                        ,
                                                                                              "DSTOREPLUG_Type_Is_Data_Driven" ,
                                                                                              1*sizeof(void*)                  ).funcPtr;
        m_api.Dest_File_Open = (TDSTOREPLUGFPTR_Dest_File_Open) GetFunctionAddress( _sohandle                   ,
                                                                                    "DSTOREPLUG_Dest_File_Open" ,
                                                                                    3*sizeof(void*)             ).funcPtr;
        m_api.Dest_File_Close = (TDSTOREPLUGFPTR_Dest_File_Close) GetFunctionAddress( _sohandle                    ,
                                                                                      "DSTOREPLUG_Dest_File_Close" ,
                                                                                      2*sizeof(void*)              ).funcPtr;
        m_api.Begin_Node_Store = (TDSTOREPLUGFPTR_Begin_Node_Store) GetFunctionAddress( _sohandle                    ,
                                                                                        "DSTOREPLUG_Begin_Node_Store" ,
                                                                                        3*sizeof(void*)+12            ).funcPtr;
        m_api.End_Node_Store = (TDSTOREPLUGFPTR_End_Node_Store) GetFunctionAddress( _sohandle                   ,
                                                                                    "DSTOREPLUG_End_Node_Store" ,
                                                                                    3*sizeof(void*)+8           ).funcPtr;
        m_api.Store_Node_Att = (TDSTOREPLUGFPTR_Store_Node_Att) GetFunctionAddress( _sohandle                   ,
                                                                                    "DSTOREPLUG_Store_Node_Att" ,
                                                                                    5*sizeof(void*)+12          ).funcPtr;
        m_api.Begin_Node_Children = (TDSTOREPLUGFPTR_Begin_Node_Children) GetFunctionAddress( _sohandle                        ,
                                                                                              "DSTOREPLUG_Begin_Node_Children" ,
                                                                                              3*sizeof(void*)                  ).funcPtr;
        m_api.End_Node_Children = (TDSTOREPLUGFPTR_End_Node_Children) GetFunctionAddress( _sohandle                      ,
                                                                                          "DSTOREPLUG_End_Node_Children" ,
                                                                                          3*sizeof(void*)                ).funcPtr;
        m_api.Src_File_Open = (TDSTOREPLUGFPTR_Src_File_Open) GetFunctionAddress( _sohandle                  ,
                                                                                  "DSTOREPLUG_Src_File_Open" ,
                                                                                  2*sizeof(void*)            ).funcPtr;
        m_api.Src_File_Close = (TDSTOREPLUGFPTR_Src_File_Close) GetFunctionAddress( _sohandle                   ,
                                                                                    "DSTOREPLUG_Src_File_Close" ,
                                                                                    2*sizeof(void*)             ).funcPtr;
        m_api.Set_Read_Handlers = (TDSTOREPLUGFPTR_Set_Read_Handlers) GetFunctionAddress( _sohandle                      ,
                                                                                          "DSTOREPLUG_Set_Read_Handlers" ,
                                                                                          4*sizeof(void*)                ).funcPtr;
        m_api.Start_Reading = (TDSTOREPLUGFPTR_Start_Reading) GetFunctionAddress( _sohandle                  ,
                                                                                  "DSTOREPLUG_Start_Reading" ,
                                                                                  2*sizeof(void*)            ).funcPtr;


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
            _sohandle = GUCEF_NULL;

            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Invalid codec module: One or more functions could not be located in the module " + PointerToString( modulePtr ) );
            return false;
        }

        /*
         *      Intialize the plugin module
         */
        TGucefCoreCApi libraryCApi;
        GUCEF_CORE_LinkCApi( &libraryCApi );
        UInt32 statusCode = m_api.Init( &_plugdata, &libraryCApi );
        if ( statusCode > 0 )
        {
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
        else
        {
            memset( &m_api, 0, sizeof(m_api) );
            _sohandle = GUCEF_NULL;
            _plugdata = GUCEF_NULL;

            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Initialization routine reported an error for module " + PointerToString( modulePtr ) );
            return false;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::Unlink( void )
{GUCEF_TRACE;

    if ( IsLoaded() )
    {
        // Call the module's shutdown()
        if ( GUCEF_NULL != m_api.Shutdown )
            m_api.Shutdown( &_plugdata );

        // Cleanup recources
        memset( &m_api, 0, sizeof(m_api) );
        m_metaData.Unlink();
        _sohandle = GUCEF_NULL;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

CDStoreCodecPlugin::CDStoreCodecPlugin( void )
    : CDStoreCodec()    
    , CIPlugin()        
    , _id( 0 )          
    , m_api()
    , _sohandle( GUCEF_NULL ) 
    , _plugdata( GUCEF_NULL ) 
    , m_metaData()
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

    return _sohandle != GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CString
CDStoreCodecPlugin::GetDescription( void ) const
{GUCEF_TRACE;

    return GetName();
}

/*-------------------------------------------------------------------------*/

void
CDStoreCodecPlugin::StoreNode( const CDataNode* n ,
                               void** filedata    )
{GUCEF_TRACE;

    int nodeType( n->GetNodeType() );
    UInt32 count( n->GetAttCount() );
    const char* name( n->GetName().C_String() );
    const CDataNode::TKeyValuePair* att = GUCEF_NULL;

    /*
     *  Begin storing the node
     */
    UInt32 valueAsAtt = n->HasValue() ? 1 : 0;
    m_api.Begin_Node_Store( &_plugdata         ,
                            filedata           ,
                            name               ,
                            nodeType           ,
                            count + valueAsAtt ,
                            n->HasChildren()   );

    /*
     *  Store all node attributes
     */
    for ( UInt32 i=0; i<count; ++i )
    {
            att = n->GetAttribute( i );
            m_api.Store_Node_Att( &_plugdata                 ,
                                  filedata                   ,
                                  name                       ,
                                  count                      ,
                                  i                          ,
                                  att->first.C_String()      ,
                                  att->second.CStyleAccess() ,
                                  n->HasChildren()           );
    }

    /*
     *  Store node value
     */
    if ( n->HasValue() )
    {
        m_api.Store_Node_Att( &_plugdata                   ,
                              filedata                     ,
                              name                         ,
                              count + valueAsAtt           ,
                              count                        ,
                              GUCEF_NULL                   ,
                              n->GetValue().CStyleAccess() ,
                              n->HasChildren()             );
    }

    /*
     *  If the node has any children we will store them and their children ect.
     */
    if ( n->HasChildren() )
    {
        m_api.Begin_Node_Children( &_plugdata ,
                                   filedata   ,
                                   name       );

        /*
         *      Iterate the child node recursively storing each node level
         */
        CDataNode::const_iterator i = n->ConstBegin();
        while ( i != n->ConstEnd() )
        {
            StoreNode( (*i), filedata );
            ++i;
        }

        m_api.End_Node_Children( &_plugdata ,
                                 filedata   ,
                                 name       );
    }

    /*
     *      We are finished with this node
     */
    m_api.End_Node_Store( &_plugdata       ,
                          filedata         ,
                          name             ,
                          count            ,
                          n->HasChildren() );
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::StoreDataTree( const CDataNode* tree   ,
                                   const CString& filename )
{GUCEF_TRACE;

    Create_Path_Directories( filename.C_String() );

    CFileAccess access( filename, "wb" );
    if ( access.IsValid() )
    {
        return StoreDataTree( tree     ,
                              &access  );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::StoreDataTree( const CDataNode* tree   ,
                                   CIOAccess* file         )
{GUCEF_TRACE;

    if ( !_sohandle )
    {
        return false;
    }

    /*
     *      Open the destination file
     */
    void* filedata;
    if ( !m_api.Dest_File_Open( &_plugdata, &filedata, file->CStyleAccess() ) )
    {
        return false;
    }

    /*
     *      Recursively traverse the tree storing each node and sub-triers as we go
     */
    StoreNode( tree      ,
               &filedata );

    /*
     *      We are finished,.. close the file
     */
    m_api.Dest_File_Close( &_plugdata, &filedata );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::BuildDataTree( CDataNode* treeroot     ,
                                   const CString& filename )

{GUCEF_TRACE;

    CFileAccess access( filename );
    if ( access.IsValid() )
    {
        return BuildDataTree( treeroot ,
                                &access  );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDStoreCodecPlugin::BuildDataTree( CDataNode* treeroot ,
                                   CIOAccess* file     )
{GUCEF_TRACE;

    if ( !_sohandle )
    {
        return false;
    }

    /*
     *  Open the source file
     */
    void* filedata;
    if ( !m_api.Src_File_Open( &_plugdata, &filedata, file->CStyleAccess() ) )
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
    m_api.Set_Read_Handlers( &_plugdata, &filedata, &rhandlers, privdata );

    /*
     *      So far so good,.. delete whatever children the treeroot may alreay have
     *      and start reading the data. This should cause the handlers to be called when
     *      needed and they in turn construct our tree based on the events.
     */
    treeroot->DelSubTree();
    UInt32 errorCode = m_api.Start_Reading( &_plugdata, &filedata );
    GUCEF_DELETE static_cast<TParserData*>(privdata);

    /*
        *      We are finished,.. close the file
        */
    m_api.Src_File_Close( &_plugdata, &filedata );
    return 0 == errorCode;
}

/*-------------------------------------------------------------------------*/

CString
CDStoreCodecPlugin::GetTypeName( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_api.Type )
        return m_api.Type( _plugdata );
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
        return m_api.Type_Is_Data_Driven( _plugdata ) != 0;
    return false; 
}

/*-------------------------------------------------------------------------*/

CString
CDStoreCodecPlugin::GetName( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_api.Name )
        return m_api.Name( _plugdata );
    return CString::Empty; 
}

/*-------------------------------------------------------------------------*/

CString
CDStoreCodecPlugin::GetCopyright( void ) const
{GUCEF_TRACE;
    
    if ( GUCEF_NULL != m_api.Copyright )
        return m_api.Copyright( _plugdata );
    return CString::Empty; 
}

/*-------------------------------------------------------------------------*/

TVersion
CDStoreCodecPlugin::GetVersion( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_api.Version )
    {
        const TVersion* versionPtr = m_api.Version( _plugdata );
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
