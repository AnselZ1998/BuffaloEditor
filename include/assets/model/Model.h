#pragma once

#include <string>

#include "math/Matrix.h"
#include "assets/model/Mesh.h"
#include "render/material/Material.h"
#include "render/shader/Shader.h"


class Model
{
public:
    Model(
        const std::string& meshPath,
        const Matrix4& meshMatrix = Matrix4::Identity()
    );

    ~Model() = default;


    const Mesh* GetMesh() const;

    const Matrix4& GetModelMatrix() const;

    void SetModelMatrix(const Matrix4& matrix);


    Material* GetMaterial();


    const bool MeshValid();


private:
    Matrix4 mModelMatrix;

    Mesh mMesh;

    Material mMaterial;

    bool IsValid;
};