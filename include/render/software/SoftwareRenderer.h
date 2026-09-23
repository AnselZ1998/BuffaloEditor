#pragma once

#include <cstdint>
#include <queue>
#include "render/Buffer.h"
#include "math/Matrix.h"
#include "scene/Camera.h"
#include "render/software/Rasterizer.h"
#include "assets/model/Model.h"
#include "render/PipelineTypes.h"

class SoftwareRenderer
{
public:
	
	SoftwareRenderer() = default;
	~SoftwareRenderer() = default;

	bool StartUp(int width, int height);
	void Resize(int width, int height);
	void ShutDown();
	void BeginFrame(std::uint32_t clearColor = 0x000000);
	void SetCameraToRender(Camera* camera);
	const Buffer<std::uint32_t>* GetColorBuffer() const;
	void drawTriangularMesh(Model* currentModel);
	ClipVertex WorldToClip(const WorldVertex& input, const Matrix4& vp);
	RasterVertex TransformToScreen(const ClipVertex& input, const Matrix4& mvp);

	static bool IsValidIndex(int index, size_t size);
	const bool IsInsideNear(const ClipVertex& v);

private:
	Buffer<std::uint32_t> mColorBuffer;
	Buffer<float> mDepthBuffer;
	Rasterizer mRasterizer;
	Camera* mCamera = nullptr;

	int swWidth, swHeight = 0;
	bool mIsStarted = false;

};
