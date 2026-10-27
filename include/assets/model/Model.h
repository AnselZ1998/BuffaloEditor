// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once
#include "math/Matrix.h"
#include "assets/model/Mesh.h"
#include "render/material/Material.h"
#include "scene/ObjectId.h"
#include <string>
class Model
{
public:
    explicit Model(const std::string& meshPath, const Matrix4& meshMatrix = Matrix4::Identity());
    ObjectId Id() const
    {
        return id_;
    }
    std::string name = "Model";
    bool visible = true;
    const Mesh* GetMesh() const;
    const Matrix4& GetModelMatrix() const;
    void SetModelMatrix(const Matrix4& matrix);
    void SetTransform(const Vector3& position, const Vector3& rotationDegrees, const Vector3& scale);
    Vector3 Position() const;
    Vector3 RotationDegrees() const;
    Vector3 Scale() const;
    const std::string& MeshPath() const
    {
        return meshPath_;
    }
    std::uint64_t MeshRevision() const
    {
        return meshRevision_;
    }
    bool LoadMesh(const std::string& path);
    Material* GetMaterial();
    bool MeshValid() const;

private:
    const ObjectId id_ = NewObjectId();
    std::string meshPath_;
    std::uint64_t meshRevision_ = 0;
    Matrix4 mModelMatrix;
    Mesh mMesh;
    Material mMaterial;
};
