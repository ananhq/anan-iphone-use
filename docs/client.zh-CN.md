# 客户端接入

[English](client.md) · 简体中文

服务是 USB 以太网上的原始 TCP **192.168.7.1:28766**，不是 HTTP/MCP。客户端绑定 USB 本地地址/接口（手机 192.168.7.2/24）。权威语法与闭集见 [PROTOCOL.md](../PROTOCOL.md)。

1. 客户端先取得用户操控许可，再连接并等 `ANAN1 READY`（Anan 超时 8 秒）。
2. 同连接每 300ms 发送 `PING\n`。独立解析 `ANAN1 PONG` 与动作回执；串行写入，等待动作时保持接收循环。
3. 分配递增非零 UInt32 ID，同时只允许一个动作。限时等待对应 `DONE`/`ERROR`，PONG 不是动作结果。
4. 停止/取消/超时/EOF 时尽力发送 `STOP\n`，关闭连接、撤销许可并说明可能的局部效果；不自动重连/重发。
5. 输入前后观察手机屏幕；只有用户显式操作才重新连接/授权。

以下仅是行格式示例（真实发送会产生输入；文档不执行）：

```text
1 TAP 32768 32768
2 DRAG 32768 50000 32768 16000 500
3 KEY 40 0
4 TYPE 416e616e205553422074657374
```

最后一行输入 `Anan USB test`。客户端按既有舍入规则把归一化坐标映射为 0…65535，设备映射为 0…32767。保持竖屏并真机验证绝对指针对应；key 40 表示 Return，不是字符编码 40。

Anan 映射：`phone.tap`→TAP、`phone.drag`→DRAG、`phone.key`→KEY、`phone.type_ascii`→TYPE。`phone.observe` 在手机 ScreenCaptureKit 客户端执行，不发送固件命令。Agent、模型网络、采屏、单 Turn 许可、附件和灵动岛仍归 [Anan](https://github.com/ananhq/anan)；迁移时接入实现未提交，不能保证某个公开 tag 已包含。

手机内客户端可访问直连 USB 子网；电脑/远程 Agent 需要明确桥接路径与独立截图来源，未来 MCP 也不会自动提供这些能力。尚未验证通用 Agent 接入或 Android 采屏。不要把未认证的输入端口暴露到物理 USB 信任边界之外。
