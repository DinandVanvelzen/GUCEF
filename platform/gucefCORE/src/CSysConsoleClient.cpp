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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CSYSCONSOLE_H
#include "CSysConsole.h"
#define GUCEF_CORE_CSYSCONSOLE_H
#endif /* GUCEF_CORE_CSYSCONSOLE_H ? */

#include "CSysConsoleClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CSysConsoleClient::CSysConsoleClient( void )
        : _system( &CORE::CCoreGlobal::Instance()->GetSysConsole() )
{GUCEF_TRACE;

        _system->InitClient( this );
}

/*-------------------------------------------------------------------------*/

CSysConsoleClient::~CSysConsoleClient()
{GUCEF_TRACE;

        _system->UnregClient( this );
}

/*-------------------------------------------------------------------------*/

bool
CSysConsoleClient::RegisterCmd( const CString& path                ,
                                const CString& command             ,
                                const CString::StringVector& args  ,
                                CISysConsoleCmdHandler* cmdhandler )
{GUCEF_TRACE;

        return _system->RegisterCmd( path       ,
                                     command    ,
                                     args       ,
                                     cmdhandler );
}

/*-------------------------------------------------------------------------*/

void
CSysConsoleClient::UnregisterCmd( const CString& path    ,
                                  const CString& command )
{GUCEF_TRACE;

        _system->UnregisterCmd( path    ,
                                command );
}

/*-------------------------------------------------------------------------*/

void
CSysConsoleClient::LeaveDir( void )
{GUCEF_TRACE;

        _system->LeaveDir( this );
}

/*-------------------------------------------------------------------------*/

bool
CSysConsoleClient::EnterDir( const CString& dirname )
{GUCEF_TRACE;

        return _system->EnterDir( this, dirname );
}

/*-------------------------------------------------------------------------*/

bool
CSysConsoleClient::JumpTo( const CString& path )
{GUCEF_TRACE;

        return _system->JumpTo( this ,
                                path );
}

/*-------------------------------------------------------------------------*/

const CString&
CSysConsoleClient::GetPath( void ) const
{GUCEF_TRACE;

        return _path;
}

/*-------------------------------------------------------------------------*/

bool
CSysConsoleClient::Execute( const CString& functionname           ,
                            const CString::StringVector& arglist  ,
                            CString::StringVector& resultdata     )
{GUCEF_TRACE;

        return _system->Execute( this         ,
                                 functionname ,
                                 arglist      ,
                                 resultdata   );
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CSysConsoleClient::GetDirList( void ) const
{GUCEF_TRACE;

        return _system->GetDirList( this );
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CSysConsoleClient::GetCmdList( void ) const
{GUCEF_TRACE;

        return _system->GetCmdList( this );
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CSysConsoleClient::GetCompletions( const CString& partialName ) const
{GUCEF_TRACE;

    return _system->GetCompletions( this, partialName );
}

/*-------------------------------------------------------------------------*/

bool
CSysConsoleClient::SplitCommandLine( const CString& userInput ,
                                     TStringVector& words     )
{GUCEF_TRACE;

    words.clear();
    CString word;
    bool inWord = false;
    bool inQuotes = false;
    const char* text = userInput.C_String();
    const UInt32 byteCount = GUCEF_NULL != text ? userInput.ByteSize() : 0;
    for ( UInt32 i=0; i<byteCount && '\0' != text[ i ]; ++i )
    {
        const char c = text[ i ];
        if ( inQuotes )
        {
            if ( '\\' == c && i+1 < byteCount && ( '"' == text[ i+1 ] || '\\' == text[ i+1 ] ) )
            {
                word += text[ ++i ];
            }
            else
            if ( '"' == c )
            {
                inQuotes = false;
            }
            else
            {
                word += c;
            }
            continue;
        }
        if ( ' ' == c || '\t' == c || '\r' == c || '\n' == c )
        {
            if ( inWord )
            {
                words.push_back( word );
                word.Clear();
                inWord = false;
            }
            continue;
        }
        inWord = true;
        if ( '"' == c )
            inQuotes = true;
        else
            word += c;
    }
    if ( inWord )
        words.push_back( word );
    return !inQuotes;
}

/*-------------------------------------------------------------------------*/

bool
CSysConsoleClient::ProcessUserInput( const CString& userInput  ,
                                     TStringVector& resultdata )
{GUCEF_TRACE;

    TStringVector words;
    if ( !SplitCommandLine( userInput, words ) )
    {
        resultdata.push_back( "ERROR> A quote is not closed" );
        return false;
    }
    if ( words.empty() )
        return false;

    // the commands of the console itself: moving through the dirs and listing them
    const CString command = words[ 0 ];
    if ( "cd" == command )
    {
        if ( words.size() < 2 )
        {
            resultdata.push_back( '\\' + GetPath() );
            return true;
        }
        if ( !JumpTo( words[ 1 ] ) )
        {
            resultdata.push_back( "ERROR> There is no dir \"" + words[ 1 ] + "\"" );
            return false;
        }
        return true;
    }
    if ( "pwd" == command )
    {
        resultdata.push_back( '\\' + GetPath() );
        return true;
    }
    if ( "dirs" == command )
    {
        resultdata = GetDirList();
        return true;
    }
    if ( "cmds" == command )
    {
        resultdata = GetCmdList();
        return true;
    }

    words.erase( words.begin() );
    return Execute( command    ,
                    words      ,
                    resultdata );
}

/*-------------------------------------------------------------------------*/

void
CSysConsoleClient::OnReturnData( const TStringVector& data )
{GUCEF_TRACE;

    /* dummy, to avoid mandatory implementation for descending classes */
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

