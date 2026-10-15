# Development tools

English · [简体中文](README.zh-CN.md)

Run from the repository root. These checks access no board, phone, simulator
or model. Set `IDF_PATH` for wireless scenarios that compile SDK sources; target
stack checks use the candidate build compiler.

| Command | Purpose |
| --- | --- |
| `bash tools/check_ui.sh` | UI, stop, battery, avatar, IMU and renderer checks |
| `bash tools/check_hid.sh` | Existing HID parser/worker scenarios |
| `bash tools/check_wireless.sh` | Existing BLE, Wi-Fi, HTTP, memory and reset scenarios |
| `python tools/check_flashing.py` | Synthetic configuration-preserving flash checks; no serial I/O |
| `python tools/ui_acceptance/source-bundle.py` | Source archive contents and source-offer checks |
| `python tools/check_ui_stack.py BUILD_DIR` | Target compiler/libc stack budget; requires a compiled candidate |
| `python tools/render_ui.py` | Synthetic bilingual previews in ignored build/ui-previews/ |
| `python tools/generate_ui_font.py FONT_FILE` | Regenerate ui/ui_font.h from the hash-checked upstream font |

`ui_acceptance/`, `hid_acceptance/` and `wireless_acceptance/` retain existing host
scenarios. `check_upstream.sh`, `check_activity.sh` and `check_responsive.sh` are
called by the UI entrypoint. `firmware_paths.py/.sh` share host header paths;
`compile_avatar_host.py` reuses native avatar compilation. `render_ui_layout.c`
and `render_eyes.c` are preview-only. Font/preview tools require Pillow12.3.0.

`source_bundle.py` produces deterministic source archives for builds/packaging.
Generated outputs stay in ignored `build/`; sanitized evidence stays in `artifacts/`.
Keep device results separate from offline checks. See [layout](../docs/repository.md)
and [validation](../docs/validation.md).
