# 仓库组织

[English](repository.md) · 简体中文

## 文件归属

| 位置 | 内容 |
| --- | --- |
| 根目录 `README*`、`CONTRIBUTING*`、`AGENTS.md` | 项目入口、贡献流程与仓库约束 |
| 根目录 `CMakeLists.txt`、`sdkconfig.defaults`、`partitions.csv`、`dependencies.lock` | 可复现的 ESP-IDF 构建配置与固定依赖，保留跟踪 |
| 根目录 `build.sh`、`flash.sh`、`flash.py`、`package.py` | 构建、显式烧录与打包入口；烧录辅助脚本在升级时跳过 BLE NVS |
| `main/` | 固件组件、头文件与嵌入页面 `web/setup.html`，见[模块职责](../main/README.zh-CN.md) |
| `docs/` | 双语使用指南、开发决策与验收流程，从[文档导航](README.zh-CN.md)进入 |
| `docs/images/setup/` | 项目 README 使用的精选合成 UI 示意图 |
| `tools/` | 主机端字体生成与静态 UI 渲染工具 |
| `artifacts/` | 按阶段归档的脱敏验证记录，见下文 |
| `licenses/`、根目录 `LICENSE`、`NOTICE`、`THIRD_PARTY*` | 项目许可、保留的上游条款与来源，见[来源声明](../THIRD_PARTY.zh-CN.md) |
| `.agents/skills/anan-iphone-use/` | Agent 引导配置 skill 及其元数据 |

当前小型固件集中在一个 `main/` 组件，通过模块职责表导航，无需改动 include
路径或构建输入。[PROTOCOL.md](../PROTOCOL.md) 保留在根目录，作为唯一的历史
ANAN1 契约；当前 MCP 行为见 [MCP 文档](mcp.zh-CN.md)。

## 开发工具

从仓库根目录执行：

| 命令 | 用途与输出 |
| --- | --- |
| `./build.sh` | 固定 ESP-IDF v5.5.5 构建，在被忽略的 `build/` 中生成合并镜像与 manifest |
| `python package.py build` | 合并已编译的构建并重建源码/镜像清单，不访问串口 |
| `python tools/render_ui.py` | 编译仅在主机运行的布局导出器，将合成预览写入被忽略的 `build/ui-previews/` |
| `python tools/generate_ui_font.py FONT_FILE` | 核对固定字体哈希，重建受跟踪的 `main/ui_font.h` |

UI 工具依赖 Pillow12.3.0，渲染还需要主机 C 编译器。字体来源与声明见
[THIRD_PARTY](../THIRD_PARTY.zh-CN.md)，预览流程见[触屏 UI](touch-ui.zh-CN.md)。
`render_ui_layout.c` 由渲染器调用，不编译进固件。生成的 `ui_font.h` 是固件输入，
保留跟踪；完整源字体保留在仓库之外。

打包源码清单包含 `main/` 下非隐藏的 `.c`、`.h`、`.md`、`.yml`、`.html` 文件和
`CMakeLists.txt`，以及 `package.py` 明确列出的根目录构建输入，排除隐藏文件与
Python 缓存。引入新的组件输入类型时，需同步更新清单筛选规则。

## 验证记录

| 目录 | 含义 |
| --- | --- |
| `artifacts/migration/` | 原始提取/源码哈希与迁移检查 |
| `artifacts/baseline/` | 基线准备与获授权的本地检查点 |
| `artifacts/mcp/` | MCP 候选构建检查、前序候选快照与脱敏用户反馈 |

现有记录对应各自的源码/镜像版本，后续整理应保留记录，不改写历史。特别是旧源码
清单可能包含 `.DS_Store`，当前打包筛选已排除它。`artifacts/mcp/build-check.json`
标识已记录的候选，不表示后续所有工作区修改均已重新构建或通过真机验证。
后续获授权的 E2E 记录遵循[验证流程](validation.zh-CN.md)。

## 跟踪文件与本地文件

[.gitignore](../.gitignore) 区分生成/本地文件和维护输入。忽略 `build/`、
`managed_components/`、生成的 `sdkconfig` 及备份、编译产物、Python/编辑器缓存、
原始日志与凭据/证书导出；保留 `sdkconfig.defaults`、`dependencies.lock`、源码
资源、合成文档图片与脱敏 JSON 证据。

临时截图、设备导出和其他本地工作文件放在被忽略的 `local/`；生成预览与构建结果
放在 `build/`。添加证据前先脱敏并检查 `git diff --cached`。忽略规则不会移除已经
跟踪的文件，也无法识别任意 JSON 或图片中的秘密。不要宽泛忽略 `*.json`、
`*.png`、`docs/` 或 `artifacts/`，以免隐藏需要维护的证据与文档。
