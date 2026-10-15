# 源码与安装

[English](SOURCE.md) · 简体中文

配置页的 **Firmware source / 固件源码** 链接从 `/source` 下载本次构建的项目
源码包，无需配对或外部账户。包含可编辑固件、引入的表情/BMI270/QR 源码、生成
的字形源码、构建配置/脚本、依赖锁和许可证；排除设备配置、Wi-Fi 密码、配对数据
和生成的 SDK 配置。构建时按明确白名单生成，发行镜像时应一起保存匹配的源码包。
`package.py` 记录其 SHA-256，拒绝打包过期源码；这不是新的 MCP 工具。

构建依赖按记录的版本另行获取：

- ESP-IDF v5.5.5，提交 `b774170ff46c393eeb5e495ea37936038d3f4f4f`：
  https://github.com/espressif/esp-idf/tree/b774170ff46c393eeb5e495ea37936038d3f4f4f
  递归克隆子模块，执行 `./install.sh esp32s3`，再 source `./export.sh`。
  SDK 库使用其锁定子模块，并保留各自许可。
- IDF 组件管理器根据 `main/idf_component.yml` 获取 `dependencies.lock` 中的
  精确版本/哈希；CO5300、TinyUSB 等锁定组件源码可从
  https://components.espressif.com/ 获取。
- 在解压后的项目根执行 `bash build.sh`，调用 `idf.py build` 与
  `python package.py build`，不打开串口。
- 安装：`PYTHON=python ./flash.sh <串口> build/anan-stopwatch-flash.bin`。
  仅适用 16MB 闪存的 StopWatch ESP32-S3。脚本保留 NVS/配对存储并核对镜像
  manifest；固件不要求签名密钥或激活锁。设备配置与控制仍需用户实体确认。

原 GPL-3.0-only 代码保留许可；新引入的表情引擎及原生移植为 AGPL-3.0-or-later。
组合采用 GPLv3/AGPLv3 第 13 节，AGPL 网络源码提供要求适用于组合体；保留两份
完整文本与逐文件许可/声明。版本、修改及历史来源缺口见 `NOTICE`、`THIRD_PARTY.md`。
源码提供不表示既有衍生关系或二进制链接通知审计已完成；SDK 无线二进制库适用
其自身条款，不宣称其实现源码开放。
