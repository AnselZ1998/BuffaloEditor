// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "render/software/SoftwareRenderer.h"
#include "render/material/Material.h"
#include "render/material/MaterialShaderParameters.h"

#include <cstdlib>

bool SoftwareRenderer::StartUp(int width, int height)
{
    if (mIsStarted)
    {
        return true;
    }

    mColorBuffer.Resize(width, height);
    mDepthBuffer.Resize(width, height);
    swWidth = width;
    swHeight = height;

    if (!mColorBuffer.IsValid() || !mDepthBuffer.IsValid())
    {
        return false;
    }

    if (!mRasterizer.StartUp(&mColorBuffer, &mDepthBuffer))
    {
        return false;
    }

    mIsStarted = true;
    return true;
}

void SoftwareRenderer::Resize(int width, int height)
{
    if (!mIsStarted || width <= 0 || height <= 0)
    {
        return;
    }

    if (width == swWidth && height == swHeight)
    {
        return;
    }

    mColorBuffer.Resize(width, height);
    mDepthBuffer.Resize(width, height);
    swWidth = width;
    swHeight = height;
}

void SoftwareRenderer::ShutDown()
{
    mIsStarted = false;
    mRasterizer.ShutDown();
    mCamera = nullptr;

    mColorBuffer = Buffer<std::uint32_t>{};
    mDepthBuffer = Buffer<float>{};

    swWidth = 0;
    swHeight = 0;
}

void SoftwareRenderer::BeginFrame(std::uint32_t clearColor)
{
    if (!mIsStarted)
    {
        return;
    }
    mColorBuffer.Clear(clearColor);
    mDepthBuffer.Clear(1.0f);
}

void SoftwareRenderer::SetCameraToRender(Camera* camera)
{
    mCamera = camera;
}

const Buffer<std::uint32_t>* SoftwareRenderer::GetColorBuffer() const
{
    assert(mIsStarted);
    return &mColorBuffer;
}

