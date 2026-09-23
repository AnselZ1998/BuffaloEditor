# 渲染与模拟：最小学习框架

这次保留 Scene、Model、现有软件光栅算法和各个编辑器面板。
借鉴 UE 的数据所有权和模块边界，尚未复制其 RHI、RenderGraph 或线程体系。

## 阅读顺序

1. `simulation/rigidbody/RigidBody.h`：初始描述、运行状态和世界参数。
2. `IRigidBodySolver.h`：唯一的解算执行接口。实现者拥有运行状态。
3. `rigidbody/cpu/CpuRigidBodySolver.cpp`：半隐式 Euler 和水平地面接触。
4. `SimulationSystem.cpp`：固定步长、播放、暂停、单步、重置。
5. `core/SceneSimulationBinding.cpp`：把模型与模拟 ID 关联，把位置同步回 Model。
6. `EngineRuntime.cpp` 和编辑器更新入口：先模拟，再刷新视口。
7. `render/opengl/`：独立 OpenGL 绘制实现；视口 presentation 负责显示。

## 依赖和生命周期

```
Editor → EngineRuntime → SimulationSystem → IRigidBodySolver ← CpuRigidBodySolver
                  └──→ SceneSimulationBinding → Model
                  └──→ RenderSystem → IRenderBackend ← Software / OpenGL
```

`BuffaloSimulation` 只依赖数学头文件和标准库，可在不构建 Qt、Scene、渲染器时测试。
`BuffaloRuntime` 负责组合。Qt/OpenGL 集成在单独的 `BuffaloOpenGL` 目标中，
未来可更换上下文提供方式而不把 Qt 引入模拟或场景。

模拟默认暂停，使用 1/120 秒固定步长。每次编辑器更新最多补 8 步，超出的墙钟时间丢弃；
这是交互演示的防卡顿策略，离线科学计算应自己按确定的步数调用解算器。
单步会暂停并推进一整步；重置恢复初始姿态、速度、时间，保持暂停。

当前示例关联场景第一个有效模型，使用其世界包围盒的半高进行水平地面接触。
地面在原始模型底部下方半个包围球半径处，是**未绘制的碰撞平面**；编辑器屏幕网格不是物理地面。
第一版只实现平移、重力、回弹和低速接触停止，没有转动、摩擦、刚体间碰撞或通用睡眠系统。
模型初始矩阵会完整保存；重置还原。绑定中的 Model 指针由 Scene 拥有，先解除绑定再销毁 Scene。
当前没有场景热切换；未来增加时必须先解除旧绑定并重新初始化模拟。

`Readback` 是显式的宿主数据读取边界，不是 Step 的强制输出。
CPU 演示在多个子步结束后读取一次结果。未来 CUDA/Taichi 可以保持设备数据驻留，
宿主读取在需要时同步；GPU 直接显示还需新增设备资源和完成信号交接，当前不声称已经实现。
算法和执行提供者不是同一维度；不建立同时包办刚体、布料、流体的大 Backend 基类。
布料位置预留在 `simulation/cloth/`，等第一个算法开始时再定义真实接口。

## 渲染约定与限制

保留现有 `IRenderBackend::Render(Scene&, width, height)`，两种后端使用同一 Scene 和 Camera。
软件后端交付借用的 BGRA CPU 像素，视口立即复制。OpenGL 后端交付共享上下文内的 GPU 纹理，
显示适配负责采样；不是每帧回读到 QImage。输出不是任意 API/设备间通用的裸纹理。
本次仍然单线程同步，纹理必须在下一次渲染/尺寸变化/销毁前消费；未来异步时须升级这个契约。

当前 CPU shader 与 GLSL 分开实现相同的基础漫反射；尚未引入 PBR、通用材质编译或统一 GPU RHI。
OpenGL shader 需处理现有矩阵的行存储以及投影深度 [0,1] 到 OpenGL [-1,1] 的转换。
网格目前静态，GPU 缓存不承担布料顶点更新；后续动态网格需要明确版本和上传策略。

场景默认模型路径由 CMake 指向本仓库 `model/toy.obj`，不再使用旧项目的本机绝对路径。
移动源码目录后请重新配置构建；资产打包和场景序列化不在本次范围。

## 建议的下一步

先对照两种渲染器学习剔除、裁切、光照，然后增加公共材质参数。
再做 CPU 布料，通过单独连接代码更新动态网格。第二个解算器/图形 API 接入时，
用实际需求检验接口，而不是提前添加没有实现的基类或空方法。
