# 学习入口与局部检查记录

## 建议的阅读路线

1. `src/app/editor_main.cpp` → `EditorApplication`：窗口组成和每轮更新的顺序。
2. `core/EngineRuntime`：组合渲染与模拟，并管理启动/关闭。
3. `simulation/SimulationSystem` → `IRigidBodySolver` → `CpuRigidBodySolver`：固定时间步与算法分离。
4. `core/SceneSimulationBinding`：唯一了解 Scene 和模拟结果的连接代码。
5. `ViewportController` → `RenderSystem` → `SoftwareRenderBackend` / `OpenGLRenderBackend`。
6. `RenderFrame` → `ViewportSurface` / `OpenGLFramePresenter`：CPU 拷贝和 GPU 纹理显示的区别。
7. `SimulationPanel`：时间游标重算只使用 Runtime 的现有控制方法，不把解算算法写进 UI。

## 本轮修正与整理

- Scene 默认资产原先指向失效的旧项目目录；改由 CMake 指向仓库内模型。
- `Material` 原先持有无释放路径的 Shader 裸指针；改成独占所有权，不改变着色算法。
- 老测试假设 Grid 默认开启，但当前产品默认关闭；测试跟随现状，并继续检查开关不污染渲染数据。
- 新增 OpenGL 共享上下文、shader 编译错误、GPU 输出与资源生命周期的验证。
- 时间控制独立于渲染帧率；模拟不依赖 Qt、Scene 或 OpenGL，宿主读取显式发生。
- 旧 Win32 入口和调度器归档至 `bak/legacy-win32/`，保留路径映射；未删模型或思维导图。
- VS Code 统一使用新构建目录，提供编辑器、模拟测试、GPU 测试的调试入口。

## 现在仍然有意保持简单的地方

| 项目 | 当前边界 |
|---|---|
| 刚体 | 只有平移、重力和水平地面接触；不是完整六自由度引擎 |
| 时间条 | 当前 CPU 小示例通过重置重算回拖；没有状态缓存、倒放或通用录制 |
| 布料/流体 | 布料留目录说明；尚无解算器或 GPU 加速实现 |
| 光照 | CPU/GLSL 各自实现基础 Lambert 漫反射；旧 BlinnPhong 类名不代表已实现高光 |
| GPU 同步 | 单线程，两端 glFinish；避免 CPU 图像回读，但未实现异步帧流水线 |
| 材质 | 仍保留现有 CPU Material；公共 PBR 材质描述与 shader 编译系统待后续真实需求 |
| 动态网格 | GPU 缓存适用于静态网格；改几何前显式清缓存，布料需要再加入增量上传 |
| 场景切换 | Binding 借用 Model 指针；切场景前必须解除绑定，当前 UI 不提供热切场景 |
| 模拟时钟 | 一次最多追 8 个 1/120 秒子步，慢帧会丢墙钟时间；离线实验请按步数运行 |
| 数值范围 | 普通有限参数已验证；极端浮点溢出、完整能量分析不属于第一版能力 |
| 外观/辅助线 | 编辑器 Grid 无世界坐标与深度，不是模拟碰撞平面 |

借鉴 UE 主要是所有权和模块边界，而非预先复制大量基类。
下一步建议对照软件和 GPU 管线研究裁切/剔除，再添加公共材质参数，之后做 CPU 布料。
不要把“保留扩展点”理解为未来 CUDA、Taichi、Vulkan、WebGL 能零修改接入。
