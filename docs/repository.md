# Repository layout

English · [简体中文](repository.zh-CN.md)

## File ownership

Organization-only acceptance: relocate modules without changing any firmware
source/header bytes, source order, optimization, SDK defaults, dependencies,
task/storage/radio configuration or input/stop behavior. Keep one component,
the existing command entrypoints and all historical validation artifacts.
Update build/host include paths and documentation links together. Compare
target object code/data/relocations and memory budgets with the pre-move build;
run the existing offline checks and verify the embedded source offer. Path
metadata and the source archive may differ; no device performance measurement,
flash or commit is part of this scope.

| Location | Contents |
| --- | --- |
| Root `README*`, `CONTRIBUTING*`, `AGENTS.md` | Project entry, contribution process and repository instructions |
| Root `CMakeLists.txt`, `sdkconfig.defaults`, `partitions.csv`, `dependencies.lock` | Reproducible ESP-IDF build configuration and pinned dependencies; keep tracked |
| Root `build.sh`, `flash.sh`, `flash.py`, `package.py` | Build, explicit flash and packaging entrypoints; flash helpers skip BLE NVS during upgrades |
| `main/` | Firmware component, headers and embedded `web/setup.html`; see [module responsibilities](../main/README.md) |
| `docs/` | Current user guides, architecture and acceptance entrypoints; start at the [document index](README.md) |
| `docs/history/` | Implementation decisions and correction scopes; see the [history index](history/README.md) |
| `docs/research/` | HID, focus navigation and approach comparisons; see the [research index](research/README.md) |
| `docs/images/setup/` | Selected synthetic UI illustrations used by the project README |
| `tools/` | [Tools index](../tools/README.md): existing offline checks, fonts, previews and source archiving |
| `artifacts/` | Sanitized validation records grouped by stage, described below |
| `licenses/`, root `LICENSE`, `NOTICE`, `THIRD_PARTY*` | Project license, retained upstream terms and provenance; see [source notices](../THIRD_PARTY.md) |
| `.agents/skills/anan-iphone-use/` | Agent-guided setup skill and its metadata |

The single `main/` component is retained. First-party files are grouped under
`app/`, `hardware/`, `input/`, `network/` and `ui/`; `vendor/` and `web/` stay
separate. Header names are unchanged, with matching firmware/host search paths.
[PROTOCOL.md](../PROTOCOL.md) stays at the root as the sole historical ANAN1
contract; current MCP behavior is documented in [MCP](mcp.md).

## Development helpers

Run commands from the repository root:

| Command | Purpose / outputs |
| --- | --- |
| `./build.sh` | Pinned ESP-IDF v5.5.5 build and merged image/manifest in ignored `build/` |
| `python package.py build` | Merge an already compiled build and regenerate its source/image manifest; no serial access |
| `bash tools/check_ui.sh` | Predefined host layout/target, battery, control/touch/key and native eye-rendering scenarios; no device access |
| `python tools/check_ui_stack.py BUILD_DIR` | Target-compiled UI/libc stack frames versus task allocations and a 2 KiB reserve; needs a compiled candidate, no device access |
| `python tools/render_ui.py` | Compile the host-only layout exporter and render synthetic previews into ignored `build/ui-previews/` |
| `python tools/generate_ui_font.py FONT_FILE` | Verify the pinned font hash and regenerate tracked `main/ui/ui_font.h` |

The UI helpers require Pillow12.3.0; rendering also requires a host C compiler.
Font provenance and notices are in [THIRD_PARTY](../THIRD_PARTY.md), and the
preview procedure is in [touch UI](history/touch-ui.md) and [control eyes](history/eyes.md). `render_ui_layout.c` and `render_eyes.c` are called
by the renderer, not built into firmware. Generated `ui_font.h` is a firmware
input and stays tracked; the full source font stays outside the repository.

The package source manifest includes visible `.c`, `.h`, `.cpp`, `.hpp`, `.md`, `.yml`, `.html`
files and `CMakeLists.txt` under `main/`, plus the explicit root build inputs in
`package.py`. Hidden files and Python caches are excluded. If a new component
input type is introduced, update the manifest selection with that change.

## Validation records

| Directory | Meaning |
| --- | --- |
| `artifacts/migration/` | Original extraction/source hashes and migration checks |
| `artifacts/baseline/` | Baseline preparation and the authorized local checkpoint |
| `artifacts/mcp/` | MCP candidate build checks, previous candidate snapshots and sanitized user feedback |
| `artifacts/ui/`, `artifacts/wireless/`, `artifacts/hid/` | Candidate-specific display, wireless and input offline evidence |
| `artifacts/discovery/`, `artifacts/flashing/` | Discovery and configuration-preserving update evidence |

Existing records describe their specific source/image versions; preserve them
instead of rewriting them for a later cleanup. In particular, an older source
manifest may include `.DS_Store`; the current packaging selection excludes it.
`artifacts/mcp/build-check.json` identifies the recorded candidate, not proof
that every subsequent working-tree change has been rebuilt or device-verified.
Future authorized E2E records follow [validation](validation.md).

## Tracked and local files

[.gitignore](../.gitignore) separates generated/local files from maintained
inputs. `build/`, `managed_components/`, generated `sdkconfig` and its backups,
compiled binaries, Python/editor caches, raw logs and credential/certificate
exports are ignored. `sdkconfig.defaults`, `dependencies.lock`, source assets,
synthetic documentation images and sanitized JSON evidence stay tracked.

Use ignored `local/` for temporary captures, device exports or other local
working files; use `build/` for generated previews and build results. Before
adding an artifact, sanitize it and inspect `git diff --cached`. Ignore rules
do not remove files already tracked and cannot detect secrets inside arbitrary
JSON or images. Avoid broad `*.json`, `*.png`, `docs/` or `artifacts/` rules,
which would hide maintained evidence and documentation.
