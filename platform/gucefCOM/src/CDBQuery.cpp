/*
 *  gucefCOM: GUCEF module providing communication implementations 
 *  for standardized protocols
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef DVOSWRAP_H
#include "DVOSWRAP.h"
#define DVOSWRAP_H
#endif /* DVOSWRAP_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H */

#include "CDBQuery.h"   /* definition of this class */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CDBQuery::CDBQuery(void)
{GUCEF_TRACE;
        column = GUCEF_NULL;
        field = GUCEF_NULL;

        columns = 0;
        rows = 0;
}

/*--------------------------------------------------------------------------*/

void
CDBQuery::Columns(UInt32 amount)
{GUCEF_TRACE;        
        columns = amount;

        column = GUCEF_NEW CDBColumn[amount];
}

/*--------------------------------------------------------------------------*/

void
CDBQuery::Rows(UInt32 amount)
{GUCEF_TRACE;
        rows = amount;

        field = GUCEF_NEW CDBField[rows * columns];
}

/*--------------------------------------------------------------------------*/

void
CDBQuery::FreeColumns(void)
{GUCEF_TRACE;

        if(column != GUCEF_NULL) {
                delete[] column;
                column = GUCEF_NULL;
        }

        columns = 0;
}

/*--------------------------------------------------------------------------*/

void
CDBQuery::FreeRows(void)
{GUCEF_TRACE;

        if(field != GUCEF_NULL) {
                delete[] field;
                field = GUCEF_NULL;
        }

        rows = 0;
}

/*--------------------------------------------------------------------------*/

void
CDBQuery::Wait_Untill_Done( UInt32 timeout )
{GUCEF_TRACE;

        UInt32 time = CORE::GUCEFGetTickCount();
        
        while ( ( state != CDB_CHANGED ) &&
                ( state != CDB_ERROR ) )
        {
                if ( timeout && ( CORE::GUCEFGetTickCount()-time > timeout ) )
                {
                        return;
                }
                MT::ThreadDelay( 10 );
        }

        #ifdef DEBUG_MODE
        switch ( state )
        {
                case CDB_CHANGED :
                {
                        CORE::tsprintf( "CDBQuery: Database data changed\n" );
                        return;
                }
                case CDB_ERROR :
                {
                        CORE::tsprintf( "CDBQuery: Error\n" );
                        return;
                }

        }
        #endif /* DEBUG_MODE */
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
