/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_CWEBVIEWSEMAPHOREENGINELOOP_H
#define GUCEF_WEB_CWEBVIEWSEMAPHOREENGINELOOP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSEMAPHORE_H
#include "gucefMT_CSemaphore.h"
#define GUCEF_MT_CSEMAPHORE_H
#endif /* GUCEF_MT_CSEMAPHORE_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_WEB_CIWEBVIEWENGINELOOP_H
#include "gucefWEB_CIWebViewEngineLoop.h"
#define GUCEF_WEB_CIWEBVIEWENGINELOOP_H
#endif /* GUCEF_WEB_CIWEBVIEWENGINELOOP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The loop of an engine that has no foreign loop of its own (the null engine, test engines, engines driven by
 *  callbacks on their own threads): RunOnce() sleeps on a semaphore that Wake() posts. One post at most is pending, so
 *  many wakes between two RunOnce() calls make one wake up.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewSemaphoreEngineLoop : public CIWebViewEngineLoop
{
    public:

    CWebViewSemaphoreEngineLoop( void );

    virtual ~CWebViewSemaphoreEngineLoop();

    virtual bool OnLoopStart( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void RunOnce( UInt32 maxWaitMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Wake( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnLoopStop( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** The number of times RunOnce() went to sleep, for tests of idle behaviour */
    UInt32 GetSleepCount( void ) const;

    private:

    MT::CSemaphore m_wake;
    MT::CTAtomic< UInt32 > m_posted;
    MT::CTAtomic< UInt32 > m_sleeps;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWSEMAPHOREENGINELOOP_H ? */