void SoftwareRenderer::drawTriangularMesh(Model* currentModel)
{
    if (!mIsStarted || currentModel == nullptr || mCamera == nullptr)
    {
        return;
    }

    const Mesh* mesh = currentModel->GetMesh();
    Material* material = currentModel->GetMaterial();
    MaterialShaderParameters params = material->GetShaderParameters();
    Shader* shader = material->GetShader();

    if (mesh == nullptr || !mesh->IsValid())
    {
        return;
    }

    if (mesh->numFaces > mesh->vertexIndices.size() || mesh->numFaces > mesh->uvIndices.size() || mesh->numFaces > mesh->normalsIndices.size())
    {
        return;
    }

    // Model (local -> world). The editor/runtime owns the transform;
    // the renderer only consumes the matrix.
    const Matrix4 model = currentModel->GetModelMatrix();
    const Vector3 column0(model._11, model._21, model._31);
    const Vector3 column1(model._12, model._22, model._32);
    const Vector3 column2(model._13, model._23, model._33);
    const float modelDeterminant = Vector3::Dot(column0, Vector3::Cross(column1, column2));
    if (std::abs(modelDeterminant) < 1e-12f)
    {
        return;
    }
    const auto transformNormal = [&](const Vector4& n)
    {
        return ((Vector3::Cross(column1, column2) * n.x + Vector3::Cross(column2, column0) * n.y +
                    Vector3::Cross(column0, column1) * n.z) *
                (1.0f / modelDeterminant))
            .Normalized();
    };

    // View (world -> view)
    const Matrix4 view = mCamera->viewMatrix;
    const Matrix4 projection = mCamera->projectionMatrix;

    // Projection (View -> Clip)
    Matrix4 vp = projection * view;

    for (int face = 0; face < mesh->numFaces; face++)
    {
        const Vector3i& vertexFace = mesh->vertexIndices[face];
        const Vector3i& uvFace = mesh->uvIndices[face];
        const Vector3i& normalFace = mesh->normalsIndices[face];

        if (!IsValidIndex(vertexFace.x, mesh->vertices.size()) || !IsValidIndex(vertexFace.y, mesh->vertices.size()) || !IsValidIndex(vertexFace.z, mesh->vertices.size()))
        {
            continue;
        }
        if (!IsValidIndex(uvFace.x, mesh->uv.size()) || !IsValidIndex(uvFace.y, mesh->uv.size()) || !IsValidIndex(uvFace.z, mesh->uv.size()))
        {
            continue;
        }
        if (!IsValidIndex(normalFace.x, mesh->normals.size()) || !IsValidIndex(normalFace.y, mesh->normals.size()) || !IsValidIndex(normalFace.z, mesh->normals.size()))
        {
            continue;
        }

        const Vector3& p0 = mesh->vertices[vertexFace.x];
        const Vector3& p1 = mesh->vertices[vertexFace.y];
        const Vector3& p2 = mesh->vertices[vertexFace.z];

        // BackFace Calling
        Vector3 worldP0 = (model * Vector4::FromPoint(p0)).XYZ();
        Vector3 worldP1 = (model * Vector4::FromPoint(p1)).XYZ();
        Vector3 worldP2 = (model * Vector4::FromPoint(p2)).XYZ();

        Vector3 viewDir = worldP0 - mCamera->position;
        Vector3 faceNormal = Vector3::Cross(worldP1 - worldP0, worldP2 - worldP0);
        if (Vector3::Dot(faceNormal, viewDir) >= 0)
        {
            continue;
        }

        const Vector3& uv0 = mesh->uv[uvFace.x];
        const Vector3& uv1 = mesh->uv[uvFace.y];
        const Vector3& uv2 = mesh->uv[uvFace.z];

        const Vector3 edge1 = worldP1 - worldP0;
        const Vector3 edge2 = worldP2 - worldP0;
        const Vector3 duv1 = uv1 - uv0;
        const Vector3 duv2 = uv2 - uv0;
        const float determinant = duv1.x * duv2.y - duv1.y * duv2.x;
        params.tangent = {};
        params.bitangent = {};
        if (std::abs(determinant) > 1e-6f)
        {
            params.tangent = (edge1 * duv2.y - edge2 * duv1.y) * (1.0f / determinant);
            params.bitangent = (edge2 * duv1.x - edge1 * duv2.x) * (1.0f / determinant);
        }

        Vector4 n0Local(mesh->normals[normalFace.x].x, mesh->normals[normalFace.x].y, mesh->normals[normalFace.x].z, 0.0f);
        Vector4 n1Local(mesh->normals[normalFace.y].x, mesh->normals[normalFace.y].y, mesh->normals[normalFace.y].z, 0.0f);
        Vector4 n2Local(mesh->normals[normalFace.z].x, mesh->normals[normalFace.z].y, mesh->normals[normalFace.z].z, 0.0f);

        Vector3 n0 = transformNormal(n0Local);
        Vector3 n1 = transformNormal(n1Local);
        Vector3 n2 = transformNormal(n2Local);

        // position and normal can changed:
        // local space -> world space
        WorldVertex v0;
        v0.position = worldP0;
        v0.normal = n0;
        v0.uv = uv0;

        WorldVertex v1;
        v1.position = worldP1;
        v1.normal = n1;
        v1.uv = uv1;

        WorldVertex v2;
        v2.position = worldP2;
        v2.normal = n2;
        v2.uv = uv2;

        // World -> Clip
        ClipVertex clipP0 = WorldToClip(v0, vp);
        ClipVertex clipP1 = WorldToClip(v1, vp);
        ClipVertex clipP2 = WorldToClip(v2, vp);

        // 6 - plane clipping
        const ClipPlane planes[] = {ClipPlane::Left, ClipPlane::Right, ClipPlane::Bottom, ClipPlane::Top, ClipPlane::Near, ClipPlane::Far};
        std::vector<ClipVertex> polygon = {clipP0, clipP1, clipP2};

        for (ClipPlane plane : planes)
        {
            polygon = ClipAgainstPlane(polygon, plane);
            if (polygon.empty())
            {
                break;
            }
        }
        for (size_t i = 1; i + 1 < polygon.size(); ++i)
        {
            const ClipVertex& v0 = polygon[0];
            const ClipVertex& v1 = polygon[i];
            const ClipVertex& v2 = polygon[i + 1];

            // Clip -> NDC -> Screen
            RasterVertex screenP0 = TransformToScreen(v0, vp);
            RasterVertex screenP1 = TransformToScreen(v1, vp);
            RasterVertex screenP2 = TransformToScreen(v2, vp);

            mRasterizer.DrawTriangle(
                screenP0,
                screenP1,
                screenP2,
                shader,
                params);
        }
    }
}

