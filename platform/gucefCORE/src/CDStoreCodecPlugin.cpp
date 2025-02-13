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

#ifndef GUCEF_CORE_DSTORECODECPLUGINSTRUCTS_H
#include "DStoreCodecPluginStructs.h"   /* shared structs */
#define GUCEF_CORE_DSTORECODECPLUGINSTRUCTS_H
#endif /* GUCEF_CORE_DSTORECODECPLUGINSTRUCTS_H ? */

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

/*-------------------------------------------------------------------------*/

enum
{
        DSTOREPLUG_INIT            = 0   ,
        DSTOREPLUG_SHUTDOWN              ,
        DSTOREPLUG_NAME                  ,
        DSTOREPLUG_COPYRIGHT             ,
        DSTOREPLUG_VERSION               ,
        DSTOREPLUG_TYPE                  ,

        DSTOREPLUG_DEST_FILE_OPEN        ,
        DSTOREPLUG_DEST_FILE_CLOSE       ,
        DSTOREPLUG_BEGIN_NODE_STORE      ,
        DSTOREPLUG_END_NODE_STORE        ,
        DSTOREPLUG_STORE_NODE_ATT        ,
        DSTOREPLUG_BEGIN_NODE_CHILDREN   ,
        DSTOREPLUG_END_NODE_CHILDREN     ,

        DSTOREPLUG_SRC_FILE_OPEN         ,
        DSTOREPLUG_SRC_FILE_CLOSE        ,
        DSTOREPLUG_SET_READ_HANDLERS     ,
        DSTOREPLUG_START_READING         ,

        DSTOREPLUG_LASTFPTR
};

/*-------------------------------------------------------------------------*/

typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Init )                ( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Shutdown )              ( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Name )           ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Copyright )      ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const TVersion* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Version )    ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Type )           ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Dest_File_Open )      ( void** plugdata, void** filedata, TIOAccess* file ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Dest_File_Close )       ( void** plugdata, void** filedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Begin_Node_Store )      ( void** plugdata, void** filedata, const char* nodename, int nodeType, UInt32 attscount, UInt32 haschildren ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_End_Node_Store )        ( void** plugdata, void** filedata, const char* nodename, UInt32 attscount, UInt32 haschildren ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Store_Node_Att )        ( void** plugdata, void** filedata, const char* nodename, UInt32 attscount, UInt32 attindex, const char* attname, const TVariantData* attvalue, UInt32 haschildren ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Begin_Node_Children )   ( void** plugdata, void** filedata, const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_End_Node_Children )     ( void** plugdata, void** filedata, const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Src_File_Open )       ( void** plugdata, void** filedata, TIOAccess* file ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Src_File_Close )        ( void** plugdata, void** filedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Set_Read_Handlers )     ( void** plugdata, void** filedata, const TReadHandlers* rhandlers, void* privdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Start_Reading )       ( void** plugdata, void** filedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

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

    if ( IsLoaded() ) return false;

    _sohandle = modulePtr;
    if ( GUCEF_NULL != _sohandle )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "DStoreCodecPlugin: Linking API using module pointer: " + PointerToString( modulePtr ) );

        _fptable[ DSTOREPLUG_INIT ] = GetFunctionAddress( _sohandle         ,
                                                          "DSTOREPLUG_Init" ,
                                                          1*sizeof(void*)   ).funcPtr;
        _fptable[ DSTOREPLUG_SHUTDOWN ] = GetFunctionAddress( _sohandle             ,
                                                              "DSTOREPLUG_Shutdown" ,
                                                              1*sizeof(void*)       ).funcPtr;
        _fptable[ DSTOREPLUG_NAME ] = GetFunctionAddress( _sohandle         ,
                                                          "DSTOREPLUG_Name" ,
                                                          1*sizeof(void*)   ).funcPtr;
        _fptable[ DSTOREPLUG_COPYRIGHT ] = GetFunctionAddress( _sohandle              ,
                                                               "DSTOREPLUG_Copyright" ,
                                                               1*sizeof(void*)        ).funcPtr;
        _fptable[ DSTOREPLUG_VERSION ] = GetFunctionAddress( _sohandle            ,
                                                             "DSTOREPLUG_Version" ,
                                                             1*sizeof(void*)      ).funcPtr;
        _fptable[ DSTOREPLUG_TYPE ] = GetFunctionAddress( _sohandle         ,
                                                          "DSTOREPLUG_Type" ,
                                                          1*sizeof(void*)   ).funcPtr;
        _fptable[ DSTOREPLUG_DEST_FILE_OPEN ] = GetFunctionAddress( _sohandle                   ,
                                                                    "DSTOREPLUG_Dest_File_Open" ,
                                                                    3*sizeof(void*)             ).funcPtr;
        _fptable[ DSTOREPLUG_DEST_FILE_CLOSE ] = GetFunctionAddress( _sohandle                    ,
                                                                     "DSTOREPLUG_Dest_File_Close" ,
                                                                     2*sizeof(void*)              ).funcPtr;
        _fptable[ DSTOREPLUG_BEGIN_NODE_STORE ] = GetFunctionAddress( _sohandle                    ,
                                                                     "DSTOREPLUG_Begin_Node_Store" ,
                                                                     3*sizeof(void*)+12            ).funcPtr;
        _fptable[ DSTOREPLUG_END_NODE_STORE ] = GetFunctionAddress( _sohandle                   ,
                                                                    "DSTOREPLUG_End_Node_Store" ,
                                                                    3*sizeof(void*)+8           ).funcPtr;
        _fptable[ DSTOREPLUG_STORE_NODE_ATT ] = GetFunctionAddress( _sohandle                   ,
                                                                    "DSTOREPLUG_Store_Node_Att" ,
                                                                    5*sizeof(void*)+12          ).funcPtr;
        _fptable[ DSTOREPLUG_BEGIN_NODE_CHILDREN ] = GetFunctionAddress( _sohandle                        ,
                                                                         "DSTOREPLUG_Begin_Node_Children" ,
                                                                         3*sizeof(void*)                  ).funcPtr;
        _fptable[ DSTOREPLUG_END_NODE_CHILDREN ] = GetFunctionAddress( _sohandle                      ,
                                                                       "DSTOREPLUG_End_Node_Children" ,
                                                                       3*sizeof(void*)                ).funcPtr;
        _fptable[ DSTOREPLUG_SRC_FILE_OPEN ] = GetFunctionAddress( _sohandle                  ,
                                                                   "DSTOREPLUG_Src_File_Open" ,
                                                                   2*sizeof(void*)            ).funcPtr;
        _fptable[ DSTOREPLUG_SRC_FILE_CLOSE ] = GetFunctionAddress( _sohandle                   ,
                                                                    "DSTOREPLUG_Src_File_Close" ,
                                                                    2*sizeof(void*)             ).funcPtr;
        _fptable[ DSTOREPLUG_SET_READ_HANDLERS ] = GetFunctionAddress( _sohandle                      ,
                                                                       "DSTOREPLUG_Set_Read_Handlers" ,
                                                                       4*sizeof(void*)                ).funcPtr;
        _fptable[ DSTOREPLUG_START_READING ] = GetFunctionAddress( _sohandle                  ,
                                                                   "DSTOREPLUG_Start_Reading" ,
                                                                   2*sizeof(void*)            ).funcPtr;


        if ( ( _fptable[ DSTOREPLUG_INIT ] == NULL ) ||
             ( _fptable[ DSTOREPLUG_SHUTDOWN ] == NULL ) ||
             ( _fptable[ DSTOREPLUG_NAME ] == NULL ) ||
             ( _fptable[ DSTOREPLUG_COPYRIGHT ] == NULL ) ||
             ( _fptable[ DSTOREPLUG_VERSION ] == NULL ) ||
             ( _fptable[ DSTOREPLUG_TYPE ] == NULL ) ||
             ( _fptable[ DSTOREPLUG_DEST_FILE_OPEN ] == NULL ) ||
             ( _fptable[ DSTOREPLUG_DEST_FILE_CLOSE ] == NULL ) ||
             ( _fptable[ DSTOREPLUG_BEGIN_NODE_STORE ] == NULL ) ||
             ( _fptable[ DSTOREPLUG_END_NODE_STORE ] == NULL ) ||
             ( _fptable[ DSTOREPLUG_STORE_NODE_ATT ] == NULL ) ||
             ( _fptable[ DSTOREPLUG_BEGIN_NODE_CHILDREN ] == NULL ) ||
             ( _fptable[ DSTOREPLUG_END_NODE_CHILDREN ] == NULL ) ||
             ( _fptable[ DSTOREPLUG_SRC_FILE_OPEN ] == NULL ) ||
             ( _fptable[ DSTOREPLUG_SRC_FILE_CLOSE ] == NULL ) ||
             ( _fptable[ DSTOREPLUG_SET_READ_HANDLERS ] == NULL ) ||
             ( _fptable[ DSTOREPLUG_START_READING ] == NULL ) )
        {
                memset( _fptable, 0, sizeof(anyPointer) * DSTOREPLUG_LASTFPTR );
                _sohandle = NULL;

                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "Invalid codec module: One or more functions could not be located in the module " + PointerToString( modulePtr ) );
                return false;
        }

        /*
         *      Intialize the plugin module
         */
        UInt32 statusCode = ( (TDSTOREPLUGFPTR_Init) _fptable[ DSTOREPLUG_INIT ] )( &_plugdata );
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
            memset( _fptable, 0, sizeof(anyPointer) * DSTOREPLUG_LASTFPTR );
            _sohandle = NULL;
            _plugdata = NULL;

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
        ( (TDSTOREPLUGFPTR_Shutdown) _fptable[ DSTOREPLUG_SHUTDOWN ] )( &_plugdata );

        // Cleanup recources
        memset( _fptable, 0, sizeof(anyPointer) * DSTOREPLUG_LASTFPTR );
        m_metaData.Unlink();
        _sohandle = NULL;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

