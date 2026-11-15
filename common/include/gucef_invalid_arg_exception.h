/*
 *  gucef common header: provides header based platform wide facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_INVALID_ARG_EXCEPTION_H
#define GUCEF_INVALID_ARG_EXCEPTION_H

#ifdef __cplusplus

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdexcept>
#include <string>

#ifndef GUCEF_CONFIG_H
#include "gucef_config.h"        /* GUCEF configuration */
#define GUCEF_CONFIG_H
#endif /* GUCEF_CONFIG_H ? */

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"      /* GUCEF platform compilation targets */
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#ifndef GUCEF_CPP_H
#include "gucef_cpp.h"       
#define GUCEF_CPP_H
#endif /* GUCEF_CPP_H ? */

#ifndef GUCEF_MEMORY_H
#include "gucef_memory.h"       
#define GUCEF_MEMORY_H
#endif /* GUCEF_MEMORY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class invalid_argument : public std::invalid_argument 
{
    private:

    int m_line;
    const char* m_file;

    public:

    explicit invalid_argument( void ) 
        : std::invalid_argument( "invalid_argument" ) 
        , m_line( 0 )
        , m_file( GUCEF_NULL )
    {GUCEF_TRACE; }

    explicit invalid_argument( const std::string& message ) 
        : std::invalid_argument( message.c_str() ) 
        , m_line( 0 )
        , m_file( GUCEF_NULL )
    {GUCEF_TRACE; }

    explicit invalid_argument( const char* message ) 
        : std::invalid_argument( message ) 
        , m_line( 0 )
        , m_file( GUCEF_NULL )
    {GUCEF_TRACE; }

    explicit invalid_argument( const char* file, int line, const char* message ) 
        : std::invalid_argument( message ) 
        , m_line( line )
        , m_file( file )
    {GUCEF_TRACE; }


    int line( void ) const {GUCEF_TRACE; return m_line; }
    const char* file( void ) const {GUCEF_TRACE; return m_file; }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* __cplusplus ? */

#endif /* GUCEF_INVALID_ARG_EXCEPTION_H ? */
