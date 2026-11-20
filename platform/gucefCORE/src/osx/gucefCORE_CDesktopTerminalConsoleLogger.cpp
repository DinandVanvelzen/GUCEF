/*
 *  gucefCORE: GUCEF module providing O/S abstractions and basic framework
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

/*
 *  The log of a macOS application which was not started from a terminal is shown in a window of the Terminal application,
 *  by way of a FIFO, the same as on Linux. What differs is how the window is opened (AppleScript) and how it is established
 *  that there is a desktop session at all, which the implementation of the Linux directory has a branch for. The build picks
 *  the files of a platform by the name of their directory, which is why this file is here and only includes that one.
 */

#include "../../include/linux/gucefCORE_CDesktopTerminalConsoleLogger.h"
#define GUCEF_CORE_CDESKTOPTERMINALCONSOLELOGGER_H

#include "../linux/gucefCORE_CDesktopTerminalConsoleLogger.cpp"
