/*
 *  ProjectGen_TestApp: GUCEF test application for the ProjectGen module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 */

#ifndef GUCEF_PROJECTGEN_TESTAPP_TESTGENERATORFEATURES_H
#define GUCEF_PROJECTGEN_TESTAPP_TESTGENERATORFEATURES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests that modules can specify compiler flags, that processing instructions can exclude files with a wildcard
 *  and that the include dirs of a dependency which a module only has on certain platforms are only used on those platforms
 */
void
PerformGeneratorFeatureTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PROJECTGEN_TESTAPP_TESTGENERATORFEATURES_H ? */
