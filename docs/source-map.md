> 历史版本说明：本文保留此前迁移记录。当前 Buffalo v1 的目录、启动链和扩展方式以 [Buffalo v1 架构](buffalo-v1.md) 为准。

# Source migration map

All 38 original files retain identical bytes outside include directives, including original encoding and line endings. Existing uncommitted source changes were preserved. All three model assets retain their SHA-256 hashes.

| Original path | Current path |
| --- | --- |
| include/BoundingVolume.h | include/math/BoundingVolume.h |
| include/Bounds.h | include/math/Bounds.h |
| include/Buffer.h | include/render/Buffer.h |
| include/Camera.h | include/scene/Camera.h |
| include/DisplayManager.h | include/platform/windows/DisplayManager.h |
| include/Engine.h | include/core/Engine.h |
| include/Material.h | include/render/material/Material.h |
| include/Matrix.h | include/math/Matrix.h |
| include/Mesh.h | include/assets/model/Mesh.h |
| include/Model.h | include/assets/model/Model.h |
| include/objParser.h | include/assets/import/objParser.h |
| include/PipelineTypes.h | include/render/PipelineTypes.h |
| include/Quaternion.h | include/math/Quaternion.h |
| include/Rasterizer.h | include/render/software/Rasterizer.h |
| include/RenderManager.h | include/render/RenderManager.h |
| include/Scene.h | include/scene/Scene.h |
| include/SceneManager.h | include/scene/SceneManager.h |
| include/Shader.h | include/render/shader/Shader.h |
| include/SoftwareRenderer.h | include/render/software/SoftwareRenderer.h |
| include/Vector.h | include/math/Vector.h |
| src/BoundingVolume.cpp | src/math/BoundingVolume.cpp |
| src/Camera.cpp | src/scene/Camera.cpp |
| src/DisplayManager.cpp | src/platform/windows/DisplayManager.cpp |
| src/Engine.cpp | src/core/Engine.cpp |
| src/main.cpp | src/app/main.cpp |
| src/Material.cpp | src/render/material/Material.cpp |
| src/Matrix.cpp | src/math/Matrix.cpp |
| src/Mesh.cpp | src/assets/model/Mesh.cpp |
| src/Model.cpp | src/assets/model/Model.cpp |
| src/objParser.cpp | src/assets/import/objParser.cpp |
| src/Quaternion.cpp | src/math/Quaternion.cpp |
| src/Rasterizer.cpp | src/render/software/Rasterizer.cpp |
| src/Renderer.cpp | src/render/legacy/Renderer.cpp |
| src/RenderManager.cpp | src/render/RenderManager.cpp |
| src/Scene.cpp | src/scene/Scene.cpp |
| src/SceneManager.cpp | src/scene/SceneManager.cpp |
| src/Shader.cpp | src/render/shader/Shader.cpp |
| src/SoftwareRenderer.cpp | src/render/software/SoftwareRenderer.cpp |
