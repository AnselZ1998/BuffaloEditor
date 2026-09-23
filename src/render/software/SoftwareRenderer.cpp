#include "render/software/SoftwareRenderer.h"
#include "render/material/Material.h"
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
    if (mIsStarted)
    {
        mIsStarted = false;
    }
    mRasterizer.ShutDown();
    mCamera = nullptr;
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
    
    // View (world -> view)
    const Matrix4 view = mCamera->viewMatrix;
    const Matrix4 projection = mCamera->projectionMatrix;

    // Projection (View -> Clip)
    Matrix4 vp = projection * view;

    for (int face = 0;face < mesh->numFaces;face++)
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

        Vector4 n0Local(mesh->normals[normalFace.x].x, mesh->normals[normalFace.x].y, mesh->normals[normalFace.x].z, 0.0f);
        Vector4 n1Local(mesh->normals[normalFace.y].x, mesh->normals[normalFace.y].y, mesh->normals[normalFace.y].z, 0.0f);
        Vector4 n2Local(mesh->normals[normalFace.z].x, mesh->normals[normalFace.z].y, mesh->normals[normalFace.z].z, 0.0f);

        Vector3 n0 = (model * n0Local).XYZ().Normalized();
        Vector3 n1 = (model * n1Local).XYZ().Normalized();
        Vector3 n2 = (model * n2Local).XYZ().Normalized();

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

        // Clip
        bool in0 = IsInsideNear(clipP0);
        bool in1 = IsInsideNear(clipP1);
        bool in2 = IsInsideNear(clipP2);

        int insideCount = static_cast<int>(in0) + static_cast<int>(in1) + static_cast<int>(in2);

        if (insideCount == 0)
        {
            continue;
        }

        // Clip -> NDC -> Screen
        RasterVertex screenP0 = TransformToScreen(clipP0, vp);
        RasterVertex screenP1 = TransformToScreen(clipP1, vp);
        RasterVertex screenP2 = TransformToScreen(clipP2, vp);

        mRasterizer.DrawTriangle(screenP0, screenP1, screenP2, shader);

    }
}

ClipVertex SoftwareRenderer::WorldToClip(const WorldVertex& input, const Matrix4& vp)
{
    ClipVertex clipOut;

    Vector4 clipPos = vp * Vector4::FromPoint(input.position);

    clipOut.position = clipPos;
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

const bool SoftwareRenderer::IsInsideNear(const ClipVertex& v)
{
    return v.position.z >= 0;
}
