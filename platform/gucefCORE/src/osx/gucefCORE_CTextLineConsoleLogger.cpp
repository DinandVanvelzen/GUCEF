/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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
 *  macOS writes to a terminal and to a standard stream the way Linux does (POSIX file descriptors), so the Linux
 *  implementation serves both. The build picks the files of a platform by the name of their directory, which is why
 *  this file is here and only includes the Linux one.
 */

#include "../linux/gucefCORE_CTextLineConsoleLogger.cpp"