ClipVertex SoftwareRenderer::WorldToClip(const WorldVertex& input, const Matrix4& vp)
{
    ClipVertex clipOut;

    Vector4 clipPos = vp * Vector4::FromPoint(input.position);

    clipOut.position = clipPos;

    // Normal also in World Space
    clipOut.normal = input.normal;
    clipOut.uv = input.uv;

    return clipOut;
}

RasterVertex SoftwareRenderer::TransformToScreen(const ClipVertex& input, const Matrix4& vp)
{
    RasterVertex rasterVOut;

    rasterVOut.invW = 1.0f / input.position.w;
    // Clip Space -> NDC Space
    Vector3 ndc = input.position.PerspectiveDivide();

    // NDC Space  -> Screen/Decice Space
    float screenX = (ndc.x + 1.0f) * 0.5f * swWidth;
    float screenY = (1.0f - ndc.y) * 0.5f * swHeight;

    rasterVOut.screenPosition = Vector2(screenX, screenY);
    rasterVOut.depth = ndc.z;
    rasterVOut.normal = input.normal;
    rasterVOut.uv = input.uv;

    return rasterVOut;
}

bool SoftwareRenderer::IsValidIndex(int index, size_t size)
{
    return index >= 0 && index < static_cast<int>(size);
}

float SoftwareRenderer::PlaneDistance(const ClipVertex& v, const ClipPlane plane)
{
    float w = v.position.w;
    float x = v.position.x;
    float y = v.position.y;
    float z = v.position.z;

    switch (plane)
    {
    case ClipPlane::Left:
        return x + w;

    case ClipPlane::Right:
        return w - x;

    case ClipPlane::Bottom:
        return y + w;

    case ClipPlane::Top:
        return w - y;

    case ClipPlane::Near:
        return z;

    case ClipPlane::Far:
        return w - z;
    }

    return -1.0f;
}

ClipVertex SoftwareRenderer::InterpolateClipVertex(const ClipVertex& a, const ClipVertex& b, float t)
{
    ClipVertex result;

    result.position = a.position + (b.position - a.position) * t;
    result.normal = a.normal + (b.normal - a.normal) * t;
    result.uv = a.uv + (b.uv - a.uv) * t;

    return result;
}

std::vector<ClipVertex> SoftwareRenderer::ClipAgainstPlane(const std::vector<ClipVertex>& input, ClipPlane plane)
{
    std::vector<ClipVertex> output;

    ClipVertex previous = input.back();
    float previousDistance = PlaneDistance(previous, plane);
    bool previousInside = previousDistance >= 0.0f;

    for (const ClipVertex& current : input)
    {
        float currentDistance = PlaneDistance(current, plane);
        bool currentInside = currentDistance >= 0.0f;

        // inside -> inside
        if (previousInside && currentInside)
        {
            output.push_back(current);
        }

        // inside -> outside
        else if (previousInside && !currentInside)
        {
            float t = previousDistance / (previousDistance - currentDistance);
            ClipVertex intersection = InterpolateClipVertex(previous, current, t);
            output.push_back(intersection);
        }

        // outside -> inside
        else if (!previousInside && currentInside)
        {
            float t = previousDistance / (previousDistance - currentDistance);

            ClipVertex intersection = InterpolateClipVertex(previous, current, t);

            output.push_back(intersection);
            output.push_back(current);
        }

        // outside -> outside
        else
        {
            // output nothing
        }

        previous = current;
        previousDistance = currentDistance;
        previousInside = currentInside;
    }

    return output;
}
