# AncestryDotComBackup

A command line tool that makes a careful, local backup of your own Ancestry.com family tree and turns it into
open formats you can keep: the GEDCOM export, every photo and document, your DNA match list as a spreadsheet,
and a complete [Gramps](https://gramps-project.org) tree that Gramps accepts as its own data.

It is built on the GUCEF framework (`tools/AncestryDotComBackup` is the command line, `tools/AncestryDotComBackupLib`
the logic).

* [What it does](#what-it-does)
* [Before you start](#before-you-start)
* [Quick start](#quick-start)
* [Commands](#commands)
* [Settings](#settings)
* [Using Gramps](#using-gramps)
* [How the tool treats the site](#how-the-tool-treats-the-site)
* [Where everything is stored](#where-everything-is-stored)
* [What is and is not backed up](#what-is-and-is-not-backed-up)
* [Building and testing](#building-and-testing)
* [Troubleshooting](#troubleshooting)

## What it does

| Capability | Command | Talks to the site? |
| --- | --- | --- |
| Log in to Ancestry in a browser the tool controls (you type the password, the tool never sees it) | `login` | yes, via your browser |
| Take in the official GEDCOM export of your tree as the starting point | `baseline`, `export` | no |
| Find out what changed in the tree since the export, so the backup can keep up | `changes` | yes, 1 or 2 requests a day of changes |
| Download every photo and document in full size, with the metadata the site holds about each | `media` | yes |
| Download your DNA match list into a spreadsheet and a JSON file | `dna` | yes |
| Convert the GEDCOM and the media into a Gramps tree (`.gramps`) | `gramps` | no |
| Learn the endpoints of the site by recording your own clicking, and report on them | `discover`, `report` | no (only listens to your browser) |

Design points:

* **No password handling.** You log in by hand in a real Chrome or Chromium. The tool drives that browser and reads
  nothing of your credentials. Cookies and authorization headers are never recorded.
* **Careful with the site.** Read only, slow, limited and self-halting. See [How the tool treats the site](#how-the-tool-treats-the-site).
* **Resumable.** Every long job remembers where it stopped and never fetches a file that is already there.
* **Nothing is thrown away.** The original GEDCOM of every export is kept next to the latest one, and every answer of the site
  is kept as it was, next to the files made from it.
* **Repeatable.** Converting the same data to Gramps twice gives the same file, byte for byte.

## Before you start

* A **Chrome or Chromium** browser. Chrome is preferred, Chromium is used when there is no Chrome.
* An **Ancestry account** with the tree you want to back up. The tool reads your own tree from your own session.
* For zip exports and the `gramps` import test: nothing more. For opening the result: Gramps 5.2 or newer
  (tested with Gramps 6.0.8, XML format 1.7.2).
* The tool on its own does not need an internet connection except for `login`, `changes`, `media` and `dna`.

**Terms of use.** Ancestry's terms limit automated access to the site. This tool is meant for your own data, from
your own session, and is built to stay far below anything that looks like a crawler. Whether using it is
acceptable for your account is your decision. If in doubt, use only `baseline` and `gramps`: they make no
requests at all and work on the GEDCOM export the site gives you through its own export button.

## Quick start

1. **Build** the tool (see [Building and testing](#building-and-testing)).
2. **Get a GEDCOM export.** On Ancestry: your tree, Tree Settings, "Export tree". When it is ready, download it. Leave the
   file in your Downloads directory.
3. **Take it in:**

       AncestryDotComBackup export treeId=default dataDir=/path/to/backup

   (or `baseline gedcom=/path/to/export.ged` for a file somewhere else).
4. **Make the Gramps tree:**

       AncestryDotComBackup gramps treeId=default dataDir=/path/to/backup

5. **Import** `/path/to/backup/default/export/default.gramps` into an empty Gramps tree (see [Using Gramps](#using-gramps)).

That is a complete backup of everything a GEDCOM can hold, without a single request to the site apart from the
export you clicked yourself. The rest of this document adds the photos, the changes since the export, and DNA.

For the full routine see [Keeping the backup up to date](#keeping-the-backup-up-to-date).

## Commands

    AncestryDotComBackup <command> [setting=value ...]

Settings are `name=value` or `--name=value`. A flag without a value (`--headless`) means true. Names are not case sensitive.
`AncestryDotComBackup --help` prints the short version of this page. Commands are blocking: they run until done,
and they pump the event loop of the application themselves.

### login

Starts the browser, opens Ancestry, waits for you to log in and press Enter, then checks that the browser holds a
session for the site. Nothing of the session is stored by the tool; it lives in the browser.

By default the tool uses **your everyday Chrome**, so that your normal login (for example "sign in with Google") is used:

1. In that Chrome, open `chrome://inspect/#remote-debugging` and tick *Allow remote debugging for this browser instance*.
2. Run the command. Chrome shows an approval popup **every time the tool connects**. There is no "remember" option, you click Allow.
3. The tool opens a tab of its own for the site. Only tabs showing a host that matches `hostFilter` are watched, so your other tabs are left alone.
4. Switch the remote debugging option off again afterwards if you like.

`attachToRunning=false` always starts a separate browser instead, with its own profile in
`<dataDir>/<treeId>/browser-profile`. Log in there by hand once, the profile keeps the login.
The tool finds Chrome first and Chromium second (`chromePath=` overrides).

### baseline

    AncestryDotComBackup baseline treeId=<id> gedcom=<file.ged|file.zip> [dataDir=<dir>]

Takes in a GEDCOM export you made on the site: checks it, copies it to `<treeId>/baseline.ged`, and keeps a copy of
the export as `<treeId>/baselines/export-<time>.ged` so that later exports can be compared with it.
`state/baseline-summary.txt` tells what the file holds (people, families, sources, media links, record ids) and what a GEDCOM
cannot hold, such as comments, stories and transcriptions. `state/baseline-individuals.txt` lists the person ids.

A `.zip` as delivered by the site is opened too (this needs the libarchive plugin, `vfspluginLIBARCHIVE`).
Without `gedcom=` the export taken in earlier is checked again. Optional: the tool can also work with nothing but the site,
but the export is the sanctioned, complete starting point.

### export

    AncestryDotComBackup export treeId=<id> [exportDirs=<dir;dir>] [exportMaxAgeDays=7]

Looks in your Downloads directory (or `exportDirs`) for a GEDCOM written by Ancestry in the last `exportMaxAgeDays` days,
and takes in the newest one that was not taken in before, as `baseline` does. It does nothing on the site.
Exit code 5 means there is no recent export.

### changes

    AncestryDotComBackup changes treeId=<id> [changesSince=<time>] [changesMaxPages=50]

Asks the site's change feed ("recent activity") what changed in your tree since the last run, or since the time of the
baseline export on the first run. A day of changes is a request or two. Result:

* `state/changes.jsonl`: what changed, one entry per line.
* `state/changed-people.txt`: the people to look at again.
* `raw/changes/`: the feed pages as the site gave them.

`changesSince=2026-10-16T21:27:48Z` sets the start by hand. Changes tell you *that* people changed. The way to bring the
details into the backup is a new GEDCOM export (see below). People you edited on the site after the export are the ones
`changes` lists.

### media

    AncestryDotComBackup media treeId=<id> [maxRequestsPerRun=500] [retryMissing=true]

Lists the media of the tree (photos, documents, stories, recordings, videos), 28 per page, into
`state/media-inventory.jsonl`, and downloads the files of the pictures and documents in full size to
`media/<id>.<ext>`, with `media/<id>.<ext>.json` next to each holding everything the site said about the item.

* It continues where the last run stopped and never fetches a file that is already there. Re-run it to pick up new photos.
* Failures go to `state/media-failed.txt`; items the site could not find go to `state/media-missing.txt`. `retryMissing=true` asks again for those.
* Some old images answer "not found" even on the website itself: bulk imports from 2016 sometimes left only a
  placeholder. Those are recorded as missing, not retried by default.
* Stories, recordings and videos are in the inventory but their files are not downloaded.

### dna

    AncestryDotComBackup dna treeId=<id> [dnaSampleId=<id>]

Downloads your DNA match list. Output:

* `dna/matches.csv`: one row per match, opens in any spreadsheet and is meant for looking matches up elsewhere (23andMe,
  FamilyTreeDNA, GEDmatch). Columns: `provider, test_id, match_id, name, initials, sex, shared_cm, shared_segments,
  longest_segment_cm, percent_shared, predicted_relationship, relationship_group, prediction_source, parent_side, cluster_code,
  meiosis, ignored, has_tree, tree_size, match_date, tags, notes, profile_url`.
  `parent_side` is the maternal / paternal side the site assigns.
* `dna/matches.jsonl`: the same, with everything the site said.
* `dna/raw/`: the answers of the site as they were.

`dnaSampleId` is the id of your DNA test, the id in the address of your match pages. It is found automatically from earlier
discovery when possible. DNA is the most protected area of the site: it asks for a short lived security confirmation, and the tool
stops and halts (see below) at the first refusal. The download is built and tested but depends on that confirmation; if the site
refuses, the tool does not push. DNA pages are never recorded by `discover` unless `recordDna=true`, the
sign-in and security pages are never touched, and the few POST requests the DNA command makes only read (their body is only ids).
The raw DNA data itself (the genotype file) is not touched: download it from the site's own settings if you want it.

### gramps

    AncestryDotComBackup gramps treeId=<id> [grampsFile=<file>] [grampsMediaPath=<dir>] [grampsHomePerson=<id>] [researcherName=<n>] [researcherEmail=<e>]

Converts the baseline GEDCOM and the downloaded media into a native Gramps tree: `<treeId>/export/<treeId>.gramps`
(Gramps XML 1.7.2). Nothing is asked of the site. It takes about 20 seconds for 160,000 people.
The result is validated against the DTD and RelaxNG schema that ship with Gramps and was imported with Gramps 6.0.8.

* **People:** names (given, surname, suffix, title, nickname, alternate names), gender, all events, notes, web links, e-mail and phone,
  attributes for what Gramps has no field for, and a link to the person's page on Ancestry.
* **Events:** birth, death, burial, baptism, marriage, residence, census and the other standard types by their Gramps names;
  anything custom stays a custom event with its own name, including source citations, notes and media on the event.
* **Families:** parents, children (with adopted/step/foster relation when the export says so), marriage and the other family events.
* **Places** become a hierarchy: `Meerssen, Limburg, Nederland` is three places, each inside the next; known country names
  are typed as Country, the rest are "Unknown" rather than guessed.
* **Sources, repositories, citations:** sources keep their repositories and Ancestry record ids; the same citation used on many
  records is one citation object, as Ancestry shares it.
* **Dates** are real Gramps dates (about, before, after, ranges, spans, BC). Day and month that could be mixed up (`3-4-1900`)
  and anything that is not a date at all (`(ja)`) stay as text exactly as written.
* **Media** is linked to the people, events and families it belongs to, the primary photo first, including the cut-out
  of the profile picture (as the region Gramps uses). Files link through the Gramps *base media path*: `grampsMediaPath`, default the `media`
  directory of the tree. Media that was not downloaded is still in the tree, tagged **Ancestry: media file not downloaded**,
  and links up when the file is there and you convert again.
* Ancestry's own markers (`_NEW`, `_COLOR`) become attributes. HTML in notes is turned into plain text.
* The home person is the first person in the file, or `grampsHomePerson=I40537067793`.

`state/gramps-report.txt` lists what was converted, which dates stayed text, references to records that are not in the
file (those are left out), and how many people were added or gone since the previous conversion.

### discover and report

    AncestryDotComBackup discover treeId=<id> [discoverSeconds=0] [recordDna=false]
    AncestryDotComBackup report   treeId=<id>

`discover` connects to the browser and records the requests the site makes while *you* click through a person's
facts, sources, media, stories and so on. Press Enter when done. It sends no requests of its own. Output:
`discovery/requests.jsonl`, the JSON responses in `discovery/bodies/`, and `discovery/endpoints.md` describing the endpoints
(methods, paths with ids as `{n}`, query parameters, paging fields, and the shape of the JSON: field names and types, not data).
Sign-in, account and credential pages are never recorded, DNA pages only with `recordDna=true`. The saved responses are your tree's data, keep them private.

`report` rebuilds `endpoints.md` from `requests.jsonl` without a browser.

## Keeping the backup up to date

1. On Ancestry, make a new GEDCOM export and download it (this is the complete, authoritative source).
2. `AncestryDotComBackup export` (takes in the newest recent export; the old one stays in `baselines/`).
3. `AncestryDotComBackup media` (new photos only; nothing already downloaded is fetched).
4. `AncestryDotComBackup changes` (optional: which people changed on the site since the export, for people edited after you exported).
5. `AncestryDotComBackup dna` (optional).
6. `AncestryDotComBackup gramps` (always converts everything; same data gives the same ids).

Between exports, `changes` is the cheap check that tells you whether a new export is worth making.

## Settings

Settings come from `AncestryDotComBackup.ini` (section `[Main/AppArgs]`, next to the tool) and from the command
line; **the command line wins**.

| Setting | Default | Meaning |
| --- | --- | --- |
| `dataDir` | `data` | Where the data of all trees goes. |
| `treeId` | `default` | Names the directory of this tree below `dataDir`. Letters, digits, `-`, `_`, `.` only. |
| `siteTreeId` | from the GEDCOM header | The numeric id Ancestry uses for the tree. Needed for requests to the site. |
| `gedcom` / `baselineGedcom` | none | GEDCOM (or zip) to take in with `baseline`; for `gramps` an alternative input. |
| `exportDirs` | Downloads | Where `export` looks, separated by `;`. |
| `exportMaxAgeDays` | `7` | How old a downloaded export may be. |
| `chromePath` | found | The browser to use. |
| `debugPort` | `9222` | Remote debugging port of the browser. |
| `attachToRunning` | `true` | Use your running Chrome instead of starting one. |
| `chromeUserDataDir` | usual places | Where to look for the running browser's `DevToolsActivePort`, separated by `;`. |
| `chromeArgs` | none | Extra browser arguments, separated by `;`. |
| `headless` | `false` | Run the browser without a window (tests). |
| `startUrl` | `https://www.ancestry.com/` | The page the browser opens. |
| `cookieDomain` | `ancestry.com` | Domain whose cookies `login` checks. |
| `hostFilter` | `ancestry` | A host must contain one of these terms (`;` or `,` separated) to be watched or recorded. |
| `noWait` | `false` | Do not wait for the user in `login` and `discover`. |
| `discoverSeconds` | `0` | End discovery after this long; 0 waits for you. |
| `maxBodyMegabytes` | `32` | Larger responses are not saved during discovery. |
| `recordDna` | `false` | Let `discover` record DNA pages too. |
| `dnaSampleId` | found | The id of your DNA test. |
| `changesSince` | last run | Time (`2026-10-16T21:27:48Z`) to look for changes from. |
| `changesMaxPages` | `50` | Most feed pages per run (20 changes each). |
| `retryMissing` | `false` | `media`: ask again for items the site could not find. |
| `requestDelayMs` | `2500` | Pause between two requests. Values under 500 are refused. |
| `requestJitterPercent` | `40` | The pause varies by up to this much. |
| `maxRequestsPerRun` | `500` | Most requests per run; 0 for no limit. |
| `maxRequestsPerDay` | `3000` | Most requests per UTC day, across runs; 0 for no limit. |
| `grampsFile` | `export/<treeId>.gramps` | Where `gramps` writes. |
| `grampsMediaPath` | the `media` directory | Base media path inside the Gramps tree. |
| `grampsHomePerson` | first person | Id (`I…`) of the Gramps home person. |
| `researcherName`, `researcherEmail` | empty | Researcher recorded in the Gramps tree. |

Exit codes: `0` ok, `1` usage or bad settings, `2` no browser or no connection to it, `3` not logged in, `4` failed,
`5` no recent export found, `6` halted by the site (see below).

A log is kept in `<dataDir>/AncestryDotComBackup_log.txt`.

## Using Gramps

**Import into an empty tree.** In Gramps: *Family Trees → Manage Family Trees → New*, open the new tree, then
*Family Trees → Import* and choose the `.gramps` file. From the command line:

    gramps -C AncestryBackup -i /path/to/backup/default/export/default.gramps

(the import of 160,000 people took about four minutes.)

Why empty: Gramps keeps the handles from the file when the tree is empty, so every run gives the same ids. Into a tree that
already contains people Gramps issues new handles to everything it imports, so importing again would add every
person a second time. To update, make a new empty tree, import there, and retire the old one.

**Media.** Gramps looks for the files in its *base media path* (*Edit → Preferences → Family Tree*, "Base media path").
The import sets it from the file when the tree's own is empty. If you move the backup, change the base media path in Gramps,
or set `grampsMediaPath` and convert again. Items tagged *Ancestry: media file not downloaded* have no file yet; use the tag filter in Gramps to list them.

**IDs.** People are `I<Ancestry id>` (for example `I40537067793`), families `F<number>`, sources `S<number>`, media `O<number>`.
Events, places, citations and notes are numbered `E0001`, `P0001`, `C0001`, `N0001` in file order.

Nothing in the file is marked private, and nothing is shared anywhere: it is all local.

## How the tool treats the site

* **Only GET requests**, and only ones on an allowlist of requests known to read (`AncestryBackup_CRequestPolicy`), for the
  tree the tool was set up for and no other. Never to sign-in, account, password or payment pages. Additional read endpoints
  can be allowed by adding path templates, one per line, to `<treeId>/endpoints.ini`.
* **From inside your own browser tab**, with your own session, as a normal page would make them.
* **Slowly:** one request every `requestDelayMs` (2.5 s ±40% by default); at most 500 per run and 3000 per day (counted across runs in
  `state/request-budget.txt`).
* **It stops by itself.** If the site answers 429, 403 or 401, serves a challenge page, or answers HTML where data was asked for,
  the tool writes `state/HALTED.txt`, stops, and refuses to make requests in any later run until you have read that file and
  deleted it. There are no automatic retries.
* The DNA pages are separate from all of this: only the `dna` command touches them (see above).

## Where everything is stored

    <dataDir>/AncestryDotComBackup_log.txt
    <dataDir>/<treeId>/
        baseline.ged                 the latest GEDCOM export taken in
        baselines/export-<time>.ged  every export taken in, kept for comparing
        media/<id>.<ext>             downloaded files
        media/<id>.<ext>.json        everything the site said about each file
        dna/matches.csv, matches.jsonl, raw/
        export/<treeId>.gramps       the Gramps tree
        state/
            baseline-summary.txt, baseline-individuals.txt
            changes.jsonl, changed-people.txt, changes-state.txt
            media-inventory.jsonl, media-failed.txt, media-missing.txt, media-list-*.txt
            gramps-report.txt, gramps-individuals.txt
            request-budget.txt, HALTED.txt (only when halted)
        raw/changes/, raw/media-lists/   the pages of the site as they were
        discovery/                   requests.jsonl, endpoints.md, bodies/
        browser-profile/             the tool's own browser profile, if it uses one
        endpoints.ini                extra endpoints you allowed

Everything under `<dataDir>` holds your family's data (living people included). Keep it somewhere you trust and back it up too.

## What is and is not backed up

Backed up:

* The whole tree as the GEDCOM export holds it: people, names, events, places, notes, families, sources, repositories, citations, media links.
* Every photo and document of the media gallery, full size (except the items the site no longer serves).
* The change feed since the export, to know what to export again.
* The DNA match list with shared cM, relationship predictions and parent side (when the site allows the download).
* A Gramps tree of all of the above.

Not backed up:

* **Comments, stories, "life stories" and transcriptions.** A GEDCOM cannot hold them and there is no downloader for them yet. Stories, audio
  and video appear in the media inventory only.
* **Record images** of the historical collections attached to the tree (hundreds of thousands of requests: not done).
* **The DNA raw data and segment details.** Only the match list. The site shows only totals (`2,014 cM | 29% shared DNA`) beyond that.
* **Hints you have not accepted**, and anything outside the tree.
* Imports of DNA lists from 23andMe or FamilyTreeDNA are not built; `dna/matches.csv` is the format meant for comparing with them.
* A `verify` command (check backup against the site) is not built.

## Building and testing

The build files are generated by GUCEF's ProjectGenerator and are in the repository. On Linux:

    cmake -S projects/CMake/targets/GUCEF_exe_AncestryDotComBackup -B build/ancestry -G Ninja
    ninja -C build/ancestry

The target builds the tool and its plugins: JSON (`dstorepluginJSONPARSER`), hash and zlib codecs, the curl HTTP backend (for https) and,
optionally, `vfspluginLIBARCHIVE` for zip exports. The `.ini` file loads the plugins from the directory of the framework libraries (`$MODULEDIR$`), which is where the build puts them. Needs a C++ compiler,
OpenSSL development files and CMake 3.20+. A Windows build uses the same generated files (Visual Studio 2026); it has not been built or tested yet.

Tests (on the GUCEF test framework):

    cmake -S projects/CMake/targets/GUCEF_exe_AncestryDotComBackup_TestApp -B build/anctest -G Ninja
    ninja -C build/anctest
    build/anctest/bin/AncestryDotComBackup_TestApp

Writes `AncestryDotComBackup_TestApp_Results.xml`. 56 tests cover the data layout, endpoint catalog, settings, GEDCOM reading, the request policy,
budget and halting, the change feed, the media inventory, DNA parsing, zip handling, the GEDCOM-to-Gramps conversion (dates, text, records and
a full small tree) and a headless Chrome end to end test against a small local site. The Chrome test is skipped when no Chrome or Chromium is
found; set `GUCEF_TEST_CHROME_NO_SANDBOX=1` to run Chrome without its sandbox in containers.

To check a conversion against Gramps' own schema (Python with lxml):

    python3 -c "import lxml.etree as E; r=E.RelaxNG(E.parse('/usr/share/gramps/grampsxml.rng')); print(r.validate(E.parse('default.gramps')))"

## Troubleshooting

* **Exit 2, no browser:** install Chrome or Chromium, or give `chromePath=`. For your running Chrome, tick the remote debugging
  option at `chrome://inspect/#remote-debugging` and click Allow in the popup when it appears.
* **Exit 3, not logged in:** log in in the tab the tool opened (or run `login`), open your tree, then run the command again.
* **Exit 5:** no GEDCOM downloaded in the last `exportMaxAgeDays` days. Export again, or use `baseline gedcom=<file>`.
* **Exit 6 / `state/HALTED.txt`:** the site showed signs of unhappiness. Read the file, wait, check the site in your browser, and only then delete it.
* **A browser from a snap or VS Code terminal crashes:** the tool starts the browser with a cleaned environment; if you start Chrome yourself, do it from a normal terminal.
* **Gramps shows "file missing" on pictures:** the base media path is wrong or the file is not downloaded yet (see the tag). Fix the path in Gramps or run `media` and `gramps` again.
* **Importing into Gramps twice made duplicates:** see "Import into an empty tree" above.
* **Many dates show as text in Gramps:** they are not dates in the source (`(ja)`, `(onbekend)`). `state/gramps-report.txt` counts them.
* **DNA download refused:** the site wants a fresh security confirmation for DNA pages. Open the DNA matches page in the browser and confirm there, read `state/HALTED.txt`, delete it, and run `dna` once more. The tool halts again at the first refusal; do not keep retrying.
