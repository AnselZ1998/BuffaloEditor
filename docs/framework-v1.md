> 历史版本说明：本文保留此前迁移记录。当前 Buffalo v1 的目录、启动链和扩展方式以 [Buffalo v1 架构](buffalo-v1.md) 为准。

# Buffalo Qt 编辑器框架

## 职责

```text
QtEditor：组装 QApplication、Log、ViewportController、EditorUI
    ↓
EditorUI：QMainWindow + 中央视口 + 三个 QDockWidget
    ├─ ViewportPanel：Renderer 下拉框、Frame all、ViewportSurface
    ├─ HierarchyPanel：对象列表占位
    ├─ InspectorPanel：属性区域占位
    └─ LogPanel：读取现有 Log

ViewportSurface → IViewport → ViewportController
                                  ├─ 原 SceneManager → Scene → Camera / Models
                                  └─ IRenderer → SoftwareRendererAdapter → SoftwareRenderer
```

Scene 继续拥有真实场景数据。Panel 只负责界面与输入映射。
UI 没有新增另一套相机、对象树、控制器或事件总线。

## UI 如何传回输入

Widget 只创建一次。Qt 收到鼠标、键盘或控件事件后调用对应函数：

```cpp
connect(frameAll, &QPushButton::clicked, this, [&viewport]()
{
    viewport.OnInput({InputAction::FrameAll});
});
```

`ViewportSurface::mouseMoveEvent()` 把拖动转换为 Orbit / Pan。
`wheelEvent()` 转换为 Zoom，键盘事件记录当前按住的 WASD。
每次刷新按经过的时间发送平移量；失去焦点、隐藏或结束拖动时发送 NavigationEnd。
控制器负责原有相机计算，Panel 不接触 Camera。

## 渲染结果如何返回界面

EditorUI 的 QTimer 定期调用 `ViewportPanel::Refresh()`，随后刷新 LogPanel。
ViewportSurface 根据实际显示大小调用 `SetSize()`，再调用 `RenderFrame()`。

```text
RenderFrame → 借用的 ViewportFrame → QImage.copy() → paintEvent / QPainter
```

`ViewportFrame` 仍约定 BGRA8、从上到下、明确的行跨度。
Windows 上 `QImage::Format_ARGB32` 对应 BGRA 字节顺序，直接传入 `rowPitchBytes`。
必须立即 copy：原指针的存储仍属于渲染器，下一次渲染或 resize 后可能失效。
Qt 重绘只读复制后的图像，不持有引擎像素裸指针。

全部在 UI 线程同步执行；当前保留原有软件渲染尺寸上限，没有引入工作线程。
空帧清除旧图像，隐藏或最小化后不再调用渲染。

## 添加功能的位置

- 新面板继承 QWidget，在 EditorUI 中创建，放进 QDockWidget，并加入 CMake 源文件列表。
- 旧的 `IPanel::Draw()` 接口已移除，由 Qt 对象树管理控件和生命周期。
- 新渲染后端继续实现 IRenderer，并在 QtEditor 的组装处调用 `controller.AddRenderer(...)`；在创建 EditorUI 前注册。
- Hierarchy 与 Inspector 当前只有空界面。以后增加选择或属性控制时，再把对应接口传入 Panel。
- LogPanel 使用 Log::Lines()/Clear()，不复制一套业务日志系统；200 条上限由现有 Log 决定。

## 保留与变更

原 Renderer、SceneManager、Camera、RenderManager、数学与资源处理的算法均保留。
ViewportController / SoftwareRendererAdapter 仅将命名空间改为 `buffalo::editor`。
Scene 的模型路径改由 CMake 提供；独立 Win32 窗口标题更新为 Buffalo Engine。

原 WindowsEditor、ImGui、D3D11ViewportPresenter 和 FrameCapture 不再参与项目。
QApplication 管理事件循环；QSettings 保存窗口布局，QWidget::grab() 保存预览截图。

FrameworkTests 验证原相机、真实模型像素、后端切换、尺寸和图像契约。
QtPanelTests 验证颜色与行跨度、像素复制、鼠标键盘、焦点切换、隐藏恢复、日志与停靠布局。
