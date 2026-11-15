# dbpluginPOSTGRESQL

GUCEF database backend "postgresql" for `COMCORE::CIDatabaseClient`: data tables as tables of a PostgreSQL database,
through libpq (module `dependencies/libpq`, PostgreSQL 18.7, linked in as a static library). General purpose: any GUCEF
application can keep its tables in PostgreSQL with it. Linux and Windows.

## Operating system packages

Package names are those of Debian and Ubuntu (apt).

Packages ending in `-dev` hold headers and link libraries. They are needed only to compile the plugin from source, in
every build configuration (debug and release), and not to run a built plugin. Running needs the run-time packages only.

| Needed for | Packages |
|---|---|
| Building, Linux | `libssl-dev` (OpenSSL 3 development headers; libpq uses OpenSSL for TLS) |
| Running, Linux | `libssl3t64` (`libssl3` before Ubuntu 24.04 and Debian 13) |
| Windows | none: libpq is built without TLS there |

No PostgreSQL package is needed: libpq is part of the code base. A PostgreSQL server (version 12 or later) is reached
over the network or a local socket.

### Example: installing the packages on Ubuntu

Build:

```bash
sudo apt install libssl-dev
```

Run time:

```bash
sudo apt install libssl3t64
```

## GUCEF modules needed at run time

* `dstorepluginJSONPARSER` (or another DStore codec registered as "json"): the definitions of the tables are kept as
  JSON in the table `_gucef_tables` of the schema. Without it the backend opens, but cannot create or open tables.

## Behaviour when something is missing

* Without the OpenSSL development headers the libpq module does not compile on Linux.
* On Windows, a server that requires TLS refuses the connection (`sslmode` other than `disable`, `allow` or `prefer`
  cannot be met).

## Settings

`CDatabaseClientSettings.backendName` is `postgresql`, `database` the name of the PostgreSQL database. The backend
options are documented in `include/dbpluginPOSTGRESQL_CPostgreSqlSettings.h`: `connectionString`, `host`, `port`,
`user`, `password`, `passwordFile`, `sslMode`, `applicationName`, `connectTimeout`, `schema`, `createSchema`,
`maxRowsPerStatement`. The password is never logged; prefer a password file or the environment of libpq (`PGPASSWORD`,
`PGPASSFILE`) for settings that are saved, since the settings are written as given.

## Tests

`tests/dbpluginPOSTGRESQL_TestApp`. The tests of the SQL, the type mapping, the parameters and the settings always run.
The tests against a server run when the environment variable `GUCEF_TEST_POSTGRESQL` holds a libpq connection string
of a test database (its role must be able to create a schema there); each run works in a schema of its own, which it
drops at the end. Without the variable those tests are skipped with a notice in the log.