CDStoreCodecPlugin::CDStoreCodecPlugin( void )
    : CDStoreCodec()    ,
      CIPlugin()        ,
      _id( 0 )          ,
      _sohandle( NULL ) ,
      _plugdata( NULL ) ,
      m_metaData()
{GUCEF_TRACE;

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

    return _sohandle != NULL;
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
    ((TDSTOREPLUGFPTR_Begin_Node_Store)_fptable[ DSTOREPLUG_BEGIN_NODE_STORE ] )( &_plugdata         ,
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
            ((TDSTOREPLUGFPTR_Store_Node_Att)_fptable[ DSTOREPLUG_STORE_NODE_ATT ])( &_plugdata                 ,
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
        ((TDSTOREPLUGFPTR_Store_Node_Att)_fptable[ DSTOREPLUG_STORE_NODE_ATT ])( &_plugdata                   ,
                                                                                 filedata                     ,
                                                                                 name                         ,
                                                                                 count + valueAsAtt           ,
                                                                                 count                        ,
                                                                                 GUCEF_NULL                   ,
                                                                                 n->GetValue().CStyleAccess() ,
                                                                                 n->HasChildren()             );
    }

    /*
     *      If the node has any children we will store them and their children ect.
     */
    if ( n->HasChildren() )
    {
        ((TDSTOREPLUGFPTR_Begin_Node_Children)_fptable[ DSTOREPLUG_BEGIN_NODE_CHILDREN ])( &_plugdata ,
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

        ((TDSTOREPLUGFPTR_End_Node_Children)_fptable[ DSTOREPLUG_END_NODE_CHILDREN ])( &_plugdata ,
                                                                                       filedata   ,
                                                                                       name       );
    }

    /*
     *      We are finished with this node
     */
    ((TDSTOREPLUGFPTR_End_Node_Store)_fptable[ DSTOREPLUG_END_NODE_STORE ])( &_plugdata       ,
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
    if ( !((TDSTOREPLUGFPTR_Dest_File_Open)_fptable[ DSTOREPLUG_DEST_FILE_OPEN ])( &_plugdata, &filedata, file->CStyleAccess() ) )
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
    ((TDSTOREPLUGFPTR_Dest_File_Close)_fptable[ DSTOREPLUG_DEST_FILE_CLOSE ])( &_plugdata, &filedata );
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
         *      Open the source file
         */
        void* filedata;
        if ( !((TDSTOREPLUGFPTR_Src_File_Open)_fptable[ DSTOREPLUG_SRC_FILE_OPEN ])( &_plugdata, &filedata, file->CStyleAccess() ) )
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
        ((TDSTOREPLUGFPTR_Set_Read_Handlers)_fptable[ DSTOREPLUG_SET_READ_HANDLERS ])( &_plugdata, &filedata, &rhandlers, privdata );

        /*
         *      So far so good,.. delete whatever children the treeroot may alreay have
         *      and start reading the data. This should cause the handlers to be called when
         *      needed and they in turn construct our tree based on the events.
         */
        treeroot->DelSubTree();
        UInt32 errorCode = ((TDSTOREPLUGFPTR_Start_Reading)_fptable[ DSTOREPLUG_START_READING ])( &_plugdata, &filedata );
        GUCEF_DELETE static_cast<TParserData*>(privdata);

        /*
         *      We are finished,.. close the file
         */
        ((TDSTOREPLUGFPTR_Src_File_Close)_fptable[ DSTOREPLUG_SRC_FILE_CLOSE ])( &_plugdata, &filedata );
        return 0 == errorCode;
}

/*-------------------------------------------------------------------------*/

CString
CDStoreCodecPlugin::GetTypeName( void ) const
{GUCEF_TRACE;
        return ((TDSTOREPLUGFPTR_Type)_fptable[ DSTOREPLUG_TYPE ])( _plugdata );
}

/*-------------------------------------------------------------------------*/

CString
CDStoreCodecPlugin::GetName( void ) const
{GUCEF_TRACE;
        return ((TDSTOREPLUGFPTR_Name)_fptable[ DSTOREPLUG_NAME ])( _plugdata );
}

/*-------------------------------------------------------------------------*/

CString
CDStoreCodecPlugin::GetCopyright( void ) const
{GUCEF_TRACE;
        return ((TDSTOREPLUGFPTR_Copyright)_fptable[ DSTOREPLUG_COPYRIGHT ])( _plugdata );
}

/*-------------------------------------------------------------------------*/

TVersion
CDStoreCodecPlugin::GetVersion( void ) const
{GUCEF_TRACE;

    return *((TDSTOREPLUGFPTR_Version)_fptable[ DSTOREPLUG_VERSION ])( _plugdata );
}

/*-------------------------------------------------------------------------*/

CICloneable*
CDStoreCodecPlugin::Clone( void ) const
{GUCEF_TRACE;

    return NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
