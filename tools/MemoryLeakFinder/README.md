# MemoryLeakFinder

A self-contained memory diagnostics library for GUCEF-based applications.
Tracks allocations, detects leaks, reports bounds violations, and profiles heap usage.
Works on **Windows** (x86/x64), **Linux** (x86/x64/ARM), and **Android** (via NDK).

---

## Features

| Category | Capability |
|---|---|
| **Leak detection** | Tracks every `new`/`malloc`/`calloc`/`realloc`/OLE alloc and reports live allocations at shutdown |
| **Bounds checking** | Sentinel padding on both sides of each allocation; VEH/SIGSEGV handler reports overflows at fault time |
| **Use-after-free** | Freed memory filled with `0xFEEDFACEDEADBEEF`; deallocation ring retains recently-freed records for cross-reference |
| **Type mismatch** | Detects `new` + `free`, `new[]` + `delete`, etc.; configurable response: log / debugger break / abort |
| **OS callstacks** | `RtlCaptureStackBackTrace` (Windows) / `backtrace()` (Linux) captured at alloc + dealloc; symbolicated at report time via DbgHelp / `backtrace_symbols` |
| **Logical callstacks** | `GUCEF_TRACE` push/pop frames interleaved with OS frames in reports |
| **Lock lifecycle** | Tracks exclusive lock create/obtain/release/abandon/destroy; reports abandonments and surplus releases at shutdown |
| **Lock range protection** | `MEMMAN_LockProtectsRange(lock, ptr, size)` — warns at dealloc time if a declared protecting lock is not held |
| **Guard pages** | Opt-in (`useGuardPages=true`): allocations backed by VirtualAlloc/mmap with a `PAGE_NOACCESS` guard page immediately after the buffer; overflows fault at access time |
| **Heap profiling** | Per-callsite allocation counts and live byte totals; size histogram; per-allocation timestamps |
| **Timeline export** | TSV file (timestampUs, size, file, line, type) for import into Excel / pandas / gnuplot |
| **Massif export** | Valgrind massif-format snapshot loadable in massif-visualizer or `ms_print` |
| **Global new override** | Optional: intercept all C++ `operator new/delete` in the process (opt-in, see below) |
| **CRT heap hook** | Optional (MSVC only): intercept `malloc`/`free` from third-party code sharing the same CRT instance |
| **Sanitizer-compatible** | Detects ASan / MSan / TSan at compile time and disables conflicting features automatically |

---

## Why In-Platform Diagnostics

External memory tools (Valgrind, Dr.Memory, Application Verifier) are powerful, but they introduce
a context switch: a separate tool, a separate report format, a separate invocation workflow, and
often a platform constraint (Valgrind is Linux-only; Dr.Memory is Windows-only). For teams building
on the **Galaxy Unlimited Platform**, MemoryLeakFinder eliminates that
friction entirely.

### Complementary, not duplicated

GUCEF already provides `GUCEF_TRACE` scope macros for logical callstack annotation. MemoryLeakFinder
does not replace them — it extends them. Every allocation report shows both the OS-level raw
frames (captured automatically) **and** the developer-annotated logical frames side by side. The
meaningful function names a developer already wrote for tracing become part of the memory report
with zero additional effort.

### Same observability pipeline

Memory reports flow through the same `GUCEF_LOG` / `GUCEF_ERROR_LOG` macros and logging
infrastructure that the rest of the application already uses. Log routing, output targets (file,
remote log service, console), and severity filtering are all inherited automatically. There is no
separate log file to find or correlate — memory diagnostics appear in the same stream as
application events.

### One tool, every platform

GUCEF targets **Windows**, **Linux**, and **Android (NDK)** from a single codebase. MemoryLeakFinder
matches that matrix exactly. Teams do not substitute Valgrind on Linux and Dr.Memory on Windows and
Application Verifier for COM; they use the same API, the same report format, and the same
configuration on every platform. CI pipelines, triage procedures, and institutional knowledge all
transfer directly.

### Reduced workflow churn

