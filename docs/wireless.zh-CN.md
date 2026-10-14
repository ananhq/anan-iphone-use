# Wi-Fi MCP 与 BLE 输入

[English](wireless.md) · 简体中文

候选 **0.7.2-wifi-qr** 在先前 Wi-Fi/BLE 版本上增加带屏幕 Wi-Fi 二维码的临时热点配网、五个
网络记忆，并修正仅 IPv4 时的原生 mDNS 探测，保留 USB。用户反馈前一无线流程成功，但没有独立核对的
镜像哈希及完整验收记录。本候选仅有编译/离线证据，见[预定义场景](wireless-setup-development.zh-CN.md)。

## 连接方式

| Agent → 设备 | 设备 → iPhone | 用途 |
| --- | --- | --- |
| USB 网络 | USB HID | 保留有线路径 |
| Wi-Fi HTTP MCP | USB HID | 电脑/手机 Agent 访问插在手机上的板子 |
| Wi-Fi HTTP MCP | BLE HID | 无线路径 |
| USB 网络 | BLE HID | 将 BLE 与 Wi-Fi 分开验证 |

七个 MCP 工具和竖屏归一化坐标不变，在设备选择 USB/BLE 输入。客户端负责截图
和效果确认，板子不接收 AirPlay，也不运行 Agent。输入方式与 MCP 网络通道独立。

## 首次配网不用连接手机数据线

首次烧录固件仍需要电脑和数据线。安装本候选后，手机配网可以使用设备热点：

1. 没有保存网络时，StopWatch 自动开启临时 **StopWatch-setup** 热点，显示
   配对页。之后可从“**设置 → 输入 / 网络 → 无线配网 / 管理Wi-Fi**”手动开启。
   停止输入、暂时关闭 BLE，但保留输入选择及绑定。热点十分钟后或退出配置时
   关闭，只供配网，不提供互联网。
2. 用手机相机扫描 StopWatch 的 Wi-Fi 二维码，选择加入。二维码包含随机 WPA2
   密码；仍可手动选择 **StopWatch-setup** 并输入屏幕密码。DHCP 自动分配地址，
   不需填写 IP、掩码、DNS。iOS 若提示无互联网，配置时保持连接。在 Safari
   打开 **http://192.168.4.1**。二维码用于加入热点，随后自行打开 Safari；本版不自动弹出门户。
3. 网页开始配对，核对六位数字，在 StopWatch 确认。此步骤授权网页配置会话，
   不开启手机控制；不需安装证书。密码和 Bearer 配置保留在用户自己的设备上。
4. 扫描附近 **2.4 GHz** Wi-Fi，点选网络并粘贴一次密码；隐藏网络仍可手填。
   保存 Wi-Fi 后，设备尝试连接，同时保留配置热点。射频切换信道可能暂时中断
   网页连接，必要时重新连同一热点并刷新。浏览器不存储密码，接口不返回密码。
5. **关闭热点前先生成并复制/下载 MCP 配置**。HTTP 导出地址统一为
   **http://stopwatch.local/mcp**，包含原 Bearer 请求头。保存网络后即使尚在连接
   也可导出，但导出不证明网络已连通。等网页显示 Wi-Fi 地址再完成；始终连不上
   时先检查密码或重试。
