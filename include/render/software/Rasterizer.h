#pragma once

#include <cstdint>
#include <vector>
#include "render/Buffer.h"
#include "math/Vector.h"
#include "math/Bounds.h"
#include "render/shader/Shader.h"
#include "render/PipelineTypes.h"
#include "render/material/MaterialShaderParameters.h"

class Rasterizer
{
public:
	Rasterizer() = default;
	~Rasterizer() = default;

	bool StartUp(Buffer<std::uint32_t>* InColorBuffer, Buffer<float>* InDepthBuffer);
	void ShutDown();

	void DrawPixel(int x, int y, float depth, std::uint32_t);
	// Draw Line use Bresenham
	void DrawLine(const Vector2& p0, const Vector2& p1, const std::uint32_t color);
	void DrawWireFrame(const Vector2& p0, const Vector2& p1, const Vector2& p2, const std::uint32_t color);
	void DrawTriangle(const RasterVertex& v0,
					  const RasterVertex& v1,
					  const RasterVertex& v2,
					  Shader* mShader,
					  MaterialShaderParameters params
					);

private:
	Buffer<std::uint32_t>* mColorBuffer = nullptr;
	Buffer<float>* mDepthBuffer = nullptr;

	std::uint32_t GetColor(bool Random);
	Bounds2 GetTriangleBounds2(const Vector2& p0, const Vector2& p1, const Vector2& p2);
	float edge(const Vector2& p, const Vector2& p0, const Vector2& p1);

	bool mIsStarted = false;
};