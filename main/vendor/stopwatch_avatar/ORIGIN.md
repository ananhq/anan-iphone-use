# StopWatch avatar source origin

Upstream: https://github.com/Trentct/m5stack-stopwatch-avatar

Commit: `204963257cb4dc2f3d7501eff900897bac55ef82`.
License: AGPL-3.0-or-later; original SPDX headers retained. Full text is
`licenses/StopWatch-avatar-AGPL-3.0.txt`. Upstream provides no separate copyright
line in these two files. Attribution: Trentct and upstream contributors.

| Original file | Original SHA-256 | Local file |
| --- | --- | --- |
| src/avatar_engine.h | 5a29db22b3b1a0a8e92d61359586396fc39eef5e2216dd32e69cca0990e95934 | avatar_engine.h |
| src/avatar_engine.cpp | 935ee9869b89cf52b94599b4d7c5332d67f55f8bc4eb9842187f3ca6f583b119 | avatar_engine.cpp |
| LICENSE | 8486a10c4393cee1c25392769ddd3b2d6c242d6ec7928e1414efff7dfb2f07ef | ../../../licenses/StopWatch-avatar-AGPL-3.0.txt |

Local modifications, 2026-10-15, Haoran Wang / Anan iPhone Use:
replace the M5Unified include with the native primitive/timing adapter; omit the
unused Arduino String serial-command parser; distinguish dirty clearing from
black eyelid primitives. Tables, interpolation, timeline, blink, projection and
rendering calculations otherwise retain upstream implementation. Local graphics
adapter and input-event mapping are separate files. No upstream main.cpp,
hardware power/display initialization or M5Stack library implementation is copied.
The touch interaction mapping follows upstream main.cpp as documented in the
[bilingual port scope](../../../docs/history/avatar-upstream.md). Upstream third-party notices are retained alongside this file.

Responsive follow-up, 2026-10-15: `updateInteraction` now uses a local analytic
critically damped follow calculation with actual elapsed time, faster eye/head
response and exponential attention blending. This replaces capped Euler updates
for gaze, head, shake and swipe return. Expression tables, blink, timeline and
projection remain upstream-derived and unchanged. No additional source imported.

简体中文：以上固定提交、原始哈希和逐文件来源适用于本目录。保留上游 AGPL
声明；本地修改仅替换图元/计时适配、移除未使用的 Arduino 串口命令解析，并区分
变化区清除与黑色眼睑图元。保留表情表、插值、时间轴、眨眼、投影与绘图计算。
没有复制上游主程序、供电/屏幕初始化或 M5Stack 库实现。触摸映射参考同版本
main.cpp，移植范围见[移植说明](../../../docs/history/avatar-upstream.zh-CN.md)。

2026-10-15 跟手修正：`updateInteraction` 使用本地解析临界阻尼跟随、真实帧间隔、
较快眼睛/头部响应及指数注意力过渡，替换目光、头部、摇动与滑动回位的截断欧拉
更新。表情表、眨眼、时间轴及投影保持上游实现；没有新增外部源码。
