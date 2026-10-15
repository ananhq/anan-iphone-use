# Source and installation

English · [简体中文](SOURCE.zh-CN.md)

The setup page's **Firmware source / 固件源码** link downloads this build's
project-source archive from `/source`, without pairing or an external account.
It contains the editable firmware, imported avatar/BMI270/QR sources, generated
UI glyph source, build configuration and scripts, dependency pins and licenses.
It excludes device settings, Wi-Fi credentials, pairing data and generated SDK
configuration. The archive is created from an explicit allowlist at build time.
Keep the matching archive beside any distributed binary. `package.py` records
its SHA-256 and refuses to package stale source. This is not a new MCP tool.

Build dependencies are obtained separately, at their recorded versions:

- ESP-IDF v5.5.5, commit `b774170ff46c393eeb5e495ea37936038d3f4f4f`:
  https://github.com/espressif/esp-idf/tree/b774170ff46c393eeb5e495ea37936038d3f4f4f
  Clone with submodules, run `./install.sh esp32s3`, then source `./export.sh`.
  Its recorded submodules supply the SDK libraries; retain their licenses.
- The IDF component manager retrieves the exact registry sources/hashes in
  `dependencies.lock` using `main/idf_component.yml`; CO5300 and TinyUSB sources
  are available through https://components.espressif.com/ at those locked versions.
- Build with `bash build.sh` in the extracted project root. This runs `idf.py
  build` and `python package.py build`; no serial port is opened.
- Installation: `PYTHON=python ./flash.sh <serial-port>
  build/anan-stopwatch-flash.bin`. Use a StopWatch ESP32-S3 with 16MB flash.
  The helper preserves existing NVS/pairing storage and verifies the image
  manifest. The firmware does not impose a signing key or activation lock.
  Device setup and control still require the user's physical confirmation.

Existing GPL-3.0-only code retains its license; the imported expression engine
and native port are AGPL-3.0-or-later. Combination uses GPLv3/AGPLv3 section 13:
the AGPL network-source requirement applies to the combination. Preserve both
complete texts and all file-specific grants/notices. See `NOTICE` and
`THIRD_PARTY.md` for versions, modifications and historical unresolved provenance.
This source offer does not declare that the pre-existing derivation or linked
binary notice audit is complete. SDK radio blobs remain under their own terms;
no claim is made that their implementations are open source.
