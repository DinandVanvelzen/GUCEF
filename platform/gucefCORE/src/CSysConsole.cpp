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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CISYSCONSOLECMDHANDLER_H
#include "CISysConsoleCmdHandler.h"
#define GUCEF_CORE_CISYSCONSOLECMDHANDLER_H
#endif /* GUCEF_CORE_CISYSCONSOLECMDHANDLER_H ? */

#ifndef GUCEF_CORE_CSYSCONSOLECLIENT_H
#include "CSysConsoleClient.h"
#define GUCEF_CORE_CSYSCONSOLECLIENT_H
#endif /* GUCEF_CORE_CSYSCONSOLECLIENT_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "CSysConsole.h"

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

struct CSysConsole::SFunctionHook
{
        CString name;
        CString::StringVector argdefs;
        CISysConsoleCmdHandler* handler;
};
typedef struct CSysConsole::SFunctionHook TFunctionHook;

/*-------------------------------------------------------------------------*/

struct CSysConsole::SCmdChannel
{
        CString name;
        GUCEF::vector< struct CSysConsole::SCmdChannel* > channels;
        GUCEF::vector< TFunctionHook* > functions;
        struct SCmdChannel* parent;
        UInt32 views;
};
typedef struct CSysConsole::SCmdChannel TCmdChannel;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CSysConsole::CSysConsole( void )
    : CGloballyConfigurable( false )  
    , CISysConsoleCmdHandler()
    , m_root( "root" )
    , m_aliases()
    , m_datalock()
{GUCEF_TRACE;

    /*
     *      The first functions are registered by the console
     *      itself. The functions allow you to register aliases
     *      using the console.
     */
    CString::StringVector args;
    args.push_back( "aliasname" );
    args.push_back( "path" );
    args.push_back( "function" );
    RegisterCmd( "GUCEF\\CORE\\CSysConsole" ,
                 "RegisterAlias"            ,
                 args                       ,
                 this                       );

    args.clear();
    args.push_back( "aliasname" );
    RegisterCmd( "GUCEF\\CORE\\CSysConsole" ,
                 "UnregisterAlias"          ,
                 args                       ,
                 this                       );
}

/*-------------------------------------------------------------------------*/

CSysConsole::~CSysConsole()
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_datalock );
    
    m_aliases.clear();
    m_root.Clear();
}

/*-------------------------------------------------------------------------*/

void
CSysConsole::DelTree( TCmdChannel* tree )
{GUCEF_TRACE;

        GUCEF::vector<TCmdChannel*>& array = tree->channels;
        TCmdChannel* chentry;
        for ( UInt32 i=0; i<array.size(); ++i )
        {
                chentry = array[ i ];
                DelTree( chentry );
                GUCEF_DELETE chentry;
        }
        array.clear();
        
        GUCEF::vector<TFunctionHook*>& farray = tree->functions;
        TFunctionHook* fhook = NULL;
        for ( UInt32 i=0; i<farray.size(); ++i )
        {
                fhook = farray[ i ];
                GUCEF_DELETE fhook;
        }
        farray.clear();

        GUCEF_DELETE tree; // is this safe ?
}

/*-------------------------------------------------------------------------*/

struct CSysConsole::SCmdChannel*
CSysConsole::FindChannel( struct SCmdChannel* curchannel ,
                          const CString& name            )
{GUCEF_TRACE;

    const GUCEF::vector< TCmdChannel* >& channels = curchannel->channels;
    TCmdChannel* chentry;
    for ( UInt32 i=0; i<channels.size(); ++i )
    {
        chentry = channels[ i ];
        if ( chentry->name == name )
        {
            return chentry;
        }
    }
    return NULL;
}

/*-------------------------------------------------------------------------*/

