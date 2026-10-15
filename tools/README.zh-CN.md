# 开发工具

[English](README.md) · 简体中文

从仓库根目录执行。以下检查不连接设备、手机、模拟器或模型。需要 SDK 源码的
无线场景应先设置 `IDF_PATH`；目标栈检查使用候选构建的编译器。

| 命令 | 用途 |
| --- | --- |
| `bash tools/check_ui.sh` | 界面、停止、电量、表情、IMU 与渲染检查 |
| `bash tools/check_hid.sh` | 现有 HID 解析与 worker 场景 |
| `bash tools/check_wireless.sh` | 现有 BLE、Wi-Fi、HTTP、内存与重启场景 |
| `python tools/check_flashing.py` | 合成的保留配置升级检查，不访问串口 |
| `python tools/ui_acceptance/source-bundle.py` | 源码归档内容与源码提供检查 |
| `python tools/check_ui_stack.py BUILD_DIR` | 目标编译器/libc 栈预算，需要已编译候选 |
| `python tools/render_ui.py` | 双语合成预览，写入被忽略的 build/ui-previews/ |
| `python tools/generate_ui_font.py FONT_FILE` | 从哈希核对后的上游字体重建 ui/ui_font.h |

`ui_acceptance/`、`hid_acceptance/`、`wireless_acceptance/` 保留现有宿主场景。
`check_upstream.sh`、`check_activity.sh`、`check_responsive.sh` 由 UI 总入口调用。
`firmware_paths.py/.sh` 统一宿主头文件路径，`compile_avatar_host.py` 复用原生表情编译；
`render_ui_layout.c` 和 `render_eyes.c` 仅用于文档预览。预览/字体工具需要 Pillow12.3.0。

`source_bundle.py` 供固件构建与打包生成确定性源码归档。`build/` 下产物不纳入版本控制；
现有脱敏证据保留在 `artifacts/`，真机结果与离线检查分开记录。详见
[仓库组织](../docs/repository.zh-CN.md)与[验证记录](../docs/validation.zh-CN.md)。
