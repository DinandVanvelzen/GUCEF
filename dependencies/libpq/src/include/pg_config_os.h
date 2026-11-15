/*
 *  pg_config_os.h: DV added, a hand written equivalent of what the configure step of PostgreSQL 18.7 makes of this
 *  header (a copy of, or link to, src/include/port/<template>.h for the platform it runs on). Here the preprocessor
 *  selects the port header of the platform.
 */

#ifndef GUCEF_LIBPQ_PG_CONFIG_OS_H
#define GUCEF_LIBPQ_PG_CONFIG_OS_H

#if defined( _WIN32 )
  #include "port/win32.h"
#elif defined( __APPLE__ )
  #include "port/darwin.h"
#elif defined( __linux__ )
  #include "port/linux.h"
#else
  #error "pg_config_os.h of libpq: this platform has no configuration yet"
#endif

#endif /* GUCEF_LIBPQ_PG_CONFIG_OS_H ? */