| Workflow step | External tool | MemoryLeakFinder (in-platform) |
|---|---|---|
| Launch | Separate tool invocation, re-link or wrap binary | `MEMMAN_Initialize()` already in the binary |
| Reproduce | Often requires specific OS / debug build | Same binary, same run, any platform |
| Report | Separate report file, different format per tool | Same `GUCEF_memlog.txt`, same format everywhere |
| Callstack | Raw addresses, external symbolication step | Symbolicated at report time, logical names included |
| Lock bugs | Separate Helgrind run | Lock lifecycle + range protection in the same report |
| Heap profile | Massif run + massif-visualizer install | `MEMMAN_DumpCallsiteReport()` / `DumpMassifFormat()` in-process |
| Sanitizer layering | Tool conflicts require separate runs | ASan / MSan / TSan detected at compile time; features auto-adjusted for same-run use |

### When to still reach for an external tool

MemoryLeakFinder is not a replacement for every diagnostic tool. The [Limitations](#limitations)
section and the [Tool Comparison](#tool-comparison) matrix are explicit about the gaps.
Use **MSan** or **Dr.Memory** for uninitialized memory reads; use **TSan** or **Helgrind** for
data race detection. MemoryLeakFinder is designed to handle everything in between — the daily
diagnostic workload — without leaving the platform.

---

## Tool Comparison

How MemoryLeakFinder compares to peer tools across the capability areas addressed by the 7 enhancement gaps.

Legend: ✅ Full support &nbsp; ⚠️ Partial / limited &nbsp; ❌ Not supported &nbsp; 🔧 Opt-in / configuration required

| Capability | MemoryLeakFinder | ASan | Valgrind (Memcheck) | Dr.Memory | TSan | Heaptrack |
|---|:---:|:---:|:---:|:---:|:---:|:---:|
| **Memory leak detection** | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ |
| **Heap profiling / callsite aggregation** | ✅ 🔧 | ❌ | ⚠️ (Massif only) | ❌ | ❌ | ✅ |
| **Size histogram** | ✅ | ❌ | ❌ | ❌ | ❌ | ⚠️ |
| **Timeline / TSV export** | ✅ | ❌ | ❌ | ❌ | ❌ | ✅ |
| **Massif-format export** | ✅ | ❌ | ✅ | ❌ | ❌ | ⚠️ (flamegraph) |
| **Bounds overflow — at checkpoint** | ✅ (sentinels) | ✅ | ✅ | ✅ | ❌ | ❌ |
| **Bounds overflow — at access time** | ✅ 🔧 (guard pages) | ✅ | ⚠️ (slow) | ✅ | ❌ | ❌ |
| **Use-after-free detection** | ✅ (poison + dealloc ring) | ✅ | ✅ | ✅ | ❌ | ❌ |
| **Uninitialized memory reads** | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ |
| **Alloc/dealloc type mismatch** | ✅ (log / break / abort) | ⚠️ | ✅ | ✅ | ❌ | ❌ |
| **OS-level callstacks at alloc** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Logical / annotated callstacks** | ✅ (GUCEF_TRACE) | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Lock lifecycle tracking** | ✅ (create/obtain/release/abandon/destroy) | ❌ | ❌ | ❌ | ⚠️ | ❌ |
| **Lock abandonment / surplus release** | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Lock range protection** | ✅ (contract validation at dealloc) | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Data race detection** | ❌ | ❌ | ✅ (Helgrind/DRD) | ❌ | ✅ | ❌ |
| **Third-party code coverage (C++)** | ✅ 🔧 (global new override) | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Third-party code coverage (C malloc)** | ✅ 🔧 (CRT hook, MSVC) | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Sanitizer compatibility** | ✅ (auto-detects ASan/MSan/TSan) | N/A | N/A | N/A | N/A | N/A |
| **Windows support** | ✅ | ⚠️ (limited) | ❌ | ✅ | ❌ | ❌ |
| **Linux / Android support** | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ |
| **In-process (no external tool needed)** | ✅ | ✅ (compiler) | ❌ | ❌ | ✅ (compiler) | ❌ |
| **Production-safe overhead** | ⚠️ (sentinel mode only) | ✅ (low) | ❌ (10–50×) | ❌ (10–50×) | ✅ (low) | ⚠️ |
| **OLE / BSTR allocation tracking** | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ |

### Key differentiators

- **Only in-process tool with OS callstacks on both Windows and Linux** that requires no compiler flags or external runner.
- **Only tool with logical (annotated) callstacks** alongside OS frames — useful when PDB/DWARF symbols are incomplete.
- **Only tool with lock lifecycle + lock range protection** — detects abandoned locks, surplus releases, and "free without holding the lock" bugs without requiring TSan's full instrumentation overhead.
- **Only tool with OLE / BSTR tracking** — relevant for COM-heavy Windows codebases.
- **Sanitizer-compatible** — can run alongside ASan/MSan/TSan rather than conflicting with them; each sanitizer's conflicting features are automatically disabled.

---

## Quick Start

### 1. Link the library

The library builds as a DLL (`MemoryLeakFinder.dll` on Windows).
Include the `include/` directory in your project.

### 2. Include the tracking headers

In your application's memory header (or a precompiled header):

```cpp
#include "gucefDRGUP_MemoryManager.h"
```

For operator `new`/`delete` interception add the macros header:

```cpp
#include "gucef_dynnewon.h"   // redirects new/delete through the tracker
```

### 3. Initialize and shut down

```cpp
#include "gucefDRGUP_MemoryManager.h"

int main()
{
    MEMMAN_Initialize();     // start tracking

    // ... your application code ...

    MEMMAN_Shutdown();        // dump report and tear down
    return 0;
}
```

`MEMMAN_Shutdown()` automatically writes the memory report to `GUCEF_memlog.txt`.

---

## Configuration

All settings are applied through `MEMMAN_Set*()` functions after `MEMMAN_Initialize()`.

| Function | Default | Description |
|---|---|---|
| `MEMMAN_SetLogFile(path)` | `"GUCEF_memlog.txt"` | Change log output path |
| `MEMMAN_SetLogAlways(1)` | `true` | Log every alloc/dealloc (verbose) |
| `MEMMAN_SetPaddingSize(n)` | `4` | Sentinel bytes on each side per allocation |
| `MEMMAN_SetExhaustiveTesting(1)` | `false` | 1024-byte padding + log always |
| `MEMMAN_CleanLogFile(1)` | `true` | Delete log file on first write |
| `MEMMAN_BreakOnAllocation(n)` | `-1` | Break into debugger on N-th allocation |
| `MEMMAN_BreakOnDeallocation(ptr)` | — | Break when `ptr` is freed |

### Type Mismatch Enforcement

Control what happens when `new`+`free` or `new[]`+`delete` is detected:

```cpp
// Log only (default)  → MISMATCH_LOG
// Debugger break      → MISMATCH_BREAK  (set via SMemoryTrackerConfig)
// Abort               → MISMATCH_ABORT

// To suppress mismatch check for a deliberately cross-allocator pointer:
MEMMAN_SuppressMismatchCheck( ptr );
```

---

## Report Types

### Automatic shutdown report

Written to `GUCEF_memlog.txt` on `MEMMAN_Shutdown()`. Sections:
- Total / peak memory usage
- Bounds violations and type mismatch count
- Memory leak list with file/line and callstacks
- Lock tracer summary (abandonments, surplus releases)
- Allocation size histogram

### On-demand reports

```cpp
MEMMAN_DumpLogReport();               // full statistics summary
MEMMAN_DumpMemoryAllocations();       // all currently live allocations
MEMMAN_DumpCallsiteReport( 20 );      // top 20 call sites by live bytes
MEMMAN_DumpSizeHistogram();           // size bucket table
MEMMAN_DumpTimeline( "heap.tsv" );    // TSV timeline (Excel/pandas-ready)
MEMMAN_DumpMassifFormat( "heap.ms" ); // Valgrind massif snapshot
```

---

## Callstack Capture

Two complementary callstack systems are active simultaneously:

**GUCEF logical callstack** — developer-annotated frames from `{GUCEF_TRACE;` macros.
Names are meaningful (e.g. `MyClass::Load()`), but only annotated functions appear.

**OS-level callstack** — actual instruction-pointer frames captured via:
- Windows: `RtlCaptureStackBackTrace()` + DbgHelp.dll for symbolication
- Linux/Android: `backtrace()` + `backtrace_symbols()` / `dladdr()`

Symbolication runs at report time (not per-alloc) to avoid DbgHelp lock overhead in the hot path.
Both stacks appear side-by-side in allocation records.

Configure depth (default: 32 frames, Windows max: 62):
```cpp
// Via SMemoryTrackerConfig before MEMMAN_Initialize()
cfg.maxRawCallstackDepth = 32;
cfg.enableRawCallstackCapture = true;
```

---

## Optional: Global operator new/delete Interception

To intercept all C++ allocations in the process (including third-party libraries):

Define `GUCEF_DRGUP_GLOBAL_NEW_OVERRIDE` **at MemoryLeakFinder DLL build time**.

```cmake
target_compile_definitions( MemoryLeakFinder PRIVATE GUCEF_DRGUP_GLOBAL_NEW_OVERRIDE )
```

**Warning:** Do not define this if any other library in the link unit also overrides
`operator new`/`delete`. The linker will report a duplicate symbol error.
A `#pragma message` warning is emitted at build time when this flag is active.

---

## Optional: CRT Heap Hook (MSVC only)

Intercepts `malloc`/`calloc`/`realloc`/`free` from code sharing the same CRT instance:

Define `GUCEF_DRGUP_HOOK_CRT_HEAP` at DLL build time:

```cmake
target_compile_definitions( MemoryLeakFinder PRIVATE GUCEF_DRGUP_HOOK_CRT_HEAP )
```

This uses `_CrtSetAllocHook`. File/line info is unavailable from the CRT hook,
but allocation size and type are tracked. Disabled automatically when ASan is active.

---

## Sanitizer Compatibility

The library auto-detects sanitizers at compile time and disables conflicting features:

| Sanitizer | Auto-disabled features |
|---|---|
| **ASan** (`-fsanitize=address`) | Sentinel writes, guard pages (shadow memory conflict) |
| **MSan** (`-fsanitize=memory`) | Body fill patterns (shadow tracking conflict) |
| **TSan** (`-fsanitize=thread`) | Lock tracer init/shutdown (false positive prevention) |

Remaining features (allocation tracking, callstacks, OLE tracking, statistics) stay active
and work alongside the sanitizer.

---

## Guard Page Mode

When `useGuardPages=true`, each allocation is backed by `VirtualAlloc` (Windows) or
`mmap`/`mprotect` (Linux/Android) instead of `malloc`. The user data is placed so its last byte
ends exactly at a page boundary; the immediately following page is mapped `PAGE_NOACCESS` /
`PROT_NONE`. Any write past the end of the buffer triggers an access violation **at the moment
of the overflow**, not at the next sentinel check.

```cpp
// Enable before first MEMMAN_Initialize(), or via SMemoryTrackerConfig:
SMemoryTrackerConfig cfg;
SMemoryTrackerConfig_SetDefaults( cfg );
cfg.useGuardPages = true;           // enable guard pages
cfg.deallocRingCapacity = 500;      // reduce ring size — each entry holds a virtual address reservation
MEMMAN_Initialize();
// or: tracker->ApplyConfig( cfg );
```

**Costs and constraints:**
- Each allocation consumes at least one full page (4 KB) of address space
- The dealloc ring capacity is automatically capped at 1,000 when guard pages are active
  to prevent address space exhaustion in 32-bit processes
- **Not for production use** — memory overhead makes this suitable for debug sessions only
- Disabled automatically when ASan is active (`GUCEF_DRGUP_ASAN_ACTIVE`)
- Front sentinel padding is written before `reportedAddress` for pre-buffer detection in
  memory dumps; back sentinels are replaced by the guard page itself

---

## Lock Range Protection

A lightweight contract-validation API that detects "forgot to take the lock before freeing this
buffer" bugs. This is **not race detection** — it is a static association check at deallocation
time, with zero overhead when the map is empty.

```cpp
// Declare that myMutex protects the buffer
MEMMAN_LockProtectsRange( &myMutex, buffer, bufferSize );

// ... elsewhere: dealloc while not holding myMutex produces a log warning ...

// Remove the association when the buffer is intentionally freed
MEMMAN_LockUnprotectsRange( &myMutex );
free( buffer );
```

When `free(buffer)` is called and `myMutex` is not currently held, the log contains:
```
MEMMAN: WARNING: Deallocating 0x... (size=...) which is declared protected by lock 0x..., but that lock is not currently held
```

**Behaviour flags:**
- Disabled when `GUCEF_DRGUP_TSAN_ACTIVE` (TSan owns lock analysis; avoid false positives)
- The `MEMMAN_LockProtectsRange` / `MEMMAN_LockUnprotectsRange` calls are no-ops in that case
- Adding a range association for a lock that does not exist in the CLockTracer inventory is
  permitted; the check simply never fires for an unknown lock

---

## Limitations

| Gap | Status | Recommendation |
|---|---|---|
| Uninitialized memory reads | Not detectable from a tracking DLL | Use **MSan** (`-fsanitize=memory`) on Linux/clang or **Dr.Memory** on Windows |
| Data race detection | Not detectable from a tracking DLL | Use **TSan** (`-fsanitize=thread`) or **Helgrind** (Valgrind) |
| Overflow at access time (production) | Guard page mode incurs per-alloc page overhead | Use **ASan** for always-on zero-overhead coverage |
| Fully opaque third-party allocators | Invisible without global new override + CRT hook | Use **Dr.Memory** or **Application Verifier** for total coverage |

---

## Architecture

```
MemoryLeakFinder.dll
├── gucefDRGUP_CMemoryTracker       — singleton registry; alloc/dealloc tracking; histogram
├── gucefDRGUP_CAllocationRecord    — per-alloc node: sizes, sentinels, callstacks, timestamps
├── gucefDRGUP_CReporter            — all formatting: log report, exception report, profiling dumps
├── gucefDRGUP_CCallStackTracker    — GUCEF_TRACE logical callstack (thread-local push/pop)
├── gucefDRGUP_callstack            — OS-level frame capture + symbolication
├── gucefDRGUP_CLockTracer          — lock lifecycle tracker; range protection; aggregate stats
├── gucefDRGUP_MemoryManager        — C API layer (MEMMAN_* functions)
└── gucefDRGUP_SMemoryTrackerConfig — POD configuration struct
```

---

## Building

The library is built via CMake as part of the GUCEF build system:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build --target MemoryLeakFinder
```

On Windows the library links `Winmm.lib` (precision timer) and optionally `DbgHelp.lib`
(symbol resolution, present on all Windows systems).

---

## File Reference

| Header | Purpose |
|---|---|
| `gucefDRGUP_MemoryManager.h` | Public C API (`MEMMAN_*` functions) |
| `gucefDRGUP_SMemoryTrackerConfig.h` | Configuration struct + sanitizer detection macros |
| `gucefDRGUP_CAllocationRecord.h` | Allocation record struct (`TRawCallStack`, `CAllocationRecord`) |
| `gucefDRGUP_CCallsiteStats.h` | Per-callsite aggregated statistics struct |
| `gucefDRGUP_callstack.h` | OS callstack capture API |
| `gucefDRGUP_locktrace.h` | Lock tracing C API |
| `gucef_dynnewon.h` | Macro header that redirects `new`/`delete` through the tracker |
| `gucef_dynnewoff.h` | Disables the redirection (used inside the library itself) |
