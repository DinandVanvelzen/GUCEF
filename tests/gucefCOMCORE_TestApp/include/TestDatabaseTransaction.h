/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_TESTDATABASETRANSACTION_H
#define GUCEF_COMCORE_TESTDATABASETRANSACTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Database transactions: the defaults of a backend without them (UNSUPPORTED, sync and async, and a scope that does
 *  not begin), and on a test backend with them the CDatabaseTransaction scope (commit keeps, the end of the scope and
 *  Rollback() undo, writes from other threads and async writes included), a second begin, calls without a
 *  transaction, the async transaction calls on a pool thread and the rollback of Close()
 */
void
PerformDatabaseTransactionTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTDATABASETRANSACTION_H ? */
