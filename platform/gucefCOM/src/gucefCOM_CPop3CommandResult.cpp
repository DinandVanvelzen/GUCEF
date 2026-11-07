/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/


#include "gucefCOM_CPop3CommandResult.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPop3ListEntry::CPop3ListEntry( void )
    : number( 0 )
    , size( 0 )
    , uid()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3CommandResult::CPop3CommandResult( void )
    : CORE::CICloneable()
    , requestId( 0 )
    , command( POP3CMD_NOOP )
    , ok( false )
    , errorClass( MAILERR_NONE )
    , text()
    , number( 0 )
    , messageCount( 0 )
    , totalSize( 0 )
    , entries()
    , data()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3CommandResult::CPop3CommandResult( const CPop3CommandResult& src )
    : CORE::CICloneable( src )
    , requestId( src.requestId )
    , command( src.command )
    , ok( src.ok )
    , errorClass( src.errorClass )
    , text( src.text )
    , number( src.number )
    , messageCount( src.messageCount )
    , totalSize( src.totalSize )
    , entries( src.entries )
    , data( src.data )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3CommandResult::~CPop3CommandResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3CommandResult&
CPop3CommandResult::operator=( const CPop3CommandResult& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        requestId = src.requestId;
        command = src.command;
        ok = src.ok;
        errorClass = src.errorClass;
        text = src.text;
        number = src.number;
        messageCount = src.messageCount;
        totalSize = src.totalSize;
        entries = src.entries;
        data = src.data;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CPop3CommandResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CPop3CommandResult( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CPop3CommandResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CPop3CommandResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COM::CPop3CommandResult";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
