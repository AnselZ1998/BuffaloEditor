// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "assets/model/Model.h"
#include "assets/import/objParser.h"
#include <algorithm>
#include <cmath>
Model::Model(const std::string& meshPath, const Matrix4& meshMatrix)
    : mModelMatrix(meshMatrix)
{
    if (!meshPath.empty())
    {
        LoadMesh(meshPath);
    }
}
bool Model::LoadMesh(const std::string& path)
{
    Mesh candidate;
    try
    {
        if (!path.empty() && !OBJ::buildMeshFromFile(candidate, path))
        {
            return false;
        }
    }
    catch (const std::exception&)
    {
        return false;
    }
    mMesh = std::move(candidate);
    meshPath_ = path;
    ++meshRevision_;
    return true;
}
const Mesh* Model::GetMesh() const
{
    return &mMesh;
}
const Matrix4& Model::GetModelMatrix() const
{
    return mModelMatrix;
}
void Model::SetModelMatrix(const Matrix4& matrix)
{
    mModelMatrix = matrix;
}
void Model::SetTransform(const Vector3& position, const Vector3& rotationDegrees, const Vector3& scale)
{
    const Vector3 r = rotationDegrees * 0.01745329252f;
    mModelMatrix = Matrix4::Translation(position.x, position.y, position.z) * Matrix4::RotationFromEuler(r.x, r.y, r.z) * Matrix4::Scale(scale.x, scale.y, scale.z);
}
Vector3 Model::Position() const
{
    return {mModelMatrix._14, mModelMatrix._24, mModelMatrix._34};
}
Vector3 Model::Scale() const
{
    return {Vector3(mModelMatrix._11, mModelMatrix._21, mModelMatrix._31).Length(),
        Vector3(mModelMatrix._12, mModelMatrix._22, mModelMatrix._32).Length(),
        Vector3(mModelMatrix._13, mModelMatrix._23, mModelMatrix._33).Length()};
}
Vector3 Model::RotationDegrees() const
{
    const auto s = Scale();
    if (s.x < 1e-6f || s.y < 1e-6f || s.z < 1e-6f)
    {
        return {};
    }
    const float y = std::asin(std::clamp(-mModelMatrix._31 / s.x, -1.0f, 1.0f));
    const bool singular = std::abs(std::cos(y)) < 1e-5f;
    const float x = singular ? std::atan2(-mModelMatrix._23 / s.z, mModelMatrix._22 / s.y) : std::atan2(mModelMatrix._32 / s.y, mModelMatrix._33 / s.z);
    const float z = singular ? 0.0f : std::atan2(mModelMatrix._21 / s.x, mModelMatrix._11 / s.x);
    return Vector3(x, y, z) * 57.29577951f;
}
Material* Model::GetMaterial()
{
    return &mMaterial;
}
bool Model::MeshValid() const
{
    return mMesh.IsValid();
}
