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
};
typedef struct CSysConsole::SCmdChannel TCmdChannel;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The names of every command in the tree */
void
CollectFunctionNames( const TCmdChannel* channel, GUCEF::set< CString >& names )
{GUCEF_TRACE;

    for ( size_t i=0; i<channel->functions.size(); ++i )
        names.insert( channel->functions[ i ]->name );
    for ( size_t i=0; i<channel->channels.size(); ++i )
        CollectFunctionNames( channel->channels[ i ], names );
}

/*-------------------------------------------------------------------------*/

/** Takes away channels without commands and sub channels, from the given one up to (not including) the root */
void
PruneEmptyChannels( TCmdChannel* channel )
{GUCEF_TRACE;

    while ( GUCEF_NULL != channel && GUCEF_NULL != channel->parent &&
            channel->functions.empty() && channel->channels.empty() )
    {
        TCmdChannel* parent = channel->parent;
        GUCEF::vector< TCmdChannel* >& siblings = parent->channels;
        for ( size_t i=0; i<siblings.size(); ++i )
        {
            if ( siblings[ i ] == channel )
            {
                siblings.erase( siblings.begin() + i );
                break;
            }
        }
        GUCEF_DELETE channel;
        channel = parent;
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CSysConsole::CSysConsole( void )
    : CGloballyConfigurable( false )
    , CISysConsoleCmdHandler()
    , m_root( GUCEF_NEW TCmdChannel() )
    , m_aliases()
    , m_datalock()
{GUCEF_TRACE;

    m_root->parent = GUCEF_NULL;

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
    DelTree( m_root );
    m_root = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CSysConsole::DelTree( TCmdChannel* tree )
{GUCEF_TRACE;

    if ( GUCEF_NULL == tree )
        return;
    for ( size_t i=0; i<tree->channels.size(); ++i )
        DelTree( tree->channels[ i ] );
    tree->channels.clear();
    for ( size_t i=0; i<tree->functions.size(); ++i )
        GUCEF_DELETE tree->functions[ i ];
    tree->functions.clear();
    GUCEF_DELETE tree;
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::ResolvePath( const CString& basePath         ,
                          const CString& path             ,
                          CString::StringVector& segments )
{GUCEF_TRACE;

    segments.clear();
    CString thePath = path.ReplaceChar( '/', '\\' );
    if ( thePath.IsNULLOrEmpty() || '\\' != thePath[ 0 ] )
        segments = basePath.ReplaceChar( '/', '\\' ).ParseElements( '\\', false );

    CString::StringVector elements = thePath.ParseElements( '\\', false );
    for ( size_t i=0; i<elements.size(); ++i )
    {
        const CString& element = elements[ i ];
        if ( "." == element )
            continue;
        if ( ".." == element )
        {
            if ( segments.empty() )
                return false;
            segments.pop_back();
            continue;
        }
        segments.push_back( element );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CSysConsole::JoinPath( const CString::StringVector& segments )
{GUCEF_TRACE;

    CString path;
    for ( size_t i=0; i<segments.size(); ++i )
    {
        if ( i > 0 )
            path += '\\';
        path += segments[ i ];
    }
    return path;
}

/*-------------------------------------------------------------------------*/

struct CSysConsole::SCmdChannel*
CSysConsole::FindChildChannel( const struct SCmdChannel* curchannel ,
                               const CString& name                  )
{GUCEF_TRACE;

    for ( size_t i=0; i<curchannel->channels.size(); ++i )
    {
        if ( curchannel->channels[ i ]->name == name )
            return curchannel->channels[ i ];
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

struct CSysConsole::SCmdChannel*
CSysConsole::FindChannel( const CString::StringVector& segments ) const
{GUCEF_TRACE;

    TCmdChannel* channel = m_root;
    for ( size_t i=0; i<segments.size() && GUCEF_NULL != channel; ++i )
        channel = FindChildChannel( channel, segments[ i ] );
    return channel;
}

/*-------------------------------------------------------------------------*/

struct CSysConsole::SCmdChannel*
CSysConsole::FindOrAddChannel( const CString::StringVector& segments )
{GUCEF_TRACE;

    TCmdChannel* channel = m_root;
    for ( size_t i=0; i<segments.size(); ++i )
    {
        TCmdChannel* child = FindChildChannel( channel, segments[ i ] );
        if ( GUCEF_NULL == child )
        {
            child = GUCEF_NEW TCmdChannel();
            child->name = segments[ i ];
            child->parent = channel;
            channel->channels.push_back( child );
        }
        channel = child;
    }
    return channel;
}

/*-------------------------------------------------------------------------*/

struct CSysConsole::SFunctionHook*
CSysConsole::FindFunction( const struct SCmdChannel* curchannel ,
                           const CString& funcname              )
{GUCEF_TRACE;

    for ( size_t i=0; i<curchannel->functions.size(); ++i )
    {
        if ( curchannel->functions[ i ]->name == funcname )
            return curchannel->functions[ i ];
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CSysConsole::FindFunctionPaths( const struct SCmdChannel* curchannel ,
                                const CString& curPath               ,
                                const CString& functionname          ,
                                CString::StringVector& paths         ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != FindFunction( curchannel, functionname ) )
        paths.push_back( curPath );
    for ( size_t i=0; i<curchannel->channels.size(); ++i )
    {
        const TCmdChannel* child = curchannel->channels[ i ];
        FindFunctionPaths( child, curPath.IsNULLOrEmpty() ? child->name : curPath + '\\' + child->name, functionname, paths );
    }
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::RegisterCmd( const CString& path                ,
                          const CString& functionname        ,
                          const CString::StringVector& args  ,
                          CISysConsoleCmdHandler* cmdhandler )
{GUCEF_TRACE;

    // a name is one word, without path separators
    if ( GUCEF_NULL == cmdhandler || functionname.IsNULLOrEmpty() ||
         functionname.HasChar( ' ', true ) >= 0 || functionname.HasChar( '\\', true ) >= 0 || functionname.HasChar( '/', true ) >= 0 )
        return false;

    CString::StringVector segments;
    if ( !ResolvePath( CString::Empty, path, segments ) )
        return false;

    MT::CScopeMutex lock( m_datalock );

    TCmdChannel* channel = FindOrAddChannel( segments );
    TFunctionHook* hook = FindFunction( channel, functionname );
    if ( GUCEF_NULL == hook )
    {
        hook = GUCEF_NEW TFunctionHook();
        hook->name = functionname;
        channel->functions.push_back( hook );
    }
    hook->argdefs = args;
    hook->handler = cmdhandler;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSysConsole::UnregisterCmd( const CString& path         ,
                            const CString& functionname )
{GUCEF_TRACE;

    CString::StringVector segments;
    if ( !ResolvePath( CString::Empty, path, segments ) )
        return;

    MT::CScopeMutex lock( m_datalock );

    TCmdChannel* channel = FindChannel( segments );
    if ( GUCEF_NULL == channel )
        return;
    for ( size_t i=0; i<channel->functions.size(); ++i )
    {
        if ( channel->functions[ i ]->name == functionname )
        {
            GUCEF_DELETE channel->functions[ i ];
            channel->functions.erase( channel->functions.begin() + i );
            break;
        }
    }
    PruneEmptyChannels( channel );
}

/*-------------------------------------------------------------------------*/

void
CSysConsole::RemoveHandler( struct SCmdChannel* curchannel        ,
                            const CISysConsoleCmdHandler* handler )
{GUCEF_TRACE;

    for ( size_t i=0; i<curchannel->functions.size(); )
    {
        if ( curchannel->functions[ i ]->handler == handler )
        {
            GUCEF_DELETE curchannel->functions[ i ];
            curchannel->functions.erase( curchannel->functions.begin() + i );
        }
        else
            ++i;
    }

    // the children first (bottom up), children left empty go
    for ( size_t i=0; i<curchannel->channels.size(); )
    {
        TCmdChannel* child = curchannel->channels[ i ];
        RemoveHandler( child, handler );
        if ( child->functions.empty() && child->channels.empty() )
        {
            curchannel->channels.erase( curchannel->channels.begin() + i );
            GUCEF_DELETE child;
        }
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CSysConsole::UnregisterCmdsOfHandler( CISysConsoleCmdHandler* cmdhandler )
{GUCEF_TRACE;

    if ( GUCEF_NULL == cmdhandler )
        return;

    MT::CScopeMutex lock( m_datalock );
    RemoveHandler( m_root, cmdhandler );
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::HasCmd( const CString& path    ,
                     const CString& command ) const
{GUCEF_TRACE;

    CString::StringVector segments;
    if ( !ResolvePath( CString::Empty, path, segments ) )
        return false;

    MT::CScopeMutex lock( m_datalock );
    const TCmdChannel* channel = FindChannel( segments );
    return GUCEF_NULL != channel && GUCEF_NULL != FindFunction( channel, command );
}

/*-------------------------------------------------------------------------*/

void
CSysConsole::InitClient( CSysConsoleClient* client )
{GUCEF_TRACE;

    // a client starts at the root
    client->_path.Clear();
}

/*-------------------------------------------------------------------------*/

void
CSysConsole::UnregClient( CSysConsoleClient* client )
{GUCEF_TRACE;

    // clients keep no data in the console
}

/*-------------------------------------------------------------------------*/

void
CSysConsole::LeaveDir( CSysConsoleClient* client )
{GUCEF_TRACE;

    JumpTo( client, ".." );
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::EnterDir( CSysConsoleClient* client ,
                       const CString& dirname    )
{GUCEF_TRACE;

    if ( dirname.IsNULLOrEmpty() || '\\' == dirname[ 0 ] || '/' == dirname[ 0 ] )
        return false;
    return JumpTo( client, dirname );
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::JumpTo( CSysConsoleClient* client ,
                     const CString& path       )
{GUCEF_TRACE;

    if ( GUCEF_NULL == client || path.IsNULLOrEmpty() )
        return false;

    CString::StringVector segments;
    if ( !ResolvePath( client->_path, path, segments ) )
        return false;

    MT::CScopeMutex lock( m_datalock );
    if ( GUCEF_NULL == FindChannel( segments ) )
        return false;
    client->_path = JoinPath( segments );
    return true;
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
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::Execute( CSysConsoleClient* client            ,
                      const CString& funcname              ,
                      const CString::StringVector& arglist ,
                      CString::StringVector& resultdata    )
{GUCEF_TRACE;

    if ( funcname.IsNULLOrEmpty() )
        return false;

    const CString clientPath = GUCEF_NULL != client ? client->_path : CString::Empty;
    CISysConsoleCmdHandler* handler = GUCEF_NULL;
    CString handlerPath;
    CString handlerFunction;
    {
        MT::CScopeMutex lock( m_datalock );

        // which command: an alias, a path to a command, a command of the current dir, or one by name anywhere
        const TFunctionHook* function = GUCEF_NULL;
        CString::StringVector segments;
        const TAliasData* alias = FindAliasFunction( funcname );
        if ( GUCEF_NULL != alias )
        {
            if ( ResolvePath( CString::Empty, alias->path, segments ) )
            {
                const TCmdChannel* channel = FindChannel( segments );
                if ( GUCEF_NULL != channel )
                    function = FindFunction( channel, alias->function );
            }
            if ( GUCEF_NULL == function )
            {
                resultdata.push_back( "ERROR> The alias \"" + funcname + "\" refers to \"" + alias->path + '\\' + alias->function + "\" which does not exist" );
                return false;
            }
        }
        else
        {
            CString name = funcname.ReplaceChar( '/', '\\' );
            Int32 lastSeparator = name.HasChar( '\\', false );
            if ( lastSeparator >= 0 )
            {
                // dir\name relative to the current dir, \dir\name and \name from the root
                const CString dirPart = 0 == lastSeparator ? CString( "\\" ) : name.SubstrFromRange( 0, (UInt32) lastSeparator );
                const CString functionPart = name.SubstrFromRange( (UInt32) lastSeparator + 1, name.Length() );
                if ( ResolvePath( clientPath, dirPart, segments ) )
                {
                    const TCmdChannel* channel = FindChannel( segments );
                    if ( GUCEF_NULL != channel )
                        function = FindFunction( channel, functionPart );
                }
            }
            else
            {
                if ( ResolvePath( clientPath, CString::Empty, segments ) )
                {
                    const TCmdChannel* channel = FindChannel( segments );
                    if ( GUCEF_NULL != channel )
                        function = FindFunction( channel, name );
                }
                if ( GUCEF_NULL == function )
                {
                    CString::StringVector paths;
                    FindFunctionPaths( m_root, CString::Empty, name, paths );
                    if ( paths.size() > 1 )
                    {
                        resultdata.push_back( "ERROR> \"" + name + "\" is the name of more than one command, give one of these:" );
                        for ( size_t i=0; i<paths.size(); ++i )
                            resultdata.push_back( "   \\" + paths[ i ] + '\\' + name );
                        return false;
                    }
                    if ( 1 == paths.size() && ResolvePath( CString::Empty, paths[ 0 ], segments ) )
                        function = FindFunction( FindChannel( segments ), name );
                }
            }
            if ( GUCEF_NULL == function )
            {
                resultdata.push_back( "ERROR> Unknown command \"" + funcname + "\"" );
                return false;
            }
        }

        // the values: as many as the command has, or at least all but the last when the last takes the rest ("...")
        const size_t defined = function->argdefs.size();
        const bool takesTheRest = defined > 0 && function->argdefs[ defined-1 ].HasSubstr( "...", false ) >= 0;
        const bool valuesFit = takesTheRest ? arglist.size() >= defined-1 : arglist.size() == defined;
        if ( !valuesFit )
        {
            resultdata.push_back( "ERROR> Expected " + ToString( (UInt32) defined ) + " values" );
            resultdata.push_back( ToString( (UInt32) arglist.size() ) + " values were entered" );
            if ( defined > 0 )
            {
                resultdata.push_back( "Expected values are:" );
                for ( size_t i=0; i<defined; ++i )
                    resultdata.push_back( "   " + function->argdefs[ i ] );
            }
            return false;
        }

        handler = function->handler;
        handlerPath = JoinPath( segments );
        handlerFunction = function->name;
    }

    // the handler runs outside of the lock: it may use the console itself
    return handler->OnSysConsoleCommand( handlerPath, handlerFunction, arglist, resultdata );
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CSysConsole::GetDirList( const CSysConsoleClient* client ) const
{GUCEF_TRACE;

    CString::StringVector list;
    CString::StringVector segments;
    if ( GUCEF_NULL == client || !ResolvePath( client->_path, CString::Empty, segments ) )
        return list;

    MT::CScopeMutex lock( m_datalock );
    const TCmdChannel* channel = FindChannel( segments );
    if ( GUCEF_NULL != channel )
        for ( size_t i=0; i<channel->channels.size(); ++i )
            list.push_back( channel->channels[ i ]->name );
    return list;
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CSysConsole::GetCmdList( const CSysConsoleClient* client ) const
{GUCEF_TRACE;

    CString::StringVector list;
    CString::StringVector segments;
    if ( GUCEF_NULL == client || !ResolvePath( client->_path, CString::Empty, segments ) )
        return list;

    MT::CScopeMutex lock( m_datalock );
    const TCmdChannel* channel = FindChannel( segments );
    if ( GUCEF_NULL != channel )
        for ( size_t i=0; i<channel->functions.size(); ++i )
            list.push_back( channel->functions[ i ]->name );
    return list;
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CSysConsole::GetCompletions( const CSysConsoleClient* client ,
                             const CString& partialName      ) const
{GUCEF_TRACE;

    // aliases, commands by name anywhere, and the dirs of the current dir, that start with what was typed
    GUCEF::set< CString > candidates;
    MT::CScopeMutex lock( m_datalock );

    for ( TAliasList::const_iterator i=m_aliases.begin(); i!=m_aliases.end(); ++i )
        candidates.insert( (*i).first );
    CollectFunctionNames( m_root, candidates );

    CString::StringVector segments;
    if ( GUCEF_NULL != client && ResolvePath( client->_path, CString::Empty, segments ) )
    {
        const TCmdChannel* channel = FindChannel( segments );
        if ( GUCEF_NULL != channel )
            for ( size_t i=0; i<channel->channels.size(); ++i )
                candidates.insert( channel->channels[ i ]->name + '\\' );
    }

    CString::StringVector matches;
    for ( GUCEF::set< CString >::const_iterator i=candidates.begin(); i!=candidates.end(); ++i )
        if ( partialName.IsNULLOrEmpty() || (*i).StartsWith( partialName ) )
            matches.push_back( (*i) );
    return matches;
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::RegisterAlias( const CString& aliasname ,
                            const CString& path      ,
                            const CString& function  )
{GUCEF_TRACE;

    if ( aliasname.IsNULLOrEmpty() || aliasname.HasChar( ' ', true ) >= 0 || function.IsNULLOrEmpty() )
        return false;

    MT::CScopeMutex lock( m_datalock );

    // an alias name is unique
    TAliasList::iterator i = m_aliases.find( aliasname );
    if ( i != m_aliases.end() )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "SysConsole: Failed to register alias \"" + aliasname + "\" for path \"" + path + "\" and function \"" + function + "\" because an alias with the given name already exists" );
        return false;
    }

    TAliasData ad;
    ad.function = function;
    ad.path = path.ReplaceChar( '/', '\\' );
    m_aliases[ aliasname ] = ad;

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "SysConsole: Registered alias \"" + aliasname + "\" for path \"" + path + "\" and function \"" + function + "\"" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSysConsole::UnregisterAlias( const CString& aliasname ,
                              const CString& path      ,
                              const CString& function  )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_datalock );

    TAliasList::iterator i = m_aliases.find( aliasname );
    if ( i != m_aliases.end() )
    {
        m_aliases.erase( i );
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "SysConsole: Unregistered alias \"" + aliasname + "\" for path \"" + path + "\" and function \"" + function + "\"" );
        return true;
    }

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "SysConsole: Failed to unregister alias \"" + aliasname + "\" because an alias with the given name cannot be found" );
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
        if ( args.size() >= 1 )
        {
            return UnregisterAlias( args[ 0 ]      ,
                                    CString::Empty ,
                                    CString::Empty );
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
