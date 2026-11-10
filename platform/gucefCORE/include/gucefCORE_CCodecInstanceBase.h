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

#ifndef GUCEF_CORE_CCODECINSTANCEBASE_H
#define GUCEF_CORE_CCODECINSTANCEBASE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICODECINSTANCE_H
#include "gucefCORE_CICodecInstance.h"
#define GUCEF_CORE_CICODECINSTANCE_H
#endif /* GUCEF_CORE_CICODECINSTANCE_H ? */

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

/**
 *  Base for instances: keeps the status and error text of the last call in a fixed buffer, so reporting
 *  an outcome never allocates.
 */
class GUCEF_CORE_PUBLIC_CPP CCodecInstanceBase : public CICodecInstance
{
    public:

    CCodecInstanceBase( void );

    virtual ~CCodecInstanceBase() GUCEF_VIRTUAL_OVERRIDE;

    virtual TCodecStatus GetLastStatus( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** Records the outcome, the message is copied (and cut to the buffer). Returns whether the status is OK */
    bool SetStatus( TCodecStatus status, const char* message = GUCEF_NULL );

    /** Records OK, returns true */
    bool SetOk( void );

    /**
     *  Checks that exactly one of the encode and decode flags is set and that the codec does that direction.
     *  Records the failure when not.
     */
    bool CheckDirection( UInt32 flags, bool canEncode, bool canDecode, bool& encode );

    /**
     *  Makes room for 'size' more bytes after the data of 'output' and gives where they go in 'out' (NULL for no bytes
     *  in a buffer without memory). False, recorded, when there is no room to be had
     */
    bool ReserveOutput( CDynamicBuffer& output, UInt32 size, UInt8*& out );

    private:

    TCodecStatus m_lastStatus;
    char m_lastErrorMessage[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CCODECINSTANCEBASE_H ? */
