ooz_extract: external tool for Kraken family decompression
==========================================================

What it is
----------
A small stand alone command line program (module ooz_extract, executable ooz_extract) that decompresses data in the
Kraken family of formats (Kraken, Mermaid, Selkie, Leviathan; the old LZNA and BitKnit decoders of the same code
are included as well). Programs that need that data start it as a child process.

IT IS AN EXTERNAL TOOL. It is never linked into a GUCEF library or into any other program. The only interface is the
command line, files and standard input and output (plain bytes). No module has a dependency on this module. This is
how the GPL licence of the tool stays on the tool, and the licence of the programs that start it is unaffected.

Origin and licence
------------------
The decompressor is the code of "ooz" by Powzix: https://github.com/powzix/ooz
Upstream commit 05038060aa68f9187ae9923b2388ca8db40e58d1 (11 February 2019, "Hydra + Leviathan + Latest Kraken, new
name ooz, better cmdline opts"), obtained from a local checkout, nothing is downloaded at build time.

The licence is GNU General Public License version 3 or (at your option) any later version. The upstream repository
has no licence file; the licence is stated in the header of src/kraken.cpp ("Kraken Decompressor for Windows,
Copyright (C) 2016, Powzix", GPL v3 or later), and that header is kept unchanged. The other upstream files
(bitknit.cpp, lzna.cpp, stdafx.h, targetver.h) have no header of their own, they are part of the same program. The
full licence text is in COPYING (the text of GPL-3 as distributed with Debian and Ubuntu, /usr/share/common-licenses/GPL-3).
src/ooz_extract.cpp is the wrapper written for this code base, Copyright (C) 2026 Dinand Vanvelzen, licensed under
GPL-3.0-or-later as a derived work of the decompressor. Distributing the tool in binary form requires shipping
COPYING and a pointer to the source (this directory, and the upstream address above) with it.

GUCEF headers (header only): the tool includes seven headers of common/include of this code base: gucef_platform.h and
gucef_endian.h (platform and CPU macros, byte swaps) and what they include in this configuration: gucef_config.h,
gucef_types.h, gucef_memory.h, gucef_ScopeStackTracer.h and gucef_dynnewoff.h.
Only headers are used: no GUCEF library and no GUCEF source file is built or linked into the tool (call stack tracing and
the memory leak checker of GUCEF are not enabled, so the headers contribute only macros, types and inline functions).
gucef_config.h, gucef_types.h, gucef_memory.h, gucef_ScopeStackTracer.h and gucef_dynnewoff.h are Apache-2.0 headers; the
headers gucef_platform.h and gucef_endian.h still carry the older notice (LGPL 2.1 or later, relabelling to Apache-2.0
pending). Each keeps its own licence and notice. Apache-2.0 code can be combined into a GPL-3.0-or-later program, and LGPL
2.1 or later can be used under the GPL, so the combined tool (the binary) is GPL-3.0-or-later. Nothing changes for GUCEF itself:
no GUCEF module includes or links the tool.

Files
-----
COPYING                  GPL-3 text
ModuleInfo.xml           module description (executable, only the header location gucefHeaders as dependency, define OOZ_EXTRACT_NO_MAIN)
src/kraken.cpp           upstream decompressor (DV edit: the original command line part is left out of the build, 8 byte
                         copies through memcpy)
src/bitknit.cpp          upstream, unmodified
src/lzna.cpp             upstream, unmodified
src/stdafx.h             upstream (DV edit: non-Windows branch)
src/targetver.h          upstream, unmodified (Windows only)
src/ooz_extract.cpp      the command line wrapper (ours)
test/run_tests.sh        tests, see below
Not taken from upstream: stdafx.cpp (empty precompiled header source), the Visual Studio files (the build is
generated from ModuleInfo.xml), the test data (not needed), the original command line tool and its use of the
vendor library (the part of kraken.cpp after Kraken_Decompress, left out by OOZ_EXTRACT_NO_MAIN).

DV edit: changes to the upstream files (the decoding logic is untouched)
------------------------------------------------------------------------
1. src/stdafx.h: the platform and the CPU are detected with the macros of the GUCEF platform header (GUCEF_PLATFORM,
   GUCEF_CPU_ARCHITECTURE); on a CPU other than x86 and x86-64 the build stops with an #error. The original content is
   kept as the Windows branch (GUCEF_PLATFORM_MSWIN). A branch for other platforms was added that includes the system
   headers, defines the same type names and __forceinline, takes the byte swaps (_byteswap_ushort/ulong/uint64) from
   SwapEndianUInt16/32/64 of the GUCEF endian header, and defines the MSVC intrinsics GUCEF does not offer
   (_BitScanReverse, _BitScanForward, _rotl) with the GCC and Clang builtins. The bit scans store 0 in the index for an
   input of 0 (undefined with the MSVC intrinsics).
2. src/kraken.cpp: the part after Kraken_Decompress (command line parsing, the loader of the vendor library, main) is
   inside #ifndef OOZ_EXTRACT_NO_MAIN, so that the wrapper provides main. The module defines OOZ_EXTRACT_NO_MAIN.
3. ModuleInfo.xml adds -fno-strict-aliasing (the decoder reads through casted pointers), -fno-aggressive-loop-optimizations
   (the initialisation of the LZNA tables in lzna.cpp loops past a table that GCC can see) and -msse2 for GCC and Clang.
4. src/kraken.cpp: COPY_64 and the 8 byte loop of Kraken_CopyWholeMatch copy through memcpy instead of a uint64 pointer
   dereference. The pointers of a match are not 8 byte aligned and a match may overlap its own output at any distance
   of 8 or more. The dereference claims 8 byte alignment, and the loop vectorizer of GCC (seen with GCC 14 and 15 at -O3,
   the release build) then takes the distance of an overlapping copy as a multiple of 8: it copies 32 bytes per step
   where the distance is 25 to 31 bytes, so the match reads bytes that are not written yet and the output is corrupt
   without any error (debug builds at -O0 are not affected). memcpy of 8 bytes compiles to the same unaligned move and
   makes no alignment claim. The upstream commit named above has the dereference as well. The tests decode hand made LZ quanta with matches at such distances.
Known limits of upstream, unchanged: it needs an x86 or x86-64 CPU (SSE2 intrinsics), and it was written for trusted
input, see "Corrupt input" below.

Interface
---------
  ooz_extract --in FILE [--offset N] [--size N] --decompressed-size N --out FILE|- [--max-output N] [--timeout S]
  ooz_extract --batch [--max-output N] [--timeout S]

  --in FILE              file with the compressed bytes
  --offset N             byte offset of the compressed bytes in the file (default 0)
  --size N               number of compressed bytes (default: from the offset to the end of the file)
  --decompressed-size N  exact size of the decompressed data (required)
  --out FILE|-           output file, or - for standard output (single request only)
  --max-output N         largest decompressed size accepted, default 1073741824 (1 GiB), at most 2147418112; the
                         compressed size may be at most N + N / 16 + 65536
  --timeout S            seconds per request, 0 for none, default 120 (POSIX)

What the decoder needs: the compressed bytes of ONE Kraken stream, which is one block of 256 KiB or a whole stream of
several blocks, with the exact compressed size (a stream shorter or longer than --size is rejected), and the exact
decompressed size. The stream does not carry the decompressed size, the caller has to know it (for container formats
that cut data in blocks this is the block size, and the remainder for the last block). Every 256 KiB of output
starts with a block header in the stream; a caller can pass the blocks of a container one by one (each is a stream
of its own) or a contiguous multi block stream in one request.

Batch mode: one request per line on standard input, the same parameters (values with spaces in double quotes, a
backslash escapes a quote or backslash inside the quotes; --max-output and --timeout of the command line are the
defaults), empty lines skipped. For every request one line on standard output, flushed at once:
  ok <bytes>
  error <code> <message>
The tool stays alive until standard input is closed and then exits with 0. A failed request does not stop it.
Lines longer than 65536 bytes are answered with error 2. In batch mode --out - is an error (standard output carries
the answers). No output file is written or left behind for a failed request.

Single request: exit code 0 and "ok <bytes>" on standard output (not with --out -); on failure "error <code> <message>" on
standard error and the exit code.

Exit codes (also the code of the error lines)
  0  ok
  2  bad usage: missing, unknown or invalid parameter, a size of 0, a size over the maximum
  3  I/O error: input missing, unreadable or not a regular file, output not writable, out of memory
  4  corrupt or truncated input: the range is (partly) behind the end of the file, the stream is not valid, is shorter
     or longer than the sizes given, or the decoder crashed or timed out on it
  5  output size mismatch: the decoder reports a size other than the requested one (with this decoder a wrong
     decompressed size is normally found as code 4, because it demands that stream and sizes end together)

Bounds: all sizes are checked before anything is allocated, only the range of the input file that was asked for is
read, and the buffers are padded with zeroed bytes on both sides because the decoder reads and writes a little past
the ends of its buffers.

Corrupt input
-------------
The decoder of ooz is documented upstream as not safe for hostile input. Therefore on POSIX systems every request is decoded
in a forked child process with its own output memory; a crash or a hang (--timeout) is reported as exit code 4 and
does not affect the tool or the following requests. On Windows the decoder runs in the process of the tool. The tests
(below) run a loop of truncated and bit flipped inputs; no decoder crash was observed in them, so the child process
is a safety net that has not been needed in the tests.

Build and test
--------------
Generated build files: projects/CMake/targets/GUCEF_exe_ooz_extract (cmake -G Ninja -S projects/CMake/targets/GUCEF_exe_ooz_extract
-B common/bin/CBNINJA_ooz_exe_oz -DCMAKE_POLICY_VERSION_MINIMUM=3.5, then ninja -j2 -C common/bin/CBNINJA_ooz_exe_oz).
The program builds as C++98.
Tests: test/run_tests.sh [path of ooz_extract]. It needs bash, coreutils, xxd and grep. Synthetic tests build streams of stored and
memset quanta and hand made LZ quanta with overlapping matches (no data files needed): usage, limits, I/O errors,
truncated and invalid streams, the batch protocol with an interactive process (answers arrive per line), and a fuzz loop (OOZ_FUZZ_ITERATIONS, default 300) over
truncated and bit flipped streams, run under timeout. With OOZ_TEST_PACKAGE (and OOZ_TEST_ENTRY for an entry of a zip
archive with stored entries) it also searches a file for a compressed buffer header (a container with the magic
B7 75 63 62, method 3 (Oodle) and blocks of 256 KiB), decodes all its blocks through the batch
mode and checks block sizes, the total decompressed size, the start of the data and the equality with a single request,
then fuzzes a real block. The tests use a scratch directory (OOZ_TEST_SCRATCH, default ~/AssetTests/ooz/run) that
is deleted afterwards.
