/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_PROCESSARGUMENTS_H
#define GUCEF_CORE_PROCESSARGUMENTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

class CValueList;

/*-------------------------------------------------------------------------*/

/**
 *  The arguments the process was started with, without the program path, split the way the C runtime of the
 *  platform splits a command line. For applications with an option grammar of their own (--option value) that have
 *  to see the same arguments on every platform, whatever the entry point macros of gucef_osmain.h hand to main().
 *
 *  POSIX: argv[ 1 .. argc-1 ] as they are.
 *  Windows: the command line of the process (GetCommandLineW(), which in service mode is the command line of the
 *  service's image path) split by CommandLineToArgvW(), converted to UTF-8, the program path dropped. 'argc' and
 *  'argv' are not used there: the GUCEF_OSMAIN_BEGIN macros give a WinMain() application the whole argument string
 *  as one argv entry.
 *
 *  @return false when the arguments could not be had (Windows: CommandLineToArgvW() failed); 'arguments' is empty then
 */
GUCEF_CORE_PUBLIC_CPP bool
GetProcessArguments( int argc                         ,
                     char** argv                      ,
                     CString::StringVector& arguments );

/*-------------------------------------------------------------------------*/

/**
 *  The arguments of a GUCEF tool in the key / value form the tools have always used ("configPath=/x.json"),
 *  added to 'keyValueList':
 *  - the program path is skipped: argv[ 0 ] on POSIX; nothing on Windows, where the GUCEF_OSMAIN_BEGIN macros hand
 *    the whole argument string over as argv[ 0 ];
 *  - every argument is split on 'pairSeparator' and each non-empty part is read as key=value: the key is the text
 *    before the first '=', the value everything after it (so a value may hold '='); a part without '=' is a key with
 *    an empty value;
 *  - options of the form --name (such as --console, which the entry point macros read) are no key / value pairs:
 *    at the start of a part they are skipped up to the next space or tab, also when several follow each other
 *    (on Windows "--console configPath=x" is one argument string and gives "configPath").
 *  Whitespace around a part is not kept.
 *
 *  POSIX "tool configPath=/x.json", "tool '*configPath=/x.json'" and "tool a=1*b=2" all work, and so does the
 *  Windows "tool.exe configPath=C:\x.json*bootstrapConfigPath=C:\y.ini".
 */
GUCEF_CORE_PUBLIC_CPP void
ParseApplicationArguments( int argc                  ,
                           char** argv               ,
                           CValueList& keyValueList  ,
                           char pairSeparator = '*'  );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_PROCESSARGUMENTS_H ? */
