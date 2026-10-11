# Cubiomes Viewer 26.3：原版 4.0.1 界面

这是按本机 `cubiomes-viewer-static-win.exe`（4.0.1）制作的独立兼容版。
直接复用 4.0.1 界面源码，接入支持 Minecraft Java 26.3 的引擎。

## 使用

解压整个压缩包，运行 `cubiomes-viewer-26.3-classic.exe`，选择版本 **26.3**。
请保留旁边的 DLL 和目录。无需 Java 或额外安装 Qt。
搜索、添加条件、相对位置、群系选择和地图操作沿用 4.0.1 的方式。
首次正常启动时会读取原版的语言、字体、主题和图标大小，存入本版本独立设置。
原版程序和设置不会被覆盖。

## 界面与内核

- 界面基线：Cubitect/cubiomes-viewer **4.0.1**，提交 `73f6f97`。
- **19 个 `.ui` 文件和 1 个 `.qss` 文件与原始源码逐字节一致**。
  SHA-256 清单：`validation/original-ui-sha256.json`。
- 使用 **Qt 5.15.2 / MinGW 8.1.0**；本机原版使用 Qt 5.15.3 / GCC 8.1.0。
  保留 Qt 5 的控件与布局方式；未使用上一版的 Qt 6 和 4.1.2 界面。
- 新增版本选项、新群系名称、试炼密室和废弃营地条目；既有界面翻译保留。
- 引擎：xpple/cubiomes `4f04235`，支持 26.3。
- 为适配旧代码，更新结构配置、维度字段、四联结构种子表接口；不调整窗口布局。
  引擎仅为 GCC 8 补上一个未使用的 C 参数名，没有改变生成算法。

## 已验证

7 组 CTest 全部通过：5 组引擎测试、Viewer 集成测试、原版 26.3 群系样本核对。
集成测试包括新增群系的包含/排除、废弃营地数量、条件保存/读取、实际双线程种子搜索、四联神殿筛选及中文窗口渲染。
8 个种子、不同地表/地下高度与两个尺度，共 **4,096 个群系样本**与 Minecraft Java 26.3 一致。
参考样本和生成探针源码在 `validation` 目录；只在临时验证服中使用探针，它会自动关闭该验证服。

这是非官方兼容版。上述为已完成的回归和抽样核对，不是所有种子与结构的穷尽验证。
结构预测仍继承 Cubiomes 的限制；决定开服前建议进游戏核实。带新条件的会话应另存。

## 源码及许可证

完整对应源码随包附带在 `source-4.0.1-mc26.3-classic1.zip`。

- Viewer：<https://github.com/Cubitect/cubiomes-viewer/tree/4.0.1>，GPL-3.0。
- 引擎：<https://github.com/xpple/cubiomes>，MIT。
- Lua 5.4.4：项目自带，MIT。
- Qt 5.15.2：动态链接，许可证文本在 `licenses`；对应源码：
  <https://download.qt.io/archive/qt/5.15/5.15.2/submodules/>。
- MinGW 8.1.0：运行库许可证在 `licenses/mingw`；构建项目：
  <https://github.com/niXman/mingw-builds>。

本移植使用新增 CMake 工程，历史 `.pro` 文件不适用于新引擎。
安装 CMake、Ninja、Qt 5.15.2 MinGW x64 和对应 MinGW 8.1 后执行：

```powershell
.\build-windows.ps1 -QtRoot 'C:\Qt\5.15.2\mingw81_64' -MinGwRoot 'C:\Qt\Tools\mingw810_64'
```

程序提供 `--verify-port <输出目录>` 运行集成检查，输出日志与截图。
