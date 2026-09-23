# Buffalo

C++ 渲染与模拟学习项目：Qt 编辑器、软件光栅、OpenGL 3.3 和最小 CPU 刚体模拟。
保留现有 Panel / Controller / Runtime / Scene 结构，不引入完整 RHI、多线程或通用物理引擎。

## 从哪里读

- [本轮架构与所有权](docs/simulation-and-rendering.md)：模块交接、算法范围和未来扩展位置。
- [学习入口与已知限制](docs/learning-guide.md)：阅读顺序、检查结果和暂未实现的功能。
- [v1 编辑器说明](docs/buffalo-v1.md)：历史版本的面板与依赖设计；新增部分以上面两份文档为准。
- [旧 Win32 归档](bak/README.md)：不参加当前构建，不影响软件光栅后端。

## 构建和启动

本机使用 Visual Studio 2022、CMake、Qt 6.8.3 MSVC x64，Qt 需包含 Widgets、OpenGL、OpenGLWidgets。

```powershell
cmake -S . -B build-editor -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Users/zhaijiafeng/Documents/Codex/Dependencies/Qt/6.8.3/msvc2022_64" -DBUFFALO_BUILD_TESTS=ON
cmake --build build-editor --config Debug --parallel
.\build-editor\Debug\BuffaloEditor.exe
```

VS Code：在 `.vscode/settings.json` 中调整 Qt 路径，Ctrl+Shift+B 构建编辑器，F5 启动。
调试下拉框也提供独立 CPU 模拟测试和真实 GPU 测试。运行任务 `CTest: regression` 执行回归。
使用统一的 `build-editor/`，不复用仍指向旧 SoftRasterLab 的构建缓存。

Simulation 与 Runtime 可独立构建；可选的 OpenGL 集成和 Editor 依赖 Runtime。
Runtime/Simulation 不依赖 Qt。只做无窗口学习可配置 `-DBUFFALO_BUILD_EDITOR=OFF`。
场景加载仓库内 `model/toy.obj`；构建系统记录源码路径，移动仓库后重新配置。

## 操作

- 渲染器下拉框：Software / OpenGL；设备初始化失败时保留软件渲染并记录日志。
- 左／右键拖动旋转，中键平移，滚轮缩放，WASD 平移，F 框选。
- 底部 Simulation：播放、暂停、单帧推进、重置及拖动时间游标。
- 时间条按 24 FPS 显示 0–240 帧，解算固定 120 Hz；回拖通过重置重算，拖动后仍暂停。
- 刚体演示仅平移和地面碰撞，使用现有模型。碰撞平面未绘制，Grid 是独立的屏幕辅助线。
- Window → Reset layout 恢复默认布局。Hierarchy / Inspector 的完整对象属性编辑尚未实现。

主题统一在 `resources/themes/buffalo_dark.qss`，新增时间条使用局部对象选择器，不全局重做面板样式。

## 验证

```powershell
ctest --test-dir build-editor -C Debug --output-on-failure
.\build-editor\Debug\OpenGLTests.exe
```

默认回归包含模拟数值、Runtime、Qt 面板与无窗口启动；真实 GPU 测试明确要求可用的桌面 OpenGL 上下文。
可用 `-DBUFFALO_TEST_OPENGL=ON` 将它加入 CTest。`OpenGLTests --capture 路径.png` 保存验证画面。
`BuffaloEditor --capture-preview` 保存编辑器截图并退出；临时验证不保存用户布局。

## 代码规范

遵循现有 `.clang-format`：四空格、Allman 大括号、指针／引用靠类型、禁止单行控制流。
仅整理本轮新增或触及的代码，不批量重排数学库、第三方 OBJ 解析器或软件光栅算法。
新增自有代码沿用 MIT / AnselZ 标记，第三方作者与许可保留。
