# Scene 编辑器改造说明

## 使用

- Hierarchy 左上角 **+ Add** 可新增 Scene、空 Model、Camera、Directional Light。新 Scene 默认有一台 Camera 和一盏方向光。
- 点击 Scene 或其对象切换当前 Scene。模型前的复选框控制 visible；隐藏模型不参与渲染和 Frame All 包围盒。
- Inspector 按选择对象显示可折叠分组。输入后点击对应 Apply；资源路径也可通过文件选择器输入，留空并 Apply 清除资源。
- Model 支持位置、XYZ 欧拉角（度）、正缩放、OBJ 路径、Base Color、BaseColor/Normal 贴图，以及 BlinnPhong（保留原有 diffuse 行为）、Unlit、Normal debug。
- Camera 支持位置、目标、up、FOV、near/far、aspect。默认随 viewport 更新 aspect；取消 Match viewport aspect 使用手动值。Use Camera 设置当前 Scene 的活动 Camera。
- Light 支持开关、位置、方向、RGB、强度。Orbit debug 绕世界 Y 轴旋转，可设置 target、radius、height、speed（度/秒，支持负数）、phase。开关即时生效，其余参数通过 Apply Light 提交；下方显示实时位置/方向。
- 方向光没有距离衰减，位置用于 orbit/debug；真正影响光照的是方向。Orbit 开启时方向自动朝向 target，关闭后保留当前位置和方向。
- 日志面板的级别选择控制后续消息的最低记录级别，默认 Debug。

## 架构与生命周期

```text
EditorApplication
├── EditorLog : ILogger              （先构造，最后销毁）
├── EngineRuntime
│   ├── SceneManager → unique_ptr<Scene>[]
│   │   └── Scene → Model[] / Camera[] / ILight[]
│   ├── RenderSystem → IRenderBackend[]
│   └── Simulation + SceneSimulationBinding
├── SelectionService → SelectionTarget { kind, sceneId, objectId }
├── SceneEditorController             （创建、选择、校验、编辑）
│   └── HierarchyPanel / InspectorPanel
└── ViewportController                （导航、Frame All、后端与 show flags）
```

所有选择均通过 ID 重新解析，不保存对象地址；ID 在进程内单调分配，Scene 容器扩容不会使选择失效。Camera 复制只复制参数，保留接收对象的身份。

SceneEditorController 统一校验变更。无效 mesh/texture 先在临时资源中加载，失败时保留旧资源和路径；错误进入 EditorLog。OBJ 导入增加坏索引校验、面三角化及缺失 UV/normal 的默认值，避免可编辑路径引入越界访问。

Viewport 每帧只更新必要的 aspect/matrices，不再覆盖 Inspector 编辑的镜头参数。导航输入开始时从活动 Camera 读取导航状态，切 Scene/Camera 不会沿用前一个 Camera 的旧 target。Frame All 仍是显式的重新构图操作。

切 Scene 或编辑当前 Scene 的 mesh/transform/visible 时，暂停并重建现有仿真绑定，避免旧 Scene 的模型被继续推进。当前仿真仍沿用原有“第一个可见有效模型”的单刚体绑定。

ILogger 位于 core，无 Qt/Editor 依赖。EditorApplication 在 Runtime 启动前注入 EditorLog；Runtime、RenderSystem 借用 logger，可为空。Debug/Info/Warning/Error 经同一 sink 进入现有面板，当前调用仍在编辑器线程。

## 渲染改动边界

- 保留原有裁剪、光栅化、深度测试、后端/帧协议；Rasterizer.cpp/.h 未因本次改造修改。
- SoftwareRenderBackend 每帧传递方向光；MaterialShaderParameters 增加颜色、灯光与 normal-map TBN 参数。
- SoftwareRenderer 仅补充三角形 tangent/bitangent 和非均匀缩放需要的逆转置法线。
- Shader 替换固定光向，读取 Scene 灯光，增加 Unlit/Normal debug；原 BlinnPhong 名称保留，尚未增加 specular 分量。
- OpenGL 只扩展现有后端的 UV、材质纹理和灯光绑定。mesh/material revision 使资源替换可自动刷新 GPU 缓存；不改变已有 context/target 生命周期。
- 默认贴图路径相对默认模型目录解析，去掉 Model 内硬编码的 D 盘路径。

## QSS 与格式

```text
resources/themes/
├── buffalo_dark.qss                 窗口、菜单、dock、状态栏
├── controls/inputs.qss              按钮、输入框、spinbox、checkbox、滚动条
└── panels/
    ├── panels.qss                  Hierarchy、Inspector、Log、Simulation
    └── viewport/viewportbar.qss     Viewport 工具条扩展
```

EditorTheme 按 shell → controls → panels 加载，资源显式纳入 CMake。现代深色底色配暖金色选中态。修改过的 C++ 使用现有 .clang-format，Allman braces、单行函数签名、空格运算符；新增 LF 规则。保留许可证和原 OBJ 作者头，不重排未涉及的渲染核心文件。

## 验证

- Windows / MSVC 19.35 / Qt 6.8.3，Debug Editor 全量目标构建通过。
- CTest：simulation_contract、framework_contract、qt_panel_contract、editor_smoke，4/4 通过。
- 关闭 BUFFALO_BUILD_EDITOR 后，Runtime/Simulation/FrameworkTests 独立构建通过，CTest 2/2 通过。
- OpenGLTests 在 NVIDIA GeForce RTX 2070 / OpenGL 3.3 / driver 576.02 上通过，覆盖真实 GPU pixels、灯光开关、Shader、BaseColor/Normal 贴图更新、原地址 mesh 替换、显隐、resize 与资源释放。
- CPU/Qt 回归覆盖 typed selection、跨 Scene ID 拒绝、72 个 Scene、参数编辑、显隐双向同步、资源加载失败回退、orbit 时间推进、相机编辑保留、日志四级别、仿真场景切换。
- 原有 Rasterizer 中数值窄化警告保留，本次未为消除警告改动其流程。
- 已检查实际窗口截图；离屏 Qt 的字体替代与真实窗口不同。

## 剩余 TODO / 明确边界

1. Scene 保存/加载、跨进程持久化 ID、撤销/重做、删除/复制对象尚未加入本次范围。
2. OpenGL 当前每次绘制取前 64 盏启用的方向光；Software 不设这个固定上限。点光源/聚光灯、阴影、真正的 Blinn-Phong specular/PBR 留待后续。
3. 仿真仍仅绑定第一个可见有效 Model，并非多刚体编辑器。
4. Transform UI 限制正缩放，不提供负缩放、shear 和父子 transform。
5. Normal map 按 tangent-space 使用；OBJ 的缺失 UV 自动补零时不能提供有意义的法线贴图方向。
6. Inspector 是显式 Apply 工作流；Camera 导航和仿真变化后重新选择对象可刷新输入值。Light 提供单独的实时读数。
