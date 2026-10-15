# Avatar rendering correction

English · [简体中文](avatar-performance.zh-CN.md)

Historical 0.8.1 correction. The user subsequently approved the [0.9.0 upstream import](avatar-upstream.md); its expression tables and primitive renderer supersede the approximation described here.

The user reports `0.8.0-avatar` has weak reactions and low frame rate. The exact
flashed-image hash, display timings and live notification trace are unverified.
Source comparison finds real design/performance gaps:

- Reference idle eyes are about 72×168px with 210px center spacing; this version
  uses 104×144px and 144px spacing. It has no brow geometry or full single/
  double/hold expression timeline. It is not a faithful reproduction.
- Its native raster runs transforms, min/max, division and three color-rounding
  operations per lit candidate pixel. Reference rendering primarily fills
  geometric spans/primitives and clears old/new bounds separately per eye.
- This version transfers a combined two-eye rectangle through 16-row DMA chunks;
  the reference uses per-eye bounds and an approximately 16.667ms scheduler.
  Reference engineering-note frame rates do not establish our hardware speed.

## Independent correction defined before implementation

Keep the existing visual geometry and GPL source while awaiting the user's
decision on importing upstream AGPL expression code. This performance correction
does not import that implementation or change input/tools/stop/IMU configuration.

- Derive rounded-rectangle scanline spans from its two rotated core rectangles
  and four corner circles. Compute transforms/intersections per row, fill white
  spans in bulk and add a gray edge; no per-pixel float shader/color conversion.
- Transfer old/new union separately for each eye, repainting both eyes into any
  overlapping region. First entry/exit remains full-frame and uncertain DMA
  completion still disables reuse. Retain 30fps target until measured evidence
  supports changing it; do not claim speed from a target constant.
- Emit bounded five-second timing summaries for active avatar updates and actual
  paint calls, including mean/max paint time. Logs contain no input text,
  coordinates, token or device identity. Measurement collection is not a device run.
- Before runtime edits, define analytic raster fixtures for zero/rotated angles,
  lids and corner boundaries, per-eye dirty/full-frame equivalence and synthetic
  timing-window aggregation. Reuse existing UI/stop/IMU/HID/wireless/flash/stack
  checks, pinned compile/package and native previews.
- Separately authorized device acceptance measures actual update/paint cadence,
  input coexistence, quick touch and MCP reactions, stop and Settings navigation.
  Upstream expression import remains dependent on the user's license decision.