struct CSysConsole::SFunctionHook*
CSysConsole::FindFunction( const struct SCmdChannel* curchannel ,
                           const CString& funcname              )
{GUCEF_TRACE;

        const GUCEF::vector< TFunctionHook* >& commands = curchannel->functions;
        TFunctionHook* cmdentry = 0;
        for ( UInt32 i=0; i<commands.size(); ++i )
        {
                cmdentry = commands[ i ];
                if ( cmdentry->name == funcname )
                {
                        return cmdentry;
                }
        }
        return NULL;
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::RegisterCmd( const CString& path                ,
                          const CString& functionname        ,
                          const CString::StringVector& args  ,
                          CISysConsoleCmdHandler* cmdhandler )
{GUCEF_TRACE;

    // We dont allow spaces in function names
    if ( functionname.HasChar( ' ', true ) >= 0 )
        return false;

    MT::CScopeMutex lock( m_datalock );
    
    CString thepath( path.ReplaceChar( '/', '\\' ) );
    CDataNode* channel = m_root.Structure( thepath, '\\' );    
    if ( GUCEF_NULL == channel )
        return false;
    
    CDataNode* functionsRoot = channel->FindOrAddChild( "__functions" );
    if ( GUCEF_NULL == channel )
        return false;

    CDataNode* functionNode = functionsRoot->FindOrAddChild( functionname );
    if ( GUCEF_NULL == channel )
        return false;

    CDataNode* argsNode = functionNode->FindOrAddChild( "__args" );
    if ( GUCEF_NULL == channel )
        return false;

    CString::StringVector::const_iterator i = args.begin();
    while ( i != args.end() )
    {
        argsNode->FindOrAddChild( (*i) );
        ++i;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CSysConsole::UnregisterCmd( const CString& path         ,
                            const CString& functionname )
{GUCEF_TRACE;

    CString functionPath = CombinePath( path, "__functions" );
    functionPath = CombinePath( functionPath, functionname );

    m_root.DelSubTree( functionPath, '\\' );
}

/*-------------------------------------------------------------------------*/

void
CSysConsole::InitClient( CSysConsoleClient* client )
{GUCEF_TRACE;

        //MT::CScopeMutex scopeLock( _datalock );
        //client->channel = _root;
        //++_root->views;
}

/*-------------------------------------------------------------------------*/

void
CSysConsole::UnregClient( CSysConsoleClient* client )
{GUCEF_TRACE;

        //_datalock.Lock();
        //TCmdChannel* channel = static_cast< TCmdChannel* >( client->channel );

        ////++_root->views;

        //// makeme
        //_datalock.Unlock();
}

/*-------------------------------------------------------------------------*/

void
CSysConsole::LeaveDir( CSysConsoleClient* client )
{GUCEF_TRACE;

        //_datalock.Lock();
        //TCmdChannel* curchannel = static_cast< TCmdChannel* >( client->channel );
        //if ( curchannel->parent )
        //{
        //        client->channel = curchannel->parent;
        //        --static_cast< TCmdChannel* >( client->channel )->views;
        //}
        //_datalock.Unlock();
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::EnterDir( CSysConsoleClient* client ,
                       const CString& dirname    )
{GUCEF_TRACE;

        //_datalock.Lock();
        //TCmdChannel* channel = FindChannel( static_cast< TCmdChannel* >( client->channel ) ,
        //                                    dirname                                        );
        //if ( channel )
        //{
        //        client->channel = channel;
        //        client->_path += ( '\\' + dirname );

        //        ++channel->views;

        //        _datalock.Unlock();
        //        return true;
        //}
        //_datalock.Unlock();
        return false;
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::JumpTo( CSysConsoleClient* client ,
                     const CString& path       )
{GUCEF_TRACE;

    // sanity check on the input
    if ( client == NULL || path.Length() == 0 ) return false;

    //_datalock.Lock();

    //CString thePath( path );
    //struct SCmdChannel* channel = NULL;

    //// Check if we should jump from the root or from the client's current dir
    //if ( path[ 0 ] == '\\' )
    //{
    //    // we start jumping from the root
    //    channel = _root;

    //    // Set the client dir to root
    //    client->channel = channel;
    //    client->_path = "";

    //    if ( path.Length() == 1 )
    //    {
    //        client->channel = channel;
    //        _datalock.Unlock();
    //        return true;
    //    }
    //    else
    //    {
    //        thePath = thePath.CutChars( 1, true );
    //    }
    //}
    //else
    //{
    //    // we start jumping from the client's current dir
    //    channel = static_cast< TCmdChannel* >( client->channel );
    //}

    //CString clientPath;
    //CString dirname = thePath.SubstrToChar( '\\' );

    //// Loop trough all the directories in the path
    //while ( thePath.Length() > 0 )
    //{
    //    channel = FindChannel( channel ,
    //                           dirname );

    //    if ( channel != NULL )
    //    {
    //        if ( thePath.Length() > dirname.Length() )
    //        {
    //            // there are more sub dirs, prepare for another loop
    //            thePath = thePath.CutChars( dirname.Length()+1, true );
    //            dirname = thePath.SubstrToChar( '\\' );
    //            if ( clientPath.Length() > 0 )
    //            {
    //                clientPath += '\\' + dirname;
    //            }
    //            else
    //            {
    //                clientPath = dirname;
    //            }
    //        }
    //        else
    //        {
    //            // have have processed the last sub dir
    //            client->channel = channel;
    //            if ( clientPath.Length() > 0 )
    //            {
    //                client->_path += clientPath + '\\' + dirname;
    //            }
    //            else
    //            {
    //                if ( client->_path.Length() > 0 )
    //                {
    //                    client->_path += '\\' + dirname;
    //                }
    //                else
    //                {
    //                    client->_path = dirname;
    //                }
    //            }
    //            _datalock.Unlock();
    //            return true;
    //        }
    //    }
    //    else
    //    {
    //        // the path is invalid
    //        _datalock.Unlock();
    //        return false;
    //    }
    //}

    //// If we get here then the input was most likely invalid
    //_datalock.Unlock();
    return false;
}

/*-------------------------------------------------------------------------*/

CSysConsole::TAliasData*
CSysConsole::FindAliasFunction( const CString& aliasname )
{GUCEF_TRACE;

    TAliasList::iterator i = m_aliases.find( aliasname );
    if ( i != m_aliases.end() )
    {
        return &(*i).second;
    }
    return NULL;
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::Execute( CSysConsoleClient* client            ,
                      const CString& funcname              ,
                      const CString::StringVector& arglist ,
                      CString::StringVector& resultdata    )
{GUCEF_TRACE;

        //_datalock.Lock();

        ///*
        // *      First check the alias list
        // */
        //TAliasData* ad = FindAliasFunction( funcname );
        //if ( ad != NULL )
        //{
        //    // Jump to the path represented by the alias
        //    if ( JumpTo( client, ad->path ) )
        //    {
        //        // Execute the actual function based on the alias
        //        bool success = Execute( client       ,
        //                                ad->function ,
        //                                arglist      ,
        //                                resultdata   );
        //        _datalock.Lock();
        //        return success;
        //    }

        //    _datalock.Lock();
        //    return false;
        //}

        ///*
        // *      Now check the functions in the client's current namespace dir
        // */
        //TFunctionHook* func = FindFunction( static_cast< TCmdChannel* >( client->channel ) ,
        //                                    funcname                                       );

        //if ( func )
        //{
        //    if ( arglist.size() == func->argdefs.size() )
        //    {
        //        if ( func->handler->OnSysConsoleCommand( client->_path ,
        //                                                 funcname      ,
        //                                                 arglist       ,
        //                                                 resultdata    ) )
        //        {
        //            _datalock.Unlock();
        //            return true;
        //        }
        //    }
        //    else
        //    {
        //        resultdata.push_back( "ERROR> Expected " + UInt32ToString( (UInt32) func->argdefs.size() ) + " values" );
        //        resultdata.push_back( UInt32ToString( (UInt32) arglist.size() ) + " values were entered" );

        //        if ( func->argdefs.size() > 0 )
        //        {
        //            resultdata.push_back( "Expected values are:" );
        //            for ( UInt32 i=0; i<func->argdefs.size(); ++i )
        //            {
        //                resultdata.push_back( "   " + func->argdefs[ i ] );
        //            }
        //        }
        //    }
        //}
        //_datalock.Unlock();
        return false;
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CSysConsole::GetDirList( const CSysConsoleClient* client ) const
{GUCEF_TRACE;

    //MT::CScopeMutex scopeLock( _datalock );
    //GUCEF::vector< TCmdChannel* >& channels = static_cast< TCmdChannel* >( client->channel )->channels;
    CString::StringVector list;
    //for ( UInt32 i=0; i<channels.size(); ++i )
    //{
    //        list.push_back( channels[ i ]->name );
    //}
    return list;
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CSysConsole::GetCmdList( const CSysConsoleClient* client ) const
{GUCEF_TRACE;

    //MT::CScopeMutex scopeLock( _datalock );
    //GUCEF::vector< TFunctionHook* >& functions = static_cast< TCmdChannel* >( client->channel )->functions;
    CString::StringVector list;
    //for ( UInt32 i=0; i<functions.size(); ++i )
    //{
    //        list.push_back( functions[ i ]->name );
    //}
    return list;
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::RegisterAlias( const CString& aliasname ,
                            const CString& path      ,
                            const CString& function  )
{GUCEF_TRACE;

    //_datalock.Lock();

    //// First make sure the alias is unique and not already registered
    //TAliasList::iterator i = _aliases.find( aliasname );
    //if ( i != _aliases.end() )
    //{
    //    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "SysConsole: Failed to register alias \"" + aliasname + "\" for path \"" + path + "\" and function \"" + function + "\" because an alias with the given name already exists" );

    //    _datalock.Unlock();
    //    return false;
    //}

    //// if we get here then the alias does not exist yet and can be registered
    //TAliasData ad;
    //ad.function = function;
    //ad.path = path;

    //_aliases.insert( std::pair< CString, TAliasData >( aliasname, ad ) );

    //_datalock.Unlock();

    //GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "SysConsole: Registered alias \"" + aliasname + "\" for path \"" + path + "\" and function \"" + function + "\"" );
    //return true;


    return false;
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::UnregisterAlias( const CString& aliasname ,
                              const CString& path      ,
                              const CString& function  )
{GUCEF_TRACE;

    //_datalock.Lock();

    //// First make sure the alias is actually registered
    //TAliasList::iterator i = _aliases.find( aliasname );
    //if ( i != _aliases.end() )
    //{
    //    // delete the alias entry
    //    _aliases.erase( i );
    //    _datalock.Unlock();
    //    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "SysConsole: Unregistered alias \"" + aliasname + "\" for path \"" + path + "\" and function \"" + function + "\"" );
    //    return true;
    //}

    //// Unable to find the alias
    //_datalock.Unlock();
    //GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "SysConsole: Failed to unregister alias \"" + aliasname + "\" for path \"" + path + "\" and function \"" + function + "\" because an alias with the given name cannot be found" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::OnSysConsoleCommand( const CString& path               ,
                                  const CString& command            ,
                                  const CString::StringVector& args ,
                                  CString::StringVector& resultdata )
{GUCEF_TRACE;

    if ( command == "RegisterAlias" )
    {
        if ( args.size() >= 3 )
        {
            return RegisterAlias( args[ 0 ] ,
                                  args[ 1 ] ,
                                  args[ 2 ] );
        }
        return false;
    }
    if ( command == "UnregisterAlias" )
    {
        if ( args.size() >= 3 )
        {
            return UnregisterAlias( args[ 0 ] ,
                                    args[ 1 ] ,
                                    args[ 2 ] );
        }
        return false;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::SaveConfig( CDataNode& tree ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::LoadConfig( const CDataNode& treeroot )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "CSysConsole: Loading configuration" );

    const GUCEF::CORE::CDataNode* n = treeroot.Find( "ConsoleAliasList" );
    if ( n != NULL )
    {
        const GUCEF::CORE::CDataNode* m = GUCEF_NULL;
        GUCEF::CORE::CDataNode::const_iterator i = n->ConstBegin();
        while ( i != n->ConstEnd() )
        {
            m = (*i);
            if ( m->GetName() == "Alias" )
            {
                // Found an alias entry, parse values and add the alias
                const GUCEF::CORE::CDataNode::TKeyValuePair* att = m->GetAttribute( "name" );
                if ( att != NULL )
                {
                    CString aliasName = att->second;
                    att = m->GetAttribute( "path" );
                    if ( att != NULL )
                    {
                        CString path = att->second;
                        att = m->GetAttribute( "function" );
                        if ( att != NULL )
                        {
                            RegisterAlias( aliasName   ,
                                           path        ,
                                           att->second );
                        }
                    }
                }
            }
            ++i;
        }
    }

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "Sucessfully Loaded SysConsole configuration" );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CSysConsole::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::CORE::CSysConsole";
    return classTypeName; 
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
