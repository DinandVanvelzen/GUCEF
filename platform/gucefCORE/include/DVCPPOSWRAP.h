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

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#define GUCEF_CORE_DVCPPOSWRAP_H

/*
 *      This file contains a set of O/S wrapping functions.
 *      The main idea is that you call these functions instead of the native
 *      O/S functions so that your code can become/stay portable.
 *      macro's are used to direct the function calls to the correct native
 *      O/S functions.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"        /* basic C versions of functions exposed here */
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

typedef std::vector< TProcessId, gucef_allocator< TProcessId > >    TProcessIdVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP UInt32
SetEnv( const CString& key   ,
        const CString& value );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP void
SetEnvOverride( const CString& key   ,
                const CString& value );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP void
SetEnvOverrides( const CString& allKeysAndValues );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
GetEnv( const CString& key );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool
GetExeNameForProcessId( TProcessId pid   ,
                        CString& exeName );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
GetHostname( void );

/*--------------------------------------------------------------------------*/

/**
 *  Attempts to determine if the specified process is running as a service
 *  For Windows this means that the software is running as a registered windows service
 *  For Linux this means that the software is running as a daemon
 */
GUCEF_CORE_PUBLIC_CPP bool 
IsProcessRunningAsService( TProcessId pid );

/*--------------------------------------------------------------------------*/

/**
 *  Attempts to determine if the software is running as a service
 *  For Windows this means that the software is running as a registered windows service
 *  For Linux this means that the software is running as a daemon
 */
GUCEF_CORE_PUBLIC_CPP bool 
IsRunningAsService( void );

/*--------------------------------------------------------------------------*/

/**
 *  Attempts to determine if the software is running as with administrative privileges
 *  Even if the user is a member of the administrators group, the software may not be running with
 *  elevated privileges. This is the case when UAC is enabled on Windows Vista and later for example.
 */
GUCEF_CORE_PUBLIC_CPP bool
IsRunningAsElevatedAdmin( void );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool
CommandLineExecute( const CString& command, CString& result, bool waitForExit );

/*--------------------------------------------------------------------------*/

/**
 *  Obtains a list of Process IDs for all the processes running on the system as visible to the
 *  account under which the software is executing
 */
GUCEF_CORE_PUBLIC_CPP bool
GetProcessList( TProcessIdVector& processList );

/*--------------------------------------------------------------------------*/


/**
 * @brief Attempts to check if the process with the given process Id is still alive
 * @param pid the process id
 * @param status output param indicating the 'alive' status of the process
 * @return whether we were able to obtain the alive status of the process
 */
GUCEF_CORE_PUBLIC_CPP bool
CheckOnProcessAliveStatus( TProcessId pid, bool& status );

/*--------------------------------------------------------------------------*/

class GUCEF_CORE_PUBLIC_CPP CProcessInformation
{
    public:

    CProcessInformation( void );

    ~CProcessInformation();

    void SetImagePath( const CString& str );

    CString& GetImagePath( void );

    void SetCommandLineArgs( const CString& str );

    CString& GetCommandLineArgs( void );

    const TProcessInformation& GetCStyleAccess( void ) const;

    void Clear( void );

    static bool TryGetProcessInformation( TProcessId pid            ,
                                          CProcessInformation& info );

    private:

    CString m_commandLineArgs;
    CString m_imagePath;
    TProcessInformation m_cStyleAccess;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */
