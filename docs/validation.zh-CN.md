# 迁移验证记录

[English](validation.md) · 简体中文

## 已定义验收范围

本次授权本地提取、文档、编译和静态检查，不含烧录、手机/HID/模型操作、模拟器/E2E 或发布。编码前定义检查：原运行/构建输入 SHA-256 不变、在 Anan 外独立编译打包、核对目标/偏移/Flash/锁/源码 manifest、检查英中链接/忽略规则、记录来源缺口。Anan 的 A–H 真机场景不自动获得本轮授权。

[source-snapshot.json](../artifacts/migration/source-snapshot.json) 标识原 16 个文件；原 12 个构建输入在提取时全部匹配历史镜像 manifest。运行源码、脚本、锁和协议行为不变。根 CMake 固定原先由 Anan Git 推导的 IDF 应用版本 `ec6bb29-dirty`，这是唯一构建配置调整。因为 package.py 会 hash main/ 所有文件，新增模块 README 会成为额外 manifest 条目，但不会编译进源码。

## 证据层级

| 证据 | 状态 |
| --- | --- |
| 历史镜像 | 455840 字节，SHA-256 `5e3e96b3bf2e1699c6406874de66a85e7efa1e3823ec83738f4971b0f2d8e230`，提取时核对源码 hash |
| 历史烧录 | 用户提供 esptool 写入/hash 成功输出，不是本次迁移真机执行 |
| 历史行为 | 用户报告以太网、静态 IP、连接、预览、observe 和手机输入可用 |
| 完整任务/持续后台 | 备忘录任务困难；Anan 后台改进有用户反馈，缺少完整固件验收轨迹 |
| 迁移构建/静态 | 命令/工具版本/结果见 [build-check.json](../artifacts/migration/build-check.json)，不是设备验收 |
| 来源 | 部分核查，USB 实现来源尚未确定 |
| 本地检查点 | 用户反馈重新烧录成功后明确授权；完整输入/停止验收与来源核实仍待完成 |

生成镜像、组件、sdkconfig 和带本机路径的构建日志忽略；可提交证据只保留脱敏结果/源码 hash，不含私人路径、串口、截图或凭据。不承诺跨机器二进制一致。

## 基线整理

整理证据见 [preparation-check.json](../artifacts/baseline/preparation-check.json)。
随后用户提供 esptool 成功写入/哈希校验输出，并明确要求首次本地提交。
脱敏当前范围见 [checkpoint.json](../artifacts/baseline/checkpoint.json)，
上面的迁移记录仍为历史证据。候选 manifest 生成在被忽略的 `build/`；
按[基线验收步骤](baseline-acceptance.zh-CN.md)对准确镜像哈希反馈结果。
手机输入/停止验收与来源核实仍待完成，不宣称稳定基线或已可公开发行。
剩余门槛见[开源发布准备](release-preparation.zh-CN.md)。

## 后续真机验收，另行授权

用迁移镜像 hash 和合成笔记验证：以太网/HID 枚举和 60 秒 READY/PONG、完整竖屏预览、仅 observe、单个 tap/ASCII/drag 及视觉确认、输入中 Stop/按钮/拔线且无新动作/重放、无 Mac多步闭环。按需记录板型版本/系统/客户端/模型，保留失败和未知局部效果。若批准 E2E，可复现脱敏证据存 `artifacts/e2e/<scenario>/<run-id>/`。执行前 deviceVerified=false。
