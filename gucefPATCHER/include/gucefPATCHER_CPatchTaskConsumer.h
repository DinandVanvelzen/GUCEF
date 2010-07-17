/*
 *  gucefPATCHER: GUCEF RAD module providing a patch delivery system
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#ifndef GUCEF_PATCHER_CPATCHTASKCONSUMER_H
#define GUCEF_PATCHER_CPATCHTASKCONSUMER_H
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H
#include "gucefCORE_CBusyWaitPulseGeneratorDriver.h"
#define GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H
#endif /* GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H ? */

#ifndef GUCEF_CORE_CITASKCONSUMER_H
#include "gucefCORE_CITaskConsumer.h"
#define GUCEF_CORE_CITASKCONSUMER_H
#endif /* GUCEF_CORE_CITASKCONSUMER_H ? */

#ifndef GUCEF_PATCHER_CPATCHENGINE_H
#include "gucefPATCHER_CPatchEngine.h"
#define GUCEF_PATCHER_CPATCHENGINE_H
#endif /* GUCEF_PATCHER_CPATCHENGINE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PATCHER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_PATCHER_EXPORT_CPP CPatchTaskConsumer : public CORE::CTaskConsumer
{
    public:
    
    CPatchTaskConsumer( void );
    
    virtual ~CPatchTaskConsumer();
    
    const CString& GetTaskName( void ) const;

    virtual CString GetType( void ) const;
    
    static const CString& GetTypeString( void );
    
    virtual bool OnTaskStart( CORE::CICloneable* taskData );
    
    virtual bool OnTaskCycle( CORE::CICloneable* taskData );
    
    virtual void OnTaskEnd( CORE::CICloneable* taskData );
    
    CPatchEngine& GetPatchEngine( void );
    
    private:
    
    CPatchTaskConsumer( const CPatchTaskConsumer& src  );
    CPatchTaskConsumer& operator=( const CPatchTaskConsumer& src );

    private:
        
    CPatchEngine* m_patchEngine;
    CORE::CPulseGenerator m_pulseGenerator;
    CORE::CBusyWaitPulseGeneratorDriver m_pulseDriver;
    CString m_taskName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PATCHER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PATCHER_CPATCHTASKCONSUMER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 29-12-2006 :
        - Dinand: Initial version

----------------------------------------------------------------------------*/
