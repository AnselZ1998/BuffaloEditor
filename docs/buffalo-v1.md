# Buffalo v1：先看所有权，再看界面

## 启动和关闭

`src/app/editor_main.cpp` 只创建 `EditorApplication` 并调用 `Run()`。

`EditorApplication` 是总装配处：

```text
QApplication
  → EngineRuntime.StartUp()
  → EditorLog + SelectionService + EditorContext
  → ViewportController（借用 Runtime）
  → Viewport / Hierarchy / Inspector / Log Panels
  → EditorUI（纯 QMainWindow shell）
  → SetCentralWidget / AddDock
  → show / Qt event loop
  → 停止刷新、保存布局
  → 销毁 UI 和 Controller
  → EngineRuntime.ShutDown()
```

临时 `unique_ptr` 在组装时将 Panel 所有权交给 Qt 父子树。窗口先销毁 Panel，随后 Controller 结束生命周期，最后 Runtime 清理后端和场景。`EngineRuntime` 的关闭可重复调用，也支持重启；后端先释放，Scene 后释放。

## 两个正式 target

| Target | 职责 | 依赖 |
| --- | --- | --- |
| BuffaloRuntime（静态库） | EngineRuntime、SceneManager、Scene、Camera、RenderSystem、软件后端、既有 assets/math/render 代码 | C++ 标准库；不依赖 Qt、Editor、DisplayManager |
| BuffaloEditor（可执行） | Application、UI、services、viewport、panels、QSS/Logo 资源 | BuffaloRuntime + Qt Widgets |

`BUFFALO_BUILD_TESTS=ON` 仅额外生成测试可执行程序，不增加产品库。没有 DLL、插件系统或额外 EditorCore/EditorUI 静态库。

Runtime 根负责已有业务对象的生命周期：

```text
EngineRuntime
├─ SceneManager
│  └─ Scene → Camera / Model（保留既有职责）
└─ RenderSystem
   └─ unique_ptr<IRenderBackend>
      └─ SoftwareRenderBackend → 既有 SoftwareRenderer
```

`RenderSystem` 管理后端注册/选择和出图；`SoftwareRenderBackend` 把统一接口翻译成现有 SoftwareRenderer 调用。这里不显示窗口。

## 文件位置

头文件在 `include/`，实现文件在 `src/`，分类对称：

```text
core/EngineRuntime.h/.cpp
render/IRenderBackend.h
render/RenderFrame.h
render/RenderSystem.h/.cpp
render/software/SoftwareRenderBackend.h/.cpp
render/opengl/.gitkeep

editor/app/EditorApplication.h/.cpp
editor/ui/EditorUI.h/.cpp
editor/ui/EditorTheme.h/.cpp
editor/ui/widgets/.gitkeep
editor/services/EditorContext.h/.cpp
editor/services/EditorLog.h/.cpp
editor/services/SelectionService.h/.cpp
editor/viewport/IViewport.h
editor/viewport/InputEvent.h
editor/viewport/ViewportFrame.h
editor/viewport/ViewportShowFlags.h
editor/viewport/ViewportController.h/.cpp
editor/panels/viewport/ViewportPanel.h/.cpp
editor/panels/viewport/ViewportSurface.h/.cpp
editor/panels/viewport/ViewportToolbar.h/.cpp
editor/panels/hierarchy/HierarchyPanel.h/.cpp
editor/panels/inspector/InspectorPanel.h/.cpp
editor/panels/log/LogPanel.h/.cpp
```

纯数据头和接口不为了目录对称添加无意义的 `.cpp`。

## Viewport：屏幕、对讲机与调度员

- `ViewportPanel`：组织顶部选择器、左侧竖向工具栏和中央屏幕。
- `ViewportSurface`：接收 Qt 输入、复制 Runtime 图像、绘制画面和辅助网格。
- `IViewport`：对讲机协议，只提供输入意图、尺寸、图像、后端名称和显示开关。
- `ViewportController`：调度员，将输入变成相机操作，请 Runtime 出图。只借用 Runtime 和 Log。

鼠标/键盘 → IViewport → Controller → Scene Camera / RenderSystem → RenderFrame → ViewportSurface。

`RenderFrame` 属于 Runtime，`ViewportFrame` 是 Editor 中的别名。帧像素由后端拥有，下一次渲染/缩放/关闭之前，Qt 必须立即复制。现有 BGRA、行跨度、上下方向契约保留。CPU 分辨率上限仍为 1280 × 720。

`ViewportShowFlags::grid` 默认开启。工具栏修改 flags，Surface 使用 QPainter 在画面上叠加简单透视辅助网格。**v1 网格是屏幕空间辅助线，不跟随相机，也不做深度遮挡**，不代表真实世界地面；它不创建 Mesh、不影响 Scene bounds/picking，也不修改后端像素。

## UI、服务和样式

`EditorUI` 只处理 QMainWindow、菜单、toolbar、Dock、状态栏、布局保存/恢复；不创建任何具体 Panel，不引用 Runtime 或 Controller。刷新计时器放在 `EditorApplication`。

`EditorContext` 只有三个共享引用：Runtime、SelectionService、EditorLog；没有查找或注册机制。各 Panel 仅接收需要的服务，不拿整个 Context 到处寻找对象。

SelectionService 暂时只保存选择 token，Hierarchy 只显示已经加载的 Scene 根；Inspector 监听选择变化，属性编辑保持占位。两个 Panel 不直接调用对方，也不拥有场景对象。

EditorLog 是主线程使用的 200 条内存队列，通过 revision 更新 LogPanel。默认不打开或写入 `buffalo.log`。

`resources/themes/buffalo_dark.qss` 是统一深灰主题，由 EditorTheme 加载。面板专属样式可放到 `resources/themes/panels/<name>.qss`，将文件加入 CMake 的资源列表后调用 `EditorTheme::ApplyPanelStyle()`；当前只有一个主样式文件。

Logo 使用原图 `resources/icons/buffalo_logo.png`，接到窗口图标和品牌区域。当前会话未取得原 PNG，因此没有生成替代图。将原图放入该位置后重新配置/编译即可嵌入，也可放入可执行程序旁的同名相对路径供下次启动读取。

## 保留范围与限制

旧 `src/app/main.cpp`、`core/Engine.*`、`render/RenderManager.*`、`platform/windows/DisplayManager.*` 原样保留，作为 legacy 参考代码；不进入 v1 正式 target。

SoftwareRenderer、Rasterizer、math、SceneManager/Scene/Camera、assets（包括第三方 objParser）不改动。本机 `Scene.cpp` 原有绝对模型路径也保留，迁移到其他机器前需要另行处理资产路径。

OpenGL 只留空目录。没有新增模拟、动画、热加载、EventBus、Undo/Redo、RenderGraph 等模块。新增/显著重写的自有代码带 MIT + AnselZ 文件头；第三方许可证保持原样。

## 继续添加功能

简单功能可以直接写 `SomePanel : QWidget`；复杂功能再拆专用 Widget 和 Controller。Widget 接收数据、发 Qt signal，Controller 操作业务层。最后在 `EditorApplication` 创建并调用 `AddDock`。主窗口不需要知道这个模块如何工作。

推荐阅读顺序：`editor_main.cpp → EditorApplication.cpp → EngineRuntime → RenderSystem → IViewport → ViewportController → ViewportPanel/Surface/Toolbar`。