6. 在 StopWatch 点“**已导入 返回**”（或返回）关闭热点。手机重新加入日常
   Wi-Fi，再将已复制的配置导入 [Anan](https://anan.sh) 或其他 HTTP MCP 客户端。
   之前选择 BLE 才恢复 BLE，控制仍关闭。手机/电脑与设备需处于同一可互访
   IPv4 局域网。
7. 无线输入选择“**设置 → 输入 / 网络 → BLE**”，在 iPhone 配对 **Anan
   StopWatch** 并开启 AssistiveTouch。返回首页，输入连接后手动“开始控制”。
   只支持一台已绑定手机。

热点是配置入口，不是路由器或常驻 MCP/BLE 工作模式。手机个人热点、需网页登录
的 Wi-Fi、企业认证不作为已验收能力。与 `192.168.7.0/24` 或 `192.168.4.0/24`
重叠的局域网会被拒绝。

## USB 配置与恢复

既有[手机 USB 配置](setup.zh-CN.md)及 `http://192.168.7.1` 网页保留。在设备
进入“设置 → 连接MCP应用”并物理配对后，可扫描、保存或管理 Wi-Fi。退出配置
也关闭当前热点。可选 HTTPS 和证书导出仍仅支持 USB，地址
`https://192.168.7.1:8443/mcp`。

USB/Wi-Fi 默认 HTTP 导出共用 `stopwatch.local`。DHCP 地址改变或 USB/BLE
输入切换不需要重新配置 MCP；旧数字 IP 配置需更新一次才能使用固定域名。
断连后客户端可能需重新连接并初始化 MCP 会话，与重新导入配置是两件事。

设备在各本地接口回应 mDNS，由手机/电脑系统解析。这不是公网 DNS。访客隔离、
组播过滤、VPN 路由或客户端解析方式可能阻止访问。解析失败仍可用数字 IP 作为
端点/配置入口，但该配置不会跟随 DHCP 地址改变。同时连接 USB/Wi-Fi 的解析及
Anan 重连表现仍待实机验收。同一局域网先使用一台 StopWatch；名称冲突通过
`/status` 报告并阻止固定域名导出，本版不自动加后缀或修改已有客户端地址。

## 网络记忆与换场所

- `mcp_kv` 保存最多 **5 个** SSID/密码，自动读取旧的单网络格式。保存同名网络
  更新密码并排到首位；第六个不同网络拒绝保存，需要先删除一个，不静默覆盖。
- 从最近保存的网络开始，断连/失败或超过十五秒连接期限后，轮换尝试其他记录，
  尝试间隔一秒；DHCP、射频和事件调度会增加耗时。当前成功的选择保留在内存，
  启动顺序不按信号强度排序。回到已保存场所不需再输密码；新场所使用相同热点
  流程。已知网络暂时不可用时，不自动开启热点。
- 仅物理配对后的 USB/热点网页可管理网络名称，使用“删除选中网络 / 忘记全部
  Wi-Fi”。`/wifi/networks` 不返回密码。MCP 身份、网络记忆和 BLE 绑定分别存储。
- 网络重连、退出配置、BLE 重连均不恢复控制授权或重放动作，USB/BLE 仍需明确
  选择。停止、选定输入断连、正在使用的 LAN 断连及重启都会撤销控制。
- BLE 绑定使用默认 `nvs`，重启及通过当前 `flash.sh` / `flash.py` 升级均保留。
  从 `0x0` 连续写入合并镜像仍会清掉绑定，请使用[烧录辅助脚本](setup.zh-CN.md)。
  已被擦除或不一致的绑定需“按住2秒重置蓝牙”、手机忘记设备后重新配对一次。
  升级保留 `mcp_kv`，不恢复控制授权。

## 实现与证据

Host/Origin 接受实际数字设备地址或 `stopwatch.local`；携带 Origin 时必须与
Host 完全一致。USB 仍要求手机地址 `192.168.7.2`；STA/热点要求实际 IPv4
子网的对端。热点拒绝 MCP，只提供配置；USB/STA 提供认证后的 MCP。HTTPS
仍为数字地址 USB-only，不加入互联网转发、中转或客户端修改。mDNS 使用锁定
SDK 的 lwIP，通过核对哈希的构建适配，在没有有效 link-local IPv6 地址时跳过
IPv6 探测。旧函数一直等待两种地址族均成功，可能无法回答 IPv4 查询；离线
场景用实际 SDK 定时函数复现并验证修正，尚非手机端已确认诊断。二维码使用
固定 Nayuki v1.8.0 C 源码并保留 MIT 通知。

扫描从最强的四十条记录保留最多二十个不同可显示名称，排除隐藏/不支持的类型，
仅返回 SSID、RSSI、开放标记。异步扫描暂停连接尝试，与写入串行；等待扫描
最多八秒，中止完成另最多一秒，SDK 启动/调度另计。AP+STA 扫描及 BLE 共存待实测。

加载 ESP-IDF **v5.5.5** 后构建/打包，不访问硬件：

```sh
idf.py -B build/wifi-qr-candidate -D SDKCONFIG=build/wifi-qr-candidate/sdkconfig build
python package.py build/wifi-qr-candidate
bash tools/check_wireless.sh
bash tools/check_hid.sh
```

使用对应镜像和 manifest，不复用旧 sdkconfig。[当前记录](../artifacts/wireless/qr-build-check.json)
区分编译/离线证据与设备验收。保留[前一无线记录](../artifacts/wireless/build-check.json)
及[扫描记录](../artifacts/wireless/scan-build-check.json)。本轮 Agent 未烧录、探测
射频/设备、操作手机、调用模型或执行 E2E。

保留[前一配网记录](../artifacts/wireless/setup-build-check.json)。二维码生成图及完整圆屏合成预览均由 ZXing-C++ 2.3.0 独立解码成功；手机相机实际识别/加入及 `.local` 解析仍待实机检查。

静态配网预览（仅示例密码，不是真机截图）：

![热点扫码配网](images/setup/zh-pair-hotspot.png)
