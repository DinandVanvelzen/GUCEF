/*
 *  pg_config_paths.h: DV added, a hand written equivalent of the header the build of PostgreSQL 18.7 writes in
 *  src/port from the installation directories it is configured with. These are the defaults of an upstream build
 *  (prefix /usr/local/pgsql). libpq uses SYSCONFDIR to find pg_service.conf when the environment variable
 *  PGSYSCONFDIR is not set; the other directories are not used by libpq.
 */

#ifndef GUCEF_LIBPQ_PG_CONFIG_PATHS_H
#define GUCEF_LIBPQ_PG_CONFIG_PATHS_H

#define PGBINDIR "/usr/local/pgsql/bin"
#define PGSHAREDIR "/usr/local/pgsql/share"
#define SYSCONFDIR "/usr/local/pgsql/etc"
#define INCLUDEDIR "/usr/local/pgsql/include"
#define PKGINCLUDEDIR "/usr/local/pgsql/include"
#define INCLUDEDIRSERVER "/usr/local/pgsql/include/server"
#define LIBDIR "/usr/local/pgsql/lib"
#define PKGLIBDIR "/usr/local/pgsql/lib"
#define LOCALEDIR "/usr/local/pgsql/share/locale"
#define DOCDIR "/usr/local/pgsql/share/doc"
#define HTMLDIR "/usr/local/pgsql/share/doc"
#define MANDIR "/usr/local/pgsql/share/man"

#endif /* GUCEF_LIBPQ_PG_CONFIG_PATHS_H ? */
