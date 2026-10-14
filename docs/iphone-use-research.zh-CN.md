# iPhone Use 实现方案调研：四个项目的原理、能力与限制

调研日期：2026-10-14。用途：交给其他 session 撰写 X / Twitter 对比内容，本文是研究材料，不是推文草稿。

## 阅读范围与证据

覆盖四个指定项目，按执行机制归为两类：**UI 测试框架驱动**与 **USB HID 输入驱动**。这是四个代表性实现的对比，不是全部方案的目录，也没有市场份额证据支持“最主流”排名。

本次只做源码和文档静态检查，没有安装运行其他项目，没有调用模型，也没有进行手机或硬件测试。下文“已实现”表示固定版本的源码存在相应实现，不等于所有设备、App、系统版本上均已验证。上游演示、作者说明、本仓库用户反馈与独立实测分开表述；不做速度或成功率排名。

| 项目 | 本次检查的源码版本 | 定位 |
| --- | --- | --- |
| [zhongerxin/iPhone-use](https://github.com/zhongerxin/iPhone-use) | [`5cc2fa8`](https://github.com/zhongerxin/iPhone-use/tree/5cc2fa8808187797d73aea2167104f370dc29724) | 将 WDA 包装为电脑 Agent 可调用的 MCP 工具 |
| [OpenMinis/iCTRL](https://github.com/OpenMinis/iCTRL) | [`99dbfec`](https://github.com/OpenMinis/iCTRL/tree/99dbfecdb39836ba26a0b2b0dcfde90f705a6835) | 手机上的自定义 XCUITest Runner，提供 JSON-RPC 自动化接口 |
| [onehumanbeing/esp32-iphone-agent](https://github.com/onehumanbeing/esp32-iphone-agent) | [`1f6a51a`](https://github.com/onehumanbeing/esp32-iphone-agent/tree/1f6a51aa3d7bdb3f12024693a559cd5b5f2b9c28) | 电脑视觉 Agent + AirPlay + ESP32 USB 鼠标键盘 |
| [ananhq/anan-iphone-use](https://github.com/ananhq/anan-iphone-use) | [`c794813`](https://github.com/ananhq/anan-iphone-use/tree/c794813f960b139089a1d4c72106e5c46c265b61)，固件 `0.4.0-mcp` | ESP32 MCP 输入执行器；与手机端 [Anan](https://anan.sh) 组成完整闭环 |

上述版本由当日 GitHub API 查询默认分支 HEAD 固定；这不保证用户下载的 Release 或已安装版本与 HEAD 相同。四个仓库在本次查询中均为 public。本仓库部分历史路线图仍写 private，是旧状态；固件仓库公开也不表示 [Anan](https://anan.sh) 客户端已经开源。

## 先分清四层

1. **Agent 在哪里运行**：任务循环、工具调用、计划和状态管理在电脑还是手机上？模型推理则可以另在云端。
2. **如何观察手机**：读取原生可访问性元素、截图、AirPlay，还是手机端系统屏幕共享？
3. **如何执行动作**：通过 XCTest UI 自动化注入触摸，还是作为外接鼠标、键盘发送 HID 报告？
4. **如何连接**：MCP、JSON-RPC 和 TCP 行协议是客户端到执行器的接口；它们本身不提供截图或系统控制权限。

WDA（WebDriverAgent）与 XCUITest 不是两种平行的底层引擎。XCUITest 通常指 XCTest 的 UI 测试体系；WDA 是建立在这个体系上的 HTTP 服务。WDA 接收请求，调用 Apple 的 UI 测试 API 完成动作或读取界面。iCTRL 则自行实现 Runner 和 RPC 服务，并不需要经过 WDA。[W1]

USB HID 路线使用 iPhone 已支持的外接输入设备能力。ESP32 枚举为鼠标、键盘，iOS 通过 AssistiveTouch 接收指针操作；它不需要在执行链中调用 WDA 或 XCUITest。Apple 官方说明支持通过 USB 或蓝牙连接指针设备。[A1]

## 横向对比

表内“需要电脑”分首次准备与日常运行；“手机端 Agent”指编排逻辑的位置，不代表模型离线运行。

| 维度 | iPhone-use | iCTRL | esp32-iphone-agent | anan-iphone-use + [Anan](https://anan.sh) |
| --- | --- | --- | --- | --- |
| 输入执行机制 | WDA → XCTest UI 自动化 | 自定义 Runner → XCTest UI 自动化；部分动作使用私有事件合成接口 | USB 相对鼠标 + 键盘 | USB 绝对鼠标 + 键盘 |
| Agent 在哪里 | 外部电脑客户端，如 Codex；项目提供工具 | 外部客户端，可在电脑或同一手机；Runner 本身不是 Agent | 项目自带 Python Planner / Actor 在电脑 | [Anan](https://anan.sh) Agent 在 iPhone；固件不运行 Agent |
| 首次电脑工作 | Mac + Xcode，签名、构建、安装、启动 WDA，设置 MCP 与 USB 转发 | 依安装路线完成签名/侧载；可用 Xcode，或 SideStore 等路线；不是所有步骤都必需 Mac | 刷 ESP32，安装 Python 服务、UxPlay / GStreamer，配置 Wi-Fi 与模型 | 构建/刷入 StopWatch 固件；安装客户端，完成手机配置与配对 |
| 日常电脑工作 | 当前项目启动方案维持 MCP、USB 转发和 `xcodebuild` 测试进程 | 不要求电脑常驻执行器；同机客户端可连 loopback，外部客户端可经网络连接 | 电脑必须运行视觉 Agent、投屏接收器和板卡 TCP 服务 | 当前手机端组合运行时不需要电脑；换成桌面 Agent 则需另提供观察与网络路径 |
| 手机上安装什么 | 开发签名的 WebDriverAgentRunner | `iCTRLUITests-Runner.ipa`；辅助 `iCTRL.ipa` / DDI 工具按路线选择 | 自动化路径不需安装手机辅助 App；开启 AirPlay 镜像和 AssistiveTouch | 安装 [Anan](https://anan.sh)；系统屏幕共享、AssistiveTouch、手动 USB Ethernet 配置 |
| 观察能力 | 原生元素信息 + 截图 + 实时 MJPEG 预览 | 原生元素信息 + 截图 + 设备端 OCR + 视频流 | AirPlay 像素画面 | 客户端 `phone.observe` 返回屏幕图像；ESP32 没有截图工具 |
| 原生组件树 | 有；上游 XML 被整理为模型可用节点列表 | 有；`page_source` 与元素 `ref` 操作 | 当前没有 | 当前没有 |
| 动作目标 | 元素 selector 或坐标 | 元素 `ref` / 查询或坐标 | 视觉坐标，经相对指针校准执行 | 归一化坐标 `0…1`，经绝对指针执行 |
| 文字输入 | 工具接受 Unicode，经 WDA 输入 | XCTest 文本事件接受 Unicode | 当前 ASCII | `type_ascii` 当前 US 布局 ASCII，每次 1…120 字节 |
| 客户端接口 | stdio MCP | HTTP JSON-RPC 2.0，配对后的加密封装；不是 MCP | Web UI / HTTP API；板卡 Wi-Fi TCP 行命令 | ESP32 上 Streamable HTTP MCP；USB NCM 传输网络数据 |
| 额外硬件 | 手机与电脑连接线 | 无 ESP32 需求；安装方式可能需要线缆 | 原生 USB Device 可用的 ESP32-S3、线缆/转接器 | 当前默认 M5Stack StopWatch（ESP32-S3）、线缆 |
| 核心部署负担 | Xcode / 签名 / 测试运行与 USB 会话 | 签名 / 嵌套测试包 / Developer Mode / DDI 与 Runner 生命周期 | AirPlay、局域网、Wi-Fi、指针校准和硬件 | 刷固件、USB 网络、屏幕共享、配对与物理控制授权 |

各行依据见后续项目章节。这里比较项目当前交付的路径，不把整个 WDA 生态限定为“永远只能 USB”，也不把整个 HID 生态限定为“永远只有 ASCII”。

## 1. zhongerxin/iPhone-use

### 安装位置与分工

**电脑**：运行外部 Agent 客户端、Python stdio MCP 服务和 Node.js USB 转发程序。首次通过完整 Xcode 及 Apple Development 签名构建、安装 WDA；运行阶段使用 `xcodebuild test-without-building` 启动测试。当前源码固定使用 Appium WebDriverAgent `16.14.0`，提交 `d17782422d55ff1e5e0ceb74eb1fd509cc0c35b6`。[I1] [I2]

**iPhone**：运行开发签名的 WebDriverAgentRunner，WDA HTTP 服务和 XCTest UI 自动化代码在手机上执行。目标 App 不需要集成这个项目的 SDK。电脑负责编译、启动和通信，不代表 WDA / UI 测试代码都在电脑上运行。[I2] [W2]

**USB**：`appium-ios-device` 把电脑本地 TCP 端口转接到手机 WDA 端口。源码默认电脑 `18100` → 手机 `8100`。这是传输 WDA 请求的 USB 端口转发，不是模拟鼠标键盘，也不是本仓库的 USB Ethernet。[I3]

```text
电脑 Agent
  → Python MCP server（pua_* 工具）
  → WDA HTTP 请求
  → appium-ios-device 的 USB 转发
  → iPhone 上的 WebDriverAgentRunner
  → XCTest UI 自动化
  → 目标 App
```

项目直接访问 WDA，当前调用链不需要另起一个完整 Appium Server。依赖里有 Appium 维护的库，不等于整个 Appium 服务都在执行链中。[I1] [I3]

### 观察和动作如何实现

- `PhoneController.tree()` 请求 WDA `/source?format=xml`，解析原生可访问性快照，提取类型、label/name/value、矩形等信息。模型看到的 `nodes` 是压缩后的节点列表，并非完整原始父子树。`observe()` 默认 `tree`，也支持 `screenshot` 和 `both`。[I4]
- `predicate()` 将 selector 转成 NSPredicate；查询通过 WDA `/elements`。元素点击最终走 `/element/{id}/click`，坐标点击走 `/wda/tap`。所以这个项目并非只有“截图 + 猜坐标”。[I4]
- 截图来自 WDA `/screenshot`。实时预览另通过 USB 连接手机 `9100` 端口读取 WDA MJPEG 流；不是 AirPlay。截图像素与操作坐标的 iPhone points 需要正确换算。[I4] [I5]

### 当前主要能力

已有 MCP 工具覆盖元素查找/点击、坐标点击、滑动、Unicode 文本输入、启动 App、按系统按钮、等待/观察和批量动作，以及环境诊断、安装启动与运行状态恢复。预览界面还能接收用户的点击、拖动。[I1]

这类方案的主要优势是**语义定位和视觉定位可以组合**：可访问性信息充分时，模型能通过按钮文字或类型选择元素；自绘内容或快照不足时，可查看截图并使用坐标。源码还提供观察上下文检查、可命中性检查和“不重放未知结果动作”的恢复约束。[I1] [I4]

### 当前限制与代价

- 当前打包的安装、启动路径依赖 Mac、完整 Xcode、签名配置与手机测试环境；日常流程里电脑也承担 MCP、转发和测试进程。这是该项目实现的限制，不是所有 XCTest 方案都绝对如此。[I2]
- 可访问性快照不是 App 的全部内部控件、数据或 DOM。自绘内容、状态切换、重复标签、不可命中元素，以及节点数量/深度限制都可能使语义定位失败。默认过滤结果也不能证明屏幕外内容不存在。[I4]
- 软件键盘、焦点、锁屏、系统确认等仍影响动作；支持 Unicode 输入不代表可以任意填写所有安全输入框。工具自身将文本输入限定为可编辑的非安全字段。[I1]
- 环境恢复不等于动作成功验证。服务恢复后仍应重新观察，不能把 MCP 返回成功直接当作业务任务完成。[I1] [I4]

## 2. OpenMinis/iCTRL

### 安装位置与分工

**iPhone**：真正的自动化引擎是 `iCTRLUITests-Runner.ipa`。其测试类继承 `XCTestCase`，`testStartServer()` 在手机启动 `NWListener` HTTP 服务，默认端口 `8200`；收到 RPC 后交给动作处理器。辅助 `iCTRL.ipa` 提供管理/DDI 等功能，不应将它与 Runner 混为一谈。[C1] [C2] [C3]

**首次准备**：需要正确签名并安装 IPA，包含嵌套的 `.xctest` 测试包；开启 Developer Mode，并建立可运行 UI 测试的 Developer Disk Image（DDI）环境。上游提供 Xcode、SideStore 等不同路线；没有 Mac 时也提供设备端 DDI 挂载路线。SideStore 首次安装仍涉及电脑，不能把“无 Mac 路线”概括成“完全不需要任何准备设备”。[C3]

**运行阶段**：Agent 可以在外部电脑，也可以是同机客户端（上游以 Minis 为例）。Runner 默认仅对本机开放；开启 Remote Access 后才允许其他设备访问。操作引擎在手机，因此不需要电脑一直充当 WDA 转发器或动作执行进程。具体客户端还必须支持它的配对、加密封装及 JSON-RPC。[C1] [C2] [C3]

```text
电脑上的 Agent 或手机上的客户端 Agent
  → HTTP JSON-RPC（已配对、加密封装）
  → iPhone 上的 iCTRLUITests-Runner
  → ActionHandler / FastActionHandler
  → XCTest UI 自动化与 testmanagerd 等测试基础设施
  → 目标 App
```

它绕过的是 **WDA 这一层服务**，没有绕过底层 UI 测试环境。`FastActionHandler.fastTap()` 创建 `XCSynthesizedEventRecord` 和 `XCPointerEventPath`，再执行 `synthesize()`；相关私有接口在桥接头中声明。[C4] [C5]

### 观察和动作如何实现

- `page_source` 从原生可访问性快照生成页面描述，并提供元素引用；`find_element`、`tap_element`、`find_and_tap` 支持元素级定位和操作。也支持普通坐标点击、拖动、滑动。[C6]
- 截图来自设备端 `XCUIScreen` 等测试截图接口。`screen_text` 使用 Vision OCR；OCR 文本不是原生控件树。视频流实现中使用 VideoToolbox H.264 编码。[C6] [C7]
- 文本输入可走 XCTest 文本事件；`fastTypeText()` 接收 Swift `String`，当前不受两个 HID 项目的 ASCII 工具限制。[C4]
- `/schema` 与 `/skill` 提供接口说明，`/pair` 建立授权，`/rpc` 执行请求。原生接口是 JSON-RPC 2.0，不能直接当作 HTTP MCP 填进任意 MCP 客户端；接 MCP 需要客户端适配层。[C2]

### 当前主要能力

已有实现涵盖截图/视频、页面元素和 OCR、元素查找与点击、点击/双击/长按、滑动/滚动/拖动、文字输入与清空、启动/切换/终止 App、打开 URL、Home 等设备操作，以及等待、诊断与日志。[C6]

优势是**手机上就能提供具有语义信息的自动化服务**，调用者可在同机或其他设备；不局限于电脑 USB WDA 工作流。已有配对授权、远程访问开关和客户端自描述入口。[C1] [C2]

### 当前限制与代价

- 仍依赖开发签名、嵌套测试包的正确签名、Developer Mode 与 DDI。上游说明设备重启后需重新挂载 DDI；免费签名还涉及周期性刷新。这些前提使它不是“普通 App 安装后即可永久控制所有 App”的路径。[C3]
- 使用 XCTest 私有接口和测试基础设施，对系统/工具链版本变化存在维护负担。这是基于依赖的架构判断；本次没有实际测定跨版本兼容性。[C4] [C5]
- 原生快照可能缺失内容，元素引用和焦点也会变化。尤其该版本对浏览器页面有明确的树内容不足提示，并建议截图 + 坐标；不能把这一条推广为所有 XCUITest 项目都永远读不到网页内容。[C6]
- 同机 Agent 可以接入，不等于任意 Agent App 在后台都可无限期运行。Runner、调用方和屏幕/应用切换各自的生命周期仍需真机验证。本次没有验证 [Anan](https://anan.sh) 与 iCTRL 集成或后台续航。

## 3. onehumanbeing/esp32-iphone-agent

### 安装位置与分工

**电脑（macOS 或 Linux）**：运行 Python / FastAPI 服务、自带 Planner / Actor Agent、模型 API 客户端、UxPlay AirPlay 接收器和 GStreamer 图像管线。模型可以用云端 OpenAI-compatible 视觉接口；自托管模型是可选方向。[E1] [E2] [E3]

**ESP32-S3**：刷入 Arduino 固件，通过原生 USB 模拟鼠标和键盘，同时通过 Wi-Fi 主动连接电脑 TCP 服务。固件使用 Arduino ESP32 的 `USBHIDMouse`、`USBHIDKeyboard` 和 TinyUSB 相关能力。它接收行命令并执行输入，不运行视觉模型或任务计划。[E4] [E5]

**iPhone**：开启 AssistiveTouch，将屏幕 AirPlay 镜像到电脑，接收 ESP32 的 USB 鼠标键盘输入。该自动化路径无需在手机安装测试 Runner；USB-C / Lightning 转接方式按设备选择。上游列出的已测板卡是 Seeed XIAO ESP32S3；其他板卡不能仅凭芯片名称认定完全适配。[E1] [E4]

```text
观察：iPhone → AirPlay → 电脑 UxPlay / GStreamer → JPEG → 视觉模型
动作：电脑 Planner / Actor → Wi-Fi TCP 行命令 → ESP32-S3 → USB HID → iPhone
```

“No Mac”在这里表示可使用 Linux 电脑，**不表示没有电脑**。当前 Agent、投屏接收器和板卡服务都运行在电脑。[E2] [E3] [E5]

### 观察和动作如何实现

- `Screen._supervise()` 启动 UxPlay，用 GStreamer 把投屏变成 JPEG 帧；Agent 读取的是像素画面，当前没有系统原生组件树查询。[E2] [E3]
- Planner 根据任务和画面生成计划，Actor 根据新画面选择动作。动作支持 tap、swipe、path/circle、type、key、home/back 等。[E3]
- 鼠标是**相对指针**。`Phone` 保存指针位置与校准结果，把目标坐标变成 `move dx dy`；需要时通过晃动指针和画面差分检测真实位置，再纠偏。精确校准并非每一次点击都无条件进行：当前 `tap` 的 `aim` 默认 false，Agent 在 `precise` 或前次疑似漏点时开启。[E6] [E7]
- Agent 比较动作前后的画面、标记是否变化，重复无效动作时重新计划。画面变化是反馈信号，不等于业务结果的独立证明。[E3] [E7]

### 当前主要能力

这个项目直接附带电脑视觉 Agent：已有 Web UI、模型接入、手动控制、自动/手动指针校准、截图步骤记录、连续路径绘图、点击/拖拽/键盘输入，以及断线和错误后的恢复逻辑。[E3] [E5] [E6]

它的优势是**不依赖手机开发签名和 Xcode UI 测试环境来执行输入**，像素识别也能用于可访问性信息不足的画布。连续 `path` 绘图能力不能直接等同于本仓库当前的两点直线 `drag` 工具。[E3] [E4] [E6]

### 当前限制与代价

- 当前需要电脑常驻，手机 AirPlay 和板卡 Wi-Fi / TCP 链路都要可用；单独插上 ESP32 不会自动变成完整 Agent。[E2] [E3] [E5]
- 相对指针会受追踪速度和校准影响；校准与必要的视觉纠偏本身有额外步骤。没有同条件测量，不能据此给出与绝对鼠标的速度/成功率排名。[E6]
- 当前键盘输入仅 ASCII；坐标/校准以竖屏为前提，横屏和非 ASCII 输入仍列在上游路线图。[E1] [E3] [E4]
- 上游记录锁屏会中断镜像，部分密码、Apple Pay、DRM 等画面不可镜像。USB 暂停时固件可尝试唤醒或返回失败，不能由此宣称能自动解锁或绕过系统验证。[E1] [E4]
- 没有原生元素语义；图片模型可能误判，画面未变化也可能是合法结果。当前读到的板卡 TCP 路径是明文行命令，没有 iCTRL 的配对加密封装或本仓库的 Bearer MCP 认证；应按实际部署网络评估，不能直接把这个演示桥描述为可公开暴露的服务。[E4] [E5]

## 4. ananhq/anan-iphone-use + Anan

### 安装位置与分工

**电脑**：构建/刷写 ESP-IDF 固件、准备安装。当前手机端组合的日常观察、Agent 编排与工具调用不依赖电脑常驻。[N1] [N2]

**M5Stack StopWatch / ESP32-S3**：运行固件和 HTTP MCP server。通过同一根 USB 线枚举为 **NCM 网络设备 + HID 输入设备**。NCM 是虚拟 Ethernet，用来传 MCP 请求；HID 携带绝对鼠标、键盘报告，用来执行输入。当前单个 HID 接口通过不同 report ID 同时提供鼠标与键盘，不是每次切换一次设备模式。[N3]

**iPhone**：安装 [Anan](https://anan.sh)，由它通过系统屏幕共享观察屏幕，运行 Agent 编排、访问模型并调用 ESP32 MCP。手机需开启 AssistiveTouch、配置 USB Ethernet、完成配对/配置导入，再在 StopWatch 上启用控制。屏幕共享需要用户确认；当前文档和已检查的客户端观察代码以 iOS 27+、竖屏、支持图像的模型为前提。[N1] [N2] [N4]

```text
观察：iPhone 系统屏幕共享 → 手机上的 Anan → 图像模型（可在云端）
动作：手机上的 Anan → MCP over USB NCM → StopWatch → USB HID → iPhone
复核：Anan 再次观察手机屏幕，判断动作效果
```

这里的“纯客户端 Agent”指 Agent 客户端运行在手机，不表示 ESP32 运行模型，也不表示模型一定在手机离线推理。屏幕在手机获取后，是否传给远端模型由客户端和模型配置决定。[N1] [N2]

### 观察和动作如何实现

- `phone.observe` 属于 [Anan](https://anan.sh) 原生工具，返回共享屏幕图像。它不属于 ESP32 MCP，固件没有截图、OCR、元素查询或 Agent 实现。[N2] [N4]
- `main/usb.c` 定义 TinyUSB 绝对鼠标和键盘报告。鼠标报告直接给出位置；`main/actions.c` 把 tap / drag 转为移动、按下、路径插值、释放，把 ASCII 字符映射为 US 键盘 usages。[N3] [N5]
- 当前 MCP 有六个工具：`device_status`、`tap`、`drag`、`key`、`type_ascii`、`stop`。`drag` 是两点间的直线插值，并没有 esp32-iphone-agent 那样的任意多点路径工具。[N6]
- MCP 服务启动与输入授权分开。固件开机初始化 MCP，但控制默认禁用；配对有设备确认，实际输入需要物理启用。停止、USB 断开或重启会撤销控制，重连不自动恢复授权，未知结果不隐式重放。[N4] [N7]

### 当前主要能力

已实现设备侧 MCP、配对/配置导出、状态和停止、归一化坐标点击/直线拖动、单次按键及有界 ASCII 输入；StopWatch 有触屏设置、控制入口与中英文 UI。[N4] [N6]

已有用户录制的三个场景：在备忘录画小狗、查看小红书粉丝数、开启勿扰模式并启动 25 分钟计时器。视频作为场景演示见 [README Demo](https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/README.md#demos-and-setup)。本次没有逐帧复核或重新执行，不把视频等同于整个固件版本的回归验收。[N1] [N8]

这套组合最明确的差异是**观察与 Agent 在手机、标准 MCP 执行器在 ESP32**，运行时可以不借助电脑投屏接收器和动作服务。固件也欢迎其他 Agent，但调用者需要自己提供观察、授权和到 USB 子网的连接能力。[N1] [N2]

### 当前限制与代价

- 当前完整硬件实现聚焦 M5Stack StopWatch。仓库提供 [anan-iphone-use skill](../.agents/skills/anan-iphone-use/SKILL.md) 与板型适配说明，欢迎其他 ESP32 设备，包括无屏设备；这不是“所有 ESP32 即插即用”。芯片原生 USB Device 能力、端口线路、电源、控制和显示/无屏授权方案都需要确认。[N9]
- 没有原生组件树，也没有元素 ID 点击。当前依赖客户端视觉判断和坐标；绝对 HID 不会自动获得按钮语义，也不能保证 App 响应。[N2] [N3] [N6]
- 当前 `type_ascii` 仅 US 布局 ASCII，每次 1…120 字节；没有 Unicode 文本注入、IME 或任意多点轨迹工具。[N6]
- 手机侧需要 USB Ethernet 手动配置、配对和物理启用，并非插线后无交互自动控制。观察受客户端屏幕共享/系统版本和生命周期条件约束；屏幕可读性与长时间后台运行没有被本次验证。[N1] [N4] [N8]
- 桌面或云端 Agent 不能因为“支持 MCP”就直接访问手机 USB 子网。当前固件没有替它们搭建网络桥，也没有 Wi-Fi / BLE 控制路径。[N2] [N4]
- MCP 返回值说明输入执行情况，不能证明目标 App 完成任务。仓库仍区分编译检查、用户反馈与版本化真机验收，最新 MCP-only 固件的完整回归、其他客户端互操作和续航待验证。[N8]

## 组件树问题：哪些是现有能力，哪些只是可选方向

原生组件树来自 iOS UI 测试/可访问性快照接口，不是鼠标、键盘协议附带的信息。HID 可以有输入、输出或 feature 报告，但它没有标准的“返回当前 iPhone 控件树”接口；本次两个 HID 项目的源码也未实现这类通道。[E4] [N3]

| 想法 | 能得到什么 | 当前状态与代价 |
| --- | --- | --- |
| 直接读 WDA / iCTRL 页面源 | 原生可访问性节点、文本、位置和元素定位 | 两个测试框架项目已有；将其加入 HID 方案会引入 Runner、签名和测试环境 |
| 截图 + OCR / UI 检测，生成节点列表 | 推测的文字、按钮框与坐标 | 可作为视觉层方向；不是系统原生树，本仓库未实现 |
| Tab / Shift-Tab 等键盘焦点导航 | 系统条件允许时按焦点移动和激活控件 | 不会返回整棵组件树；本仓库未验证该工作流 |
| WDA 读树 + HID 执行动作的混合方案 | 分开选择观察与执行机制 | 架构上可组合，但不再省掉 WDA 的部署前提；没有在本仓库实现或验证 |

因此文章应同时比较“观察”和“执行”。有原生树的方案仍可做坐标操作；纯视觉方案也能组织推测的节点，但两种“节点”不能被当作相同证据。

## 给后续写作 session 的使用说明

可以围绕三个轴组织对比：**测试框架 vs 外接输入、原生元素 vs 像素观察、电脑常驻 vs 手机端闭环**。技术链路、组件部署和当前工具能力可以从本文提炼；引用具体实现时保留项目名和原始链接。

以下表述容易失真，写作时需保留限定：

- “WDA 在电脑上”：电脑负责构建、启动、转发；手机运行 WDA Runner 和 UI 测试代码。
- “iCTRL 不用 WDA，所以不依赖 XCTest”：它有自己的 XCTest Runner 与测试环境。
- “不用 Mac = 不用电脑”：esp32-iphone-agent 支持 Linux，但当前仍需电脑常驻。
- “所有 XCTest 方案都要电脑常驻”：iCTRL 的同机/无线运行路线是反例，但安装、签名和 DDI 前提仍在。
- “手机端 Agent = 手机本地模型 / 不上传截图”：Agent 位置与模型位置、图像数据去向是不同问题。
- “USB HID 可以返回原生组件树”：当前没有；视觉推测节点不能写成系统树。
- “HID 能操作任何 App / 绕过验证码、支付或 Face ID”：像素与输入可用性不等于所有任务可完成，也没有绕过系统验证的证据。
- “支持 MCP = 任意 Agent 都能立即接入”：还要匹配传输、会话、认证、网络可达性和观察能力。
- “本项目支持任意 ESP32 / 自动开机控制”：当前 StopWatch 是默认实现，其他板卡需适配；服务开机启动与控制授权是两件事。
- “哪个最快、成功率最高、稳定后台运行”：本次没有统一模型、设备和任务的对照测试，不能排名。

如需发布时态的“当前”“已开源”或兼容性断言，应再查 HEAD、Release、客户端状态和目标系统。本文不包含 App Store 分发结论，也没有测定四者的功耗、延迟或长期稳定性。

## 源码证据索引

以下 GitHub 代码链接固定到本次检查版本；文件与函数名是重新核查的入口。只总结与链接，没有将上游实现代码复制到本仓库。

### 公共机制

- [W1] [Appium WebDriverAgent 官方说明](https://github.com/appium/WebDriverAgent#webdriveragent)：链接 XCTest 并调用 Apple API；WDA 与测试框架关系。
- [W2] [WDA `UITestingUITests.m`](https://github.com/appium/WebDriverAgent/blob/d17782422d55ff1e5e0ceb74eb1fd509cc0c35b6/WebDriverAgentRunner/UITestingUITests.m)：`testRunner` 启动 `FBWebServer`。
- [A1] [Apple：通过 AssistiveTouch 使用指针设备](https://support.apple.com/en-us/111775)：iPhone 外接指针设备支持。

### iPhone-use

- [I1] [`server/iphone_use.py`](https://github.com/zhongerxin/iPhone-use/blob/5cc2fa8808187797d73aea2167104f370dc29724/server/iphone_use.py)：`SCHEMAS`、`TOOLS`、`Runtime` 与 stdio MCP 分发；动作、Unicode 输入和恢复约束。
- [I2] [`server/wda_setup.py`](https://github.com/zhongerxin/iPhone-use/blob/5cc2fa8808187797d73aea2167104f370dc29724/server/wda_setup.py)：`WDA_COMMIT` / `WDA_VERSION`、`_build_command`、`build-for-testing` / `test-without-building`。
- [I3] [`tooling/forward.mjs`](https://github.com/zhongerxin/iPhone-use/blob/5cc2fa8808187797d73aea2167104f370dc29724/tooling/forward.mjs)：`net.createServer`、`iosDevice.utilities.connectPort`、双向 pipe。
- [I4] [`server/wda_controller.py`](https://github.com/zhongerxin/iPhone-use/blob/5cc2fa8808187797d73aea2167104f370dc29724/server/wda_controller.py)：`predicate`、`compact_node`、`tree`、`observe`、`capture`、`query`、`tap`。
- [I5] [`tooling/screen-stream.mjs`](https://github.com/zhongerxin/iPhone-use/blob/5cc2fa8808187797d73aea2167104f370dc29724/tooling/screen-stream.mjs)：独立 WDA MJPEG USB 转发。

### iCTRL

- [C1] [`iCTRLUITests/iCTRLUITests.swift`](https://github.com/OpenMinis/iCTRL/blob/99dbfecdb39836ba26a0b2b0dcfde90f705a6835/iCTRLUITests/iCTRLUITests.swift)：`XCTestCase`、`testStartServer`、`configureServerCallbacks`、Remote Access 与 loopback 选择。
- [C2] [`iCTRLKit/JSONRPCServer.swift`](https://github.com/OpenMinis/iCTRL/blob/99dbfecdb39836ba26a0b2b0dcfde90f705a6835/iCTRLKit/JSONRPCServer.swift)：`NWListener`，`/schema`、`/skill`、`/pair`、`/rpc` 与 sealed envelope 验证。
- [C3] [固定版本 README](https://github.com/OpenMinis/iCTRL/blob/99dbfecdb39836ba26a0b2b0dcfde90f705a6835/README.md)：安装前提、签名路线、DDI、Developer Mode、同机/远程客户端。
- [C4] [`iCTRLUITests/FastActionHandler.swift`](https://github.com/OpenMinis/iCTRL/blob/99dbfecdb39836ba26a0b2b0dcfde90f705a6835/iCTRLUITests/FastActionHandler.swift)：`fastTap`、`fastTypeText`、事件合成。
- [C5] [`iCTRLUITests/XCTestPrivate-Bridging-Header.h`](https://github.com/OpenMinis/iCTRL/blob/99dbfecdb39836ba26a0b2b0dcfde90f705a6835/iCTRLUITests/XCTestPrivate-Bridging-Header.h)：私有事件类型声明。
- [C6] [`iCTRLUITests/ActionHandler.swift`](https://github.com/OpenMinis/iCTRL/blob/99dbfecdb39836ba26a0b2b0dcfde90f705a6835/iCTRLUITests/ActionHandler.swift)：`dispatchInner`、`pageSource`、截图/OCR、元素引用和浏览器提示；网页限制是本版本自身说明。
- [C7] [`iCTRLUITests/VideoStreamer.swift`](https://github.com/OpenMinis/iCTRL/blob/99dbfecdb39836ba26a0b2b0dcfde90f705a6835/iCTRLUITests/VideoStreamer.swift)：VideoToolbox H.264 编码实现。

### esp32-iphone-agent

- [E1] [固定版本 README](https://github.com/onehumanbeing/esp32-iphone-agent/blob/1f6a51aa3d7bdb3f12024693a559cd5b5f2b9c28/README.md)：安装、已测板卡、系统设置、上游记录的镜像限制及路线图。
- [E2] [`server/hidagent/screen.py`](https://github.com/onehumanbeing/esp32-iphone-agent/blob/1f6a51aa3d7bdb3f12024693a559cd5b5f2b9c28/server/hidagent/screen.py)：`Screen._supervise`、UxPlay 和 GStreamer JPEG 管线。
- [E3] [`server/hidagent/agent.py`](https://github.com/onehumanbeing/esp32-iphone-agent/blob/1f6a51aa3d7bdb3f12024693a559cd5b5f2b9c28/server/hidagent/agent.py)：Planner / Actor、`_step`、`_do`、`_repeating`、`precise` 与画面反馈。
- [E4] [`hardware/hid_bridge/hid_bridge.ino`](https://github.com/onehumanbeing/esp32-iphone-agent/blob/1f6a51aa3d7bdb3f12024693a559cd5b5f2b9c28/hardware/hid_bridge/hid_bridge.ino)：Arduino HID、相对 `move`、ASCII `type`、Wi-Fi TCP、USB 暂停处理和重连。
- [E5] [`server/hidagent/device.py`](https://github.com/onehumanbeing/esp32-iphone-agent/blob/1f6a51aa3d7bdb3f12024693a559cd5b5f2b9c28/server/hidagent/device.py) / [`app.py`](https://github.com/onehumanbeing/esp32-iphone-agent/blob/1f6a51aa3d7bdb3f12024693a559cd5b5f2b9c28/server/hidagent/app.py)：板卡 TCP 服务、行命令、HTTP UI 与 Agent 初始化。
- [E6] [`server/hidagent/phone.py`](https://github.com/onehumanbeing/esp32-iphone-agent/blob/1f6a51aa3d7bdb3f12024693a559cd5b5f2b9c28/server/hidagent/phone.py)：`start_calibration`、`_aim`、`tap`、`drag`、相对指针位置管理。
- [E7] [`server/hidagent/vision.py`](https://github.com/onehumanbeing/esp32-iphone-agent/blob/1f6a51aa3d7bdb3f12024693a559cd5b5f2b9c28/server/hidagent/vision.py)：`changed` 与 `pointer` 画面差分。

### anan-iphone-use

- [N1] [固定版本 README](https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/README.md)：架构、配置前提、iOS 27+ 与三段演示。
- [N2] [`docs/client.md`](https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/docs/client.md)：观察/推理/执行分工、客户端要求和独立观察。
- [N3] [`main/usb.c`](https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/main/usb.c)：HID report descriptors、NCM、`bridge_mouse` / `bridge_key`。
- [N4] [`docs/mcp.md`](https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/docs/mcp.md)：配对、Bearer、物理授权、Streamable HTTP 与运行约束。
- [N5] [`main/actions.c`](https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/main/actions.c)：`ascii_key`、`stroke`、`execute`、两点 drag 插值。
- [N6] [`main/mcp_tools.h`](https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/main/mcp_tools.h)：六个 MCP 工具与参数边界。
- [N7] [`main/main.c`](https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/main/main.c) / [`main/device.c`](https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/main/device.c)：开机初始化、控制 grant、停止与断线撤销。
- [N8] [`docs/validation.md`](https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/docs/validation.md)：编译、用户反馈与真机验收的证据边界。
- [N9] [`docs/porting.md`](https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/docs/porting.md) / [安装与适配 skill](../.agents/skills/anan-iphone-use/SKILL.md)：其他板卡最小适配、原生 USB 与无屏设备要求。

补充：本次只读复查了独立客户端源码 `apps/ios/Anan/Tools/Phone/PhoneTools.swift` 与 `PhoneScreenCapture.swift`，确认 `phone.observe`、iOS 27 可用性声明、系统共享确认与图像输出；它们不在本固件仓库中，客户端尚未公开的源码不作为读者可访问的链接。可公开引用的组合能力以 [N1] [N2] 与演示为依据；不能据此宣称客户端已开源。

[W1]: https://github.com/appium/WebDriverAgent#webdriveragent
[W2]: https://github.com/appium/WebDriverAgent/blob/d17782422d55ff1e5e0ceb74eb1fd509cc0c35b6/WebDriverAgentRunner/UITestingUITests.m
[A1]: https://support.apple.com/en-us/111775
[I1]: https://github.com/zhongerxin/iPhone-use/blob/5cc2fa8808187797d73aea2167104f370dc29724/server/iphone_use.py
[I2]: https://github.com/zhongerxin/iPhone-use/blob/5cc2fa8808187797d73aea2167104f370dc29724/server/wda_setup.py
[I3]: https://github.com/zhongerxin/iPhone-use/blob/5cc2fa8808187797d73aea2167104f370dc29724/tooling/forward.mjs
[I4]: https://github.com/zhongerxin/iPhone-use/blob/5cc2fa8808187797d73aea2167104f370dc29724/server/wda_controller.py
[I5]: https://github.com/zhongerxin/iPhone-use/blob/5cc2fa8808187797d73aea2167104f370dc29724/tooling/screen-stream.mjs
[C1]: https://github.com/OpenMinis/iCTRL/blob/99dbfecdb39836ba26a0b2b0dcfde90f705a6835/iCTRLUITests/iCTRLUITests.swift
[C2]: https://github.com/OpenMinis/iCTRL/blob/99dbfecdb39836ba26a0b2b0dcfde90f705a6835/iCTRLKit/JSONRPCServer.swift
[C3]: https://github.com/OpenMinis/iCTRL/blob/99dbfecdb39836ba26a0b2b0dcfde90f705a6835/README.md
[C4]: https://github.com/OpenMinis/iCTRL/blob/99dbfecdb39836ba26a0b2b0dcfde90f705a6835/iCTRLUITests/FastActionHandler.swift
[C5]: https://github.com/OpenMinis/iCTRL/blob/99dbfecdb39836ba26a0b2b0dcfde90f705a6835/iCTRLUITests/XCTestPrivate-Bridging-Header.h
[C6]: https://github.com/OpenMinis/iCTRL/blob/99dbfecdb39836ba26a0b2b0dcfde90f705a6835/iCTRLUITests/ActionHandler.swift
[C7]: https://github.com/OpenMinis/iCTRL/blob/99dbfecdb39836ba26a0b2b0dcfde90f705a6835/iCTRLUITests/VideoStreamer.swift
[E1]: https://github.com/onehumanbeing/esp32-iphone-agent/blob/1f6a51aa3d7bdb3f12024693a559cd5b5f2b9c28/README.md
[E2]: https://github.com/onehumanbeing/esp32-iphone-agent/blob/1f6a51aa3d7bdb3f12024693a559cd5b5f2b9c28/server/hidagent/screen.py
[E3]: https://github.com/onehumanbeing/esp32-iphone-agent/blob/1f6a51aa3d7bdb3f12024693a559cd5b5f2b9c28/server/hidagent/agent.py
[E4]: https://github.com/onehumanbeing/esp32-iphone-agent/blob/1f6a51aa3d7bdb3f12024693a559cd5b5f2b9c28/hardware/hid_bridge/hid_bridge.ino
[E5]: https://github.com/onehumanbeing/esp32-iphone-agent/blob/1f6a51aa3d7bdb3f12024693a559cd5b5f2b9c28/server/hidagent/device.py
[E6]: https://github.com/onehumanbeing/esp32-iphone-agent/blob/1f6a51aa3d7bdb3f12024693a559cd5b5f2b9c28/server/hidagent/phone.py
[E7]: https://github.com/onehumanbeing/esp32-iphone-agent/blob/1f6a51aa3d7bdb3f12024693a559cd5b5f2b9c28/server/hidagent/vision.py
[N1]: https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/README.md
[N2]: https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/docs/client.md
[N3]: https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/main/usb.c
[N4]: https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/docs/mcp.md
[N5]: https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/main/actions.c
[N6]: https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/main/mcp_tools.h
[N7]: https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/main/main.c
[N8]: https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/docs/validation.md
[N9]: https://github.com/ananhq/anan-iphone-use/blob/c794813f960b139089a1d4c72106e5c46c265b61/docs/porting.md
